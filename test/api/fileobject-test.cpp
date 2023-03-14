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
  const string prefix = "sss-api-test-file-";
  const string bucketName = prefix + ToLower(Timestamp());
  {
    S3Client s3(cfg.access, cfg.secret, cfg.url);
    s3.CreateBucket(bucketName);
  }
  string objName = prefix + "obj-" + ToLower(Timestamp());
  ByteArray data = ByteArray(1024);
  iota(begin(data), end(data), 0);
  TempFile tmp = OpenTempFile("wb", prefix);
  FILE *f = tmp.pFile;
  if (!f) {
    cerr << "Cannot open file " << tmp.path << " for writing" << endl;
    exit(EXIT_FAILURE);
  }
  if (fwrite(data.data(), data.size(), 1, f) != 1) {
    cerr << "Error writing to file " << tmp.path << endl;
    fclose(f);
    filesystem::remove(tmp.path);
    exit(EXIT_FAILURE);
  }
  fclose(f);
  ////
  string action = "PutFileObject";
  try {
    S3Client s3(cfg.access, cfg.secret, cfg.url);
    const ETag etag = s3.PutFileObject(tmp.path, bucketName, objName);
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
  filesystem::remove(tmp.path);
  ////
  tmp = OpenTempFile("wb", prefix);
  fclose(tmp.pFile);
  action = "GetFileObject";
  try {
    S3Client s3(cfg.access, cfg.secret, cfg.url);
    s3.GetFileObject(tmp.path, bucketName, objName);
    ByteArray tmpData(data.size());
    FILE *f = fopen(tmp.path.c_str(), "rb");
    if (fread(tmpData.data(), tmpData.size(), 1, f) != 1) {
      fclose(f);
      throw logic_error("Failed to read from " + tmp.path);
    }
    fclose(f);
    if (data != tmpData)
      throw logic_error("Data mismatch");
    TestOutput(action, true, TEST_PREFIX);
  } catch (const exception &e) {
    TestOutput(action, false, TEST_PREFIX, e.what());
  }
  filesystem::remove(tmp.path);
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
