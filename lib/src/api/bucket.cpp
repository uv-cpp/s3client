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

#include "aws_sign.h"
#include "common.h"
#include "response_parser.h"
#include "s3-api.h"
#include "s3-client.h"

using namespace std;

namespace sss {
namespace api {

std::vector<BucketInfo> ParseBuckets(const std::string &xml);
AccessControlPolicy ParseACL(const std::string &xml);
std::string GenerateAclXML(const AccessControlPolicy &);

//------------------------------------------------------------------------------
bool S3Api::TestBucket(const string &bucket) {
  try {
    Send({.method = "GET", .bucket = bucket, .signUrl = SigningEndpoint()});
    return true;
  } catch (...) {
    return false;
  }
}

//------------------------------------------------------------------------------
string S3Api::CreateBucket(const std::string &bucket, const Headers &headers,
                           const string &locationConstraint) {
  const S3Api::SendParams reqParams = [&]() -> SendParams {
    if (locationConstraint.empty()) {
      return {.method = "PUT", .bucket = bucket, .headers = headers};
    } else {
      const string req =
          string("<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
                 "<CreateBucketConfiguration "
                 "xmlns=\"http://s3.amazonaws.com/doc/2006-03-01/\">"
                 "<LocationConstraint>") +
          locationConstraint +
          "</LocationConstraint></CreateBucketConfiguration>";
      return {.method = "PUT",
              .bucket = bucket,
              .headers = headers,
              .uploadData = req};
    }
  }();
  auto &wc = Send(reqParams);
  auto h = ParseHeaders(wc.GetHeaderText());
  for (auto kv : h) {
    if (ToLower(kv.first) == "location") {
      return kv.second;
    }
  }
  return "";
}

//------------------------------------------------------------------------------
void S3Api::DeleteBucket(const std::string &bucket, const Headers &headers) {
  Send({.method = "DELETE", .bucket = bucket, .headers = headers});
}

//------------------------------------------------------------------------------
Headers S3Api::HeadBucket(const string &bucket, const Headers &headers) {
  const auto &wc =
      Send({.method = "HEAD", .bucket = bucket, .headers = headers});
  return HTTPHeaders(wc.GetContentText());
}

//------------------------------------------------------------------------------
vector<BucketInfo> S3Api::ListBuckets(const Headers &headers) {
  const auto &wc = Send({.method = "GET", .headers = headers});
  const string &c = wc.GetContentText();
  return ParseBuckets(wc.GetContentText());
}

//------------------------------------------------------------------------------
AccessControlPolicy S3Api::GetBucketAcl(const string &bucket) {
  const auto &c =
      Send({.method = "GET", .bucket = bucket, .params = {{"acl", ""}}})
          .GetContentText();
  return ParseACL(c);
}

//------------------------------------------------------------------------------
void S3Api::PutBucketAcl(const string &bucket, const AccessControlPolicy &acl) {
  const string xml = GenerateAclXML(acl);
  const auto &c = Send({.method = "PUT",
                        .bucket = bucket,
                        .params = {{"acl", ""}},
                        .uploadData = xml})
                      .GetContentText();
}
//------------------------------------------------------------------------------
TagMap ParseTaggingResponse(const std::string &xml);
TagMap S3Api::GetBucketTagging(const string &bucket) {
  auto r =
      Send({.method = "GET", .bucket = bucket, .params = {{"tagging", ""}}})
          .GetContentText();
  return ParseTaggingResponse(r);
}

//------------------------------------------------------------------------------
S3Api::SendParams GeneratePutBucketTaggingRequest(const std::string &bucket,
                                                  const TagMap &tags,
                                                  const Headers &headers);
void S3Api::PutBucketTagging(const string &bucket, const TagMap &tags,
                             const Headers &headers) {
  auto r = GeneratePutBucketTaggingRequest(bucket, tags, headers);
  Send(r);
}

//------------------------------------------------------------------------------
void S3Api::DeleteBucketTagging(const std::string &bucket,
                                const Headers &headers) {
  Send({.method = "DELETE",
        .bucket = bucket,
        .params = {{"tagging", ""}},
        .headers = headers});
}

//-----------------------------------------------------------------------------
void S3Api::PutBucketVersioning(const string &bucket, bool enabled) {
  const string req = string("<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
                            "<VersioningConfiguration xmlns="
                            "\"http://s3.amazonaws.com/doc/2006-03-01/\">"
                            "<Status>") +
                     (enabled ? "Enabled" : "Suspended") +
                     "</Status></VersioningConfiguration>";

  Send({.method = "PUT",
        .bucket = bucket,
        .params = {{"versioning", ""}},
        .uploadData = req});
}

//-----------------------------------------------------------------------------
S3Api::VersioningInfo S3Api::GetBucketVersioning(const string &bucket) {
  auto xml =
      Send({.method = "GET", .bucket = bucket, .params = {{"versioning", ""}}})
          .GetContentText();
  return {ToLower(XMLTag(xml, "status")) == "enabled",
          ToLower(XMLTag(xml, "mfadelete")) == "enabled"};
}
} // namespace api
} // namespace sss
