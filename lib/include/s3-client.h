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
#include "webclient.h"
#include <string>
#include <vector>

//------------------------------------------------------------------------------
struct S3Args {
  std::string s3AccessKey;
  std::string s3SecretKey;
  std::string endpoint; // actual endpoint where requests are sent
  std::string
      signUrl; // url used to sign, allows requests to work across tunnels
  std::string bucket;
  std::string key;
  std::string params;
  std::string method = "GET";
  std::string headers;
  std::string data;
  std::string outfile;
};

struct UploadConfig {
  std::string s3AccessKey;
  std::string s3SecretKey;
  std::string endpoint;
  std::string bucket;
  std::string key;
  std::string file;
  std::string credentials;
  std::string awsProfile;
  std::vector<std::string> endpoints;
  int maxRetries = 1;
  int jobs = 1;
};
//------------------------------------------------------------------------------
void Validate(const S3Args &args);
sss::WebClient SendS3Request(S3Args &args);
void DownloadFile(const std::string &s3AccessKey,
                  const std::string &s3SecretKey, const std::string &endpoint,
                  const std::string &bucket, const std::string &key,
                  const std::string &file, int retries = 0, int jobs = 1,
                  const std::string &signUrl = "");
std::string UploadFile(UploadConfig);
