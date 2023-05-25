#!/usr/bin/env bash
# BSD 3-Clause License
#
# Copyright (c) 2020-2023, Ugo Varetto
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
# 1. Redistributions of source code must retain the above copyright notice, this
#    list of conditions and the following disclaimer.
#
# 2. Redistributions in binary form must reproduce the above copyright notice,
#    this list of conditions and the following disclaimer in the documentation
#    and/or other materials provided with the distribution.
#
# 3. Neither the name of the copyright holder nor the names of its
#    contributors may be used to endorse or promote products derived from
#    this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
# FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
# SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
# CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
# OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#Run minio minio server on localhost through and configures alias with
#secret and access keys
#usage: ./minio_run_server.sh <alias name> <data directory path>
#The data directory is created if it does not exist
#Usind default ports 9000 (API) and 9090 (admin)
#Requires s3-gen-credentials to be built and accessible through PATH variable 
#
# Example, invoking from <root>/build directory, storing env vars into env.vars file:
# PATH=build/debug:$PATH ../minio_run_server.sh s3test ~/tmp/s3test env.vars

#check command line parameters
if (($# == 0))
then
  echo "Run minio server and store credentials"
  echo "usage: $0 <minio alias name> <data path> [env_vars_file]"
  echo
  echo "If 'env_vars_file' is specified the commands to set the environment variables"
  echo "for access, secret and url are stored inside file 'env_vars_file"
  echo "Data path is created if it does not exist"
  echo "S3CLIENT_TEST_SECRET and S3CLIENT_TEST_ACCESS env variables contain generated access and secret"
  echo "An entry is added into $HOME/.mc/config.json"
  echo "Running the script overwrites the etry in config.json."
  echo "Access and secret keys are the username and password for the minio console"
  exit 0
fi
if (($# < 2))
then
  echo "Error - invalid number of arguments"
  echo "usage: $0 <minio alias name> <data path>"
  exit 2
fi
#create data directory
mkdir -p $2
if [ $? -ne 0 ]; then
  echo "Error creating directory"
  exit 2
fi
PORT=9000
ADMIN_PORT=9090
DATA_PATH=$2
URL="http://127.0.0.1:$PORT"
ACCESS=`s3-gen-credentials access`
SECRET=`s3-gen-credentials secret`
export MINIO_ROOT_USER=$ACCESS 
export MINIO_ROOT_PASSWORD=$SECRET 
minio server $DATA_PATH &
while ! ps aux | grep minio >> /dev/null;
do
  sleep 0.5s
done
mc alias set $1 $URL $ACCESS $SECRET
#create alias using default credentials set above
if [ $? -ne 0 ]; then
  echo "Error running 'mc alias set'"
  exit 2
fi
echo export S3CLIENT_TEST_ACCESS=$ACCESS
echo export S3CLIENT_TEST_SECRET=$SECRET
echo export S3CLIENT_TEST_URL=$URL
if (($# == 3)); then
  echo "Writing env variables to file $3"
  echo "export S3CLIENT_TEST_ACCESS=$ACCESS" > $3
  echo "export S3CLIENT_TEST_SECRET=$SECRET" >> $3
  echo "export S3CLIENT_TEST_URL=$URL" >> $3
  echo "Execute source $3 to set env variables"
fi
