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
#include <algorithm>
#include <chrono>
#include <iostream>
#include <stdexcept>
#include <string>

using namespace std;
using namespace sss;
using namespace api;
/**
 * \file bucket-test.cpp
 * \brief bucket tests
 */
/**
 * \brief Bucket tests
 */
int main(int argc, char **argv) {
  const Params cfg = ParseCmdLine(argc, argv);
  TestS3Access(cfg);
  const string TEST_PREFIX = "Bucket";
  const string prefix = "sss-api-test-bucket";
  string bucketName;
  /// [CreateBucket]
  string action = "CreateBucket";
  try {
    S3Api s3(cfg.access, cfg.secret, cfg.url);
    bucketName = prefix + ToLower(Timestamp());
    s3.CreateBucket(bucketName);
    TestOutput(action, true, TEST_PREFIX);
  } catch (const exception &e) {
    TestOutput(action, false, TEST_PREFIX, e.what());
  }
  /// [CreateBucket]
  action = "HeadBucket";
  try {
    S3Api s3(cfg.access, cfg.secret, cfg.url);
    s3.HeadBucket(bucketName);
    TestOutput(action, true, TEST_PREFIX);
  } catch (const exception &e) {
    TestOutput(action, false, TEST_PREFIX, e.what());
  }
  /// [GetBucketAcl]
  action = "GetBucketAcl";
  try {
    S3Api s3(cfg.access, cfg.secret, cfg.url);
    auto accessPolicy = s3.GetBucketAcl(bucketName);
    if (accessPolicy.grants.empty()) {
      throw logic_error("Empty Grants list");
    }
    if (accessPolicy.grants.front().permission != "FULL_CONTROL") {
      throw logic_error("permission not equal to 'FULL_CONTROL'");
    }
    TestOutput(action, true, TEST_PREFIX);
  } catch (const exception &e) {
    TestOutput(action, false, TEST_PREFIX, e.what());
  }
  /// [GetBucketAcl]
  action = "Put/GetBucketTagging";
  try {
    S3Api s3(cfg.access, cfg.secret, cfg.url);
    TagMap tags = {{"tag1", "value1"}, {"tag2", "value2"}};
    s3.PutBucketTagging(bucketName, tags);
    auto t = s3.GetBucketTagging(bucketName);
    if(t != tags) {
      if(t.empty()) {
        throw logic_error("No tag set");
      }
      throw logic_error("Tags do not match");
    }
    TestOutput(action, true, TEST_PREFIX);
  } catch (const exception &e) {
    TestOutput(action, false, e.what());
  }
  /// [ListBuckets]
  action = "ListBuckets";
  try {
    S3Api s3(cfg.access, cfg.secret, cfg.url);
    auto buckets = s3.ListBuckets();
    if (buckets.empty()) {
      throw logic_error("Empty bucket list");
    }
    if (end(buckets) ==
        find_if(begin(buckets), end(buckets),
                [&](const auto &b) { return b.name == bucketName; })) {
      throw logic_error("Missing bucket");
    }
    TestOutput(action, true, TEST_PREFIX);
  } catch (const exception &e) {
    TestOutput(action, false, TEST_PREFIX, e.what());
  }
  /// [ListBuckets]

  /// [DeleteBucket]
  action = "DeleteBucket";
  try {
    S3Api s3(cfg.access, cfg.secret, cfg.url);
    s3.DeleteBucket(bucketName);
    TestOutput(action, true, TEST_PREFIX);
  } catch (const exception &e) {
    TestOutput(action, false, TEST_PREFIX, e.what());
  }
  /// [DeleteBucket]
}
