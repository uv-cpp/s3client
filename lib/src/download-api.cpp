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

#include "s3-api.h"

#include <algorithm>
#include <fstream>
#include <future>
#include <thread>
using namespace std;

namespace sss {
using namespace api;

namespace {
// Log number or download retries;
atomic<int> retriesG;
} // namespace

int GetDownloadRetries() {
  return retriesG - 1;
} // if retriesG++, adds one before the last

//-----------------------------------------------------------------------------
void DoDownloadPart(S3Client &s3, const S3DataTransferConfig &cfg, int id,
                    size_t chunkSize, size_t objectSize, size_t offset,
                    size_t sz) {
  s3.GetFileObject(cfg.file, cfg.bucket, cfg.key, offset, offset, offset + sz);
}

//-----------------------------------------------------------------------------
// Extract bytes from object by specifying range in HTTP header:
// E.g. "Range: bytes=100-1000"
void DownloadPart(const S3DataTransferConfig &cfg, int id, size_t chunkSize,
                  size_t objectSize) {
  const auto endpoint = cfg.endpoints[RandomIndex(0, cfg.endpoints.size() - 1)];
  const size_t offset = id * chunkSize;
  const size_t sz = min(chunkSize, objectSize - offset);
  S3Client s3(cfg.accessKey, cfg.secretKey, endpoint);
  try {
    DoDownloadPart(s3, cfg, id, chunkSize, objectSize, offset, sz);
  } catch (const exception &e) {
    if (retriesG++ > cfg.maxRetries)
      throw e;
    else
      DoDownloadPart(s3, cfg, id, chunkSize, objectSize, offset, sz);
  }
}

//-----------------------------------------------------------------------------
void DownloadParts(const S3DataTransferConfig &args, size_t chunkSize,
                   int firstPart, int lastPart, size_t objectSize) {
  cout << firstPart << " " << lastPart << endl;
  for (int i = firstPart; i != lastPart; ++i) {
    DownloadPart(args, i, chunkSize, objectSize);
  }
}

//-----------------------------------------------------------------------------
void DownloadFile(S3DataTransferConfig config) {
  retriesG = 0;
  vector<future<void>> dloads(config.jobs);
  if (config.endpoints.empty()) {
    throw std::logic_error("No endpoint specified");
  }
  S3Client s3(config.accessKey, config.secretKey, config.endpoints[0]);
  // retrieve file size from remote object
  const bool caseInsensitive = false;
  const size_t fileSize = s3.GetObjectSize(config.bucket, config.key);
  const size_t numParts = config.jobs * config.chunksPerJob;
  const size_t chunkSize = (fileSize + numParts - 1) / numParts;
  const size_t partsPerJob = (numParts + config.jobs - 1) / config.jobs;
  cout << "PPJ: " << partsPerJob << endl;
  // create output file
  std::ofstream ofs(config.file, std::ios::binary | std::ios::out);
  ofs.seekp(fileSize);
  ofs.write("", 1);
  ofs.close();
  // initiate request
  for (size_t i = 0; i != config.jobs; ++i) {
    const size_t parts = min(partsPerJob, numParts - partsPerJob * i);
    dloads[i] = async(launch::async, DownloadParts, config, chunkSize,
                      i * partsPerJob, i * partsPerJob + parts, fileSize);
  }
  for (auto &i : dloads) {
    i.wait();
  }
}
} // namespace sss
