#include "response_parser.h"
#include <cassert>
#include <iostream>

using namespace sss;
using namespace std;

void ParseXMLTag() {
  const char *xml = R"(
  <tag1>
    <tag1_1> tag1_1 </tag1_1>
    <tag1_2> tag1_2 </tag1_2>
  </tag1)
  )";
  assert(XMLTag(xml, "tag1_2") == "tag1_2");
}

void ParseXMLTags() {
  const char *xml = R"(
  <tag1>
    <tag1_1> 1_1 </tag1_1>
    <tag1_2> 1_2_1</tag1_2>
    <tag1_2> <tag1_2_2> abc </tag1_2_2></tag1_2>
  </tag1)
  )";
  auto res = XMLTags(xml, "tag1_2");
  assert(res[0] == "1_2_1");
  assert(res[1] == "<tag1_2_2> abc </tag1_2_2>");
}

void ParseXMLTagPath() {
  const char *xml = R"(
  <tag1>
    <tag1_1> 1_1 </tag1_1>
    <tag1_2> 1_2_1</tag1_2>
    <tag1_2> <tag1_2_2> abc </tag1_2_2></tag1_2>
  </tag1)
  )";
  auto res = XMLTagPath(xml, "tag1/tag1_2/tag_1_2_2");
  cout << res << endl;
}

static const char *listBuckets = R"(
<?xml version="1.0" encoding="utf-8"?>
<listallmybucketsresult xmlns="http://s3.amazonaws.com/doc/2006-03-01/">
  <owner>
    <id>02d6176db174dc93cb1b899f7c6078f08654445fe8cf1b6ce98d8855f66bdbf4</id>
    <displayname>minio</displayname>
  </owner>
  <buckets>
    <bucket>
      <name>04raujrpup41xhrfj1vun4d50vjoi39z</name>
      <creationdate>2023-03-03t08:03:54.000z</creationdate>
    </bucket>
    <bucket>
      <name>0o1p8ftlxbf8iqrufbu8de5jbqxv6f0c</name>
      <creationdate>2023-03-01t08:47:15.843z</creationdate>
    </bucket>
    <bucket>
      <name>mybucket</name>
      <creationdate>2023-03-01t08:47:15.843z</creationdate>
    </bucket>
  </buckets>
</listallmybucketsresult>
)";
int main(int, char **) {
  ParseXMLTag();
  ParseXMLTags();
  ParseXMLTagPath();
  auto res = XMLTags(listBuckets, "bucket");
  for (auto i : res) {
    cout << i << endl;
    // cout << XMLTag(i, "name") << " " << XMLTag(i, "creationdate") << endl;
  }
  return 0;
}
