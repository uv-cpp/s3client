/*******************************************************************************
 * BSD 3-Clause License
 *
 * Copyright (c) 2020-2022, Ugo Varetto
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 *ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 *LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 *CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 *SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 *INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 *CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 *ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *POSSIBILITY OF SUCH DAMAGE.
 ******************************************************************************/
/**
 * \file webclient.cpp
 * \brief implementation of WebClient class, wrapper around libcurl
 */

#include "webclient.h"

#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#include <algorithm>
#include <cassert>
#include <cstring>
#include <fstream>
#include <iostream>
#include <stdexcept>
// #ifdef IGNORE_SIGPIPE REQUIRED!
#include <signal.h>
// #endif

#include "common.h"

namespace sss {

using namespace std;

// Track number of instances created.
std::atomic<int> WebClient::numInstances_{0};
// Used to serialize access to init and cleanup libcurl functions, guaranteeing
// tha only one init and one cleanup happens.
std::mutex WebClient::cleanupMutex_;

// All the following functions are invoked from libcurl and the FILE* pointer
// is moved to the proper offset before the functions are passed to libcurl
/** \addtogroup internal
 * @{
 */
/// Read from file
size_t ReadFile(void *ptr, size_t size, size_t nmemb, void *userdata) {
  FILE *f = static_cast<FILE *>(userdata);
  if (ferror(f))
    return CURL_READFUNC_ABORT;
  return fread(ptr, size, nmemb, f) * size;
}
/// Read from file using unbuffered I/O
size_t ReadFileUnbuffered(void *ptr, size_t size, size_t nmemb,
                          void *userdata) {
  const int fd = *static_cast<int *>(userdata);
  return max(ssize_t(0), read(fd, ptr, size * nmemb));
}
/// Write to file
size_t WriteFile(char *ptr, size_t size, size_t nmemb, void *userdata) {
  FILE *writehere = static_cast<FILE *>(userdata);
  size = size * nmemb;
  fwrite(ptr, size, nmemb, writehere);
  return size;
}
/// Write to file using unbuffered I/O
size_t WriteFileUnbuffered(char *ptr, size_t size, size_t nmemb,
                           void *userdata) {
  const int fd = *static_cast<int *>(userdata);
  return max(ssize_t(0), write(fd, ptr, size * nmemb));
}
/**
 * @}
 */

// public:
/// Cleanup and invoke cleanup on libcurl in case of last instance
WebClient::~WebClient() {
  if (curlHeaderList_) {
    curl_slist_free_all(curlHeaderList_);
  }
  if (curl_) { // current object could have been moved
    // libcurl init and cleanup functions ar NOT thread safe and the
    // behaviour is unpredictable in case of use in a multithreaded
    // environment, this just guarantees that there is only a single
    // call to curl_global_cleanup
    curl_easy_cleanup(curl_);
    const std::lock_guard<std::mutex> lock(cleanupMutex_);
    --numInstances_;
    if (numInstances_ == 0)
      curl_global_cleanup();
  }
}
// Send request
bool WebClient::Send() {
  const bool ret = Status(curl_easy_perform(curl_));
  curl_easy_getinfo(curl_, CURLINFO_RESPONSE_CODE, &responseCode_);
  return ret;
}
// Set SSL verification options: peer and/or host
// It is useful to disable everything when sending https requests through
// e.g. httos tunnel
bool WebClient::SSLVerify(bool verifyPeer, bool verifyHost) {
  return Status(curl_easy_setopt(curl_, CURLOPT_SSL_VERIFYPEER,
                                 verifyPeer ? 1L : 0L)) &&
         ///@warning insecure!!!
         Status(curl_easy_setopt(curl_, CURLOPT_SSL_VERIFYHOST,
                                 verifyHost ? 1L : 0L));
}
// Set full URL
bool WebClient::SetUrl(const std::string &url) {
  url_ = url;
  return curl_easy_setopt(curl_, CURLOPT_URL, url_.c_str()) == CURLE_OK;
}
// Set endpoint: <proto>://<server>:<port>
void WebClient::SetEndpoint(const std::string &ep) {
  endpoint_ = ep;
  BuildURL();
}
// Set URL path /.../...
void WebClient::SetPath(const std::string &path) {
  path_ = path;
  BuildURL();
}
// Store headers internally
void WebClient::SetHeaders(const Map &headers) {
  headers_ = headers;
  if (curlHeaderList_) {
    curl_slist_free_all(curlHeaderList_);
    curlHeaderList_ = NULL;
  }
  if (!headers_.empty()) {
    for (auto kv : headers_) {
      curlHeaderList_ = curl_slist_append(
          curlHeaderList_, (kv.first + ": " + kv.second).c_str());
    }
  }
  curl_easy_setopt(curl_, CURLOPT_HTTPHEADER, curlHeaderList_);
}
// Regenerate URL with new parameters
void WebClient::SetReqParameters(const Map &params) {
  params_ = params;
  BuildURL();
}
// Set HTTP method
void WebClient::SetMethod(const std::string &method, size_t size) {
  method_ = ToUpper(method);
  if (method_ == "GET") {
    curl_easy_setopt(curl_, CURLOPT_NOBODY, 0L);
    curl_easy_setopt(curl_, CURLOPT_HTTPGET, 1L);
  } else if (method == "HEAD") {
    curl_easy_setopt(curl_, CURLOPT_NOBODY, 1L);
    curl_easy_setopt(curl_, CURLOPT_CUSTOMREQUEST, "HEAD");
  } else if (method == "DELETE") {
    curl_easy_setopt(curl_, CURLOPT_CUSTOMREQUEST, "DELETE");
  } else if (method == "POST") {
    curl_easy_setopt(curl_, CURLOPT_CUSTOMREQUEST, "POST");
    curl_easy_setopt(curl_, CURLOPT_POSTFIELDSIZE, urlEncodedPostData_.size());
    curl_easy_setopt(curl_, CURLOPT_POSTFIELDS, urlEncodedPostData_.c_str());
  } else if (method_ == "PUT") {
    curl_easy_setopt(curl_, CURLOPT_UPLOAD, 1L);
    curl_easy_setopt(curl_, CURLOPT_INFILESIZE_LARGE, size);
    curl_easy_setopt(curl_, CURLOPT_CUSTOMREQUEST, "PUT");
  }
}
// Url-encode and store data be posted from {key,value} map
void WebClient::SetUrlEncodedPostData(const Map &postData) {
  urlEncodedPostData_ = UrlEncode(postData);
  curl_easy_setopt(curl_, CURLOPT_POSTFIELDSIZE, urlEncodedPostData_.size());
  curl_easy_setopt(curl_, CURLOPT_COPYPOSTFIELDS, urlEncodedPostData_.c_str());
}
// Url-encode and store data to be posted from text
void WebClient::SetUrlEncodedPostData(const std::string &postData) {
  urlEncodedPostData_ = UrlEncode(postData);
  curl_easy_setopt(curl_, CURLOPT_POSTFIELDSIZE, urlEncodedPostData_.size());
  curl_easy_setopt(curl_, CURLOPT_COPYPOSTFIELDS, urlEncodedPostData_.c_str());
}
// Set url-encoded data to be posted
void WebClient::SetPostData(const std::string &data) {
  curl_easy_setopt(curl_, CURLOPT_POSTFIELDSIZE, data.size());
  curl_easy_setopt(curl_, CURLOPT_COPYPOSTFIELDS, data.c_str());
}
// Return status code of last executed request
long WebClient::StatusCode() const { return responseCode_; }
// Return full URL
const std::string &WebClient::GetUrl() const { return url_; }
// Return response content
const std::vector<uint8_t> &WebClient::GetResponseBody() const {
  return writeBuffer_.data;
}
// Return raw header buffer
const std::vector<uint8_t> &WebClient::GetResponseHeader() const {
  return headerBuffer_;
}
// Set write function to use to write received data
bool WebClient::SetWriteFunction(WriteFunction f, void *ptr) {
  if (curl_easy_setopt(curl_, CURLOPT_WRITEFUNCTION, f) != CURLE_OK)
    return false;
  if (curl_easy_setopt(curl_, CURLOPT_WRITEDATA, ptr) != CURLE_OK)
    return false;
  return true;
}
// Set read function to use to read data to be sent
bool WebClient::SetReadFunction(ReadFunction f, void *ptr) {
  if (curl_easy_setopt(curl_, CURLOPT_READFUNCTION, f) != CURLE_OK)
    return false;
  if (curl_easy_setopt(curl_, CURLOPT_READDATA, ptr) != CURLE_OK)
    return false;
  return true;
}
// Fill buffer with data to be uploaded
void WebClient::SetUploadData(const std::vector<uint8_t> &data) {
  readBuffer_.data = data;
  readBuffer_.offset = 0;
}
// Upload entire file
bool WebClient::UploadFile(const std::string &fname, size_t fsize) {
  const size_t size = fsize ? fsize : FileSize(fname);
  FILE *file = fopen(fname.c_str(), "rb");
  if (!file) {
    throw std::runtime_error("Cannot open file " + fname);
  }
  // when read function is NULL the passed void* is interpreted as
  // a FILE* by libcurl
  if (!SetReadFunction(NULL, file)) {
    throw std::runtime_error("Cannot set read function");
  }
  SetMethod("PUT", size);
  const bool result = Send();
  if (!result) {
    throw std::runtime_error("Error sending request: " + ErrorMsg());
    fclose(file);
  }
  fclose(file);
  return result;
}
// Upload content from memory buffer, equivalent to uploading file from memory
bool WebClient::UploadDataFromBuffer(const char *data, size_t offset,
                                     size_t size) {

  if (curl_easy_setopt(curl_, CURLOPT_READFUNCTION, MemReader) != CURLE_OK) {
    throw std::runtime_error("Cannot set curl read function");
  }
  if (curl_easy_setopt(curl_, CURLOPT_READDATA, &refBuffer_) != CURLE_OK) {
    throw std::runtime_error("Cannot set curl read data buffer");
  }
  // std::cout << refBuffer_.data << " " << refBuffer_.offset << std::endl;
  refBuffer_.data = data;
  refBuffer_.offset = offset;
  refBuffer_.size = size;
  SetMethod("PUT", size);
  return Send();
}
// Upload file starting at offset
bool WebClient::UploadFile(const std::string &fname, size_t offset,
                           size_t size) {
  FILE *file = fopen(fname.c_str(), "rb");
  if (!file) {
    throw std::runtime_error("Cannot open file " + fname);
  }
  if (fseek(file, offset, SEEK_SET)) {
    throw std::runtime_error("Cannot move file pointer");
  }
  if (!SetReadFunction(ReadFile, file)) {
    throw std::runtime_error("Cannot set read function");
  }
  SetMethod("PUT", size);
  const bool result = Send();
  if (!result) {
    throw std::runtime_error("Error sending request: " + ErrorMsg());
    fclose(file);
  }
  fclose(file);
  return result;
}
// Upload file starting at offset, using unbuffered I/O
bool WebClient::UploadFileUnbuffered(const std::string &fname, size_t offset,
                                     size_t size) {
#ifndef __APPLE__
  int file = open(fname.c_str(), S_IRGRP | O_LARGEFILE);
#else
  int file = open(fname.c_str(), S_IRGRP);
#endif
  if (file < 0) {
    throw std::runtime_error(strerror(errno));
  }
  if (lseek(file, offset, SEEK_SET) < 0) {
    throw std::runtime_error(strerror(errno));
  }
  if (!SetReadFunction(ReadFileUnbuffered, &file)) {
    throw std::runtime_error("Cannot set read function");
  }
  SetMethod("PUT", size);
  const bool result = Send();
  if (!result) {
    throw std::runtime_error("Error sending request: " + ErrorMsg());
    close(file);
  }
  close(file);
  return result;
}
// Upload memory mapped file
bool WebClient::UploadFileMM(const std::string &fname, size_t offset,
                             size_t size) {
#ifndef __APPLE__
  int fd = open(fname.c_str(), O_RDONLY | O_LARGEFILE);
#else
  int fd = open(fname.c_str(), O_RDONLY);
#endif

  if (fd < 0) {
    throw std::runtime_error("Error cannot open input file: " +
                             std::string(strerror(errno)));
    exit(EXIT_FAILURE);
  }
  char *src = (char *)mmap(NULL, size, PROT_READ, MAP_PRIVATE, fd, offset);
  if (src == MAP_FAILED) { // mmap returns (void *) -1 == MAP_FAILED
    throw std::runtime_error("Error mapping memory: " +
                             std::string(strerror(errno)));
    exit(EXIT_FAILURE);
  }
  if (!UploadDataFromBuffer(src, 0, size)) {
    throw std::runtime_error("Error uploading memory mapped data");
    exit(EXIT_FAILURE);
  }
  if (munmap(src, size)) {
    throw std::runtime_error("Error unmapping memory: " +
                             std::string(strerror(errno)));
    exit(EXIT_FAILURE);
  }
  if (close(fd)) {
    throw std::runtime_error("Error closing file" +
                             std::string(strerror(errno)));
    exit(EXIT_FAILURE);
  }
  return true;
}
// Return libcurl error.
std::string WebClient::ErrorMsg() const { return errorBuffer_.data(); }
// Passthrough to curl_easy_setopt
// https://curl.se/libcurl/c/curl_easy_setopt.html
CURLcode WebClient::SetOpt(CURLoption option, va_list argp) {
  return curl_easy_setopt(curl_, option, argp);
}
// Passthrough method to curl_easy_getinfo
// https://curl.se/libcurl/c/curl_easy_getinfo.html
CURLcode WebClient::GetInfo(CURLINFO info, va_list argp) {
  return curl_easy_getinfo(curl_, info, argp);
}
// Enable verbose mode
void WebClient::SetVerbose(bool verbose) {
  curl_easy_setopt(curl_, CURLOPT_VERBOSE, verbose ? 1L : 0);
}
// Returns content as text
std::string WebClient::GetContentText() const {
  std::vector<uint8_t> content = GetResponseBody();
  return std::string(begin(content), end(content));
}
// Returns headers as text
std::string WebClient::GetHeaderText() const {
  std::vector<uint8_t> header = GetResponseHeader();
  return std::string(begin(header), end(header));
}

// private:

// @warning !!!HACK Check status and discards SIGPIPE errors
bool WebClient::Status(CURLcode cc) const {
  if (cc == 0)
    return true;
  // deal with "SSL_write() returned SYSCALL, errno = 32"
  if (cc == CURLE_SEND_ERROR) {
    const std::string err(begin(errorBuffer_), end(errorBuffer_));
    if (err.find("32") != std::string::npos)
      return true;
    if (ToUpper(err).find("PIPE") != std::string::npos)
      return true;
  }
  return false;
}
// Initializes libcurl, makes sure curl_global_init() is called only by the
// first instance
void WebClient::InitEnv() {
  // disable SIGPIPE signal per-thread
  const struct sigaction sa {
    SIG_IGN
  };
  sigaction(SIGPIPE, &sa, NULL);
  // first thread initializes curl, the others wait until
  // initialization is complete; the same mutex is used to initialize
  // and cleanup, see ~WebClient().
  // NOTE: libcurl initialization and cleanup are not thread safe,
  // this code just guarantees that curl_global_init is called only
  // once
  const std::lock_guard<std::mutex> lock(cleanupMutex_);
  if (numInstances_ == 0) {
    if (curl_global_init(CURL_GLOBAL_DEFAULT) != CURLE_OK) {
      throw std::runtime_error("Cannot initialize libcurl");
    }
  }
  ++numInstances_;
  Init();
}
// Initialize curl internal state
bool WebClient::Init() {
  curl_ = curl_easy_init();
  // curl_easy_setopt(curl_, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_2_0);
  if (!curl_) {
    throw(std::runtime_error("Cannot create Curl connection"));
    return false;
  }
  if (curl_easy_setopt(curl_, CURLOPT_ERRORBUFFER, errorBuffer_.data()) !=
      CURLE_OK)
    goto handle_error;
  if (curl_easy_setopt(curl_, CURLOPT_FOLLOWLOCATION, 1L) != CURLE_OK)
    goto handle_error;
  if (curl_easy_setopt(curl_, CURLOPT_WRITEFUNCTION, Writer) != CURLE_OK)
    goto handle_error;
  if (curl_easy_setopt(curl_, CURLOPT_WRITEDATA, &writeBuffer_) != CURLE_OK)
    goto handle_error;
  if (curl_easy_setopt(curl_, CURLOPT_READFUNCTION, Reader) != CURLE_OK)
    goto handle_error;
  if (curl_easy_setopt(curl_, CURLOPT_READDATA, &readBuffer_) != CURLE_OK)
    goto handle_error;
  if (curl_easy_setopt(curl_, CURLOPT_HEADERFUNCTION, HeaderWriter) != CURLE_OK)
    goto handle_error;
  if (curl_easy_setopt(curl_, CURLOPT_HEADERDATA, &headerBuffer_) != CURLE_OK)
    goto handle_error;
  // disable signal handlers
  if (curl_easy_setopt(curl_, CURLOPT_NOSIGNAL, 1L) != CURLE_OK) {
    goto handle_error;
  }
  if (curl_easy_setopt(curl_, CURLOPT_NOPROGRESS, 1L) != CURLE_OK) {
    goto handle_error;
  };
  if (curl_easy_setopt(curl_, CURLOPT_ACCEPT_ENCODING, "") != CURLE_OK) {
    goto handle_error;
  }
  if (method_.size()) {
    SetMethod(method_);
  }
  if (!headers_.empty()) {
    SetHeaders(headers_);
  }
  if (!params_.empty()) {
    SetReqParameters(params_);
  }
  if (!endpoint_.empty()) {
    BuildURL();
  }
  signal(SIGPIPE, SIG_IGN);
  curl_easy_setopt(curl_, CURLOPT_NOSIGNAL, 1L);
  return true;
handle_error:
  throw(std::runtime_error(errorBuffer_.data()));
  return false;
}
// Build URL from <proto>://<server>:<port> AND /<path>
bool WebClient::BuildURL() {
  url_ = endpoint_ + path_;
  if (!params_.empty()) {
    url_ += "?" + UrlEncode(params_);
  }
  return SetUrl(url_);
}
// Writer function: appends received content to buffer.
size_t WebClient::Writer(char *data, size_t size, size_t nmemb,
                         Buffer *outbuffer) {
  assert(outbuffer);
  size = size * nmemb;
  outbuffer->data.insert(begin(outbuffer->data) + outbuffer->offset,
                         (uint8_t *)data, (uint8_t *)data + size);
  outbuffer->offset += size;
  return size;
}
// Writer function for headers: appends response headers to buffer.
size_t WebClient::HeaderWriter(char *data, size_t size, size_t nmemb,
                               std::vector<uint8_t> *writerData) {
  assert(writerData);
  writerData->insert(writerData->end(), (uint8_t *)data,
                     (uint8_t *)data + size * nmemb);
  return size * nmemb;
}
// Reader function, writes data to be sent into outPtr buffer in chunks.
// Data is read from vector<> inside buffer object and read offset
// updated after each read operation
size_t WebClient::Reader(void *outPtr, size_t size, size_t nmemb,
                         Buffer *inBuffer) {
  // start element
  const auto b = begin(inBuffer->data) + inBuffer->offset;
  if (b >= end(inBuffer->data)) {
    return 0; // 0 marks the end of buffer
  }
  // chunk size in bytes = block size (size) x number of blocks(nmemb)
  size = size * nmemb;
  // end point min between end of buffer and start + chunk size
  const auto e = std::min(b + size, end(inBuffer->data));
  // copy from vector into curl internal buffer
  std::copy(b, e, (uint8_t *)outPtr);
  // update offset
  size = size_t(e - b);
  inBuffer->offset += size;
  return size;
}
// Same as Reader but reading from memory
size_t WebClient::MemReader(void *ptr, size_t size, size_t nmemb,
                            MemReadBuffer *inBuffer) {
  // start element
  const auto b = inBuffer->data + inBuffer->offset;
  // one past last element
  const auto end = inBuffer->data + inBuffer->offset + inBuffer->size;
  if (b >= end) {
    return 0; // 0 marks the end of buffer
  }
  // chunk size in bytes = block size (size) x number of blocks(nmemb)
  size = size * nmemb;
  // end point min between end of buffer and start + chunk size
  const auto e = std::min(b + size, end);
  // copy from memory into curl internal buffer
  std::copy(b, e, (char *)ptr);
  // update offset
  size = size_t(e - b);
  inBuffer->offset += size;
  return size;
}

// Redirect stderr to file. Returns \c false when it fails.
bool WebClient::RedirectSTDErr(FILE *f) {
  return curl_easy_setopt(curl_, CURLOPT_STDERR, f) == CURLE_OK;
}

} // namespace sss
