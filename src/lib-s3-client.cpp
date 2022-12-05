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

// Send S3v4 signed REST requests

#include "lib-s3-client.h"
#include "aws_sign.h"
#include "common.h"

#include <fstream>
#include <set>

using namespace std;
using namespace sss;

//------------------------------------------------------------------------------
void Validate(const S3Args &args) {
  if (args.s3AccessKey.empty() && !args.s3SecretKey.empty() ||
      args.s3SecretKey.empty() && !args.s3AccessKey.empty()) {
    throw invalid_argument(
        "ERROR: both access and secret keys have to be specified");
  }
#ifdef VALIDATE_URL
  const URL url = ParseURL(args.endpoint);
  if (url.proto != "http" && url.proto != "https") {
    throw invalid_argument(
        "ERROR: only 'http' and 'https' protocols supported");
  }
  regex re(R"((\w+\.)*\w+\.\w+)");
  if (!regex_match(url.host, re)) {
    throw invalid_argument("ERROR: invalid endpoint format, should be "
                           "http[s]://hostname[:port]");
  }
  if (url.port > 0xFFFF) {
    throw invalid_argument(
        "ERROR: invalid port number, should be in range[1-65535]");
  }
#endif
  const set<string> methods({"get", "put", "post", "delete", "head"});
  if (methods.count(ToLower(args.method)) == 0) {
    throw invalid_argument(
        "ERROR: only 'get', 'put', 'post', 'delete', 'head' methods "
        "supported");
  }
}

//-----------------------------------------------------------------------------
WebClient SendS3Request(S3Args &args) {
  // verify peer and host certificate only if signing url == endpoint
  const bool verify = args.signUrl.empty();
  ///\warning disabling verification for both peer and host
  const bool verifyHost = verify;
  const bool verifyPeer = verify;
  if (args.signUrl.empty())
    args.signUrl = args.endpoint;
  string path;
  if (!args.bucket.empty()) {
    path += "/" + args.bucket;
    if (!args.key.empty())
      path += "/" + args.key;
  }
  const Map params = ParseParams(args.params);
  Map headers = ParseHeaders(args.headers);
  if (!args.s3AccessKey.empty()) {
    auto signedHeaders =
        SignHeaders(args.s3AccessKey, args.s3SecretKey, args.signUrl,
                    args.method, args.bucket, args.key, "", params, headers);
    headers.insert(begin(signedHeaders), end(signedHeaders));
  }
  WebClient req(args.endpoint, path, args.method, params, headers);
  req.SSLVerify(verifyPeer, verifyHost);
  FILE *of = NULL;
  if (!args.outfile.empty()) {
    of = fopen(args.outfile.c_str(), "wb");
    req.SetWriteFunction(NULL, of); // default is to write to file
  }
  if (!args.data.empty()) {
    if (args.data[0] != '@') {
      if (ToLower(args.method) == "post") {
        req.SetUrlEncodedPostData(ParseParams(args.data));
        req.SetMethod("POST");
        req.Send();
      } else { // "put"
        vector<uint8_t> data(begin(args.data), end(args.data));
        // req.SetUploadData(data);
        // req.Send();
        req.UploadDataFromBuffer(args.data.c_str(), 0, data.size());
      }
    } else {
      if (ToLower(args.method) == "put") {
        req.UploadFile(args.data.substr(1));
      } else if (args.method == "post") {
        ifstream t(args.data.substr(1));
        const string str((istreambuf_iterator<char>(t)),
                         istreambuf_iterator<char>());
        req.SetMethod("POST");
        req.SetPostData(str);
        req.Send();
      } else {
        throw domain_error("Wrong method " + args.method);
      }
    }
  } else
    req.Send();
  if (of)
    fclose(of);
  return req;
}
