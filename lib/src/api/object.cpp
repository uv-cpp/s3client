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

// Upload files

#include "aws_sign.h"
#include "common.h"
#include "response_parser.h"
#include "s3-api.h"
#include "s3-client.h"

#include <algorithm>
#include <fstream>
#include <future>
#include <set>
#include <thread>
using namespace std;

namespace sss {
namespace api {

extern void HandleError(const WebClient &, const string & = "");

//------------------------------------------------------------------------------
ETag S3Client::PutObject(const std::string &bucket, const std::string &key,
                         const ByteArray &buffer, const Headers &headers) {
  Clear();
  webClient_.SetMethod("PUT");
  webClient_.SetPath(bucket + "/" + key);
  webClient_.SetHeaders(headers);
  webClient_.SetUploadData(buffer);
  webClient_.Send();
  HandleError(webClient_);
  const string etag = XMLTag(webClient_.GetContentText(), "ETag");
  if (etag.empty()) {
    throw runtime_error("Missing ETag");
  }
  return etag;
}

//------------------------------------------------------------------------------
ETag S3Client::PutObject(const std::string &bucket, const std::string &key,
                         const char *buffer, size_t size, size_t offset,
                         const Headers &headers) {

  Clear();
  webClient_.SetMethod("PUT");
  webClient_.SetPath(bucket + "/" + key);
  webClient_.SetHeaders(headers);
  webClient_.UploadDataFromBuffer(buffer, size, offset);
  HandleError(webClient_);
  const string etag = XMLTag(webClient_.GetContentText(), "ETag");
  if (etag.empty()) {
    throw runtime_error("Missing ETag");
  }
  return etag;
}

//------------------------------------------------------------------------------
ByteArray S3Client::GetObject(const std::string &bucket, const std::string &key,
                              size_t begin, size_t end, Headers headers) {

  Clear();
  webClient_.SetMethod("GET");
  webClient_.SetPath(bucket + "/" + key);
  if (end > 0) {
    headers.insert(
        {"Range", "bytes=" + to_string(begin) + "-" + to_string(end)});
  }
  webClient_.SetHeaders(headers);
  webClient_.Send();
  HandleError(webClient_);
  return webClient_.GetResponseBody();
}

//------------------------------------------------------------------------------
void S3Client::GetObject(const std::string &bucket, const std::string &key,
                         ByteArray &buffer, size_t offset, size_t begin,
                         size_t end, Headers headers) {
  Clear();
  webClient_.SetMethod("GET");
  webClient_.SetPath(bucket + "/" + key);
  if (end > 0) {
    headers.insert(
        {"Range", "bytes=" + to_string(begin) + "-" + to_string(end)});
  }
  webClient_.SetHeaders(headers);
  webClient_.Send();
  HandleError(webClient_);
  const auto &bytes = webClient_.GetResponseBody();
  if (buffer.begin() + offset + bytes.size() >= buffer.end()) {
    throw range_error("Out buffer too small");
  }
  copy(bytes.begin(), bytes.end(), buffer.begin() + offset);
}

//------------------------------------------------------------------------------
void S3Client::GetObject(const std::string &bucket, const std::string &key,
                         char *buffer, size_t offset, size_t begin, size_t end,
                         Headers headers) {

  Clear();
  webClient_.SetMethod("GET");
  webClient_.SetPath(bucket + "/" + key);
  if (end > 0) {
    headers.insert(
        {"Range", "bytes=" + to_string(begin) + "-" + to_string(end)});
  }
  webClient_.SetHeaders(headers);
  webClient_.Send();
  HandleError(webClient_);
  const auto &bytes = webClient_.GetResponseBody();
  copy(bytes.begin(), bytes.end(), buffer + offset);
}

//------------------------------------------------------------------------------
void S3Client::DeleteObject(const std::string &bucket, const std::string &key,
                            const Headers &headers) {
  Clear();
  webClient_.SetMethod("DELETE");
  webClient_.SetPath(bucket + "/" + key);
  webClient_.SetHeaders(headers);
  webClient_.Send();
  HandleError(webClient_);
}
} // namespace api
} // namespace sss
