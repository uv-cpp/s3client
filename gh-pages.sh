#!/usr/bin/env bash
if [[-d "./repo"]]; then
  cd repo
  git pull
else
  git clone --recurse-submodules https://github.com/uv-cpp/s3client.git repo
fi
doxygen repo/doc/Doxyfile.ghpages > doxygen.log 2> doxygen.errors
git add -A
git commit -m "Generated"
git push origin gh-pages


