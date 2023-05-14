#!/usr/bin/env bash
git clone --recurse-submodules https://github.com/uv-cpp/s3client.git repo
doxygen repo/doc/Doxyfile.ghpages
git push origin gh-pages


