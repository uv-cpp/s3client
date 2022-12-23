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
/**
 * \file s3-cliant.h
 * \brief declaration of functions and data types for sending S3 requests,
 * signing URLs and downloading and uploading files.
 */
#pragma once
#include "common.h"
#include "webclient.h"
#include <string>
#include <vector>

namespace sss {

//------------------------------------------------------------------------------
/// S3 Credentials in AWS format
struct S3Credentials {
  std::string accessKey;
  std::string secretKey;
};

/// Parameters for SendS3Request calls.
struct S3ClientConfig {
  std::string accessKey;
  std::string secretKey;
  std::string endpoint; //< actual endpoint where requests are sent
  std::string
      signUrl; //< url used to sign, allows requests to work across tunnels
  std::string bucket;
  std::string key;
  Parameters params;
  std::string method = "GET";
  Headers headers;
  std::string outfile; // if not empty stores returned response body into file
  /// if dataIsFileName == true  assume filename, if
  /// not send 'data' bytes
  std::vector<char> data;
  bool dataIsFileName = false; //< if true interpret 'data' as file name
};

/// Parameters for calls to upload and download file functions.
struct S3FileTransferConfig {
  std::string accessKey;
  std::string secretKey;
  std::string signUrl; //< @warning not implemented @todo implement
  std::string bucket;
  std::string key;
  std::string file;
  std::string awsProfile;
  std::vector<std::string> endpoints;
  int maxRetries =
      1; //< mximum number of retries per chunk, only implementd for upload
  int jobs = 1; //< number of parallel threads of execution
};

/// Parameters for calls to SignS3URL function.
struct S3SignUrlConfig {
  std::string accessKey;
  std::string secretKey;
  std::string endpoint;
  int expiration; //< expiration time in seconds  @todo Unsigned!
  std::string method;
  std::string bucket;
  std::string key;
  std::string params; //< URL parameters: "param1=val1;param2=var2"
  std::string region = "us-east";
};

//------------------------------------------------------------------------------
/// Validate S3 client request parameters.
void Validate(const S3ClientConfig &s);
/// Send S3 request to endpoint.
WebClient SendS3Request(S3ClientConfig);
/// Parallel download of object to file, @see S3FileTransferConfig
void DownloadFile(S3FileTransferConfig);
/// Parallel upload of file to object, @see S3FileTransferConfig
std::string UploadFile(const S3FileTransferConfig &,
                       const MetaDataMap & = MetaDataMap{});
/// Sign S3 request.
std::string SignS3URL(const S3SignUrlConfig &);
/// Read S3 credentials from file. Whn reading from .aws folder an aws profile
/// can be selected.
S3Credentials GetS3Credentials(const std::string &fileName,
                               std::string awsProfile);

using ByteArray = std::vector<uint8_t>;
using StringArray = std::vector<std::string>;
using Etag = std::string;
using MetaDataMap = std::map<std::string, std::string>;
using UploadId = std::string;

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
  StringArray ListBuckets();
  Headers ListBucket(const std::string &bucket);
  StringArray ListObjects(const std::string &bucket);
  Headers ListObject(const std::string &bucket, const std::string &key);
  void CreateBucket(const std::string &bucket);
  ByteArray Get(const std::string &bucket, const std::string &key,
                size_t begin = 0, size_t end = 0);
  Etag Put(const std::string &bucket, const std::string &key,
           const uint8_t *data, size_t size,
           const MetaDataMap &metaData = MetaDataMap{});
  void DeleteObject(const std::string &bucket, const std::string &key);
  void DeleteBucket(const std::string &bucket);
  Etag PutNoOverwrite(const std::string &bucket, const std::string &key,
                      const uint8_t *data, size_t size,
                      const MetaDataMap &metaData = MetaDataMap{});
  bool IsBucket(const std::string &bucket);
  bool IsKey(const std::string &bucket, const std::string &key);
  Etag UploadFile(const std::string &bucket, const std::string &key,
                  const std::string &fileName);
  void DownloadFile(const std::string &bucket, const std::string &key,
                    const std::string &fileName);
  Etag UploadData(const std::string &bucket, const std::string &key,
                  const uint8_t *data, size_t size);
  UploadId BeginUpload(const std::string &bucket, const std::string &key);
  Etag UploadPart(const UploadId &uploadId, const std::string &bucket,
                  const std::string &key, const uint8_t *data, size_t size);
  Etag EndUpload(const std::vector<UploadId> &ids);

private:
  sss::WebClient webClient_;
  std::string access_;
  std::string secret_;
  std::string endpoint_;
  std::string signingEndpoint_;
};

} // namespace sss
