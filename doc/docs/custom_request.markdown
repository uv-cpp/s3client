# Creating custom requests

The provided functions and classes only support a basic subset of
the AWS; all the methods simply invoke the `S3Api::Send` method and return
the parsed response.

The main value of the current implementation (and other simlar C++ clients) 
is the generation and parsing of XML text in addition to S3v4 signing.
However, client code will most likely need to translate from the returned
objects to their own domain-specific internal representation and it might
therefore be easier to just parse the XML directly.

Any of the requests listed [here](https://docs.aws.amazon.com/AmazonS3/latest/API/API_Operations_Amazon_Simple_Storage_Service.html)
can be sent using this library by:

1. creating a function that returns
    - request parameters as an `S3Api::SendParams` instance
    - request body as text in XML formant if required
2. sending the request using the `S3Api::Send` method
3. creating a function that returns the parsed response body and HTTP headers
    - HTTP headers are already parsed and stored into a `map` object
    - for parsing the returned XML you can use the provided parsing functions
    or use a library like pugixml (XPath compliant, value semantics, supports
range based loops)
    - in addition to high-level parsing functions, tinyXML2 is included in the
    source tree, used internally, and can be used as well by client code

## Example: Bucket tagging

We wold like to add the ability to tag buckets using code like:

```cpp
  TagMap tags = {{"tag1", "value1"}, {"tag2", "value2"}};
  S3Api s(access, secret, endpointUrl);
  TagBucket(s3, "MyBucket", tags);
```
and read the tags with:

```cpp
  S3Api s3(access, secret, endpointUrl);
  TagMap tags = BucketTags(s3, bucket);
  assert(tags["tags1"] == "value1" && tags["tags2"] == "value2");
```

In the next section we are going to implement the 
`TagBucket` and `BucketTags` functions. 

### `TagBucket`: PutBucketTagging request

XML request body:
 
[AWS Action](https://docs.aws.amazon.com/AmazonS3/latest/API/API_PutBucketTagging.html) 

```xml
PUT /?tagging HTTP/1.1
Host: Bucket.s3.amazonaws.com
Content-MD5: ContentMD5
x-amz-sdk-checksum-algorithm: ChecksumAlgorithm
x-amz-expected-bucket-owner: ExpectedBucketOwner
<?xml version="1.0" encoding="UTF-8"?>
<Tagging xmlns="http://s3.amazonaws.com/doc/2006-03-01/">
   <TagSet>
      <Tag>
         <Key>string</Key>
         <Value>string</Value>
      </Tag>
   </TagSet>
</Tagging>
```
C++ implementation:

```cpp
using namespace sss;
using namespace api;
using XML = std::string;
using TagMap = std::unordered_map<std::string, std::string>;
```

```cpp
// return SendParams and XML request body
std::pair<SendParams, XML> 
GeneratePutBucketTaggingRequest(const std::string& bucket, 
                                const TagMap& tags,
                                const Headers& headers) {
  XMLDocument doc;
  XMLOStream os(doc);
  //@warning: different S3 implementations might have different capitalisation
  // requirements e.g. MINIO version RELEASE.2022-11-17T23-20-09Z
  // supports lowercase for ListObjectsV2 but requires CamelCase for tagging!!
  os["Tagging/TagSet"]; // <Tagging><TagSet>
  for(auto kv: tags) {
    os["Tag"]; // <Tag>
    os["Key"] = kv.first; // <Key>
    os["Value"] = kv.second; // <Value>
    os["/"]; // </Tag>
  }
  return {{.method = "PUT",
           .bucket = bucket,
           .params = {{"tagging", ""}},
           .headers = headers},
          os}; // automatic conversion to string  
}
```

```cpp
// throws std::logic_error or std::runtime_error
void TagBucket(S3Api& s3, const std::string& bucket,
               const TagMap& tags, const Headers& headers) {
  auto args = GeneratePutBucketRequest(bucket, tags, headers);
  args.uploadData = args.second.c_str();
  args.uploadDataSize = args.second.size();
  s3.Send(args.first, args.second);
}
```

### `BucketTags`: GetBucketTagging response

XML response body:

[AWS Action](https://docs.aws.amazon.com/AmazonS3/latest/API/API_GetBucketTagging.html)

```xml
HTTP/1.1 200
<?xml version="1.0" encoding="UTF-8"?>
<Tagging>
   <TagSet>
      <Tag>
         <Key>string</Key>
         <Value>string</Value>
      </Tag>
   </TagSet>
</Tagging>
```

C++ implementation:

```cpp
using namespace sss;
using namespace api;
using XML = std::string;
using TagMap = std::unordered_map<std::string, std::string>;
```

```cpp
// parse XML text and return {key, value} tag map
TagMap ParseTaggingResponse(const XML& xml) {
  if(xml.empty()) {
    return {};
  }
  XMLIStream is(xml);
  // return all the <tag></tag> elements
  XMLRecords r = is["tagging/tagset/tag"];
  TagMap m;
  for(auto i: r) {
    const auto k = Get(i,"/key");
    if(k.empty()) {
      continue;
    }
    const auto v = Get(i, "/value");
    if(v.empty()) {
      continue;
    }
    m[k] = v;
  }
  return m;
}

TagMap BucketTags(S3Api& s3, const std::string& bucket) {
  s3.SendRequest({.method = "GET", 
                  .bucket = bucket,
                  .params = {{"tagging", ""}});
  return ParseTaggingResponse(s3.GetBodyText());
}
```
