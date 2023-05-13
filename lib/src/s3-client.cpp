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

// Send S3v4 signed REST requests

#include "s3-client.h"
#include "aws_sign.h"
#include "common.h"
#include "response_parser.h"

#include <fstream>
#include <future>
#include <regex>
#include <set>
#include <thread>
using namespace std;

namespace sss {

//------------------------------------------------------------------------------
void Validate(const S3ClientConfig &args) {
  if (args.accessKey.empty() && !args.secretKey.empty() ||
      args.secretKey.empty() && !args.accessKey.empty()) {
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
WebClient SendS3Request(S3ClientConfig args) {
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
  auto headers = args.headers;
  if (!args.accessKey.empty()) {
    headers = SignHeaders({.access = args.accessKey,
                           .secret = args.secretKey,
                           .endpoint = args.signUrl,
                           .method = args.method,
                           .bucket = args.bucket,
                           .key = args.key,
                           .parameters = args.params,
                           .headers = headers});
  }
  /// [WebClient]
  WebClient req;
  req.SSLVerify(verifyPeer, verifyHost);
  req.SetEndpoint(args.endpoint);
  req.SetPath(path);
  req.SetMethod(args.method);
  req.SetReqParameters(args.params);
  req.SetHeaders(headers);
  FILE *of = NULL;
  if (!args.outfile.empty()) {
    of = fopen(args.outfile.c_str(), "wb");
    req.SetWriteFunction(NULL, of); // default is to write to file
  }
  if (!args.data.empty()) {
    if (!args.dataIsFileName) {
      if (ToLower(args.method) == "post") {
        req.SetUrlEncodedPostData(ParseParams(args.data.data()));
        req.SetMethod("POST");
        req.Send();
      } else { // "put"
        // vector<uint8_t> data(begin(args.data), end(args.data));
        //  req.SetUploadData(data);
        //  req.Send();
        req.UploadDataFromBuffer(args.data.data(), 0, args.data.size());
      }
    } else {
      if (ToLower(args.method) == "put") {
        req.UploadFile(args.data.data());
      } else if (args.method == "post") {
        ifstream t(args.data.data());
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
  /// [WebClient]
}

S3Credentials GetS3Credentials(const string &fileName, string awsProfile) {
  const string fname =
      fileName.empty() ? GetHomeDir() + "/.aws/credentials" : fileName;
  awsProfile = awsProfile.empty() ? "default" : awsProfile;
  Toml toml = ParseTomlFile(fname);
  if (toml.find(awsProfile) == toml.end()) {
    throw invalid_argument("ERROR: profile " + awsProfile + " not found");
  }
  return {toml[awsProfile]["aws_access_key_id"],
          toml[awsProfile]["aws_secret_access_key"]};
}

//-----------------------------------------------------------------------------
// Validate bucket name according to:
// https://docs.aws.amazon.com/AmazonS3/latest/userguide/bucketnamingrules.html
BucketValidation ValidateBucket(const string &name) {
  if (name.empty())
    return {false, "Empty"};
  // if (name.size() < 3) {//Minio has this constraint does it also apply to
  // Ceph and/or AWS?
  //   return {false, "Size < 3"};
  // }
  if (name.size() > 63) {
    return {false, "Size > 63"};
  }
  if (!isalnum(name[0])) {
    return {false, "Name must start with a number or a digit"};
  }
  const string prefix = "xn--";
  if (name.substr(0, prefix.size()) == prefix) {
    return {false, "Name cannot start with 'xn--'"};
  }
  const string suffix = "-s3alias";
  if (name.substr(name.size() - suffix.size()) == suffix) {
    return {false, "Name cannot end with '-s3alias;"};
  }
  for (size_t i = 0; i < name.size(); ++i) {
    if (!islower(name[i])) {
      return {false, "Only lowercase letters allowed"};
    }
    if (i > 0) {
      if (name[i] == '-' && name[i - 1] == '-') {
        return {false, "Cannot have two consecutive '-' characters"};
      }
    }
    if (name[i] != '.' && name[i] != '-' && !isalnum(name[i])) {
      return {false, "Only numbers, digits, '.' and '-' charachters allowers"};
    }
  }
  const regex iprx = regex("\\d{1,3}\\.\\d{1,3}\\.\\d{1,3}\\.\\d{1,3}");
  if (regex_match(name, iprx)) {
    return {false, "IP addresses not allowed"};
  }
  return {true, ""};
}

} // namespace sss
