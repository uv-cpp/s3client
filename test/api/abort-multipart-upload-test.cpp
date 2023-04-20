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
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
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
#include "../utility.h"
#include "s3-api.h"
#include <iostream>
#include <string>
#include <vector>
using namespace std;
using namespace sss;
using namespace api;

int main(int argc, char **argv) {
  const Params cfg = ParseCmdLine(argc, argv);
  TestS3Access(cfg);
  const string TEST_PREFIX = "AbortMultipartUpload";
  const size_t SIZE = 19000000;
  // Check the default configuration for the minimum part size.
  // Om AWS the minimum size is 5MiB.
  // An "EntityTooSmall" error is returned when the part size is too small.
  const size_t NUM_CHUNKS = 3;
  const size_t CHUNK_SIZE = (SIZE + NUM_CHUNKS - 1) / NUM_CHUNKS;
  const vector<char> data(SIZE);
  const string prefix = "sss-api-test-abort";
  const string bucket = prefix + ToLower(Timestamp());
  const string key = prefix + "obj-" + ToLower(Timestamp());

  try {
    S3Client s3(cfg.access, cfg.secret, cfg.url);
    // create bucket
    s3.CreateBucket(bucket);
  } catch (...) {
    cerr << "Error creating bucket " << bucket << endl;
    exit(EXIT_FAILURE);
  }

  ///
  string action = "CreateMultipartUpload";
  UploadId uid;
  try {
    S3Client s3(cfg.access, cfg.secret, cfg.url);
    uid = s3.CreateMultipartUpload(bucket, key);
    TestOutput(action, true, TEST_PREFIX);
  } catch (const exception &e) {
    TestOutput(action, false, TEST_PREFIX, e.what());
  }
  ///
  action = "UploadPart";
  vector<ETag> etags;
  try {
    S3Client s3(cfg.access, cfg.secret, cfg.url);
    for (size_t i = 0; i != 3; ++i) {
      const size_t size = min(CHUNK_SIZE, SIZE - CHUNK_SIZE * i);
      const auto etag =
          s3.UploadPart(bucket, key, uid, i, &data[i * CHUNK_SIZE], size);
      etags.push_back(etag);
    }
    TestOutput(action, true, TEST_PREFIX);
  } catch (const exception &e) {
    TestOutput(action, false, TEST_PREFIX, e.what());
  }
  ///
  action = "AbortMultipartUpload";
  try {
    S3Client s3(cfg.access, cfg.secret, cfg.url);
    s3.AbortMultipartUpload(bucket, key, uid);
    TestOutput(action, true, TEST_PREFIX);
  } catch (const exception &e) {
    TestOutput(action, false, TEST_PREFIX, e.what());
  }
  try {
    S3Client s3(cfg.access, cfg.secret, cfg.url);
    // delete bucket
    s3.DeleteBucket(bucket);
  } catch (...) {
    cerr << "Error deleting bucket " << bucket << endl;
    exit(EXIT_FAILURE);
  }
  return 0;
}
