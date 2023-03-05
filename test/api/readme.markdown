#  API Test

## Configuration

All tests read the value of access, key and url endpoint from environment variables which need
to be passed on the command line, e.g. `./bucket-test S3TEST_ACCESS S3TEST_SECRET S3TEST_URL`.

Each test driver contains multiple tests whose output is in *CSV* format: 

`<test name>,<result = `1` for pass | `0` for fail>,<blank | optional error message>`

To test locally, a script (`minio_setup.sh`) in the root folder is provided which downloads and runs *minio* server with *podman*.

It is also possible to test everything except for the `ListBuckets` API call using the
free `play.min.io` service.

Each test driver will first check that an S3 endpoint is available and accessible and report an error in case of failure. 
The test drivers will create buckets named `sss-api-test-` + timestamp.

