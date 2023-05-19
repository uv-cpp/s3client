#include "s3-api.h"
#include "tinyxml2.h"
#include "xml_path.h"
#include <cassert>
#include <stack>

using namespace std;
using namespace tinyxml2;

namespace sss {
namespace api {

//-----------------------------------------------------------------------------
// XML generator @todo expose externally
//-----------------------------------------------------------------------------
class XMLOStream {
public:
  enum MoveAction { UP, DOWN, REWIND };
  void Up(int level = 1) {
    if (!cur_) {
      throw logic_error("Cannot pop, Null element");
    }
    if (!cur_->Parent()) {
      throw logic_error("Cannot pop, Null parent");
    }
    for (; level; cur_ = cur_->Parent()->ToElement(), level--)
      ;
  }
  void Down() { down_ = true; }

  void Rewind() {
    if (!cur_) {
      return;
    }
    for (; cur_->Parent();
         cur_ = cur_->Parent() ? cur_->Parent()->ToElement() : nullptr)
      ;
  }
  XMLOStream &InsertText(const std::string &text) {
    cur_->SetText(text.c_str());
    return *this;
  }
  XMLOStream &Insert(const std::string &s) {
    if (!cur_) {
      cur_ = doc_.NewElement(s.c_str());
      doc_.InsertFirstChild(cur_);
      down_ = false;
    } else {
      auto e = cur_->InsertNewChildElement(s.c_str());
      if (down_) {
        cur_ = e;
        down_ = false;
      }
    }
    return *this;
  }
  XMLOStream &Move(MoveAction a, int level = 1) {
    switch (a) {
    case UP:
      Up(level);
      break;
    case DOWN:
      Down();
      break;
    case REWIND:
      Rewind();
      break;
    default:
      break;
    }
    return *this;
  }

  XMLOStream &operator[](const std::string &s) {
    if (all_of(begin(s), end(s), [](auto c) { return c == '/'; })) {
      Up(s.size());
      return *this;
    }
    down_ = true;
    return Insert(s);
  }
  XMLOStream &operator[](MoveAction a) { return Move(a); }
  XMLOStream(XMLDocument &d) : doc_(d) {}
  XMLOStream &operator=(const std::string &s) {
    InsertText(s);
    Up();
    return *this;
  }

  operator std::string() { return XMLToText(doc_, true, 0, 0); }

  const XMLDocument &GetDocument() const { return doc_; }

private:
  XMLDocument &doc_;
  XMLElement *cur_ = nullptr;
  bool down_ = false;
};
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
    const Grant g = {{select("grantee/displayname"),
                      select("grantee/emailaddress"), select("grantee/id"),
                      select("grantee/type"), select("grantee/uri")},
                     select("permission")};
    res.grants.push_back(g);
  }
  return res;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
std::string GenerateAclXML(const AccessControlPolicy &acl) {
  XMLDocument doc;
  XMLOStream os(doc);
  os["accesscontrolpolicy"]["accesscontrollist"];
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
  }
  os["/"]; // </owner>
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
  return os;
}
} // namespace api
} // namespace sss
