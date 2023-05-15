# S3 Toolkit

C++ client library and tools.
Originally developed to test *Ceph* object storage, parallel transfers between 
Lustre and *Ceph* on high speed (>= 100Gb/s) networks, and learn the S3 API.

All the code is tested on MacOS (x86 and ARM) and Linux SuSE and Ubuntu,
x86 only. 

## Library

* `libs3client`: high level functions to sign and send requests and perform 
parallel uploads/downloads.

S3 API accessible through the `libs3client` library, see `s3-api.h` and 
`s3-client.h` includes. 

Check out the `app` and `test` folders for usage examples.

A small subset of the S3 action is implemented, but any request can be sent 
through the `S3API::Send` method and `SendS3Request` function.

XML requests and responses can be generated and parsed using the provide 
high-level XML parsing and generation functions;
see `Parsing` module in the Doxygen-generated API documentation or look into the
`xml_path.h` and `test/xml-parse-test.cpp` files.

Parallel upload and download works best when reading/writing from SSDs or RAID &
parallel file-systems with `stripe size = N * (part size)`.

The code is `C++17` compliant.

When disabling S3v4 signing the library can be used as a generic HTTP client
library.

Work to enable multi-level asynchronous operations is in ongoing: each thread
will be using async I/O for sending multiple requests.

## Applications

* `s3-client`: send requests and print raw responses
* `s3-presign`: generate pre-signed `URL`
* `s3-upload`: (single file) parallel upload
* `s3-download`: (singe file) parallel download
* `s3-gen-credentials`: generate access and secret keys

Launch without arguments to see options.


## Build and install

