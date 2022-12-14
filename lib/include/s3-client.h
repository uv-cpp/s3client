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
#include "webclient.h"
#include <string>
#include <vector>

//------------------------------------------------------------------------------
/// S3 Credentials in AWS format
struct S3Credentials {
  std::string s3AccessKey;
  std::string s3SecretKey;
};

/// Parameters for SendS3Request calls.
struct S3ClientConfig {
  std::string s3AccessKey;
  std::string s3SecretKey;
  std::string endpoint; //< actual endpoint where requests are sent
  std::string
      signUrl; //< url used to sign, allows requests to work across tunnels
  std::string bucket;
  std::string key;
  std::string params; //< "param1=val1;param2=val2..."
  std::string method = "GET";
  std::string headers; //< "Header1:value1;Header2:value2..."
  std::string outfile; // if not empty stores returned response body into file
  /// if dataIsFileName == true  assume filename, if
  /// not send 'data' bytes
  std::vector<char> data;
  bool dataIsFileName = false; //< if true interpret 'data' as file name
};

/// Parameters for calls to upload and download file functions.
struct S3FileTransferConfig {
  std::string s3AccessKey;
  std::string s3SecretKey;
  std::string endpoint;
  std::string signUrl; //< @warning not implemented @todo implement
  std::string bucket;
  std::string key;
  std::string file;
  /// credentials file; @warning not implemented @todo remove
  /// and use GetS3Credentials to retrieve credential information
  /// before calling other functions.
  std::string credentials;
  std::string awsProfile;
  std::vector<std::string> endpoints;
  int maxRetries =
      1; //< mximum number of retries per chunk, only implementd for upload
  int jobs = 1; //< number of parallel threads of execution
};

/// Parameters for calls to SignS3URL function.
struct S3SignUrlConfig {
  std::string s3AccessKey;
  std::string s3SecretKey;
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
sss::WebClient SendS3Request(S3ClientConfig);
/// Parallel download of object to file, @see S3FileTransferConfig
void DownloadFile(S3FileTransferConfig);
/// Parallel upload of file to object, @see S3FileTransferConfig
std::string UploadFile(S3FileTransferConfig);
/// Sign S3 request.
std::string SignS3URL(const S3SignUrlConfig &);
/// Read S3 credentials from file. Whn reading from .aws folder an aws profile
/// can be selected.
S3Credentials GetS3Credentials(const std::string &fileName,
                               std::string awsProfile);
