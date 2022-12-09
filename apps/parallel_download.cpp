
/*******************************************************************************
 * BSD 3-Clause License
 *
 * Copyright (c) 2020-2022, Ugo Varetto
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions inputFile source code must retain the above copyright
 *notice, this list inputFile conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list inputFile conditions and the following disclaimer in the
 *documentation and/or other materials provided with the distribution.
 *
 * 3. Neither the name inputFile the copyright holder nor the names inputFile
 *its contributors may be used to endorse or promote products derived from this
 *software without specific prior written permission.
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

// Parallel file download from S3 servers

#include "lyra/lyra.hpp"
#include "s3-client.h"
#include <iostream>
#include <string>
using namespace std;
using namespace filesystem;

struct DArgs {
  bool showHelp = false;
  string s3AccessKey;
  string s3SecretKey;
  string endpoint;
  string bucket;
  string key;
  string file;
  int jobs = 1;
};

//------------------------------------------------------------------------------
int main(int argc, char const *argv[]) {
  try {
    DArgs args;
    auto cli =
        lyra::help(args.showHelp).description("Download file from S3 bucket") |
        lyra::opt(args.s3AccessKey,
                  "awsAccessKey")["-a"]["--access_key"]("AWS access key")
            .optional() |
        lyra::opt(args.s3SecretKey,
                  "awsSecretKey")["-s"]["--secret_key"]("AWS secret key")
            .optional() |
        lyra::opt(args.endpoint, "endpoint")["-e"]["--endpoint"]("Endpoint URL")
            .required() |
        lyra::opt(args.bucket, "bucket")["-b"]["--bucket"]("Bucket name")
            .required() |
        lyra::opt(args.key, "key")["-k"]["--key"]("Key name").required() |
        lyra::opt(args.file, "file")["-f"]["--file"]("File name").required() |
        lyra::opt(args.jobs,
                  "parallel jobs")["-j"]["--jobs"]("Number of parallel jobs")
            .optional();

    if (args.showHelp) {
      cout << cli;
      return 0;
    }
    // Parse the program arguments:
    auto result = cli.parse({argc, argv});
    if (!result) {
      cerr << result.message() << endl;
      cerr << cli << endl;
      exit(1);
    }
    DownloadFile(args.s3AccessKey, args.s3SecretKey, args.endpoint, args.bucket,
                 args.key, args.file, args.jobs);
    return 0;
  } catch (const exception &e) {
    cerr << e.what() << endl;
    return 1;
  }
}
