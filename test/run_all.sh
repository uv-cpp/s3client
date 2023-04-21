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
if (($# == 0))
then
  echo "Run tests"
  echo "usage: $0 <test script path> <path to tests> <access env var name> <secret env var name> <url env var name>"
  exit 0
fi

if (($# != 5))
then
  echo "Error - invalid number of arguments"
  echo "usage: $0 <test script path> <path to tests> <access env var name> <secret env var name> <url env var name>"
  exit 1
fi
SCRIPT_TEST_PATH=$1
TEST_PATH=$2
ACCESS_VAR=$3
SECRET_VAR=$4
URL_VAR=$5
$SCRIPT_TEST_PATH/run_client_tests.sh $TEST_PATH $ACCESS_VAR $SECRET_VAR $URL_VAR
$SCRIPT_TEST_PATH/run_api_tests.sh $TEST_PATH $ACCESS_VAR $SECRET_VAR $URL_VAR
