/*******************************************************************************
* BSD 3-Clause License
* 
* Copyright (c) 2020-2022, Ugo Varetto
* All rights reserved.
* 
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
* 
* 1. Redistributions of source code must retain the above copyright notice, this
*    list of conditions and the following disclaimer.
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
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
* DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
* FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
* SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
* CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
* OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
* OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*******************************************************************************/
#include <iostream>
#include <string>
#include <map>

#include "aws_sign.h"
#include "url_utility.h"
#include "lyra/lyra.hpp"
#include "common.h"

using namespace std;

using namespace sss;

//------------------------------------------------------------------------------
struct Args {
    bool showHelp = false;
    string awsAccessKey;
    string awsSecretKey;
    string endpoint;
    string bucket;
    string key;
    string params;
    string method;
    int expiration = 3600;
};

//------------------------------------------------------------------------------
void PrintArgs(const Args& args) {
    cout << "awsAccessKey: " << args.awsAccessKey << endl
         << "awsSecretKey: " << args.awsSecretKey << endl
         << "endpoint:     " << args.endpoint << endl
         << "method:       " << ToUpper(args.method) << endl
         << "bucket:       " << args.bucket << endl
         << "key:          " << args.key << endl
         << "expiration:   " << args.expiration << endl
         << "parameters:   " << args.params << endl;
}

//------------------------------------------------------------------------------
// Generate and sign url for usage with AWS S3 compatible environment such
// as Amazone services and Ceph object gateway
int main(int argc, char const* argv[]) {
    // The parser with the multiple option arguments and help option.
    Args args;
    auto cli =
        lyra::help(args.showHelp).description("Pre-sign S3 URLs") |
        lyra::opt(args.awsAccessKey,
                  "awsAccessKey")["-a"]["--access_key"]("AWS access key")
            .required() |
        lyra::opt(args.awsSecretKey,
                  "awsSecretKey")["-s"]["--secret_key"]("AWS secret key")
            .required() |
        lyra::opt(args.endpoint, "endpoint")["-e"]["--endpoint"]("Endpoint URL")
            .required() |
        lyra::opt(args.method, "method")["-m"]["--method"](
            "HTTP method: get | put | post | delete")
            .required() |
        lyra::opt(args.params, "params")["-p"]["--params"](
            "URL request parameters. key1=value1;key2=...")
            .optional() |
        lyra::opt(args.bucket, "bucket")["-b"]["--bucket"]("Bucket name") |
        lyra::opt(args.expiration, "expiration")["-t"]["--expiration"](
            "expiration time in seconds")
            .optional() |
        lyra::opt(args.key, "key")["-k"]["--key"]("Key name").optional();

    // Parse the program arguments:
    auto result = cli.parse({argc, argv});
    if (!result) {
        cerr << result.errorMessage() << endl;
        cerr << cli << endl;
        exit(1);
    }
    if (args.showHelp) {
        cout << cli;
        return 0;
    }
    //PrintArgs(args);
    const Map params = ParseParams(args.params); 
    const string signedURL =
        SignedURL(args.awsAccessKey, args.awsSecretKey, args.expiration,
                  args.endpoint, ToUpper(args.method), args.bucket, args.key);
    cout << signedURL;
    return 0;
}
