#!/usr/bin/env bash
git clone --recurse-submodules https://github.com/uv-cpp/s3client.git repo
doxygen repo/doc/Doxyfile.ghpages > doxygen.log 2> doxygen.errors
git push origin gh-pages


