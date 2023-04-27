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
// #include <sha256.h>

#include "s3hash/sha256.h"

void hmac256(const uint8_t *data, size_t length, const uint8_t *key,
             size_t key_length, uint8_t hmac_hash[64]);

#include <ctime>
#include <iomanip>
#include <map>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "aws_sign.h"
#include "common.h"
#include "url_utility.h"

using namespace std;

namespace sss {

//------------------------------------------------------------------------------
using Bytes = vector<uint8_t>;

std::string SHA256(const string &s) {

  uint32_t hash[8];
  sha256((const uint8_t *)s.c_str(), (uint32_t)s.size(), hash);
  char h[65];
  hash_to_text(hash, h);
  return h;
}

Bytes HMAC256(const Bytes &d, const Bytes &k) {
  vector<uint8_t> hash(32);
  hmac256(d.data(), d.size(), k.data(), k.size(), hash.data());
  return hash;
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
Bytes Hash(const Bytes &key, const Bytes &msg) { return HMAC256(msg, key); }

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
/// @warnihg: x-amz- fields seem not to be required to sign, metatdata
/// x-amz-meta work without adding to signed headers
string SignedURL(const S3SignUrlConfig &cfg) {

  // const string &accessKey, const string &secretKey,
  //              int expiration, const string &endpoint, const string &method,
  //              const string &bucketName, const string &keyName,
  //              const Parameters &params, const string &region,
  //              const Time &dates) {
  const URL url = ParseURL(cfg.endpoint);
  const string host =
      url.port <= 0 ? url.host : url.host + ":" + to_string(url.port);
  Time t = cfg.dates.dateStamp.empty() ? GetDates() : cfg.dates;

  const string credentials =
      cfg.access + "/" + t.dateStamp + "/" + cfg.region + "/s3/aws4_request";

  // headers: add x-amz headers to singned headers
  Headers signHeaders = {{"host", host}};
  for (auto kv : cfg.headers) {
    if (kv.first.find("x-amz-")) {
      signHeaders.insert(kv);
    }
  }
  string signedHeadersStr;
  for (auto kv : signHeaders) {
    signedHeadersStr += kv.first + ';';
  }
  signedHeadersStr.pop_back(); // remove last ';'

  Headers headers = cfg.headers;
  headers.insert({"host", host});

  ostringstream os;
  for (auto kv : headers) {
    os << kv.first << ':' << kv.second << '\n';
  }

  const string canonicalHeaders = os.str();
  ////

  Parameters parameters = {{"X-Amz-Algorithm", "AWS4-HMAC-SHA256"},
                           {"X-Amz-Credential", credentials},
                           {"X-Amz-Date", t.timeStamp},
                           {"X-Amz-Expires", to_string(cfg.expiration)},
                           {"X-Amz-SignedHeaders", signedHeadersStr}};

  if (!cfg.params.empty()) {
    parameters.insert(begin(cfg.params), end(cfg.params));
  }
  const string canonicalQueryStringUrlEncoded = UrlEncode(parameters);

  string canonicalResource = "/";

  if (!cfg.bucket.empty()) {
    canonicalResource += cfg.bucket;
    if (!cfg.key.empty()) {
      canonicalResource += "/" + cfg.key;
    }
  }

  const string payloadHash = "UNSIGNED-PAYLOAD";

  const string canonical_request = cfg.method + "\n" + canonicalResource +
                                   "\n" + canonicalQueryStringUrlEncoded +
                                   "\n" + canonicalHeaders + "\n" +
                                   signedHeadersStr + "\n" + payloadHash;

  // text to sign
  const string hashingAlgorithm = "AWS4-HMAC-SHA256";
  const string credentialCtx =
      t.dateStamp + "/" + cfg.region + "/" + "s3" + "/" + "aws4_request";
  SHA256 sha256;
  const string stringToSign = hashingAlgorithm + "\n" + t.timeStamp + "\n" +
                              credentialCtx + "\n" + sha256(canonical_request);

  // generate the signature
  const Bytes signatureKey =
      CreateSignatureKey(cfg.secret, t.dateStamp, cfg.region, "s3");

  const string signature =
      Hex(HMAC256(Bytes(begin(stringToSign), end(stringToSign)), signatureKey));

  string requestUrl = cfg.endpoint;
  if (!cfg.bucket.empty()) {
    requestUrl += "/" + cfg.bucket;
    if (!cfg.key.empty()) {
      requestUrl += "/" + cfg.key;
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
Signature ComputeSignature(const ComputeSignatureConfig &cfg) {

#ifndef NDEBUG
  // do not want to waste time converting to lowercase
  for (auto kv : cfg.headers) {
    if (kv.first != ToLower(kv.first)) {
      throw invalid_argument("Header keys must be lowecase");
    }
  }
#endif
  const string payloadHash =
      cfg.payloadHash.empty() ? "UNSIGNED-PAYLOAD" : cfg.payloadHash;
  const URL url = ParseURL(cfg.endpoint);
  const string host =
      url.port <= 0 ? url.host : url.host + ":" + to_string(url.port);
  Time t = cfg.dates.dateStamp.empty() ? GetDates() : cfg.dates;
  const string reqParameters =
      cfg.parameters.empty() ? "" : UrlEncode(cfg.parameters);
  string canonicalURI = "/";

  if (!cfg.bucket.empty()) {
    canonicalURI += cfg.bucket;
    if (!cfg.key.empty()) {
      canonicalURI += "/" + cfg.key;
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
  for (auto kv : cfg.headers) {
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
      ToUpper(cfg.method) + '\n' + canonicalURI + '\n' + canonicalQueryString +
      '\n' + canonicalHeadersStr + '\n' + signedHeadersStr + '\n' + payloadHash;

  const string algorithm = "AWS4-HMAC-SHA256";
  const string credentialScope =
      t.dateStamp + '/' + cfg.region + '/' + cfg.service + '/' + "aws4_request";

  // SHA256 sha256;
  const string stringToSign = algorithm + '\n' + t.timeStamp + '\n' +
                              credentialScope + '\n' + SHA256(canonicalRequest);
  // generate the signature
  const Bytes signatureKey =
      CreateSignatureKey(cfg.secret, t.dateStamp, cfg.region, cfg.service);

  const auto s =
      Hex(HMAC256(Bytes(begin(stringToSign), end(stringToSign)), signatureKey));
  const string signature(begin(s), end(s));
  return {signature, credentialScope, signedHeadersStr, defaultHeaders};
}

//------------------------------------------------------------------------------
/// Sign HTTP headers: return dictionary with {key, value} pairs containing
/// per-header information.
Headers SignHeaders(const ComputeSignatureConfig &cfg) {
  const auto s = ComputeSignature(cfg);
  const string algorithm = "AWS4-HMAC-SHA256";
  // build authorisaton header
  const string authorizationHeader =
      algorithm + ' ' + string("Credential=") + cfg.access + '/' +
      s.credentialScope + ", " + string("SignedHeaders=") + s.signedHeadersStr +
      ", " + string("Signature=") + s.signature;
  auto allHeaders = s.defaultHeaders;
  allHeaders.insert({"Authorization", authorizationHeader});
  allHeaders.insert(begin(cfg.headers), end(cfg.headers));
  return allHeaders;
}

} // namespace sss
