
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

#include "s3-api.h"
#include <stdio.h>

using namespace std;

namespace sss {
namespace api {
/// [WebClient::Config]
WebClient &S3Api::Config(const SendParams &p) {
  // if credentials empty send regular unsigned request
  auto sh = Access().empty() ? Headers()
                             : SignHeaders({.access = Access(),
                                            .secret = Secret(),
                                            .endpoint = Endpoint(),
                                            .method = p.method,
                                            .bucket = p.bucket,
                                            .key = p.key,
                                            .payloadHash = p.payloadHash,
                                            .parameters = p.params,
                                            .headers = p.headers,
                                            .region = p.region});
  std::string path;
  if (!p.bucket.empty()) {
    path += "/" + p.bucket;
    if (!p.key.empty()) {
      path += "/" + p.key;
    }
  }
  Clear();
  webClient_.SetEndpoint(Endpoint());
  webClient_.SetPath(path);
  webClient_.SetMethod(p.method);
  webClient_.SetReqParameters(p.params);
  webClient_.SetHeaders(sh);
  return webClient_;
}
/// [WebClient::Config]

ssize_t S3Api::GetObjectSize(const string &bucket, const string &key) {
  const bool caseInsensitive = false;
  try {
    const string v =
        GetValue(HeadObject(bucket, key), "length", caseInsensitive);
    // if object exists header should always be present, but just in case...
    return v.empty() ? -1 : stoll(v);
  } catch (...) {
    return -1;
  }
}
} // namespace api
} // namespace sss