The `install.sh` script includes the code for checking out the code, building
and installing to a user specified path.
Just [download](https://raw.githubusercontent.com/uv-cpp/s3client/main/install.sh)
and run.


```sh
bash ./install.sh <install path>
```

`libcurl` must be available for the code to build.

...or build it manually 

E.g. Building and installing a release version under `$HOME/.local`
(default is `/usr/local`):

1. `git clone --recurse-submodules https://github.com/uv-cpp/s3client.git`
2. `mkdir -p s3client/build`
3. `cd s3client/build`
4. `cmake ../ -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=~/.local`;
5. `make -j 8`
6. `make install`

Include files are found under `${CMAKE_INSTALL_PREFIX}/include/s3client`.
The library is found under `${CMAKE_INSTALL_PREFIX}/lib` and the executables
under `${CMAKE_INSTALL_PREFIX}/bin`.


### Compilation options

By default the library and all the command line tools and the S3 API extensions are built.

To disable building the command line tools set `APPS=OFF`.

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

In cases where the *minio* server is already installed and available, you can 
invoke the `minio_run_server.sh` instead.

```sh
chmod u+x ./minio_run_server.sh
./minio_run_server.sh myalias ~/tmp/minio_data
```

Both scripts ouput access, secret and URL which should be stored into 
environment variables.

The `test` directory includes tests for the high level interface and the S3 API.

All tests require passing the name of the environment variables storing access,
secret and endpoint URL information.

The provided `.sh` scripts inside the `test` directory can run all the tests
at once.

## License

This software is distributed under the BSD three-clause license and has
dependencies on the following software libraries:

* *libcurl* - distributed unded the curl license, derived from MIT/X
* *Lyra*, by Rene Rivera - distributed under the Boost license version 1.0
* *Tinyxml2* - zlib (included in source tree)
* *Doxygen Awesome* - MIT 

## Sending S3 requests

The `s3-client` application is just a wrapper around the `sss::SendS3Request` function.

Sample requests are shown below, look here for a complete list:

https://docs.aws.amazon.com/AmazonS3/latest/API/API_Operations.html

### List bucket content

Command line:

```sh
s3-client -a $S3TEST_ACCESS -s $S3TEST_SECRET -e $S3TEST_URL \
          -b bucket1 -m get
```

C++:

```cpp
#include "s3-client.h"

...
S3ClientConfig args;
args.accessKey = "Naowkmo0786XzwrF";
args.secretKey = "gpM6KV0Andtn2myBWePjoHXmSBy71UDK";
args.endpoint = "http://127.0.0.1:9000";
args.bucket = "bucket1" 
args.method = "GET";

sss::WebClient req = sss::SendS3Request(s3args);

cout << req.GetContentText() << endl;
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
s3-client -a $S3TEST_ACCESS -s $S3TEST_SECRET -e $S3TEST_URL \
          -b bucket1 -k object -m get -H "Range:bytes=100-150"
```

C++:

```cpp
#include "s3-client.h"
...
S3ClientConfig args;
args.accessKey = "Naowkmo0786XzwrF";
args.secretKey = "gpM6KV0Andtn2myBWePjoHXmSBy71UDK";
args.endpoint = "http://127.0.0.1:9000";
args.bucket = "bucket1" 
args.method = "GET";
args.headers =  "Range:bytes=100-150";

sss::WebClient req = sss::SendS3Request(s3args);

cout << "Status: " << req.StatusCode() << endl << endl;
// Response body
cout << req.GetContentText() << endl << endl;
// Response header
 h = req.GetHeaderText() << endl;
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

### Store data into object

Command line:

```sh
s3-client -a $S3TEST_ACCESS -s $S3TEST_SECRET -e $S3TEST_URL \
          -b bucket1 -k variable_name -v "10,20,30" -m put
```

C++

```cpp
#include "s3-client.h"

...
S3ClientConfig args;
args.accessKey = "Naowkmo0786XzwrF";
args.secretKey = "gpM6KV0Andtn2myBWePjoHXmSBy71UDK";
args.endpoint = "http://127.0.0.1:9000";
args.bucket = "bucket1" 
args.method = "GET";
args.headers = "Range:bytes=100-150";
args.method = "PUT";
args.data = "10,20,30";  

sss::WebClient req = sss::SendS3Request(s3args);

cout << "Status: " << req.StatusCode() << endl << endl;
// Response body
cout << req.GetContentText() << endl << endl;
// Response header
cout << req.GetHeaderText() << endl;
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

## Parallel upload

Parallel uploads are supported from both file and memory in the
API and command line application.

Command line

```
s3-upload -a $S3_ACCESS -s $S3_SECRET -e $S3_URL -f myfile \
          -b bucket2 -k key -j $NUM_JOBS -n $PARTS_PER_JOBS -r $NUM_RETRIES
```

C++

Extracted from the file transfer tests.

```cpp
...
  #include <s3-api.h>
...
  string action = "Parallel file upload";
  ////
  try {
    S3DataTransferConfig c = {.accessKey = cfg.access,
                              .secretKey = cfg.secret,
                              .bucket = bucket,
                              .key = key,
                              .file = tmp.path,
                              .endpoints = {cfg.url},
                              .jobs = NUM_JOBS,
                              .partsPerJob = CHUNKS_PER_JOB};
    /*auto etag =*/ Upload(c);
    S3Api s3(cfg.access, cfg.secret, cfg.url);
    const CharArray uploaded = s3.GetObject(bucket, key);
    if (uploaded != data)
      throw logic_error("Data verification failed");
    TestOutput(action, true, TEST_PREFIX);
  } catch (const exception &e) {
    TestOutput(action, false, TEST_PREFIX, e.what());
  }
```

## Parallel file download

Parallel downloads are supported to both file and memory in the
API.

Command line

```
s3-download -a $S3_ACCESS -s $S3_SECRET -e $S3_URL -f myfile \
            -b bucket2 -k key -j $NUM_JOBS -n $PARTS_PER_JOBS -r $NUM_RETRIES
```

C++

Extracted from the file transfer tests.

```cpp
...
  #include <s3-api.h>
...
  action = "Parallel file download";
  try {
    S3DataTransferConfig c = {.accessKey = cfg.access,
                              .secretKey = cfg.secret,
                              .bucket = bucket,
                              .key = key,
                              .file = tmp.path,
                              .endpoints = {cfg.url},
                              .jobs = NUM_JOBS,
                              .partsPerJob = CHUNKS_PER_JOB};
    Download(c);
    FILE *fi = fopen(tmp.path.c_str(), "rb");
    vector<char> input(SIZE);
    if (fread(input.data(), SIZE, 1, fi) != 1) {
      throw std::runtime_error("Cannot open file for reading");
    }
    //
    if (input == data) {
      TestOutput(action, true, TEST_PREFIX);
    } else {
      throw logic_error("Data verification failed");
    }
  } catch (const exception &e) {
    TestOutput(action, false, TEST_PREFIX, e.what());
  }
```

## Parallel data transfer

The following considerations apply to the transfer of single large files
over fast connections only (at least 10 Gb/s).

### Lustre

When transferring from *Lustre* make sure that the stripe
size is as close as possible to a multiple of the part size.
Use: `lfs getstripe <filename>` to retrieve the stripe size.

Whe downloading data to *Lustre* make sure that the stripe size
for the directory is a multiple of the part size or create a file
with the desired stripe size first using the `lfs setstripe` command.

### Others

*RAID* configurations and other parallel filesystems have 
predefined stripe/block sizes, please do refer to the documentation
before configuring the number of tasks and parts per task.

Parallel reads and writes from/to flash memory are faster than serial 
ones, but with spinning disks the overall performance might not be 
impacted by trasferring data in parallel and even cause a slowdown.
