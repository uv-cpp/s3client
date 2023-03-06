/*******************************************************************************
 * BSD 3-Clause License
 *
 * Copyright (c) 2020-2022, Ugo Varetto
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
#include <iostream>
#include <stdexcept>
#include <string>

using namespace std;
using namespace sss;
using namespace api;

int main(int argc, char **argv) {
  const Params cfg = ParseCmdLine(argc, argv);
  TestS3Access(cfg);
  const string prefix = "sss-api-test-";
  const string bucketName = prefix + ToLower(Timestamp());
  {
    S3Client s3(cfg.access, cfg.secret, cfg.url);
    s3.CreateBucket(bucketName);
  }
  string objName = prefix + "obj-" + ToLower(Timestamp());
  ByteArray data = ByteArray(1024);
  ////
  string action = "PutObject";
  try {
    S3Client s3(cfg.access, cfg.secret, cfg.url);
    s3.PutObject(bucketName, objName, data);
    TestOutput(action, true);
  } catch (const exception &e) {
    TestOutput(action, false, e.what());
  }
  ////
  action = "HeadObject";
  try {
    S3Client s3(cfg.access, cfg.secret, cfg.url);
    s3.HeadObject(bucketName, objName);
    TestOutput(action, true);
  } catch (const exception &e) {
    TestOutput(action, false, e.what());
  }
  ////
  action = "ListObjectsV2";
  try {
    S3Client s3(cfg.access, cfg.secret, cfg.url);
    auto objects = s3.ListObjectsV2(bucketName);
    if (objects.empty()) {
      throw runtime_error("Empty bucket list");
    }
    TestOutput(action, true);
  } catch (const exception &e) {
    TestOutput(action, false, e.what());
  }
  ////
  action = "GetObject";
  try {
    S3Client s3(cfg.access, cfg.secret, cfg.url);
    const auto &obj = s3.GetObject(bucketName, objName);
    if (obj.size() != data.size()) {
      throw logic_error("Data size mismatch");
    }
    TestOutput(action, true);
  } catch (const exception &e) {
    TestOutput(action, false, e.what());
  }
  ////
  action = "DeleteObject";
  try {
    S3Client s3(cfg.access, cfg.secret, cfg.url);
    s3.DeleteObject(bucketName, objName);
    TestOutput(action, true);
  } catch (const exception &e) {
    TestOutput(action, false, e.what());
  }
}