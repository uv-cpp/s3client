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
 * \file aws_sigh.h
 * \brief Public interface to signing URLs and HTTP headers with the S3v4
 *        protocol standard.
 */

#pragma once
#include "common.h"
#include "url_utility.h"
#include <string>

namespace sss {

/// Parameters for calls to ComputerSignature and SignHeaders function
struct ComputeSignatureConfig {
  std::string access;
  std::string secret;
  std::string endpoint;
  std::string method;
  std::string bucket;
  std::string key;
  std::string payloadHash;
  Parameters parameters;
  Headers headers;
  std::string region = "us-east";
  std::string service = "s3";
  Time dates;
};

/// Parameters for calls to SignedURL function.
struct S3SignUrlConfig {
  std::string access;
  std::string secret;
  std::string endpoint;
  int expiration = 0; //< expiration time in seconds  @todo Unsigned!
  std::string method;
  std::string bucket;
  std::string key;
  Parameters params; //< URL parameters: "param1=val1;param2=var2"
  Headers headers;   //< Headers: "header1:content1;header2:content2..."
  std::string region = "us-east";
  Time dates;
};

struct Signature {
  std::string signature;
  std::string credentialScope;
  std::string signedHeadersStr;
  Headers defaultHeaders;
};
/// Compute signature.
Signature ComputeSignature(const ComputeSignatureConfig &);

/// Generate presigned URL.
std::string SignedURL(const S3SignUrlConfig &);

/// Sign headers
Headers SignHeaders(const ComputeSignatureConfig &);

} // namespace sss
