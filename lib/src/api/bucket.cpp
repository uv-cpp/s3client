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

#include <fstream>
#include <future>
#include <set>
#include <thread>
using namespace std;

namespace sss {
namespace api {

//------------------------------------------------------------------------------
bool S3Client::TestBucket(const string &bucket) {
  try {
    Send({.method = "GET", .bucket = bucket, .signUrl = SigningEndpoint()});
    return true;
  } catch (...) {
    return false;
  }
}

//------------------------------------------------------------------------------
void S3Client::CreateBucket(const std::string &bucket, const Headers &headers) {
  Send({.method = "PUT", .bucket = bucket, .headers = headers});
}

//------------------------------------------------------------------------------
void S3Client::DeleteBucket(const std::string &bucket, const Headers &headers) {
  Send({.method = "DELETE", .bucket = bucket, .headers = headers});
}

//------------------------------------------------------------------------------
Headers S3Client::HeadBucket(const string &bucket, const Headers &headers) {
  const auto &wc =
      Send({.method = "HEAD", .bucket = bucket, .headers = headers});
  return HTTPHeaders(wc.GetContentText());
}

//------------------------------------------------------------------------------
vector<BucketInfo> S3Client::ListBuckets(const Headers &headers) {
  const auto &wc = Send({.method = "GET", .headers = headers});
  const string content = wc.GetContentText();
  return {};
  auto tags = XMLTags(content, "Bucket");
  vector<BucketInfo> bi;
  for (const auto &i : tags) {
    const auto name = XMLTag(i, "Name");
    const auto creationDate = XMLTag(i, "CreationDate");
    bi.push_back({name, creationDate});
  }
  return bi;
}

} // namespace api
} // namespace sss
