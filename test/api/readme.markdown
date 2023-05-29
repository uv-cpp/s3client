#  API Test

## Configuration

All tests read the value of access, key and url endpoint from environment variables which need
to be passed on the command line, e.g. `./bucket-test S3TEST_ACCESS S3TEST_SECRET S3TEST_URL`.

Each test driver contains multiple tests whose output is in *CSV* format: 

`<test name>,<result = `1` for pass | `0` for fail>,<blank | optional error message>`

To test locally, two scripts to run *minio* are provided in the root folder:
- `minio_podman_setup.sh`:  downloads and runs *minio* server with *podman*
- `minio_run_server.sh`: runs the pre-installed *minio* server

The scripts print and optionally store into a file the url, access and secret to use.

Both scripts require the following arguments:
- *instance name*: *minio* instance name; added into `.mc/config.json`
- *storage path*: directory where data are stored
- (optional): file name where code to set environment variable is written


It is also possible to test everything  using the free `play.min.io` service.
Credentials for `play.min.ui` should already be present in the `.mc/config.json` file
created by the *minio* client.

Each test driver will first check that an S3 endpoint is available and accessible and report an error in case of failure. 
The test drivers will create buckets and objects named `sss-api-test-` + timestamp.

