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
ETag DoUploadPart(S3Api &s3, const string &file, size_t offset, size_t size,
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
ETag DoUploadPart(S3Api &s3, const char *data, size_t offset, size_t size,
                  const string &bucket, const string &key, UploadId uid,
                  int part, int maxRetries) {

  try {
    return s3.UploadPart(bucket, key, uid, part, data + offset, size,
                         maxRetries);
  } catch (const exception &e) {
    if (retriesG++ < maxRetries) {
      return DoUploadPart(s3, data, offset, size, bucket, key, uid, part,
                          maxRetries);
    } else {
      throw e;
    }
  }
}
//-----------------------------------------------------------------------------
vector<string> UploadParts(const S3DataTransferConfig &cfg,
                           const string &uploadId, size_t chunkSize,
                           int firstPart, int lastPart, size_t fileSize,
                           int jobId) {

  S3Api s3(cfg.accessKey, cfg.secretKey,
           cfg.endpoints[RandomIndex(0, cfg.endpoints.size() - 1)]);

  vector<string> etags;
  size_t offset = jobId * chunkSize;
  chunkSize = min(chunkSize, fileSize - offset);
  const int numParts = lastPart - firstPart;
  const size_t partSize = (chunkSize + numParts - 1) / numParts;
  for (int i = 0; i != numParts; ++i) {
    const size_t size = min(partSize, chunkSize - i * partSize);
    etags.push_back(DoUploadPart(s3, cfg.data ? cfg.data : cfg.file, offset,
                                 size, cfg.bucket, cfg.key, uploadId,
                                 firstPart + i, cfg.maxRetries));
    offset += size;
  }
  return etags;
}

//-----------------------------------------------------------------------------
string UploadFile(const S3DataTransferConfig &cfg, const MetaDataMap &metaData,
                  bool sync) {
  retriesG = 0;
  FILE *inputFile = fopen(cfg.file.c_str(), "rb");
  if (!inputFile) {
    throw runtime_error(string("cannot open file ") + cfg.file);
  }
  fclose(inputFile);
  // retrieve file size
  const size_t fileSize = sss::FileSize(cfg.file);
  if (cfg.endpoints.empty())
    throw std::logic_error("Missing endpoint information");
  const string endpoint =
      cfg.endpoints[RandomIndex(0, cfg.endpoints.size() - 1)];
  S3Api s3(cfg.accessKey, cfg.secretKey, endpoint);
  // begin uplaod request -> get upload id
  const auto uploadId =
      s3.CreateMultipartUpload(cfg.bucket, cfg.key, 0, metaData);

  // per-job part size
  const size_t perJobSize = (fileSize + cfg.jobs - 1) / cfg.jobs;
  // send parts in parallel and store ETags
  vector<future<vector<string>>> etags(cfg.jobs);
  for (int i = 0; i != cfg.jobs; ++i) {
    etags[i] = async(sync ? launch::deferred : launch::async, UploadParts, cfg,
                     uploadId, perJobSize, i * cfg.partsPerJob,
                     i * cfg.partsPerJob + cfg.partsPerJob, fileSize, i);
  }
  vector<ETag> vetags;
  for (auto &f : etags) {
    const auto &w = f.get();
    for (const auto &i : w) {
      vetags.push_back(i);
    }
  }
  return s3.CompleteMultipartUpload(uploadId, cfg.bucket, cfg.key, vetags);
}

//-----------------------------------------------------------------------------
string UploadData(const S3DataTransferConfig &cfg, const MetaDataMap &metaData,
                  bool sync) {
  retriesG = 0;
  if (!cfg.data) {
    throw logic_error("NULL data buffer");
  }
  if (cfg.endpoints.empty())
    throw std::logic_error("Missing endpoint information");
  const string endpoint =
      cfg.endpoints[RandomIndex(0, cfg.endpoints.size() - 1)];
  S3Api s3(cfg.accessKey, cfg.secretKey, endpoint);
  // begin uplaod request -> get upload id
  const auto uploadId =
      s3.CreateMultipartUpload(cfg.bucket, cfg.key, 0, metaData);

  // per-job part size
  const size_t perJobSize = (cfg.size + cfg.jobs - 1) / cfg.jobs;
  // send parts in parallel and store ETags
  vector<future<vector<string>>> etags(cfg.jobs);
  for (int i = 0; i != cfg.jobs; ++i) {
    etags[i] = async(sync ? launch::deferred : launch::async, UploadParts, cfg,
                     uploadId, perJobSize, i * cfg.partsPerJob,
                     i * cfg.partsPerJob + cfg.partsPerJob, cfg.size, i);
  }
  vector<ETag> vetags;
  for (auto &f : etags) {
    const auto &w = f.get();
    for (const auto &i : w) {
      vetags.push_back(i);
    }
  }
  return s3.CompleteMultipartUpload(uploadId, cfg.bucket, cfg.key, vetags);
}

//-----------------------------------------------------------------------------
string Upload(const S3DataTransferConfig &cfg, const MetaDataMap &metaData,
              bool sync) {

  if (cfg.data) {
    if (!cfg.size) {
      throw logic_error("Zero size for upload data buffer");
    }
    return UploadData(cfg, metaData, sync);
  } else {
    if (cfg.file.empty()) {
      throw logic_error("Empty file name");
    }
    return UploadFile(cfg, metaData, sync);
  }
}
} // namespace sss
