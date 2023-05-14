#!/usr/bin/env bash
if [[ -d "./repo" ]]; then
  cd repo
  git pull
  cd ..
else
  git clone --recurse-submodules https://github.com/uv-cpp/s3client.git repo
fi
cd repo/doc
echo "Generating Doxygen documentation..."
doxygen Doxyfile.ghpages > doxygen.log 2> doxygen.errors
echo "Pushing to gh-pages..."
cd ../../
git add -A
git commit -m "Generated"
git push origin gh-pages
echo "...done"
#rm -rf doxygen-docs


