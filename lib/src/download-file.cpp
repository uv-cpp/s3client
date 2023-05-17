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

#include <fstream>
using namespace std;

namespace sss {
using namespace api;

namespace {
// Log number or download retries;
atomic<int> retriesG;
} // namespace

int GetDownloadRetries() { return retriesG; }

//-----------------------------------------------------------------------------
void DownloadPart(S3Api &s3, const string &file, const string &bucket,
                  const string &key, size_t offset, size_t partSize,
                  int maxRetries) {
  try {
    s3.GetFileObject(file, bucket, key, offset, offset, offset + partSize - 1);
  } catch (const exception &e) {
    if (retriesG++ > maxRetries)
      throw e;
    else
      DownloadPart(s3, file, bucket, key, offset, partSize, maxRetries);
  }
}

//-----------------------------------------------------------------------------
void DownloadPart(S3Api &s3, char *data, const string &bucket,
                  const string &key, size_t offset, size_t partSize,
                  int maxRetries) {
  try {
    s3.GetObject(bucket, key, data, offset, offset, offset + partSize - 1);
  } catch (const exception &e) {
    if (retriesG++ > maxRetries)
      throw e;
    else
      DownloadPart(s3, data, bucket, key, offset, partSize, maxRetries);
  }
}
//-----------------------------------------------------------------------------
void DownloadParts(const S3DataTransferConfig &cfg, size_t chunkSize,
                   int firstPart, int lastPart, size_t objectSize, int jobId) {
  size_t offset = jobId * chunkSize;
  chunkSize = min(chunkSize, objectSize - offset);
  const int numParts = lastPart - firstPart;
  const size_t partSize = (chunkSize + numParts - 1) / numParts;
  const auto endpoint = cfg.endpoints[RandomIndex(0, cfg.endpoints.size() - 1)];
  S3Api s3(cfg.accessKey, cfg.secretKey, endpoint);
  for (int i = 0; i != numParts; ++i) {
    const size_t size = min(partSize, chunkSize - i * partSize);
    DownloadPart(s3, cfg.data ? cfg.data : cfg.file, cfg.bucket, cfg.key,
                 offset, size, cfg.maxRetries);
    offset += size;
  }
}

//-----------------------------------------------------------------------------
void DownloadFile(const S3DataTransferConfig &cfg, bool sync) {
  retriesG = 0;
  if (cfg.endpoints.empty()) {
    throw std::logic_error("No endpoint specified");
  }
  S3Api s3(cfg.accessKey, cfg.secretKey, cfg.endpoints[0]);
  const size_t fileSize = s3.GetObjectSize(cfg.bucket, cfg.key);
  // create output file
  std::ofstream ofs(cfg.file, std::ios::binary | std::ios::out);
  ofs.seekp(fileSize - 1);
  ofs.write("", 1);
  ofs.close();
  // initiate request
  const size_t perJobSize = (fileSize + cfg.jobs - 1) / cfg.jobs;
  // send parts in parallel and store ETags
  vector<future<void>> dloads(cfg.jobs);
  for (int i = 0; i != cfg.jobs; ++i) {
    dloads[i] = async(sync ? launch::deferred : launch::async, DownloadParts,
                      cfg, perJobSize, i * cfg.partsPerJob,
                      i * cfg.partsPerJob + cfg.partsPerJob, fileSize, i);
  }
  for (auto &i : dloads) {
    i.wait();
  }
}

//-----------------------------------------------------------------------------
void DownloadData(const S3DataTransferConfig &cfg, bool sync) {
  retriesG = 0;
  if (!cfg.data) {
    throw logic_error("NULL data buffer");
  }
  if (cfg.endpoints.empty()) {
    throw std::logic_error("No endpoint specified");
  }
  S3Api s3(cfg.accessKey, cfg.secretKey, cfg.endpoints[0]);
  // initiate request
  const size_t perJobSize = (cfg.size + cfg.jobs - 1) / cfg.jobs;
  // send parts in parallel and store ETags
  vector<future<void>> dloads(cfg.jobs);
  for (int i = 0; i != cfg.jobs; ++i) {
    dloads[i] = async(sync ? launch::deferred : launch::async, DownloadParts,
                      cfg, perJobSize, i * cfg.partsPerJob,
                      i * cfg.partsPerJob + cfg.partsPerJob, cfg.size, i);
  }
  for (auto &i : dloads) {
    i.wait();
  }
}

//-----------------------------------------------------------------------------
void Download(const S3DataTransferConfig &cfg, bool sync) {
  if (!cfg.data && cfg.file.empty()) {
    throw logic_error("File name and data buffer pointer are both NULL");
  }
  if (cfg.data) {
    DownloadData(cfg, sync);
  } else {
    DownloadFile(cfg, sync);
  }
}
} // namespace sss
