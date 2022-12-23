
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
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
using namespace std;
using namespace sss;
using namespace filesystem;

//------------------------------------------------------------------------------
int main(int argc, char const *argv[]) {
  try {
    S3FileTransferConfig config;
    bool showHelp = false;
    string endpoint;
    string endpointsFile;
    string credentialsFile;
    string awsProfile;
    bool overwrite = false;
    auto cli =
        lyra::help(showHelp).description("Download file from S3 bucket") |
        lyra::opt(config.accessKey,
                  "awsAccessKey")["-a"]["--access_key"]("AWS access key")
            .optional() |
        lyra::opt(config.secretKey,
                  "awsSecretKey")["-s"]["--secret_key"]("AWS secret key")
            .optional() |
        lyra::opt(endpoint, "endpoint")["-e"]["--endpoint"]("Endpoint URL")
            .optional() |
        lyra::opt(endpointsFile, "endpointsFile")["-E"]["--endpoints-file"](
            "File with list of endpoints")
            .optional() |
        lyra::opt(config.bucket, "bucket")["-b"]["--bucket"]("Bucket name")
            .required() |
        lyra::opt(config.key, "key")["-k"]["--key"]("Key name").required() |
        lyra::opt(config.file, "file")["-f"]["--file"]("File name").required() |
        lyra::opt(credentialsFile, "credentials file")["-c"]["--credentials"](
            "Credentials file, AWS cli format")
            .optional() |
        lyra::opt(awsProfile, "AWS config profile")["-p"]["--profile"](
            "Profile in AWS config file")
            .optional() |
        lyra::opt(config.maxRetries, "Max retries")["-r"]["--retries"](
            "Max number of per-multipart part retries")
            .optional() |
        lyra::opt(config.jobs,
                  "parallel jobs")["-j"]["--jobs"]("Number of parallel jobs")
            .optional() |
        lyra::opt(overwrite)["-y"]["--overwrite"](
            "Overwrite exsisting file, default is 'false'")
            .optional();
    if (showHelp) {
      cout << cli;
      return 0;
    }
    // Parse program arguments:
    auto result = cli.parse({argc, argv});
    if (!result) {
      cerr << result.message() << endl;
      cerr << cli << endl;
      exit(1);
    }

    if (exists(config.file) && !overwrite) {
      cerr << "File exists, use the '-y' command line switch to overwrite"
           << endl;
      exit(EXIT_FAILURE);
    }
    if (!endpoint.empty()) {
      config.endpoints.push_back(endpoint);
    } else {
      if (endpointsFile.empty()) {
        cerr << "Specify either an endpoing or a file to read endpoints from"
             << endl;
        exit(EXIT_FAILURE);
      }
      ifstream is(endpointsFile);
      if (!is) {
        cerr << "Cannot read from " << endpointsFile << endl;
        exit(EXIT_FAILURE);
      }
      string line;
      while (getline(is, line)) {
        Trim(line);
        if (line.empty() || line[0] == '#')
          continue;
        config.endpoints.push_back(line);
      }
    }
    if (config.accessKey.empty() || config.secretKey.empty()) {
      if (credentialsFile.empty()) {
        credentialsFile = GetHomeDir() + "/.aws/credentials";
      }
      auto c = GetS3Credentials(credentialsFile, awsProfile);
      config.accessKey = c.accessKey;
      config.secretKey = c.secretKey;
    }
    DownloadFile(config);
    return 0;
  } catch (const exception &e) {
    cerr << e.what() << endl;
    return 1;
  }
}
