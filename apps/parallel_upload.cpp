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

// Parallel file upload to S3 servers

#include <iostream>
#include <string>
using namespace std;

#include "lyra/lyra.hpp"
#include "s3-client.h"
//------------------------------------------------------------------------------
int main(int argc, char const *argv[]) {
  try {
    UploadConfig config;
    bool showHelp = false;
    auto cli =
        lyra::help(showHelp).description("Upload file to S3 bucket") |
        lyra::opt(config.s3AccessKey,
                  "awsAccessKey")["-a"]["--access_key"]("AWS access key")
            .optional() |
        lyra::opt(config.s3SecretKey,
                  "awsSecretKey")["-s"]["--secret_key"]("AWS secret key")
            .optional() |
        lyra::opt(config.endpoint,
                  "endpoint")["-e"]["--endpoint"]("Endpoint URL")
            .required() |
        lyra::opt(config.bucket, "bucket")["-b"]["--bucket"]("Bucket name")
            .required() |
        lyra::opt(config.key, "key")["-k"]["--key"]("Key name").required() |
        lyra::opt(config.file, "file")["-f"]["--file"]("File name").required() |
        lyra::opt(config.jobs, "parallel jobs")["-j"]["--jobs"](
            "Number of parallel upload jobs")
            .optional() |
        lyra::opt(config.credentials,
                  "credentials file")["-c"]["--credentials"](
            "Credentials file, AWS cli format")
            .optional() |
        lyra::opt(config.awsProfile, "AWS config profile")["-p"]["--profile"](
            "Profile in AWS config file")
            .optional() |
        lyra::opt(config.maxRetries, "Max retries")["-r"]["--retries"](
            "Max number of per-multipart part retries")
            .optional();

    // Parse the program arguments:
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
    cout << UploadFile(config);
    return 0;
  } catch (const exception &e) {
    cerr << e.what() << endl;
    return 1;
  }
}
