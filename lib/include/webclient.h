/*******************************************************************************
 * BSD 3-Clause License
 *
 * Copyright (c) 2020-2023, Ugo Varetto
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
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
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 ******************************************************************************/
/**
 * \file webclient.h
 * \brief declaration of WebClient class wrapping libcurl to send web requests.
 *
 * Data upload Workflow:
 * - Create WebClient instance
 * - Set headers and/or post data and/or request parameters and invoke Send() OR
 * - Upload file OR
 * - Upload data from memory buffer
 */

#pragma once

#include <curl/curl.h>
#include <curl/easy.h>

#include <array>
#include <atomic>
#include <map>
#include <mutex>
#include <string>
#include <vector>

#include "common.h"
#include "url_utility.h"
#include "utility.h"

namespace sss {

/**
 * \brief Send web requests through libcurl.
 * \ingroup WebClient
 *
 * Configure state then invoke WebClient::Send to send request.
 *
 * Retrieve data through:
 *   - GetResponseBody()
 *   - GetResponseHeader()
 *   - GetContentText()
 *   - GetHeaderText()
 *
 * Error handling is managed by having libcurl log errors into a char buffer.
 * When a method fails returning \c false, you can extract the error message
 * by invoking the WebClient::ErrorMsg() method.
 *
 * \section usage Usage
 *
 * \code
 * WebClient req;
 * req.SSLVerify(verifyPeer, verifyHost);
 * req.SetEndpoint(args.endpoint);
 * req.SetPath(path);
 * req.SetMethod(args.method);
 * req.SetReqParameters(args.params);
 * req.SetHeaders(headers);
 * FILE *of = NULL;
 * if (!args.outfile.empty()) {
 *   of = fopen(args.outfile.c_str(), "wb");
 *   req.SetWriteFunction(NULL, of); // default is to write to file
 * }
 * if (!args.data.empty()) {
 *   if (!args.dataIsFileName) {
 *     if (ToLower(args.method) == "post") {
 *       req.SetUrlEncodedPostData(ParseParams(args.data.data()));
 *       req.SetMethod("POST");
 *       req.Send();
 *     } else { // "put"
 *       req.UploadDataFromBuffer(args.data.data(), 0, args.data.size());
 *     }
 *   } else {
 *     if (ToLower(args.method) == "put") {
 *       req.UploadFile(args.data.data());
 *     } else if (args.method == "post") {
 *       ifstream t(args.data.data());
 *       const string str((istreambuf_iterator<char>(t)),
 *                        istreambuf_iterator<char>());
 *       req.SetMethod("POST");
 *       req.SetPostData(str);
 *       req.Send();
 *     } else {
 *       throw domain_error("Wrong method " + args.method);
 *     }
 *   }
 * } else
 *   req.Send();
 * if (of)
 *   fclose(of);
 * return req;
 *
 * \endcode
 *
 */
class WebClient {
public:
  /// Function type invoked by \e libcurl to write received data
  /// \param[in]  nmemb number of blocks of size \c size
  /// \param[in] writerData user data passed to write function
  using WriteFunction = size_t (*)(char *data, size_t size, size_t nmemb,
                                   void *writerData);
  /// Function type invoked by \e libcurl to read data to send
  /// \param[in] nmemb number of blocks of size \c size
  /// \param[in] readerData user data passed to reade function
  using ReadFunction = size_t (*)(void *ptr, size_t size, size_t nmemb,
                                  void *readerData);

private:
  /// Buffer keeping track of end of last read operation.
  struct Buffer {
    size_t offset = 0;      ///< pointer to next insertion point
    std::vector<char> data; ///< buffer
  };
  /// Same as Buffer but used to point to memory region instead of reading
  /// from vector<>.
  struct MemReadBuffer {
    size_t offset = 0; ///< pointer to next insertion point
    const char *data;  ///< buffer
    size_t size = 0;   ///< buffer size
  };

public:
  /// Disable copy constructor: only one libcurl handle per thread allwed
  WebClient(const WebClient &) = delete;
  /// Move constructor
  WebClient(WebClient &&other)
      : endpoint_(other.endpoint_), path_(other.path_), method_(other.method_),
        params_(other.params_), headers_(other.headers_),
        writeBuffer_(other.writeBuffer_), headerBuffer_(other.headerBuffer_),
        curl_(other.curl_) {
    other.curl_ = NULL;
  }
  /// Default constructor. First instance initializes libcurl.
  WebClient() { InitEnv(); }
  /// Constructor initializing only URL
  WebClient(const std::string &url) : url_(url), method_("GET") { InitEnv(); }
  /// Constructor
  /// \param[in] endPoint endpoint in the format \c <proto>://<server>:port
  /// \param[in] path path to be added to endPoint to compete URL: /.../...
  /// \param[in] method HTTP method
  /// \param[in] params key,value map of parameters \c k1=value1&k2=value2&...
  WebClient(const std::string &endPoint, const std::string &path,
            const std::string &method = "GET", const Map &params = Map(),
            const Map headers = Map())
      : endpoint_(endPoint), path_(path), method_(method), params_(params),
        headers_(headers) {
    InitEnv();
  }
  /// Destructor. Last instance cleans up libcurl.
  ~WebClient();
  /// Send request.
  ///
  /// \return \c false if error, retrieve error message through
  /// WebClient::ErroMsg
  bool Send();
  /// Set SSL verification options: peer and/or host
  /// Verification should be disabled when sending https requests through
  /// SSH tunnels.
  /// \param[in] verifyPeer verify the authenticity of the peer's certificate
  /// \param[in] verifyHost verify the identity of the server
  bool SSLVerify(bool verifyPeer, bool verifyHost = true);
  /// Set full URL
  /// \param[in] url endpoint + paramters URL
  bool SetUrl(const std::string &url);
  /// Set endpoint: \c <proto>://<server>:<port>
  void SetEndpoint(const std::string &ep);
  /// Set URL path.
  /// \param[in] path URL with endpoint part removed.
  void SetPath(const std::string &path);
  /// Store headers into internal buffer.
  /// \param[in] headers HTTP headers
  void SetHeaders(const Map &headers);
  /// Store request parameters into internal buffer.
  /// \param[in] params URL parameters: key=value&...
  void SetReqParameters(const Map &params);
  /// Set HTTP method.
  /// \param[in] method \c "GET", \c "POST", \c "PUT", \c "DELETE", \c "HEAD"
  void SetMethod(const std::string &method, size_t size = 0);
  /// Url-encode and store data to be posted from {key,value} map.
  /// \param[in] postData {key, value} map of URL paramters.
  void SetUrlEncodedPostData(const Map &postData);
  /// Url-encode and store data to be posted from string in the standard
  /// \c key=value&key2=value2... format.
  /// \param[in] postData \c "key1=value1&key2=value2..."
  void SetUrlEncodedPostData(const std::string &postData);
  /// Store data to be posted.
  /// \param[in] data text to be posted.
  void SetPostData(const std::string &data);
  /// Return status code from last executed request.
  /// \return HTTP status (20*, 30*, 40*, 50*).
  long StatusCode() const;
  /// Return full URL.
  /// \return endpoint + url parameters
  const std::string &GetUrl() const;
  /// Get response content.
  /// \return response bytes (\c char used because it's the type used within \a
  /// libcurl)
  const std::vector<char> &GetResponseBody() const;
  /// Get reponse header.
  /// \return raw response headers as a single byte array.
  const std::vector<char> &GetResponseHeader() const;
  /// Get response body as text.
  std::string GetContentText() const;
  /// Get headers as text.
  /// \return response headers as a single string.
  std::string GetHeaderText() const;
  /// Set function libcurl uses to store response data.
  /// \param[in] f pointer to function called by \a libcurl to consume returned
  /// data.
  /// \param[in] userData pointer to user data.
  bool SetWriteFunction(WriteFunction f, void *userData);
  /// Set function libcurl uses to read data to send.
  /// \param[in] f pointer to function called by \a libcurl to read data to
  /// send; set to \c NULL to read from file.
  /// \param[in] userData pointer to user data.
  bool SetReadFunction(ReadFunction f, void *userData);
  /// \brief Upload file
  ///
  /// \param[in] fname file name
  /// \param[in] fsize file size, if zero file size will be computed
  /// \return \c true if successful, \c false otherwise
  bool UploadFile(const std::string &fname, size_t fsize = 0);
  /// \brief Upload file starting from offset.
  ///
  /// \param[in] fname file name
  /// \param[in] offset offset
  /// \param[in] size file size, if zero size is computed from file.
  /// \return \c true if successful, \c false otherwise
  bool UploadFile(const std::string &fname, size_t offset, size_t size);
  /// \brief Upload file starting from offset using unbuffered I/O read.
  ///
  /// \param[in] fname file name
  /// \param[in] offset offset
  /// \param[in] size file size, if zero size will be computer as `(file size) -
  /// offset`. \return \c true if successful, \c false otherwise
  bool UploadFileUnbuffered(const std::string &fname, size_t offset,
                            size_t size);
  /// \brief Upload file starting from offset using unbuffered memory mapping of
  /// file.
  ///
  /// \param[in] fname file name
  /// \param[in] offset offset
  /// \param[in] size file size, if zero it will compute file size on its ow
  /// \return \c true if successful, \c false otherwise
  bool UploadFileMM(const std::string &fname, size_t offset, size_t size);
  /// \brief Upload data from memory buffer.
  ///
  /// \param[in] data pointer to data
  /// \param[in] offset offset
  /// \param[in] size data size
  /// \return \c true if successful, \c false otherwise
  bool UploadDataFromBuffer(const char *data, size_t offset, size_t size);
  /// Return curl error.
  /// \return \a libcurl error as returned by \c curl_easy_strerror or \c
  /// curl_multi_strerror.
  std::string ErrorMsg() const;
  /// \brief Passthrough to \c curl_easy_setopt
  ///
  /// https://curl.se/libcurl/c/curl_easy_setopt.html
  CURLcode SetOpt(CURLoption option, va_list argp);
  /// \brief Passthrough method to \c curl_easy_getinfo
  ///
  /// https://curl.se/libcurl/c/curl_easy_getinfo.html
  CURLcode GetInfo(CURLINFO info, va_list argp);
  /// Send verbose output to \c stderr or the stream mapped to CURLOPT_STDERR.
  void SetVerbose(bool verbose);
  /// Redirect stderr to file. Returns \c false when it fails.
  bool RedirectSTDErr(FILE *f);
  /// Clear internal buffers
  void ClearBuffers() {
    writeBuffer_.data.clear();
    writeBuffer_.offset = 0;
    headerBuffer_.clear();
  }
  /// Reset read/write functions to default.
  void ResetRWFunctions() {
    SetReadFunction((size_t(*)(void *, size_t, size_t, void *))Reader,
                    &readBuffer_);
    SetWriteFunction((size_t(*)(char *, size_t, size_t, void *))Writer,
                     &writeBuffer_);
  }

private:
  /**
   * \addtogroup Internal
   * @{
   */
  bool Status(CURLcode cc) const;
  void InitEnv();
  bool Init();
  bool BuildURL();
  static size_t Writer(char *data, size_t size, size_t nmemb,
                       Buffer *outbuffer);
  static size_t HeaderWriter(char *data, size_t size, size_t nmemb,
                             std::vector<char> *writerData);
  static size_t Reader(void *ptr, size_t size, size_t nmemb, Buffer *inBuffer);
  static size_t MemReader(void *ptr, size_t size, size_t nmemb,
                          MemReadBuffer *inBuffer);

private:
  CURL *curl_ = NULL; ///< curl handle C pointer
  std::string url_;   ///< full url address <protocol>://<server name>:port/path
  std::array<char, CURL_ERROR_SIZE> errorBuffer_; ///< holds error message
  Buffer writeBuffer_;                            ///< store received response
  std::vector<char> headerBuffer_; ///< store received response buffer
  std::string endpoint_;           ///< https://a.b.c:8080
  std::string path_;               ///< /root/child1/child1.1
  Map headers_;        ///<{{{"host", "myhost"},...} --> host: myhost
  Map params_;         ///< {{"key1", "val1"}, {"key2", "val2"},...} -->
                       ///< key1=val1&key2=val2...
  std::string method_; ///< GET | POST | PUT | HEAD | DELETE
  curl_slist *curlHeaderList_ = NULL; ///< C struct --> NULL not nullptr
  long responseCode_ = 0;             ///< CURL uses a long type for status
  std::string urlEncodedPostData_;    ///< store url-encodd post data
  Buffer readBuffer_;                 ///< store data to send
  MemReadBuffer refBuffer_;           ///< pointer to input memory region.
                                      /**
                                       * @}
                                       */
private:
  static std::atomic<int> numInstances_; ///< track number of instances.
  static std::mutex cleanupMutex_;       ///< guarantee that initialization and
                                         ///< cleanup happen only once.
};

} // namespace sss
