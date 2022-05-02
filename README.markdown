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

The default *CMake* configuration generates static executables.

The code is `C++17` compliant.


## License

This software is distributed under the BSD three-clause license and has
dependencies on the following software libraries:

* libcurl - distributed unded the curl license, derived from MIT/X
* Lyra, by Rene Rivera - distributed under the Boost license version 1.0
* Portable Hashing Library, by Stephan Brumme - distributed under the
  zlib license
  