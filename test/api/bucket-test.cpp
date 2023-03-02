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
 *this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *notice, this list of conditions and the following disclaimer in the
 *documentation and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 *ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 *LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 *CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 *SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 *INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 *CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 *ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *POSSIBILITY OF SUCH DAMAGE.
 ******************************************************************************/ \
#include <iostream>
#include "s3-api.h"
#include "utility.h"
#include <algorithm>
#include <chrono>
#include <stdexcept>
#include <string>

using namespace std;
using namespace sss;
using namespace api;

int main(int argc, char **argv) {
  const Params cfg = ParseCmdLine(argc, argv);
  TestS3Access(cfg);
  const string prefix = "sss-api-test-";
  string bucketName;
  ////
  string name = "CreateBucket";
  try {
    S3Client s3(cfg.access, cfg.secret, cfg.url);
    bucketName = prefix + TimeStamp();
    s3.CreateBucket(bucketName);
    TestOutput(name, true);
  } catch (const exception &e) {
    TestOutput(name, false, e.what());
  }
  ////
  name = "HeadBucket";
  try {
    S3Client s3(cfg.access, cfg.secret, cfg.url);
    bucketName = prefix + TimeStamp();
    s3.HeadBucket(bucketName);
    TestOutput(name, true);
  } catch (const exception &e) {
    TestOutput(name, false, e.what());
  }
  ////
  name = "ListBuckets";
  try {
    S3Client s3(cfg.access, cfg.secret, cfg.url);
    auto buckets = s3.ListBuckets();
    bool found = false;
    for (const auto &b : buckets) {
      cout << b.name << endl;
      if (b.name == bucketName) {
        found = true;
      }
    }
    if (!found) {
      throw logic_error("Bucket " + bucketName + " not found");
    }
    TestOutput(name, true);
  } catch (const exception &e) {
    TestOutput(name, false, e.what());
  }
  ////
  name = "DeleteBucket";
  try {
    S3Client s3(cfg.access, cfg.secret, cfg.url);
    s3.DeleteBucket(bucketName);
    TestOutput(name, true);
  } catch (const exception &e) {
    TestOutput(name, false, e.what());
  }
}
