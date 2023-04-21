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

#include "aws_sign.h"
#include "common.h"
#include "error.h"
#include "response_parser.h"
#include "s3-api.h"
#include "s3-client.h"

#include <algorithm>
#include <filesystem>
using namespace std;

namespace sss {
namespace api {

S3Client::ListObjectV2Result ParseObjects(const std::string &xml);

//------------------------------------------------------------------------------
ETag S3Client::PutObject(const std::string &bucket, const std::string &key,
                         const CharArray &buffer, Headers headers,
                         const string &payloadHash) {

  headers.insert({"content-length", to_string(buffer.size())});
  const auto &wc = Send({.method = "PUT",
                         .bucket = bucket,
                         .key = key,
                         .headers = headers,
                         .payloadHash = payloadHash,
                         .uploadData = buffer.data(),
                         .uploadDataSize = buffer.size()});
  const string etag = HTTPHeader(wc.GetHeaderText(), "ETag");
  if (etag.empty()) {
    throw runtime_error("Missing ETag");
  }
  return etag;
}

//------------------------------------------------------------------------------
ETag S3Client::PutObject(const std::string &bucket, const std::string &key,
                         const char *buffer, size_t size, Headers headers,
                         const string &payloadHash) {

  headers.insert({"content-length", to_string(size)});
  const auto &wc = Send({.method = "PUT",
                         .bucket = bucket,
                         .key = key,
                         .headers = headers,
                         .payloadHash = payloadHash,
                         .uploadData = buffer,
                         .uploadDataSize = size});
  const string etag = HTTPHeader(wc.GetHeaderText(), "ETag");
  if (etag.empty()) {
    throw runtime_error("Missing ETag");
  }
  return etag;
}

//------------------------------------------------------------------------------
ETag S3Client::PutFileObject(const std::string &fileName,
                             const std::string &bucket, const std::string &key,
                             size_t offset, size_t size, Headers headers,
                             const std::string &payloadHash) {
  const size_t fsize = size ? size : filesystem::file_size(fileName);
  headers.insert({"content-length", to_string(fsize)});
  Config({.method = "PUT",
          .bucket = bucket,
          .key = key,
          .headers = headers,
          .payloadHash = payloadHash});
  if (!webClient_.UploadFile(fileName, 0, fsize)) {
    throw runtime_error("Error uploading file - " + webClient_.ErrorMsg());
  }
  HandleError(webClient_);
  return HTTPHeader(webClient_.GetHeaderText(), "ETag");
}

//------------------------------------------------------------------------------
const CharArray &S3Client::GetObject(const std::string &bucket,
                                     const std::string &key, size_t begin,
                                     size_t end, Headers headers) {

  if (end > 0) {
    headers.insert(
        {"Range", "bytes=" + to_string(begin) + "-" + to_string(end)});
  }
  const auto &wc =
      Send({.method = "GET", .bucket = bucket, .key = key, .headers = headers});
  return wc.GetResponseBody();
}

//------------------------------------------------------------------------------
void S3Client::GetObject(const std::string &bucket, const std::string &key,
                         CharArray &buffer, size_t offset, size_t begin,
                         size_t end, Headers headers) {
  if (end > 0) {
    headers.insert(
        {"Range", "bytes=" + to_string(begin) + "-" + to_string(end)});
  }
  const auto &wc =
      Send({.method = "GET", .bucket = bucket, .key = key, .headers = headers});
  const auto &bytes = wc.GetResponseBody();
  if (buffer.begin() + offset + bytes.size() >= buffer.end()) {
    throw range_error("Out buffer too small");
  }
  copy(bytes.begin(), bytes.end(), buffer.begin() + offset);
}

//------------------------------------------------------------------------------
void S3Client::GetObject(const std::string &bucket, const std::string &key,
                         char *buffer, size_t offset, size_t begin, size_t end,
                         Headers headers) {

  if (end > 0) {
    headers.insert(
        {"Range", "bytes=" + to_string(begin) + "-" + to_string(end)});
  }
  const auto &wc =
      Send({.method = "GET", .bucket = bucket, .key = key, .headers = headers});
  const auto &bytes = wc.GetResponseBody();
  copy(bytes.begin(), bytes.end(), buffer + offset);
}

//------------------------------------------------------------------------------
void S3Client::GetFileObject(const std::string &fileName,
                             const std::string &bucket, const std::string &key,
                             size_t offset, size_t begin, size_t end,
                             Headers headers) {

  FILE *out = !filesystem::exists(fileName) ? fopen(fileName.c_str(), "wb")
                                            : fopen(fileName.c_str(), "r+b");
  if (!out) {
    throw runtime_error("Cannot open file " + fileName + " for writing");
  }
  fseek(out, offset, SEEK_SET);

  headers.insert({"range", "bytes=" + to_string(begin) + "-" + to_string(end)});
  Config({.method = "GET", .bucket = bucket, .key = key, .headers = headers});
  webClient_.SetWriteFunction(nullptr, out);
  webClient_.Send();
  fclose(out);
  HandleError(webClient_);
}

//------------------------------------------------------------------------------
void S3Client::DeleteObject(const std::string &bucket, const std::string &key,
                            const Headers &headers) {
  Send({.method = "DELETE", .bucket = bucket, .key = key, .headers = headers});
}

//------------------------------------------------------------------------------
Headers S3Client::HeadObject(const std::string &bucket, const std::string &key,
                             const Headers &headers) {
  const auto &wc = Send(
      {.method = "HEAD", .bucket = bucket, .key = key, .headers = headers});
  return HTTPHeaders(wc.GetHeaderText());
}

//------------------------------------------------------------------------------
bool S3Client::TestObject(const std::string &bucket, const std::string &key) {
  try {
    HeadObject(bucket, key);
    return true;
  } catch (...) {
    return false;
  }
}

//------------------------------------------------------------------------------
S3Client::ListObjectV2Result
S3Client::ListObjectsV2(const std::string &bucket,
                        const ListObjectV2Config &config,
                        const Headers &headers) {
  Map params;
  params["continuation_token"] = config.continuationToken;
  params["delimiter"] = config.delimiter;
  params["encoding-type"] = config.encodingType;
  params["fetch-owner"] = config.fetchOwner;
  // params["max-keys"] = to_string(config.maxKeys);
  params["prefix"] = config.prefix;
  params["start-after"] = config.startAfter;
  const auto &wc = Send({.method = "GET",
                         .bucket = bucket,
                         .params = params,
                         .headers = headers});
  return ParseObjects(webClient_.GetContentText());
}

} // namespace api
} // namespace sss
