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
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *******************************************************************************/
#include "response_parser.h"
#include "xml_path.h"
#include <algorithm>
#include <cassert>
#include <iostream>
#include <iterator>

using namespace sss;
using namespace std;

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

void ParseXMLTagTest() {
  const char *xml = R"(
    <tag1>
      <tag1_1> tag1_1 </tag1_1>
      <tag1_2> tag1_2 </tag1_2>
    </tag1>
    )";
  assert(XMLTag(xml, "tag1_2") == "tag1_2");
}

void ParseXMLTagPathTest() {
  const char *xml = R"(
  <tag1>
    <tag1_1> 1_1 </tag1_1>
    <tag1_2> 1_2_1</tag1_2>
    <tag1_2><tag1_2_2>abc</tag1_2_2></tag1_2>
  </tag1>
  )";
  assert(XMLTagPath(xml, "tag1/tag1_2/tag1_2_2") == "abc");
}

void ParseXMLPathTest() {
  assert(ParseXMLPath(listBuckets,
                      "listallmybucketsresult/owner/displayname") == "minio");
}
void ParseXMLMultiPathTest() {
  // serch for <bucket> under <listallmybucketsresult>/<buckets> and return
  //<bucket>/<cretiondata> text elements
  auto el = ParseXMLMultiPathText(listBuckets, "listallmybucketsresult/buckets",
                                  "bucket/creationdate");
  assert(el.size() == 3);
  assert(el[0] == "2023-03-03t08:03:54.000z");
  assert(el[1] == "2023-03-01t08:47:15.843z");
  assert(el[2] == "2023-03-01t08:47:15.843z");
}

//@todo turn into test using shorter xml text
void PrintDOMToDict() {
  unordered_map<string, vector<string>> d = DOMToDict(listBuckets);
  for (auto kv : d) {
    cout << kv.first << endl;
    for (auto i : kv.second) {
      cout << "    " << i << endl;
    }
  }
  // extract buckets information
  auto r = ExtractRecord("/listallmybucketsresult/buckets", d);
  for (auto kv : r) {
    cout << kv.first << endl;
    for (auto i : kv.second) {
      cout << "    " << i << endl;
    }
  }
}

int main(int, char **) {
  cout << "XMLTag" << endl;
  ParseXMLTagTest();
  cout << "XMLTagPath" << endl;
  ParseXMLTagPathTest();
  cout << "ParseXMLTagPathTest" << endl;
  ParseXMLPathTest();
  cout << "ParseXMLTagMultiPathTest" << endl;
  ParseXMLMultiPathTest();
  // PrintDOMToDict();

  return 0;
}
