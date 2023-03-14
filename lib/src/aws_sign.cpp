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
// Functions to sign S3 URL and HTTP headers
// See:
// https://docs.aws.amazon.com/AmazonS3/latest/API/sig-v4-authenticating-requests.html
// https://docs.aws.amazon.com/AmazonS3/latest/API/sig-v4-header-based-auth.html
#include <hmac.h>
#include <sha256.h>

#include <ctime>
#include <iomanip>
#include <map>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "common.h"
#include "url_utility.h"

using namespace std;

namespace sss {

//------------------------------------------------------------------------------
using Bytes = vector<uint8_t>;

//------------------------------------------------------------------------------
// Compute HMAC hash of data and key using MD5, SHA1 or SHA256
// using byte arrays instead of strings
// Code modified from
// hmac.h
// Copyright (c) 2015 Stephan Brumme. All rights reserved.
// see http://create.stephan-brumme.com/disclaimer.html
template <typename HashMethod> Bytes hmacb(const Bytes &d, const Bytes &k) {
  const void *data = d.data();
  const size_t numDataBytes = d.size();
  const void *key = k.data();
  const size_t numKeyBytes = k.size();
  // initialize key with zeros
  unsigned char usedKey[HashMethod::BlockSize] = {0};

  // adjust length of key: must contain exactly blockSize bytes
  if (numKeyBytes <= HashMethod::BlockSize) {
    // copy key
    memcpy(usedKey, key, numKeyBytes);
  } else {
    // shorten key: usedKey = hashed(key)
    HashMethod keyHasher;
    keyHasher.add(key, numKeyBytes);
    keyHasher.getHash(usedKey);
  }

  // create initial XOR padding
  for (size_t i = 0; i < HashMethod::BlockSize; i++)
    usedKey[i] ^= 0x36;

  // inside = hash((usedKey ^ 0x36) + data)
  unsigned char inside[HashMethod::HashBytes];
  HashMethod insideHasher;
  insideHasher.add(usedKey, HashMethod::BlockSize);
  insideHasher.add(data, numDataBytes);
  insideHasher.getHash(inside);

  // undo usedKey's previous 0x36 XORing and apply a XOR by 0x5C
  for (size_t i = 0; i < HashMethod::BlockSize; i++)
    usedKey[i] ^= 0x5C ^ 0x36;

  // hash((usedKey ^ 0x5C) + hash((usedKey ^ 0x36) + data))
  HashMethod finalHasher;
  finalHasher.add(usedKey, HashMethod::BlockSize);
  finalHasher.add(inside, HashMethod::HashBytes);
  std::vector<uint8_t> b(HashMethod::HashBytes);
  finalHasher.getHash(b.data());
  return b;
}

//------------------------------------------------------------------------------
/// Return time in the two formats required to sign AWS requests:
/// - full date-time
/// - date only
Time GetDates() {
  time_t t;
  time(&t);
  struct tm *ts;
  ts = gmtime(&t);

  const size_t BUFSIZE = 128;
  vector<char> buf1(BUFSIZE, '\0');
  strftime(buf1.data(), BUFSIZE, "%Y%m%dT%H%M%SZ", ts);
  const string timeStamp(buf1.data());
  vector<char> buf2(BUFSIZE, '\0');
  strftime(buf2.data(), BUFSIZE, "%Y%m%d", ts);
  const string dateStamp(buf2.data());
  return {timeStamp, dateStamp};
}

//------------------------------------------------------------------------------
/// HMAC encoding byte arrays --> byte array
Bytes Hash(const Bytes &key, const Bytes &msg) {
  return hmacb<SHA256>(msg, key);
}

//------------------------------------------------------------------------------
/// Create AWS signature key as byte array
Bytes CreateSignatureKey(const string &key, const string &dateStamp,
                         const string &region, const string &service) {
  const string k = "AWS4" + key;
  const Bytes kb = Bytes(k.begin(), k.end());
  const Bytes dsb = Bytes(dateStamp.begin(), dateStamp.end());
  const Bytes dateKey = Hash(kb, dsb);
  const Bytes regionKey = Hash(dateKey, Bytes(begin(region), end(region)));
  const Bytes serviceKey = Hash(regionKey, Bytes(begin(service), end(service)));
  const string aws = "aws4_request";
  const Bytes signingKey = Hash(serviceKey, Bytes(begin(aws), end(aws)));
  return signingKey;
}

//------------------------------------------------------------------------------
/// Byte to hex string conversion
string Hex(const Bytes &b) {
  ostringstream os;

  for (auto x : b) {
    os << std::setfill('0') << std::setw(2) << std::hex << int(x);
  }
  return os.str();
}

//------------------------------------------------------------------------------
/// Presign url, 'expiration' time must be specified in seconds
string SignedURL(const string &accessKey, const string &secretKey,
                 int expiration, const string &endpoint, const string &method,
                 const string &bucketName, const string &keyName,
                 const Parameters &params, const string &region,
                 const Time &dates) {
  const URL url = ParseURL(endpoint);
  const string host =
      url.port <= 0 ? url.host : url.host + ":" + to_string(url.port);
  Time t = dates.dateStamp.empty() ? GetDates() : dates;
  const string credentials =
      accessKey + "/" + t.dateStamp + "/" + region + "/s3/aws4_request";

  Parameters parameters = {{"X-Amz-Algorithm", "AWS4-HMAC-SHA256"},
                           {"X-Amz-Credential", credentials},
                           {"X-Amz-Date", t.timeStamp},
                           {"X-Amz-Expires", to_string(expiration)},
                           {"X-Amz-SignedHeaders", "host"}};

  if (!params.empty()) {
    parameters.insert(begin(params), end(params));
  }
  const string canonicalQueryStringUrlEncoded = UrlEncode(parameters);

  string canonicalResource = "/";

  if (!bucketName.empty()) {
    canonicalResource += bucketName;
    if (!keyName.empty()) {
      canonicalResource += "/" + keyName;
    }
  }

  const string payloadHash = "UNSIGNED-PAYLOAD";
  const string canonicalHeaders = "host:" + host;
  const string signedHeaders = "host";

  const string canonical_request = method + "\n" + canonicalResource + "\n" +
                                   canonicalQueryStringUrlEncoded + "\n" +
                                   canonicalHeaders + "\n" + "\n" +
                                   signedHeaders + "\n" + payloadHash;

  // text to sign
  const string hashingAlgorithm = "AWS4-HMAC-SHA256";
  const string credentialCtx =
      t.dateStamp + "/" + region + "/" + "s3" + "/" + "aws4_request";
  SHA256 sha256;
  const string stringToSign = hashingAlgorithm + "\n" + t.timeStamp + "\n" +
                              credentialCtx + "\n" + sha256(canonical_request);

  // generate the signature
  const Bytes signatureKey =
      CreateSignatureKey(secretKey, t.dateStamp, region, "s3");

  const string signature = Hex(hmacb<SHA256>(
      Bytes(begin(stringToSign), end(stringToSign)), signatureKey));

  string requestUrl = endpoint;
  if (!bucketName.empty()) {
    requestUrl += "/" + bucketName;
    if (!keyName.empty()) {
      requestUrl += "/" + keyName;
    }
  }
  requestUrl +=
      "?" + canonicalQueryStringUrlEncoded + "&X-Amz-Signature=" + signature;

  return requestUrl;
}
//------------------------------------------------------------------------------
/// Sign HTTP headers: return dictionary with {key, value} pairs containing
/// per-header information.
/// @TODO: replace arguments with struct.
Headers SignHeaders(const string &accessKey, const string &secretKey,
                    const string &endpoint, const string &method,
                    const string &bucketName, const string &keyName,
                    string payloadHash, const Parameters &parameters,
                    const Headers &additionalHeaders, const string &region,
                    const string &service, const Time &dates) {
#ifndef NDEBUG
  // do not want to waste time converting to lowercase
  for (auto kv : additionalHeaders) {
    if (kv.first != ToLower(kv.first)) {
      throw invalid_argument("Header keys must be lowecase");
    }
  }
#endif
  if (payloadHash.empty()) {
    payloadHash = "UNSIGNED-PAYLOAD";
  }
  const URL url = ParseURL(endpoint);
  const string host =
      url.port <= 0 ? url.host : url.host + ":" + to_string(url.port);
  Time t = dates.dateStamp.empty() ? GetDates() : dates;
  const string reqParameters = parameters.empty() ? "" : UrlEncode(parameters);
  string canonicalURI = "/";

  if (!bucketName.empty()) {
    canonicalURI += bucketName;
    if (!keyName.empty()) {
      canonicalURI += "/" + keyName;
    }
  }

  const string canonicalQueryString = reqParameters;

  // Canonical and signed headers
  const Map defaultHeaders = {{"host", host},
                              {"x-amz-content-sha256", payloadHash},
                              {"x-amz-date", t.timeStamp}};

  Map canonicalHeaders = defaultHeaders;
  set<string> sortedSignedHeadersKeys;
  for (auto kv : defaultHeaders) {
    sortedSignedHeadersKeys.insert(kv.first);
  }
  // add x-amz headers
  for (auto kv : additionalHeaders) {
    if (kv.first.find("x-amz-") == 0 || kv.first.find("content-length") == 0) {
      canonicalHeaders.insert(kv);
      sortedSignedHeadersKeys.insert(kv.first);
    }
  }

  // canonical headers string
  ostringstream os;
  for (auto kv : canonicalHeaders) {
    os << kv.first << ":" << kv.second << "\n";
  }

  string canonicalHeadersStr = os.str();

  os.str("");
  for (auto k : sortedSignedHeadersKeys) {
    os << k << ";";
  }

  string signedHeadersStr = os.str();
  signedHeadersStr = signedHeadersStr.substr(0, signedHeadersStr.size() - 1);

  // canonical request
  const string canonicalRequest =
      ToUpper(method) + '\n' + canonicalURI + '\n' + canonicalQueryString +
      '\n' + canonicalHeadersStr + '\n' + signedHeadersStr + '\n' + payloadHash;

  const string algorithm = "AWS4-HMAC-SHA256";
  const string credentialScope =
      t.dateStamp + '/' + region + '/' + service + '/' + "aws4_request";

  SHA256 sha256;
  const string stringToSign = algorithm + '\n' + t.timeStamp + '\n' +
                              credentialScope + '\n' + sha256(canonicalRequest);
  // generate the signature
  const Bytes signatureKey =
      CreateSignatureKey(secretKey, t.dateStamp, region, service);

  const string signature = Hex(hmacb<SHA256>(
      Bytes(begin(stringToSign), end(stringToSign)), signatureKey));

  // build authorisaton header
  const string authorizationHeader =
      algorithm + ' ' + string("Credential=") + accessKey + '/' +
      credentialScope + ", " + string("SignedHeaders=") + signedHeadersStr +
      ", " + string("Signature=") + signature;

  auto allHeaders = defaultHeaders;
  allHeaders.insert(begin(defaultHeaders), end(defaultHeaders));
  allHeaders.insert({"Authorization", authorizationHeader});
  allHeaders.insert(begin(additionalHeaders), end(additionalHeaders));
  return allHeaders;
}

} // namespace sss
