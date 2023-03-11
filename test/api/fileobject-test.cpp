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
 * this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
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
#include <algorithm>
#include <chrono>
#include <filesystem>
#include <iostream>
#include <numeric>
#include <stdexcept>
#include <string>

using namespace std;
using namespace sss;
using namespace api;

int main(int argc, char **argv) {
  const Params cfg = ParseCmdLine(argc, argv);
  TestS3Access(cfg);
  const string TEST_PREFIX = "File transfer";
  const string prefix = "sss-api-test-file";
  const string bucketName = prefix + ToLower(Timestamp());
  {
    S3Client s3(cfg.access, cfg.secret, cfg.url);
    s3.CreateBucket(bucketName);
  }
  string objName = prefix + "obj-" + ToLower(Timestamp());
  ByteArray data = ByteArray(1024);
  iota(begin(data), end(data), 0);
  string tmpFile = tmpnam(nullptr);
  FILE *f = fopen(tmpFile.c_str(), "wb");
  if (!f) {
    cerr << "Cannot open file " << tmpFile << " for writing" << endl;
    exit(EXIT_FAILURE);
  }
  if (fwrite(data.data(), data.size(), 1, f) != 1) {
    cerr << "Error writing to file " << tmpFile << endl;
    fclose(f);
    filesystem::remove(tmpFile);
    exit(EXIT_FAILURE);
  }
  fclose(f);
  ////
  string action = "PutFileObject";
  try {
    S3Client s3(cfg.access, cfg.secret, cfg.url);
    const ETag etag = s3.PutFileObject(tmpFile, bucketName, objName);
    if (etag.empty())
      throw runtime_error("Empty etag");
    // S3Client s32(cfg.access, cfg.secret, cfg.url);
    const ByteArray &obj = s3.GetObject(bucketName, objName);
    if (obj != data)
      throw logic_error("Data mismatch");
    TestOutput(action, true, TEST_PREFIX);
  } catch (const exception &e) {
    TestOutput(action, false, TEST_PREFIX, e.what());
  }
  filesystem::remove(tmpFile);
  ////
  tmpFile = tmpnam(nullptr);
  action = "GetFileObject";
  try {
    S3Client s3(cfg.access, cfg.secret, cfg.url);
    s3.GetFileObject(tmpFile, bucketName, objName);
    ByteArray tmp(data.size());
    FILE *f = fopen(tmpFile.c_str(), "rb");
    fread(tmp.data(), tmp.size(), 1, f);
    fclose(f);
    filesystem::remove(tmpFile);
    if (data != tmp)
      throw logic_error("Data mismatch");
    TestOutput(action, true, TEST_PREFIX);
  } catch (const exception &e) {
    TestOutput(action, false, TEST_PREFIX, e.what());
  }
  ////
  action = "DeleteObject";
  try {
    S3Client s3(cfg.access, cfg.secret, cfg.url);
    s3.DeleteObject(bucketName, objName);
    TestOutput(action, true, TEST_PREFIX);
  } catch (const exception &e) {
    TestOutput(action, false, TEST_PREFIX, e.what());
  }
}
