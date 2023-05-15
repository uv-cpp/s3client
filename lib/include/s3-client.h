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
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 ******************************************************************************/
/**
 * \file s3-client.h
 * \brief declaration of functions and data types for sending S3 requests,
 * signing URLs and downloading and uploading files.
 */
#pragma once
#include "aws_sign.h"
#include "common.h"
#include "webclient.h"
#include <string>
#include <vector>

namespace sss {
/**
 * \addtogroup Types
 * @{
 */
using CharArray = std::vector<char>;
using StringArray = std::vector<std::string>;
using ETag = std::string;
using MetaDataMap = std::map<std::string, std::string>;
using UploadId = std::string;
/**
 * @}
 */
/**
 * \addtogroup S3Client
 * \brief S3 client interface.
 * Functions to send S3 requests, and perform parallel uploading and downloading
 * data from memory and files.
 * @{
 */
//------------------------------------------------------------------------------
/// S3 Credentials in AWS format
struct S3Credentials {
  std::string accessKey;
  std::string secretKey;
};

/// Parameters for SendS3Request calls.
/// \see SendS3Request
struct S3ClientConfig {
  std::string accessKey;
  std::string secretKey;
  std::string endpoint; ///< actual endpoint where requests are sent
  std::string
      signUrl; ///< url used to sign, allows requests to work across tunnels
  std::string bucket;
  std::string key;
  Parameters params;
  std::string method = "GET";
  Headers headers;
  std::string outfile; ///< if not empty stores returned response body into file
  /// container for data or file name
  std::vector<char> data;
  /// if dataIsFileName == true read interpret content of \c data field as file
  /// name else interpret as data source
  bool dataIsFileName = false;
};

/// Parameters for calls to upload and download functions.
struct S3DataTransferConfig {
  std::string accessKey;
  std::string secretKey;
  std::string proxyUrl; ///< @warning not implemented @todo implement
  std::string bucket;
  std::string key;
  std::string file;
  char *data = nullptr; ///< input for upload and output for download
  size_t offset = 0;
  size_t size = 0;
  std::string awsProfile;
  std::vector<std::string>
      endpoints;           ///< list of endpoints for client-side load balancing
  int maxRetries = 1;      ///< maximum number of retries
  int jobs = 1;            ///< number of parallel upload/download tasks
  size_t partsPerJob = 1;  ///< number of parts per job
  std::string payloadHash; ///< payload hash if empty the literal \c
                           ///< "UNSIGNED-PAYLOAD" is used instead of the SHA256
                           ///< hash code
};

//------------------------------------------------------------------------------
/// Returned from ValidateBucket function.
struct BucketValidation {
  bool valid = false; ///< \c true if bucket name valid, \c false otherwise
  std::string error;  ///< error message if not valid
  operator bool() const { return valid; }
};
/// Validate bucket name.
BucketValidation ValidateBucket(const std::string name);
/// Translate metadata (key,value) pairs to \c amz- format and return
/// header map which can be merged with other headers using the
/// \c std::map::merge method, since C++17.
Headers ToMeta(const std::map<std::string, std::string> &metaData);
/// Validate S3 client request parameters.
void Validate(const S3ClientConfig &s);
/// Send S3 request to endpoint.
WebClient SendS3Request(S3ClientConfig);
/// Parallel upload
/// If \c cfg.data not \c NULL data is read from memory, from file
/// specified in \c cfg.file instead.
/// \param[in] cfg data transfer configuration, \see S3DataTransferConfig
/// \param[in] sync if `sync==true` perform serial transfer
ETag Upload(const S3DataTransferConfig &cfg, const MetaDataMap &mm = {},
            bool sync = false);
/// Parallel upload
/// If \c cfg.data not \c NULL data is written into \c cfg.data buffer,
/// to file specified in \c cfg.file instead.
/// \param[in] cfg data transfer configuration, \see S3DataTransferConfig
/// \param[in] sync if `sync==true` perform serial transfer
void Download(const S3DataTransferConfig &cfg, bool sync = false);
/// Read S3 credentials from file.
/// \param[in] fileName name of configuration file in AWS TOML format
/// \param[in] awsProfile profile
S3Credentials GetS3Credentials(const std::string &fileName,
                               std::string awsProfile);
/**
 * @}
 */

} // namespace sss
