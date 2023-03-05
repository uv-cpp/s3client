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

#include <fstream>
#include <iostream>
#include <string>

#include "lyra/lyra.hpp"
#include "s3-client.h"
#include "url_utility.h"
#include "utility.h"

using namespace std;
using namespace sss;
//------------------------------------------------------------------------------
int main(int argc, char const *argv[]) {
  try {
    S3FileTransferConfig config;
    bool showHelp = false;
    string credentialsFile;
    string awsProfile;
    string endpoint;
    string endpointsFile;
    string metaData;
    auto cli =
        lyra::help(showHelp).description("Upload file to S3 bucket") |
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
        lyra::opt(config.file, "file")["-f"]["--file"]("File name").optional() |
        lyra::opt(config.jobs, "parallel jobs")["-j"]["--jobs"](
            "Number of parallel upload jobs")
            .optional() |
        lyra::opt(config.chunksPerJob,
                  "chunks per job")["-n"]["--parts_per_job"](
            "Number of parts per job")
            .optional() |
        lyra::opt(credentialsFile, "credentials file")["-c"]["--credentials"](
            "Credentials file, AWS cli format")
            .optional() |
        lyra::opt(awsProfile, "AWS config profile")["-p"]["--profile"](
            "Profile in AWS config file")
            .optional() |
        lyra::opt(config.maxRetries, "Max retries")["-r"]["--retries"](
            "Max number of per-multipart part retries")
            .optional() |
        lyra::opt(metaData, "metaData")["-m"]["--meta"](
            "Metadata list formatted as headers: "
            "meta_key1:meta_value1;meta_key2:meta_value2")
            .optional();

    // Parse the program arguments:
    auto result = cli.parse({argc, argv});
    if (!result) {
      cerr << result.message() << endl;
      cerr << cli << endl;
      exit(EXIT_FAILURE);
    }
    if (showHelp) {
      cout << cli;
      exit(EXIT_SUCCESS);
    }
    if (config.file.empty())
      config.file = config.key;
    if (endpoint.empty() && endpointsFile.empty()) {
      cerr << "Specify either an endpoint URL or a file name containing a list "
              "of URLs, one per line"
           << endl;
    }
    if (!endpoint.empty()) {
      config.endpoints.push_back(endpoint);
    } else {
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

    MetaDataMap mm;
    if (!metaData.empty()) {
      for (auto i : SplitRange(metaData, ";")) {
        auto s = begin(SplitRange(i, ":"));
        if (s == end(SplitRange(i, ":"))) {
          throw std::logic_error("Missing metadata");
        }
        auto k = *s++;
        if (s == end(SplitRange(metaData, ";"))) {
          throw std::logic_error(
              "Wrong metadata format: should be 'meta key:meta value'");
        }
        auto v = *s;
        mm["x-amz-meta-" + ToLower(k)] = v;
      }
    }
    cout << UploadFile(config, mm);
    return 0;
  } catch (const exception &e) {
    cerr << e.what() << endl;
    return 1;
  }
}
