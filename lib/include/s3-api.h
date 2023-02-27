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
 *ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 *LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 *CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 *SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 *INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 *CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 *ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *POSSIBILITY OF SUCH DAMAGE.
 ******************************************************************************/
#pragma once

#include "s3-client.h"

namespace sss {

namespace api {
struct BucketInfo {};
struct ObjectInfo {};
struct PartInfo {};

// https://docs.aws.amazon.com/AmazonS3/latest/API/API_Operations_Amazon_Simple_Storage_Service.html
class S3Client {
public:
  S3Client(const std::string &access, const std::string &secret,
           const std::string &endpoint, const std::string &signingEndpoint = "")
      : access_(access), secret_(secret), endpoint_(endpoint),
        signingEndpoint_(signingEndpoint) {}
  S3Client() = delete;
  S3Client(const S3Client &) = delete;
  S3Client(S3Client &&other)
      : access_(other.access_), secret_(other.secret_),
        endpoint_(other.endpoint_), signingEndpoint_(other.signingEndpoint_),
        webClient_(std::move(other.webClient_)) {}

public:
  void AbortMultipartUpload(const UploadId &);
  // [x]
  ETag CompleteMultiplartUpload(const UploadId &uid, const std::string &bucket,
                                const std::string &key,
                                const std::vector<ETag> &etags);
  // [x]
  void CreateBucket(const std::string &bucket, const Headers & = {{}});
  // [x]
  UploadId CreateMultipartUpload(const std::string &bucket,
                                 const std::string &key,
                                 const MetaDataMap &metaData = {});
  // [x]
  void DeleteBucket(const std::string &bucket);
  bool DeleteObjects(const std::string &bucket,
                     const std::vector<std::string> &objects);
  ByteArray GetObject(const std::string &bucket, const std::string &key,
                      size_t begin = 0, size_t end = 0);
  bool GetObject(const std::string &bucket, const std::string &key,
                 ByteArray &buffer, size_t offset, size_t begin = 0,
                 size_t end = 0);
  MetaDataMap GetObjectAttributes(const std::string &bucket,
                                  const std::string &key);
  Headers HeadBucket(const std::string &bucket);
  Headers HeadObject(const std::string &bucket, const std::string &key);
  std::vector<BucketInfo> ListBuckets();
  std::vector<ObjectInfo> ListObjects(const std::string &bucket);
  std::vector<ObjectInfo> ListObjectsV2(const std::string &bucket,
                                        const std::string &prefix,
                                        bool fetchOwner, size_t maxKeys);
  std::vector<PartInfo> ListParts(const std::string &bucket,
                                  const std::string &key, const UploadId &uid,
                                  int max_parts);
  ETag PutObject(const std::string &bucket, const std::string &key,
                 const ByteArray &buffer, size_t size, size_t offset = 0);
  // [x]
  ETag UploadPart(const std::string &bucket, const std::string &key,
                  const UploadId &uid, int partNum, const char *data,
                  size_t size, int maxRetries);
  const std::string &Access() const { return access_; }
  const std::string &Secret() const { return secret_; }
  const std::string &Endpoint() const { return endpoint_; }
  const std::string &SigningEndpoint() const { return signingEndpoint_; }
  WebClient &GetWebClient() { return webClient_; }

private:
  sss::WebClient webClient_;
  std::string access_;
  std::string secret_;
  std::string endpoint_;
  std::string signingEndpoint_;
};
} // namespace api
} // namespace sss
