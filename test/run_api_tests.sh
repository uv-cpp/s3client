#!/usr/bin/env bash
if (($# == 0))
then
  echo "Run API tests"
  echo "usage: $0 <path to tests> <access env var name> <secret env var name> <url env var name>"
  exit 0
fi

if (($# != 4))
then
  echo "Error - invalid number of arguments"
  echo "usage: $0 <path to tests> <access env var name> <secret env var name> <url env var name>"
  exit 1
fi
TEST_PATH=$1
ACCESS_VAR=$2
SECRET_VAR=$3
URL_VAR=$4
$TEST_PATH/bucket-test $ACCESS_VAR $SECRET_VAR $URL_VAR
$TEST_PATH/object-test $ACCESS_VAR $SECRET_VAR $URL_VAR
$TEST_PATH/multipart-upload-test $ACCESS_VAR $SECRET_VAR $URL_VAR
$TEST_PATH/abort-multipart-upload-test $ACCESS_VAR $SECRET_VAR $URL_VAR
$TEST_PATH/fileobject-test $ACCESS_VAR $SECRET_VAR $URL_VAR

