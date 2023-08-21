#  API Test

## Configuration

All tests read the value of access, key and url endpoint from environment variables which need
to be passed on the command line, e.g. `./bucket-test S3TEST_ACCESS S3TEST_SECRET S3TEST_URL`.

Each test driver contains multiple tests printing output to `stdin` in *CSV* format: 

`<test name>,<result = `1` for pass | `0` for fail>,<blank | optional error message>`

and checks that an S3 endpoint is available and accessible and reports an error in case of failure. 

The test drivers will create buckets and objects named `sss-api-test-` + timestamp.

Tests require an S3 endpoint to be accessible.

One option is to use the free *play.min.io* service; other options are to 
configure a local instance of the *minio* server or use an AWS S3 account.

When testing with an AWS S3 account extract credentials from file `.aws/credentials`
and use the url: `https://s3.<region e.g. us-east-1>.amazonaws.com` as the endpoint.

The access and secret keys for *play.min.io* are stored inside the
`~/.mc/config.json` file configured after installing the
[minio client](https://min.io/docs/minio/linux/reference/minio-mc.html).

The script `minio_podman_setup.sh` downloads configures and runs a *minio* server instance
inside a container using *podman*.

Run the script after building all the applications, making sure that
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

Both scripts output access, secret and URL which should be stored into 
environment variables, the optional third argument to the scripts is the name of
the file where environment variables are stored: use `source` <filename> to set
all environment variables.

The `test` directory includes tests for the high level interface and the S3 API.

All tests require passing the name of the environment variables storing access,
secret and endpoint URL information.

The provided `.sh` scripts inside the `test` directory can run all the tests
at once.


