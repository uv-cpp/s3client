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
#include "s3-api.h"
#include "utility.h"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <numeric>
#include <string>
#include <vector>
using namespace std;
using namespace sss;
using namespace api;

int main(int argc, char **argv) {
  const Params cfg = ParseCmdLine(argc, argv);
  TestS3Access(cfg);
  const string TEST_PREFIX = "Parallel file transfer";
  const size_t SIZE = 38000007;
  // Check the default configuration for the minimum part size.
  // Om AWS the minimum size is 5MiB.
  // An "EntityTooSmall" error is returned when the part size is too small.
  const size_t CHUNKS_PER_JOB = 2;
  const size_t NUM_JOBS = 3;
  vector<char> data(SIZE);
  for (size_t i = 0; i != data.size(); ++i) {
    data[i] = char(i % 128);
  }
  const string prefix = "sss-api-test-par";
  const string bucket = prefix + ToLower(Timestamp());
  const string key = prefix + "obj-" + ToLower(Timestamp());

  TempFile tmp = OpenTempFile("wb", prefix);
  FILE *file = tmp.pFile;
  if (!file) {
    cerr << "Cannot open file " << tmp.path << " for writing" << endl;
    exit(EXIT_FAILURE);
  }
  if (fwrite(data.data(), SIZE, 1, file) != 1) {
    fclose(file);
    cerr << "Cannot write to file " << tmp.path << endl;
    exit(EXIT_FAILURE);
  }
  fclose(file);
  try {
    S3Client s3(cfg.access, cfg.secret, cfg.url);
    // create bucket
    s3.CreateBucket(bucket);
  } catch (...) {
    cerr << "Error creating bucket " << bucket << endl;
    exit(EXIT_FAILURE);
  }
  string action = "Parallel file upload";
  ////
  try {
    S3DataTransferConfig c = {.accessKey = cfg.access,
                              .secretKey = cfg.secret,
                              .bucket = bucket,
                              .key = key,
                              .file = tmp.path,
                              .endpoints = {cfg.url},
                              .jobs = NUM_JOBS,
                              .partsPerJob = CHUNKS_PER_JOB};
    UploadFile(c);
    S3Client s3(cfg.access, cfg.secret, cfg.url);
    const CharArray uploaded = s3.GetObject(bucket, key);
    if (uploaded != data)
      throw logic_error("Data verification failed");
    TestOutput(action, true, TEST_PREFIX);
  } catch (const exception &e) {
    TestOutput(action, false, TEST_PREFIX, e.what());
  }
  // reset file
  {
    ofstream os(tmp.path, ios::binary);
    os << 0;
  }
  ////
  action = "Parallel file download";
  try {
    S3DataTransferConfig c = {.accessKey = cfg.access,
                              .secretKey = cfg.secret,
                              .bucket = bucket,
                              .key = key,
                              .file = tmp.path,
                              .endpoints = {cfg.url},
                              .jobs = NUM_JOBS,
                              .partsPerJob = CHUNKS_PER_JOB};
    DownloadFile(c);
    FILE *fi = fopen(tmp.path.c_str(), "rb");
    vector<char> input(SIZE);
    if (fread(input.data(), SIZE, 1, fi) != 1) {
      throw std::runtime_error("Cannot open file for reading");
    }
    //
    if (input == data) {
      TestOutput(action, true, TEST_PREFIX);
    } else {
      throw logic_error("Data verification failed");
    }
  } catch (const exception &e) {
    TestOutput(action, false, TEST_PREFIX, e.what());
  }
  ///
  if (!filesystem::remove(tmp.path)) {
    cerr << "Error removing file " << tmp.path << endl;
    exit(EXIT_FAILURE);
  }
  ///
  try {
    S3Client s3(cfg.access, cfg.secret, cfg.url);
    // delete object
    s3.DeleteObject(bucket, key);
  } catch (...) {
    cerr << "Error deleting object " << bucket << "/" << key << endl;
    exit(EXIT_FAILURE);
  }
  // delete bucket
  try {
    S3Client s3(cfg.access, cfg.secret, cfg.url);
    s3.DeleteBucket(bucket);
  } catch (...) {
    cerr << "Error deleting bucket " << bucket;

    exit(EXIT_FAILURE);
  }
  return 0;
}
