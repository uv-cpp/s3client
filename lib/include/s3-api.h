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
struct BucketInfo {
  std::string name;
  std::string creationDate; //@todo replace with std::tm
};

struct ObjectInfo {
  std::string checksumAlgo;
  std::string key;
  std::string lastModified; //@todo replace with std:tm
  ETag etag;
  size_t size;
  std::string storageClass;
  // ObjectOwner
  std::string ownerDisplayName;
  std::string ownerID;
};
//@todo
// struct PartInfo {};

// https://docs.aws.amazon.com/AmazonS3/latest/API/API_Operations_Amazon_Simple_Storage_Service.html
class S3Client {
public:
  struct ListObjectV2Config {
    std::string continuationToken;
    std::string delimiter;
    std::string encodingType;
    std::string fetchOwner;
    size_t maxKeys = 0;
    std::string prefix;
    std::string startAfter;
  };

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
  bool TestBucket(const std::string &bucket);
  bool TestObject(const std::string &bucket, const std::string &key);
  void Clear() {
    webClient_.SetPath("");
    webClient_.SetHeaders({{}});
    webClient_.SetReqParameters({{}});
    webClient_.SetPostData("");
    webClient_.SetUploadData({});
  }

public:
  void AbortMultipartUpload(const std::string &bucket, const std::string &key,
                            const UploadId &);

  ETag CompleteMultipartUpload(const UploadId &uid, const std::string &bucket,
                               const std::string &key,
                               const std::vector<ETag> &etags);

  void CreateBucket(const std::string &bucket, const Headers & = {{}});

  UploadId CreateMultipartUpload(const std::string &bucket,
                                 const std::string &key,
                                 const MetaDataMap &metaData = {});

  void DeleteBucket(const std::string &bucket);

  void DeleteObject(const std::string &bucket, const std::string &key,
                    const Headers & = {{}});
  // @todo
  // bool DeleteObjects(const std::string &bucket,
  //                   const std::vector<std::string> &objects);
  ByteArray GetObject(const std::string &bucket, const std::string &key,
                      size_t begin = 0, size_t end = 0, Headers = {{}});

  void GetObject(const std::string &bucket, const std::string &key,
                 ByteArray &buffer, size_t offset, size_t begin = 0,
                 size_t end = 0, Headers headers = {{}});

  void GetObject(const std::string &bucket, const std::string &key,
                 char *buffer, size_t offset, size_t begin = 0, size_t end = 0,
                 Headers headers = {{}});

  Headers HeadBucket(const std::string &bucket);
  Headers HeadObject(const std::string &bucket, const std::string &key,
                     const Headers & = {{}});

  std::vector<BucketInfo> ListBuckets();

  std::vector<ObjectInfo> ListObjectsV2(const std::string &bucket,
                                        const ListObjectV2Config &config,
                                        const Headers & = {{}});
  // not implemented
  // std::vector<PartInfo> ListParts(const std::string &bucket,
  //                                 const std::string &key, const UploadId
  //                                 &uid, int max_parts);

  ETag PutObject(const std::string &bucket, const std::string &key,
                 const ByteArray &buffer, const Headers & = {{}});

  ETag PutObject(const std::string &bucket, const std::string &key,
                 const char *, size_t size, size_t offset = 0,
                 const Headers & = {{}});

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
