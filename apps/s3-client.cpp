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
 * \addtogroup Applications
 * @{
 */
/**
 * \file s3-client.cpp
 * \brief Send S3v4-signed requests to S3 andpoint and print raw responses.
 */
/// [Generic S3 Client]
#include <algorithm>
#include <iostream>
#include <vector>

#include "lyra/lyra.hpp"
#include "s3-client.h"
#include "url_utility.h"

using namespace std;
using namespace sss;

//------------------------------------------------------------------------------
int main(int argc, char const *argv[]) {
  try {
    S3ClientConfig args;
    bool showHelp = false;
    string data;
    string params;
    string headers;
    auto cli =
        lyra::help(showHelp).description(
            "Send REST request with S3v4 signing") |
        lyra::opt(args.accessKey,
                  "awsAccessKey")["-a"]["--access_key"]("AWS access key")
            .optional() |
        lyra::opt(args.secretKey,
                  "awsSecretKey")["-s"]["--secret_key"]("AWS secret key")
            .optional() |
        lyra::opt(args.endpoint,
                  "endpoint")["-e"]["--endpoint"]("End-point URL")
            .required() |
        lyra::opt(args.method, "method")["-m"]["--method"](
            "HTTP method: get | put | post | delete | head")
            .optional() |
        lyra::opt(params, "params")["-p"]["--params"](
            "URL request parameters. key1=value1;key2=...")
            .optional() |
        lyra::opt(args.bucket, "bucket")["-b"]["--bucket"]("Bucket name")
            .optional() |
        lyra::opt(args.key, "key")["-k"]["--key"]("Key name").optional() |
        lyra::opt(data, "content")["-d"]["--data"](
            "Value data if -F option not present, filename otherwise")
            .optional() |
        lyra::opt(headers, "headers")["-H"]["--headers"](
            "URL request headers. header1:value1;header2:...")
            .optional() |
        lyra::opt(args.outfile,
                  "output file")["-o"]["--out-file"]("output file")
            .optional() |
        lyra::opt(args.signUrl, "signing url")["-S"]["--sign-url"](
            "URL for signing; can be different from endpoint to support "
            "tunnels")
            .optional() |
        lyra::opt(args.dataIsFileName)["-F"]["--data-is-filename"](
            "Interpret 'data' field as file name and read data from file")
            .optional();

    // Parse program arguments:
    auto result = cli.parse({argc, argv});
    if (!result) {
      cerr << result.message() << endl;
      cerr << cli << endl;
      exit(1);
    }
    if (showHelp) {
      cout << cli;
      return 0;
    }
    if (!params.empty())
      args.params = ParseParams(params);
    if (!headers.empty())
      args.headers = ParseHeaders(headers);
    if (!data.empty()) {
      std::copy(begin(data), end(data), back_inserter(args.data));
      if (args.dataIsFileName) {
        args.data.push_back('\0');
      }
    }
    auto req = SendS3Request(args);
    // Status code 20* = no error
    cout << "Status: " << req.StatusCode() << endl << endl;
    // Response body
    vector<char> resp = req.GetResponseBody();
    string t(begin(resp), end(resp));
    cout << t << endl << endl;
    // Response header
    vector<char> h = req.GetResponseHeader();
    string hs(begin(h), end(h));
    cout << hs << endl;
    return 0;
  } catch (const exception &e) {
    cerr << e.what() << endl;
    return 1;
  }
}
/// [Generic S3 Client]
/**
 * @}
 */

// to use a tunnel:
// ssh -f -i ~/.ssh/id_rsa -L 127.0.0.1:8080:<final endpoint>:8080 \
// -N <username>@<server behind the firewall>
