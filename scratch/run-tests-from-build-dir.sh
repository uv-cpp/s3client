#! /usr/bin/env bash
# Setup podman minio server, set env vars, run tests
# Only works when run from <project root>/build directory
#../minio_run_server.sh s3test ~/tmp/s3test ./env-vars
#source ./env-vars
../test/run_all.sh ../test build/debug/test \
S3CLIENT_TEST_ACCESS S3CLIENT_TEST_SECRET S3CLIENT_TEST_URL | grep 0