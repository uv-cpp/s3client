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
 * \file aws_sigh.h
 * \brief Public interface to signing URLs and HTTP headers with the S3v4 
 *        protocol standard.
 */

#pragma once
#include <string>
#include "common.h"

namespace sss {

/// Generate presigned URL
std::string SignedURL(const std::string& accessKey,
                      const std::string& secretKey, int expiration,
                      const std::string& endpoint, const std::string& method,
                      const std::string& bucketName = "",
                      const std::string& keyName = "",
                      const Map& params = Map(),
                      const std::string& region = "us-east-1");

/// Sign headers
Map SignHeaders(const std::string& accessKey, const std::string& secretKey,
                const std::string& endpoint, const std::string& method,
                const std::string& bucketName = "",
                const std::string& keyName = "", std::string payloadHash = "",
                const Map& parameters = Map(),
                const Map& additionalHeaders = Map(),
                const std::string& region = "us-east-1",
                const std::string& service = "s3");

/// Struct 
struct SignHeadersInfo {
    std::string key;
    std::string secret;
    std::string endpoint;
    std::string method;
    std::string bucket;
    std::string bucketKey;
    std::string payloadHash;
    Map parameters;
    Map additionalHeaders;
    std::string region{"us-east-1"};
    std::string service{"s3"};
};

/// Sign headers. Alternative signature using a single \c stuct 
/// instead of multiple parameters
Map SignHeaders(const SignHeadersInfo& hi);

} // namespace