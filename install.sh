#!/usr/bin/env bash
if [ -d "./s3client" ] 
then
    echo "Error - Directory s3client exists"
    exit 1
fi
if (($# != 1))
then
  echo "Error - missing install prefix"
  echo "usage: $0 <install prefix>"
  exit 2
fi
git clone --recurse-submodules https://github.com/uv-cpp/s3client.git
mkdir -p s3client/build
cd s3client/build
cmake ../ -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=$1
make -j 8
make install
