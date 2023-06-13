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

#include "s3-api.h"
#include "tinyxml2.h"
#include "xml_path.h"
#include "xmlstreams.h"

#include <variant>

using namespace std;
using namespace tinyxml2;

namespace sss {
namespace api {

//------------------------------------------------------------------------------
std::vector<BucketInfo> ParseBuckets(const std::string &xml) {
  if (xml.empty())
    return {};
  auto d = DOMToDict(xml);
  if (d.empty()) {
    return {};
  }
  // always use lowercase when case insensitive (default) enabled
  auto r = ExtractSubPaths("/listallmybucketsresult/buckets", d);
  if (r.empty()) {
    return {};
  }
  vector<BucketInfo> ret;
  auto numBuckets = r["/bucket/creationdate"].size();
  if (numBuckets != r["/bucket/name"].size()) {
    throw std::logic_error("Malformed response: number of creation dates does "
                           "not match number of buckets");
    return {};
  }
  for (size_t i = 0; i != numBuckets; ++i) {
    ret.push_back({.name = r["/bucket/name"][i],
                   .creationDate = r["/bucket/creationdate"][i]});
  }
  return ret;
}

std::string Trim(const std::string &text) {
  auto b = cbegin(text);
  while (isspace(*b) && (b++ != cend(text)))
    ;
  auto e = --cend(text);
  while (isspace(*e) && (e-- != cbegin(text)))
    ;
  return std::string(b, ++e);
}

bool ParseBool(const string &s) {
  if (s.empty())
    return false;
  return ToLower(Trim(s)) == "true";
}

//------------------------------------------------------------------------------
// <ListBucketResult>
//    <IsTruncated>boolean</IsTruncated>
//    <Contents>
//       <ChecksumAlgorithm>string</ChecksumAlgorithm>
//       ...
//       <ETag>string</ETag>
//       <Key>string</Key>
//       <LastModified>timestamp</LastModified>
//       <Owner>
//          <DisplayName>string</DisplayName>
//          <ID>string</ID>
//       </Owner>
//       <Size>integer</Size>
//       <StorageClass>string</StorageClass>
//    </Contents>
//    ...
//    <Name>string</Name>
//    <Prefix>string</Prefix>
//    <Delimiter>string</Delimiter>
//    <MaxKeys>integer</MaxKeys>
//    <CommonPrefixes>
//       <Prefix>string</Prefix>
//    </CommonPrefixes>
//    ...
//    <EncodingType>string</EncodingType>
//    <KeyCount>integer</KeyCount>
//    <ContinuationToken>string</ContinuationToken>
//    <NextContinuationToken>string</NextContinuationToken>
//    <StartAfter>string</StartAfter>
// </ListBucketResult>
S3Api::ListObjectV2Result ParseObjects(const std::string &xml) {
  if (xml.empty())
    return {};
  XMLIStream is(xml);
  S3Api::ListObjectV2Result res;
  const string truncated = is["/istruncated"];
  res.truncated = ParseBool(truncated);
  XMLRecords r = is["listbucketresult/contents"];
  vector<ObjectInfo> v;
  for (const auto &i : r) {
    res.keys.push_back(
        {.checksumAlgo = Get(i, "/checksumalgo"),
         .key = Get(i, "/key"),
         .lastModified = Get(i, "/lastmodified"),
         .etag = Get(i, "/etag"),
         .size = Get(i, "/size").empty() ? 0 : stoul(Get(i, "/size")),
         .storageClass = Get(i, "/storageclass"),
         .ownerDisplayName = Get(i, "/owner/displayname"),
         .ownerID = Get(i, "/owner/id")});
  }
  return res;
}

//-----------------------------------------------------------------------------
// HTTP/1.1 200
// <?xml version="1.0" encoding="UTF-8"?>
// <AccessControlPolicy>
//       <DisplayName>string</DisplayName>
//       <ID>string</ID>
//    </Owner>
//    <AccessControlList>
//       <Grant>
//          <Grantee>
//             <DisplayName>string</DisplayName>
//             <EmailAddress>string</EmailAddress>
//             <ID>string</ID>
//             <xsi:type>string</xsi:type>
//             <URI>string</URI>
//          </Grantee>
//          <Permission>string</Permission>
//       </Grant>
//    </AccessControlList>
// </AccessControlPolicy>

AccessControlPolicy ParseACL(const std::string &xml) {
  if (xml.empty())
    return {};
  XMLIStream is(xml);
  AccessControlPolicy res;
  const string prefix = "/accesscontrolpolicy";
  res.ownerDisplayName = is[prefix + "/owner/displayname"];
  res.ownerID = is[prefix + "/owner/id"];
  XMLRecords r = is["accesscontrolpolicy/accesscontrollist/grant"];
  for (auto &i : r) {
    const Grant g = {{Get(i, "/grantee/displayname"),
                      Get(i, "/grantee/emailaddress"), Get(i, "/grantee/id"),
                      Get(i, "/grantee/type"), Get(i, "/grantee/uri")},
                     Get(i, "/permission")};
    res.grants.push_back(g);
  }
  return res;
}
//-----------------------------------------------------------------------------
/// [GenerateAclXML]
std::string GenerateAclXML(const AccessControlPolicy &acl) {
  XMLDocument doc;
  XMLOStream os(doc);
  os["accesscontrolpolicy/accesscontrollist"];
  if (!acl.ownerDisplayName.empty() || !acl.ownerID.empty()) {
    os["owner"]; // <accesscontrolpolicy><accesscontrollist><owner>
    if (!acl.ownerDisplayName.empty()) {
      //<accesscontrolpolicy><accesscontrollist><owner><displayname>
      os["displayname"] = acl.ownerDisplayName;
      //</displayname> automatically move to upper level after assignment
    }
    if (!acl.ownerID.empty()) {
      //<accesscontrolpolicy><accesscontrollist><owner><ownerid>
      os["ownerid"] = acl.ownerDisplayName;
      //</ownerid> automatically move to upper level after assignment
    }
    os["/"]; // </owner>
  }
  for (const auto &g : acl.grants) {
    if (g.permission.empty()) {
      throw logic_error("Missing required field 'permission'");
    }
    os["grant"]; // <grant>
    const Grant::Grantee &i = g.grantee;
    if (!i.Empty()) {
      os["grantee"]; // <grant><grantee>
      if (!i.displayName.empty()) {
        // <grant><grantee><displayname>
        os["displayname"] = i.displayName;
        // </displayname>
      }
      if (!i.emailAddress.empty()) {
        // <grant><grantee><emailaddress>
        os["emailaddress"] = i.emailAddress;
        // </emailaddress>
      }
      if (!i.id.empty()) {
        // <grant><grantee><id>
        os["id"] = i.id;
        // </id>
      }
      if (!i.xsiType.empty()) {
        // <grant><grantee><type>``
        os["type"] = i.xsiType;
        // </type>
      }
      if (!i.uri.empty()) {
        // <grant><grantee><uri>
        os["uri"] = i.uri;
        // <uri>
      }
    }
    os["/"] // </grantee>
            // <grant><permission>
      ["permission"] = g.permission;
    // </permission>
  }
  return os.XMLText();
}
/// [GenerateAclXML]

//-----------------------------------------------------------------------------
S3Api::SendParams GeneratePutBucketTaggingRequest(const std::string &bucket,
                                                  const TagMap &tags,
                                                  const Headers &headers = {}) {
  XMLDocument doc;
  XMLOStream os(doc);
  //@warning: different S3 implementations might have different capitalisation
  // requirements e.g. MINIO version RELEASE.2022-11-17T23-20-09Z
  // supports lowercase for ListObjectsV2 but requires CamelCase for tagging!!
  os["Tagging/TagSet"]; // <Tagging><TagSet>
  for (auto kv : tags) {
    os["Tag"];               // <Tag>
    os["Key"] = kv.first;    // <Key>
    os["Value"] = kv.second; // <Value>
    os["/"];                 // </Tag>
  }
  return {.method = "PUT",
          .bucket = bucket,
          .params = {{"tagging", ""}},
          .headers = headers,
          .uploadData = os.XMLText()};
}

//-----------------------------------------------------------------------------
TagMap ParseTaggingResponse(const std::string &xml) {
  if (xml.empty()) {
    return {};
  }
  XMLIStream is(xml);
  // return all the <tag></tag> elements
  XMLRecords r = is["tagging/tagset/tag"];
  TagMap m;
  for (auto i : r) {
    const auto k = Get(i, "/key");
    if (k.empty()) {
      continue;
    }
    const auto v = Get(i, "/value");
    if (v.empty()) {
      continue;
    }
    m[k] = v;
  }
  return m;
}

//-----------------------------------------------------------------------------
S3Api::SendParams GeneratePutObjectTaggingRequest(const std::string &bucket,
                                                  const std::string &key,
                                                  const TagMap &tags,
                                                  const Headers &headers) {
  XMLDocument doc;
  XMLOStream os(doc);
  //@warning: different S3 implementations might have different capitalisation
  // requirements e.g. MINIO version RELEASE.2022-11-17T23-20-09Z
  // supports lowercase for ListObjectsV2 but requires CamelCase for tagging!!
  os["Tagging/TagSet"]; // <Tagging><TagSet>
  for (auto kv : tags) {
    os["Tag"];               // <Tag>
    os["Key"] = kv.first;    // <Key>
    os["Value"] = kv.second; // <Value>
    os["/"];                 // </Tag>
  }
  return {.method = "PUT",
          .bucket = bucket,
          .key = key,
          .params = {{"tagging", ""}},
          .headers = headers,
          .uploadData = os.XMLText()};
}

//-----------------------------------------------------------------------------
// <ListVersionsResult>
//    <IsTruncated>boolean</IsTruncated>
//    <KeyMarker>string</KeyMarker>
//    <VersionIdMarker>string</VersionIdMarker>
//    <NextKeyMarker>string</NextKeyMarker>
//    <NextVersionIdMarker>string</NextVersionIdMarker>
//    <Version>
//       <ChecksumAlgorithm>string</ChecksumAlgorithm>
//       ...
//       <ETag>string</ETag>
//       <IsLatest>boolean</IsLatest>
//       <Key>string</Key>
//       <LastModified>timestamp</LastModified>
//       <Owner>
//          <DisplayName>string</DisplayName>
//          <ID>string</ID>
//       </Owner>
//       <Size>integer</Size>
//       <StorageClass>string</StorageClass>
//       <VersionId>string</VersionId>
//    </Version>
//    ...
//    <DeleteMarker>
//       <IsLatest>boolean</IsLatest>
//       <Key>string</Key>
//       <LastModified>timestamp</LastModified>
//       <Owner>
//          <DisplayName>string</DisplayName>
//          <ID>string</ID>
//       </Owner>
//       <VersionId>string</VersionId>
//    </DeleteMarker>
//    ...
//    <Name>string</Name>
//    <Prefix>string</Prefix>
//    <Delimiter>string</Delimiter>
//    <MaxKeys>integer</MaxKeys>
//    <CommonPrefixes>
//       <Prefix>string</Prefix>
//    </CommonPrefixes>
//    ...
//    <EncodingType>string</EncodingType>
// </ListVersionsResult>

pair<vector<string>, vector<string>>
ParseListObjectVersions(const string &xml) {
  XMLIStream is(xml);
  vector<string> versions;
  XMLRecords vs = is["ListVersionsResult/Version"];
  for (auto i : vs) {
    const auto v = Get(i, "/VersionId");
    if (!v.empty()) {
      versions.push_back(v);
    }
  }
  XMLRecords ds = is["ListVersionsResult/DeleteMarker"];
  vector<string> deleteMarkers;
  for (auto i : ds) {
    const auto d = Get(i, "/VersionId");
    if (!d.empty()) {
      deleteMarkers.push_back(d);
    }
  }
  return {versions, deleteMarkers};
}
} // namespace api
} // namespace sss
