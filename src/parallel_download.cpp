
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

#include <aws_sign.h>

#include <filesystem>
#include <fstream>
#include <future>
#include <iostream>
#include <numeric>
#include <regex>
#include <set>
#include <vector>

#include "common.h"
#include "lyra/lyra.hpp"
#include "response_parser.h"
#include "webclient.h"

using namespace std;
using namespace filesystem;

using namespace sss;

//------------------------------------------------------------------------------
struct Args {
  bool showHelp = false;
  string s3AccessKey;
  string s3SecretKey;
  string endpoint;
  string bucket;
  string key;
  string file;
  int jobs = 1;
};

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
}

using Headers = Map;
using Parameters = Map;

size_t ObjectSize(const Args &args, const string &path) {
  auto signedHeaders =
      SignHeaders(args.s3AccessKey, args.s3SecretKey, args.endpoint, "HEAD",
                  args.bucket, args.key);
  Headers headers(begin(signedHeaders), end(signedHeaders));
  WebClient req(args.endpoint, path, "HEAD", {}, headers);
  req.Send();
  const vector<uint8_t> h = req.GetResponseHeader();
  const string hs(begin(h), end(h));
  const string cl = HTTPHeader(hs, "[Cc]ontent-[Ll]ength");
  char *ns;
  return size_t(strtoull(cl.c_str(), &ns, 10));
}

// Extract bytes from object by specifying range in HTTP header:
// E.g. "Range: bytes=100-1000"
int DownloadPart(const Args &args, const string &path, int id, size_t chunkSize,
                 size_t lastChunkSize) {
  auto signedHeaders = SignHeaders(args.s3AccessKey, args.s3SecretKey,
                                   args.endpoint, "GET", args.bucket, args.key);
  Headers headers(begin(signedHeaders), end(signedHeaders));
  const size_t sz = id < args.jobs - 1 ? chunkSize : lastChunkSize;
  const string range = "bytes=" + to_string(id * chunkSize) + "-" +
                       to_string(id * chunkSize + sz - 1);
  headers.insert({"Range", range});
  WebClient req(args.endpoint, path, "GET", {}, headers);
  FILE *out = fopen(args.file.c_str(), "wb");
  fseek(out, id * chunkSize, SEEK_SET);
  req.SetWriteFunction(NULL, out);
  req.Send();
  return req.StatusCode();
}

//------------------------------------------------------------------------------
int main(int argc, char const *argv[]) {
  try {
    Args args;
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
        lyra::opt(args.jobs, "parallel jobs")["-j"]["--jobs"](
            "Number inputFile parallel jobs")
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
    Validate(args);
    string path = "/" + args.bucket + "/" + args.key;
    vector<future<int>> status(args.jobs);
    // retrieve file size from remote object
    const size_t fileSize = ObjectSize(args, path);
    // compute chunk size
    const size_t chunkSize = (fileSize + args.jobs - 1) / args.jobs;
    // compute last chunk size
    const size_t lastChunkSize = fileSize - chunkSize * (args.jobs - 1);
    cout << chunkSize << ' ' << lastChunkSize << ' ' << fileSize << endl;
    // create output file
    std::ofstream ofs(args.file, std::ios::binary | std::ios::out);
    ofs.seekp(fileSize);
    ofs.write("", 1);
    ofs.close();
    // initiate request
    for (size_t i = 0; i != args.jobs; ++i) {
      status[i] = async(launch::async, DownloadPart, args, path, i, chunkSize,
                        lastChunkSize);
    }
    for (auto &i : status) {
      if (i.get() > 300)
        throw runtime_error("Error downloading file");
    }
    return 0;
  } catch (const exception &e) {
    cerr << e.what() << endl;
    return 1;
  }
}
