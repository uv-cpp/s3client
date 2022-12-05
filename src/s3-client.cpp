
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

#include <fstream>
#include <iostream>
#include <regex>
#include <set>
#include <streambuf>

#include "aws_sign.h"
#include "common.h"
#include "lyra/lyra.hpp"
#include "webclient.h"

using namespace std;
using namespace sss;

//------------------------------------------------------------------------------
struct Args {
  bool showHelp = false;
  string s3AccessKey;
  string s3SecretKey;
  string endpoint; // actual endpoint where requests are sent
  string signUrl;  // url used to sign, allows requests to work across tunnels
  string bucket;
  string key;
  string params;
  string method = "GET";
  string headers;
  string data;
  string outfile;
};

//------------------------------------------------------------------------------
void Validate(const Args &args) {
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
WebClient SendS3Request(Args &args) {
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

//------------------------------------------------------------------------------
int main(int argc, char const *argv[]) {
  try {
    Args args;
    auto cli =
        lyra::help(args.showHelp)
            .description("Send REST request with S3v4 signing") |
        lyra::opt(args.s3AccessKey,
                  "awsAccessKey")["-a"]["--access_key"]("AWS access key")
            .optional() |
        lyra::opt(args.s3SecretKey,
                  "awsSecretKey")["-s"]["--secret_key"]("AWS secret key")
            .optional() |
        lyra::opt(args.endpoint,
                  "endpoint")["-e"]["--endpoint"]("End-point URL")
            .required() |
        lyra::opt(args.method, "method")["-m"]["--method"](
            "HTTP method: get | put | post | delete | head")
            .optional() |
        lyra::opt(args.params, "params")["-p"]["--params"](
            "URL request parameters. key1=value1;key2=...")
            .optional() |
        lyra::opt(args.bucket, "bucket")["-b"]["--bucket"]("Bucket name")
            .optional() |
        lyra::opt(args.key, "key")["-k"]["--key"]("Key name").optional() |
        lyra::opt(args.data, "content")["-v"]["--value"](
            "Value data, use '@' prefix for file name")
            .optional() |
        lyra::opt(args.headers, "headers")["-H"]["--headers"](
            "URL request headers. header1:value1;header2:...")
            .optional() |
        lyra::opt(args.outfile,
                  "output file")["-o"]["--out-file"]("output file")
            .optional() |
        lyra::opt(args.signUrl, "signing url")["-S"]["--sign-url"](
            "URL for signing; can be different from endpoint to support "
            "tunnels")
            .optional();

    // Parse rogram arguments:
    auto result = cli.parse({argc, argv});
    if (!result) {
      cerr << result.message() << endl;
      cerr << cli << endl;
      exit(1);
    }
    if (args.showHelp) {
      cout << cli;
      return 0;
    }
    Validate(args);
    auto req = std::move(SendS3Request(args));
    // Status code 0 = no error
    cout << "Status: " << req.StatusCode() << endl << endl;
    // Response body
    vector<uint8_t> resp = req.GetResponseBody();
    string t(begin(resp), end(resp));
    cout << t << endl << endl;
    // Response header
    vector<uint8_t> h = req.GetResponseHeader();
    string hs(begin(h), end(h));
    cout << hs << endl;
    return 0;
  } catch (const exception &e) {
    cerr << e.what() << endl;
    return 1;
  }
}

// to use a tunnel:
// ssh -f -i ~/.ssh/id_rsa -L 127.0.0.1:8080:<final endpoint>:8080 \
// -N <username>@<server behind the firewall>
