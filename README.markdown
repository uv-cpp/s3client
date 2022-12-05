# S3 toolkit

Set of C++ libraries and command line utilities to sign URLs and headers
and send S3 REST requests.

* `s3-client`: send raw requests
* `s3-presign`: generate pre-signed `URL`
* `s3-upload`: parallel upload
* `s3-download`: parallel download

The `s3-client` is a very low level interface which can log the raw XML/JSON
requests and responses.

The upload/download tools work best when reading/writing from SSDs or RAID &
parallel file-systems with `stripe size = chunk size`.

The upload and download applications read credentials from the standard AWS
configuration file in the user's home directory or from env variables.

Note that these tools have the ability to use a URL for signing the request
which can be different from the endpoint, which means that they work across
SSH tunnels and *netcat* bridges, not the case with other clients.
The `aws` cli tool does have an `ssm` option to create a tunnel, but it relies
on a complex mechanism which requires describing the instance first by
invoking an `ec2` command and it won't work with standalone *Ceph*
deployments.

Originally developed to test *Ceph* object storage.

The default *CMake* configuration generates static executables on Linux.

The code is `C++17` compliant.

Use `git clone --recurse-submodules` to download dependencies.

The *Portable Hashing Library* code is copied to a local path (`dep/hash`) because the original
version does not compile on *MacOS* and the pull requests were not accepted.
The plan is to replace the current hash library with:
 *  https://github.com/h5p9sl/hmac_sha256
 *  https://github.com/amosnier/sha-2

## License

This software is distributed under the BSD three-clause license and has
dependencies on the following software libraries:

* libcurl - distributed unded the curl license, derived from MIT/X
* Lyra, by Rene Rivera - distributed under the Boost license version 1.0
* Portable Hashing Library, by Stephan Brumme - distributed under the
  zlib license

## Sending S3 requests

The `s3-client` application is just a wrapper around the `sss::SendS3Request` function.
A `lib-s3-client` library will be provided in the future, but for the time being, source
code needs to be compiled together with the application.

E.g.

```cmake
set(S3_CLIENT_SRCS "s3-client.cpp" url_utility.cpp aws_sign.cpp 
    webclient.cpp utility.cpp lib-s3-client.cpp)
```

### List bucket content

Command line:

```sh
bin/debug/s3-client -a $S3TEST_ACCESS -s $S3TEST_SECRET -e $S3TEST_URL -b bucket1 -m get
```

C++:

```cpp
#include "lib-s3-client.h"

...
S3Args s3args;
s3args.s3AccessKey = "Naowkmo0786XzwrF";
s3args.s3SecretKey = "gpM6KV0Andtn2myBWePjoHXmSBy71UDK";
s3args.endpoint = "http://127.0.0.1:9000";
s3args.bucket = "bucket1" 
s3args.method = "GET";

sss::WebClient req = sss::SendS3Request(s3args);

cout << req.GetResponseBody() << endl;
```
Output:

```sh
<?xml version="1.0" encoding="UTF-8"?>
<ListBucketResult xmlns="http://s3.amazonaws.com/doc/2006-03-01/">
   <Name>bucket1</Name>
   <Prefix />
   <Marker />
   <MaxKeys>1000</MaxKeys>
   <Delimiter />
   <IsTruncated>false</IsTruncated>
   <Contents>
      <Key>data</Key>
      <LastModified>2022-12-05T06:00:06.188Z</LastModified>
      <ETag>"d41d8cd98f00b204e9800998ecf8427e"</ETag>
      <Size>0</Size>
      <Owner>
         <ID>02d6176db174dc93cb1b899f7c6078f08654445fe8cf1b6ce98d8855f66bdbf4</ID>
         <DisplayName>minio</DisplayName>
      </Owner>
      <StorageClass>STANDARD</StorageClass>
   </Contents>
   <Contents>
      <Key>data2</Key>
      <LastModified>2022-12-05T06:57:48.013Z</LastModified>
      <ETag>"3a54774d91ce84eb33f49681664f93ba"</ETag>
      <Size>4</Size>
      <Owner>
         <ID>02d6176db174dc93cb1b899f7c6078f08654445fe8cf1b6ce98d8855f66bdbf4</ID>
         <DisplayName>minio</DisplayName>
      </Owner>
      <StorageClass>STANDARD</StorageClass>
   </Contents>
   <Contents>
      <Key>data3</Key>
      <LastModified>2022-12-05T07:36:38.611Z</LastModified>
      <ETag>"e2fc714c4727ee9395f324cd2e7f331f"</ETag>
      <Size>4</Size>
      <Owner>
         <ID>02d6176db174dc93cb1b899f7c6078f08654445fe8cf1b6ce98d8855f66bdbf4</ID>
         <DisplayName>minio</DisplayName>
      </Owner>
      <StorageClass>STANDARD</StorageClass>
   </Contents>
   <Contents>
      <Key>object</Key>
      <LastModified>2022-12-03T03:24:52.893Z</LastModified>
      <ETag>"79c1833d66d63cf1d4f4b2284ac1f9b2"</ETag>
      <Size>1050621</Size>
      <Owner>
         <ID>02d6176db174dc93cb1b899f7c6078f08654445fe8cf1b6ce98d8855f66bdbf4</ID>
         <DisplayName>minio</DisplayName>
      </Owner>
      <StorageClass>STANDARD</StorageClass>
   </Contents>
   <Contents>
      <Key>presign</Key>
      <LastModified>2022-12-03T02:54:16.526Z</LastModified>
      <ETag>"b22ca8f1ddff8ec023cb0d074f7da15e"</ETag>
      <Size>277175</Size>
      <Owner>
         <ID>02d6176db174dc93cb1b899f7c6078f08654445fe8cf1b6ce98d8855f66bdbf4</ID>
         <DisplayName>minio</DisplayName>
      </Owner>
      <StorageClass>STANDARD</StorageClass>
   </Contents>
</ListBucketResult>
```

