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

// Upload files in parallel using S3Client

#include "s3-api.h"

#include <fstream>
#include <future>
#include <set>
#include <thread>
using namespace std;

namespace sss {
using namespace api;

namespace {
atomic<int> retriesG;
}

//-----------------------------------------------------------------------------
int GetUploadRetries() { return retriesG; }

//-----------------------------------------------------------------------------
ETag DoUploadPart(S3Client &s3, const string &file, size_t offset, size_t size,
                  const string &bucket, const string &key, UploadId uid,
                  int part, int maxRetries) {

  try {
    return s3.UploadFilePart(file, offset, size, bucket, key, uid, part);
  } catch (const exception &e) {
    if (retriesG++ < maxRetries) {
      return DoUploadPart(s3, file, offset, size, bucket, key, uid, part,
                          maxRetries);
    } else {
      throw e;
    }
  }
}

//-----------------------------------------------------------------------------
vector<string> UploadParts(const S3FileTransferConfig &cfg,
                           const string &uploadId, size_t chunkSize,
                           int firstPart, int lastPart, size_t fileSize) {

  S3Client s3(cfg.accessKey, cfg.secretKey, cfg.endpoints[0]);
  vector<string> etags;
  size_t offset = firstPart * chunkSize;
  for (int i = firstPart; i != lastPart; ++i) {
    const size_t size = min(chunkSize, fileSize - offset);
    etags.push_back(DoUploadPart(s3, cfg.file, offset, size, cfg.bucket,
                                 cfg.key, uploadId, i, cfg.maxRetries));
    offset += chunkSize;
  }
  return etags;
}

//-----------------------------------------------------------------------------
string UploadFile(const S3FileTransferConfig &config,
                  const MetaDataMap &metaData) {
  retriesG = 0;
  FILE *inputFile = fopen(config.file.c_str(), "rb");
  if (!inputFile) {
    throw runtime_error(string("cannot open file ") + config.file);
  }
  fclose(inputFile);
  // retrieve file size
  const size_t fileSize = sss::FileSize(config.file);
  string path = "/" + config.bucket + "/" + config.key;
  if (config.endpoints.empty())
    throw std::logic_error("Missing endpoint information");
  const string endpoint =
      config.endpoints[RandomIndex(0, config.endpoints.size() - 1)];
  S3Client s3(config.accessKey, config.secretKey, endpoint);
  if (config.jobs > 1) {
    S3ClientConfig args;
    // begin uplaod request -> get upload id
    const auto uploadId =
        s3.CreateMultipartUpload(config.bucket, config.key, 0, metaData);
    const size_t numParts = config.chunksPerJob * config.jobs;
    const size_t partsPerJob = (numParts + config.jobs - 1) / config.jobs;
    const size_t chunkSize = (fileSize + numParts - 1) / numParts;
    // send parts in parallel and store ETags
    vector<future<vector<string>>> etags(config.jobs);
    for (int i = 0; i != config.jobs; ++i) {
      const size_t parts = min(partsPerJob, numParts - partsPerJob * i);
      etags[i] = async(launch::async, UploadParts, config, uploadId, chunkSize,
                       i * partsPerJob, i * partsPerJob + parts, fileSize);
    }
    vector<ETag> vetags;
    for (auto &f : etags) {
      const auto &w = f.get();
      for (const auto &i : w) {
        vetags.push_back(i);
      }
    }
    return s3.CompleteMultipartUpload(config.bucket, config.key, uploadId,
                                      vetags);
  } else {
    S3Client s3(config.accessKey, config.secretKey, config.endpoints[0]);
    return s3.PutFileObject(config.file, config.bucket, config.key);
  }
}
} // namespace sss
