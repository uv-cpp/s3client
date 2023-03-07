#include "s3-api.h"
#include "tinyxml2.h"
#include <cassert>

using namespace std;
using namespace tinyxml2;

namespace sss {
namespace api {

//------------------------------------------------------------------------------
std::vector<BucketInfo> ParseBuckets(const std::string &xml) {
  XMLDocument doc;
  assert(doc.Parse(xml.c_str()) == XML_SUCCESS);
  //<xml...>
  const XMLNode *pRoot = doc.FirstChild();
  //<ListAllMyBucketsResult>
  pRoot = pRoot->NextSibling();
  if (!pRoot)
    return {};
  const XMLElement *pBuckets = pRoot->FirstChildElement("Buckets");
  if (!pBuckets)
    return {};
  const XMLElement *pElement = pBuckets->FirstChildElement("Bucket");
  if (!pBuckets)
    return {};
  vector<BucketInfo> bi;
  do {
    const XMLElement *pDate = pElement->FirstChildElement("CreationDate");
    const string creationDate = pDate ? pDate->GetText() : "";
    const XMLElement *pName = pElement->FirstChildElement("Name");
    const string name = pName ? pName->GetText() : "";
    bi.push_back({.name = name, .creationDate = creationDate});
  } while ((pElement = pElement->NextSiblingElement()));
  return bi;
}

//------------------------------------------------------------------------------
std::vector<ObjectInfo> ParseObjects(const std::string &xml) {
  XMLDocument doc;
  doc.Parse(xml.c_str());
  //<xml...>
  const XMLNode *pRoot = doc.FirstChild();
  //<ListBucketResult>
  pRoot = pRoot->NextSibling();
  if (!pRoot)
    return {};
  // const XMLElement* pTrunc = pRoot->FirstChildElement("IsTruncated");
  // bool truncated = false;
  // if(pTrunc) pTrunc->QueryBoolText(&truncated);
  const XMLElement *pElement = pRoot->FirstChildElement("Contents");
  if (!pElement)
    return {};
  vector<ObjectInfo> oi;
  do {
    ObjectInfo obj;
    const XMLElement *e = pElement->FirstChildElement("ChecksumAlgorithm");
    obj.checksumAlgo = e ? e->GetText() : "";
    e = pElement->FirstChildElement("ETag");
    obj.etag = e ? e->GetText() : "";
    e = pElement->FirstChildElement("Key");
    obj.key = e ? e->GetText() : "";
    e = pElement->FirstChildElement("LastModified");
    obj.lastModified = e ? e->GetText() : "";
    e = pElement->FirstChildElement("Size");
    obj.size = e ? stoul(e->GetText()) : 0;
    e = pElement->FirstChildElement("Owner");
    if (e) {
      e = pElement->FirstChildElement("DisplayName");
      obj.ownerDisplayName = e ? e->GetText() : "";
      e = pElement->FirstChildElement("ID");
      obj.ownerID = e ? e->GetText() : "";
    }
    oi.push_back(obj);

  } while ((pElement = pElement->NextSiblingElement()));
  return oi;
}
// std::vector<PartInfo> ParseParts(const std::string& xml);
} // namespace api
} // namespace sss

// ListBuckets output:
//
// <?xml version="1.0" encoding="UTF-8"?>
// <ListAllMyBucketsResult>
//    <Buckets>
//       <Bucket>
//          <CreationDate>timestamp</CreationDate>
//          <Name>string</Name>
//       </Bucket>
//    </Buckets>
//    <Owner>
//       <DisplayName>string</DisplayName>
//       <ID>string</ID>
//    </Owner>
// </ListAllMyBucketsResult>

// LisObjectsV2 output:
//
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
