#!/usr/bin/env bash
if [[ -d repo ]]; then
  cd repo
  git pull
  cd ..
else
  git clone --recurse-submodules https://github.com/uv-cpp/s3client.git repo
fi
if [[ -d doxygen-docs ]]; then
  echo "  deleting previously generated documentation..."
  rm -rf doxygen-docs
fi
if [[ -d build ]]; then
  rm -rf build
fi
if [[ -d doc ]]; then
  rm -rf doc
fi
if [[ -d .cache ]]; then
  rm -rf .cache
fi
cd repo/doc
echo "Generating Doxygen documentation..."
echo "  running doxygen..."
doxygen Doxyfile.ghpages > doxygen.log 2> doxygen.errors
echo "Pushing to gh-pages..."
cd ../../
git add -A
git commit -m "Generated" > /dev/null 2>&1
git push origin gh-pages > /dev/null 2>&1
echo "...done"

