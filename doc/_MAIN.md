# S3 Toolkit {#mainpage}

C++ client library and tools.
Originally developed to test *Ceph* object storage and learn the S3 API.
All the code is tested on MacOS (x86 and ARM) and Linux SuSE and Ubuntu, x86 only. 


The following applications are available:

* `s3-client`: send raw requests
* `s3-presign`: generate pre-signed `URL`
* `s3-upload`: parallel upload
* `s3-download`: parallel download
* `s3-gen-credentials`: generate access and secret keys

Launch without arguments to see options.

The *libs3client* library includes a high level and an S3 API implementing a small subset
of the S3 Actions, however, every single action can be executed by invoking the
`Send` methods and functions and passing body, url parameters and headers.

All data transfer opeations are parallelised, and very soon an additional async interface
will be available, where each thead will move individual chunks of data by invoking async I/O operations.

When disabling S3v4 signing the library can be used as a generic HTTP client library.

A high level XML parsing library based on *Tinyxml2* is provided for simplifying
request generation and response parsing.


## Build and install

The `install.sh` script includes the code for checking out the code, building
and installing to a user specified path.
Just [download](https://raw.githubusercontent.com/uv-cpp/s3client/main/install.sh) and run.
```sh
bash ./install.sh <install path>
```
`libcurl` must be available for the code to build.

...or build it manually 
E.g. Building and installing a release version under `$HOME/.local` (default is `/usr/local`).

1. `git clone --recurse-submodules https://github.com/uv-cpp/s3client.git`
2. `mkdir -p s3client/build`
3. `cd s3client/build`
4. `cmake ../ -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=~/.local`;
5. `make -j 8`
6. `make install`

Include files are found under `${CMAKE_INSTALL_PREFIX}/include/s3client`.
The library is found under `${CMAKE_INSTALL_PREFIX}/lib` and the executables
under `${CMAKE_INSTALL_PREFIX}/bin`.

The `s3-client` tool is a low level interface which can log all the XML/JSON
requests and responses.  

Specify *URL* parameters and headers on the command line and data either through the command line 
or external files.

The upload/download tools work best when reading/writing from SSDs or RAID &
parallel file-systems with `stripe size = chunk size`.

The code is `C++17` compliant.

Requests are sent through a `WebClient` class which wraps `libcurl` and *XML*
responses are parsed using the standard regex library provided by the C++
compiler. 

### Compilation options

By default the library and all the command line tools and the S3 API extensions are built.
To disable building the command line tools set `APPS=OFF`.
To disable building the S3 API set `API=OFF`.

The tests currently cover the S3 API only and are built together the API.

## Test

In order to test the tools and API you need access to an S3 storage service.
One option is to use the free *play.min.io* service, another option is to 
configure a local instance of the *minio* server.

The access and secret keys for *play.min.io* are stored inside the
`~/.mc/config.json` files configured after installing the the
[minio client](https://min.io/docs/minio/linux/reference/minio-mc.html).

The script `minio_podman_setup.sh` downloads configures and runs a minio server instance
inside a contatiner using *podman*.

Run the script after building all the applications and making sure that
the `s3-gen-credentials` executable is findable through the `PATH` variable.

Usage
```sh
./minio_podman_setup.sh <minio alias> <data path>
```
Run
```sh
chmod u+x ./minio_podman_setup.sh
./minio_podman_setup.sh myalias ~/tmp/minio_data
```


## License

This software is distributed under the BSD three-clause license and has
dependencies on the following software libraries:

* *libcurl* - distributed unded the curl license, derived from MIT/X
* *Lyra*, by Rene Rivera - distributed under the Boost license version 1.0
* *Tinyxml2* - zlib (included in source tree)
* *Doxygen Awesome* - MIT 

## Sending S3 requests

The `s3-client` application is just a wrapper around the `sss::SendS3Request` function.

E.g.

```cmake
set(S3_CLIENT_SRCS "s3-client.cpp" url_utility.cpp aws_sign.cpp 
    webclient.cpp utility.cpp lib-s3-client.cpp)
```

Sample requests are shown below, look here for a complete list:

https://docs.aws.amazon.com/AmazonS3/latest/API/API_Operations.html

### List bucket content

Command line:

```sh
bin/debug/s3-client -a $S3TEST_ACCESS -s $S3TEST_SECRET -e $S3TEST_URL -b bucket1 -m get
```

C++:

```cpp
#include "s3-client.h"

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
#include "s3-client.h"
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

C++

```cpp
#include "s3-client.h"

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
