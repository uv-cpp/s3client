#include "s3-api.h"
#include "tinyxml2.h"
#include "xml_path.h"
#include <cassert>

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
  auto d = DOMToDict(xml);
  if (d.empty()) {
    return {};
  }
  S3Api::ListObjectV2Result res;
  auto truncated = FindElementText(xml, "istruncated");
  res.truncated = ParseBool(truncated);
  const string prefix = "/listbucketresult/contents";
  auto r = RecordList(prefix, d);
  vector<ObjectInfo> v;
  for (const auto &i : r) {
    auto select = [&](const string &s) {
      const auto pre = "/" + s;
      const auto &it = i.find(pre);
      if (it == i.cend()) {
        return string("");
      }
      return it->second;
    };
    res.keys.push_back(
        {.checksumAlgo = select("checksumalgo"),
         .key = select("key"),
         .lastModified = select("lastmodified"),
         .etag = select("etag"),
         .size = select("size").empty() ? 0 : stoul(select("size")),
         .storageClass = select("storageclass"),
         .ownerDisplayName = select("owner/displayname"),
         .ownerID = select("owner/id")});
  }
  return res;
}

//-----------------------------------------------------------------------------
// HTTP/1.1 200
// <?xml version="1.0" encoding="UTF-8"?>
// <AccessControlPolicy>
//    <Owner>
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

//------------------------------------------------------------------------------
AccessControlPolicy ParseACL(const std::string &xml) {
  if (xml.empty())
    return {};
  auto d = DOMToDict(xml);
  if (d.empty()) {
    return {};
  }
  AccessControlPolicy res;
  auto get = [&](string key) {
    key = "/accesscontrolpolicy/" + key;
    return d.count(key) ? d[key].front() : "";
  };
  res.ownerDisplayName = get("/owner/displayname");
  res.ownerID = get("/owner/id");
  auto r = RecordList("/accesscontrolpolicy/accesscontrollist/grant", d);
  for (const auto &i : r) {
    auto select = [&](const string &s) {
      const auto pre = "/" + s;
      const auto &it = i.find(pre);
      if (it == i.cend()) {
        return string("");
      }
      return it->second;
    };
    res.permission = i.count("/permission") ? i.at("/permission") : "";
    const Grantee g = {select("grantee/displayname"),
                       select("grantee/emailaddress"), select("grantee/id"),
                       select("grantee/type"), select("grantee/uri")};
    res.grants.push_back(g);
  }
  return res;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
class XMLStream {
public:
  enum StackAction { POP, PUSH, REWIND };
  void Pop(int level = 1) {
    for (; level; cur_ = cur_->Parent()->ToElement(), level--)
      ;
  }
  void Push() { push_ = true; }

  void Rewind() {
    if (!cur_) {
      return;
    }
    for (; cur_->Parent();
         cur_ = cur_->Parent() ? cur_->Parent()->ToElement() : nullptr)
      ;
  }
  XMLStream &InsertText(const std::string &text) {
    cur_->InsertNewText(text.c_str());
    return *this;
  }
  XMLStream &Insert(const std::string &s) {
    if (!cur_) {
      cur_ = doc_.NewElement(s.c_str());
    } else {
      auto e = cur_->InsertNewChildElement(s.c_str());
      if (push_) {
        cur_ = e;
        push_ = false;
      }
    }
    return *this;
  }
  XMLStream &Insert(StackAction a, int level = 1) {
    switch (a) {
    case POP:
      Pop(level);
      break;
    case PUSH:
      Push();
      break;
    case REWIND:
      Rewind();
      break;
    default:
      break;
    }
    return *this;
  }

  XMLStream &operator[](const std::string &s) { return Insert(s); }
  XMLStream &operator[](int i) {
    switch (i) {
    case 1:
      Push();
      break;
    case 0:
      Rewind();
      break;
    case -1:
      Pop();
      break;
    default:
      break;
    }
    return *this;
  }
  XMLStream &operator[](StackAction a) { return Insert(a); }
  XMLStream &operator[](std::pair<StackAction, int> s) {
    return Insert(s.first, s.second);
  }
  XMLStream(XMLDocument &d) : doc_(d) {}
  XMLStream &operator=(const std::string &s) { return InsertText(s); }

  operator std::string() { return XMLToText(doc_); }

private:
  XMLDocument &doc_;
  XMLElement *cur_ = nullptr;
  bool push_ = false;
};

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

std::string GenerateAclXML(const AccessControlPolicy &acl) {
  XMLDocument doc;
  XMLStream os(doc);
  os["accesscontrolpolicy"]["accesscontrollist"];
  for (const auto &i : acl.grants) {
    os["Grant"];
    if (!i.displayName.empty()) {
      (os["grantee"]["displayname"] = i.displayName)[-1];
    }
    if (!i.emailAddress.empty()) {
      (os["grantee"]["emailaddress"] = i.emailAddress)[-1];
    }
    if (!i.id.empty()) {
      (os["grantee"]["id"] = i.id)[-1];
    }
    if (!i.xsiType.empty()) {
      (os["grantee"]["type"] = i.xsiType)[-1];
    }
    if (!i.uri.empty()) {
      (os["grantee"]["uri"] = i.uri)[-1];
    }
    os["permission"] = i
  }
  return os;
}
} // namespace api
} // namespace sss