### Extract bytes 100-150 from object

Command line:

```sh
bin/debug/s3-client -a $S3TEST_ACCESS -s $S3TEST_SECRET -e $S3TEST_URL -b bucket1 -k object -m get -H "Range:bytes=100-150"
```

C++:
```cpp
#include "lib-s3-client.h"

...
S3Args s3args;
s3args.s3AccessKey = "Naowkmo0786XzwrF";
s3args.s3SecretKey = "gpM6KV0Andtn2myBWePjoHXmSBy71UDK";
s3args.endpoint = "http://127.0.0.1:9000";
s3args.bucket = "bucket1" 
s3args.method = "GET";
s3args.headers =  "Range:bytes=100-150";


sss::WebClient req = sss::SendS3Request(s3args);

cout << "Status: " << req.StatusCode() << endl << endl;
// Response body
vector<uint8_t> resp = req.GetResponseBody();
string t(begin(resp), end(resp));
cout << t << endl << endl;
// Response header
vector<uint8_t> h = req.GetResponseHeader();
string hs(begin(h), end(h));
cout << hs << endl;

```

Output:

```sh
Status: 0

!; @:OQ]qjlr
            TlGKP{
                  q

HTTP/1.1 206 Partial Content
Accept-Ranges: bytes
Content-Length: 51
Content-Range: bytes 100-150/1050621
Content-Security-Policy: block-all-mixed-content
Content-Type: binary/octet-stream
ETag: "79c1833d66d63cf1d4f4b2284ac1f9b2"
Last-Modified: Sat, 03 Dec 2022 03:24:52 GMT
Server: MinIO
Strict-Transport-Security: max-age=31536000; includeSubDomains
Vary: Origin
Vary: Accept-Encoding
X-Amz-Request-Id: 172DD7623C30EFE8
X-Content-Type-Options: nosniff
X-Xss-Protection: 1; mode=block
Date: Mon, 05 Dec 2022 08:10:53 GMT
```

### Store data into variable/object

Command line:

```sh
bin/debug/s3-client -a $S3TEST_ACCESS -s $S3TEST_SECRET -e $S3TEST_URL -b bucket1 -k variable_name -v "10,20,30" -m put
```

```cpp
#include "lib-s3-client.h"

...
S3Args s3args;
s3args.s3AccessKey = "Naowkmo0786XzwrF";
s3args.s3SecretKey = "gpM6KV0Andtn2myBWePjoHXmSBy71UDK";
s3args.endpoint = "http://127.0.0.1:9000";
s3args.bucket = "bucket1";
s3args.key = "variable_name"
s3args.method = "PUT";
s3args.data = "10,20,30";  

sss::WebClient req = sss::SendS3Request(s3args);

cout << "Status: " << req.StatusCode() << endl << endl;
// Response body
vector<uint8_t> resp = req.GetResponseBody();
string t(begin(resp), end(resp));
cout << t << endl << endl;
// Response header
vector<uint8_t> h = req.GetResponseHeader();
string hs(begin(h), end(h));
cout << hs << endl;
```

Output

```
Status: 0


HTTP/1.1 100 Continue

HTTP/1.1 200 OK
Accept-Ranges: bytes
Content-Length: 0
Content-Security-Policy: block-all-mixed-content
ETag: "9dac2971b753257ec5588962fca0503a"
Server: MinIO
Strict-Transport-Security: max-age=31536000; includeSubDomains
Vary: Origin
Vary: Accept-Encoding
X-Amz-Request-Id: 172DD80A53A91EB8
X-Content-Type-Options: nosniff
X-Xss-Protection: 1; mode=block
Date: Mon, 05 Dec 2022 08:22:55 GMT
```
The variable type can e.g. be stored into the metadata by sending an `x-amz-meta-type:IntArray` header together with the request.



