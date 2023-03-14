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

// Download objects

#include "aws_sign.h"
#include "common.h"
#include "response_parser.h"
#include "s3-client.h"

#include <algorithm>
#include <fstream>
#include <future>
#include <thread>
using namespace std;

namespace sss {
//-----------------------------------------------------------------------------
size_t ObjectSize(const string &s3AccessKey, const string &s3SecretKey,
                  const string &endpoint, const string &bucket,
                  const string &key, const string &signUrl = "") {
  S3ClientConfig args;
  args.accessKey = s3AccessKey;
  args.secretKey = s3SecretKey;
  args.endpoint = endpoint;
  args.signUrl = signUrl;
  args.bucket = bucket;
  args.key = key;
  auto req = SendS3Request(args);
  const vector<char> h = req.GetResponseHeader();
  const string hs(begin(h), end(h));
  const string cl = sss::HTTPHeader(hs, "Content-Length");
  char *ns;
  return size_t(strtoull(cl.c_str(), &ns, 10));
}

namespace {
// Log number or download retries;
atomic<int> retriesG;
} // namespace

int GetDownloadRetries() { return retriesG; }
// Extract bytes from object by specifying range in HTTP header:
// E.g. "Range: bytes=100-1000"
int DownloadPart(const S3FileTransferConfig &args, const string &path, int id,
                 size_t chunkSize, size_t objectSize) {
  const auto endpoint = args.endpoints[RandomIndex(0, args.endpoints.size())];
  const auto signedHeaders = SignHeaders(
      args.accessKey, args.secretKey, endpoint, "GET", args.bucket, args.key);
  Headers headers(begin(signedHeaders), end(signedHeaders));
  const size_t sz = min(chunkSize, objectSize - id * chunkSize);
  const bool lastChunk = chunkSize * id + sz == objectSize;
  const string range = "bytes=" + to_string(id * chunkSize) + "-" +
                       to_string(id * chunkSize + sz - (lastChunk ? 0 : 1));
  headers.insert({"Range", range});
  WebClient req(endpoint, path, "GET", {}, headers);
  FILE *out = fopen(args.file.c_str(), "wb");
  fseek(out, id * chunkSize, SEEK_SET);
  req.SetWriteFunction(NULL, out);
  auto retries = args.maxRetries;
  while (!req.Send() && retries) {
    --retries;
    retriesG++;
  }
  return req.StatusCode();
}

//-----------------------------------------------------------------------------
int DownloadParts(const S3FileTransferConfig &args, const string &path,
                  size_t chunkSize, int firstPart, int lastPart,
                  size_t objectSize) {
  vector<int> status;
  for (int i = firstPart; i != lastPart; ++i) {
    status.push_back(DownloadPart(args, path, i, chunkSize, objectSize));
  }
  return *max_element(begin(status), end(status));
}

//-----------------------------------------------------------------------------
void DownloadFile(S3FileTransferConfig config) {

  retriesG = 0;
  vector<future<int>> status(config.jobs);
  if (config.endpoints.empty()) {
    throw std::logic_error("No endpoint specified");
  }
  // retrieve file size from remote object
  const size_t fileSize =
      ObjectSize(config.accessKey, config.secretKey, config.endpoints.front(),
                 config.bucket, config.key, config.signUrl);

  const size_t numParts = config.jobs * config.chunksPerJob;
  const size_t chunkSize = (fileSize + numParts - 1) / numParts;
  const size_t partsPerJob = (numParts + config.jobs - 1) / config.jobs;
  // create output file
  std::ofstream ofs(config.file, std::ios::binary | std::ios::out);
  ofs.seekp(fileSize);
  ofs.write("", 1);
  ofs.close();
  // initiate request
  int chunksPerJob = 2;
  string path = "/" + config.bucket + "/" + config.key;
  for (size_t i = 0; i != config.jobs; ++i) {
    const size_t parts = min(partsPerJob, numParts - partsPerJob * i);
    status[i] = async(launch::async, DownloadParts, config, path, chunkSize,
                      i * partsPerJob, i * partsPerJob + parts, fileSize);
  }
  for (auto &i : status) {
    if (i.get() > 300)
      throw runtime_error("Error downloading file");
  }
}
} // namespace sss
