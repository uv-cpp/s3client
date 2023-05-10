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
 * \file aws_sign.h
 * \brief Public interface to signing URLs and HTTP headers with the S3v4
 *        protocol standard.
 */

#pragma once
#include "common.h"
#include "url_utility.h"
#include <string>

namespace sss {
/**
 * \addtogroup Sign
 * \brief AWS S3v4 signing functions.
 * @{
 */
/// \brief Parameters for calls to ComputeSignature and SignHeaders function
struct ComputeSignatureConfig {
  std::string access;
  std::string secret;
  std::string endpoint;
  std::string method;
  std::string bucket;
  std::string key;
  std::string payloadHash; ///< if empty "UNSIGNED-PAYLOAD" is sent instead
  Parameters parameters;
  Headers headers;
  std::string region = "us-east";
  std::string service = "s3";
  Time dates;
};

/// \brief Parameters for calls to SignedURL function.
struct S3SignUrlConfig {
  std::string access;
  std::string secret;
  std::string endpoint;
  int expiration = 0; ///< expiration time in seconds  @todo Unsigned!
  std::string method;
  std::string bucket;
  std::string key;
  Parameters params; ///< URL parameters: "param1=val1;param2=var2"
  Headers headers;   ///< Headers: "header1:content1;header2:content2..."
  std::string region = "us-east";
  Time dates;
};

/// \brief Signature information
struct Signature {
  std::string signature;        ///< computed HMAC hash
  std::string credentialScope;  ///< credential scope
  std::string signedHeadersStr; ///< signature string to add to header
  /** Default headers used to sign the request.
   *  The following headers are always included:
   *  \code
   *  host
   *  x-amz-content-sha256
   *  x-amz-data
   *  \endcode
   *  any other header starting with \c x-amz- is added to the
   *  above set.
   */
  Headers defaultHeaders;
};

/// \brief Compute signature.
/// \ingroup Examples
/// \see ComputeSignatureConfig
/// \see Signature
///
/// \section ex1 Example
/// Signing a \c GET request for downloading object \c /bucket1/key1.
/// This code is exteacted from a test case which outputs results in CSV format
/// where success is mapped to '1' and failure to '0'.
/// \snippet sign-test.cpp Create signature example
/// \section ex2 Expected output
/// \code{.sh}
/// Sign,Sign request,1
/// \endcode
Signature ComputeSignature(const ComputeSignatureConfig &);

/// \brief Generate presigned URL.
/// \see S3SignUrlConfig
std::string SignedURL(const S3SignUrlConfig &);

/** \brief Sign headers.
 *
 * \param[in] cfg configuration information for computing signature;
 *            headers are passed as a data member and returned together
 *            with the additional signed header.
 * \return map of {header name->header value} pairs.
 * \see ComputeSignatureConfig
 */
Headers SignHeaders(const ComputeSignatureConfig &cfg);
/**
 * @}
 */

} // namespace sss
