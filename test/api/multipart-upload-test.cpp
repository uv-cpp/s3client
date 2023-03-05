#include "s3-api.h"
#include "utility.h"
#include <iostream>
#include <string>
#include <vector>
using namespace std;
using namespace sss;
using namespace api;

int main(int argc, char **argv) {
  const Params cfg = ParseCmdLine(argc, argv);
  TestS3Access(cfg);
  const size_t SIZE = 7000000;
  // Check the default configuration for the minimum object size, 5MiB should
  // be supported on most systems
  // 2 x 3M + 1M chunks
  const size_t CHUNK_SIZE = SIZE / 2;
  const size_t LAST_CHUNK_SIZE = SIZE % 2;
  const vector<char> data(13000000);
  const string prefix = "sss-api-test-";
  const string bucket = prefix + ToLower(Timestamp());
  const string key = prefix + "obj-" + ToLower(Timestamp());

  try {
    S3Client s3(cfg.access, cfg.secret, cfg.url);
    // create bucket
    s3.CreateBucket(bucket);
  } catch (...) {
    cerr << "Error creating bucket " << bucket << endl;
    exit(EXIT_FAILURE);
  }

  ///
  string action = "CreateMultipartUpload";
  UploadId uid;
  try {
    S3Client s3(cfg.access, cfg.secret, cfg.url);
    uid = s3.CreateMultipartUpload(bucket, key);
    TestOutput(action, true);
  } catch (const exception &e) {
    TestOutput(action, false, e.what());
  }
  ///
  action = "UploadPart";
  vector<ETag> etags;
  try {
    S3Client s3(cfg.access, cfg.secret, cfg.url);
    for (size_t i = 0; i != 3; ++i) {
      const size_t size = i < 2 ? CHUNK_SIZE : LAST_CHUNK_SIZE;
      etags.push_back(
          s3.UploadPart(bucket, key, uid, i, &data[i * CHUNK_SIZE], size));
    }
    TestOutput(action, true);
  } catch (const exception &e) {
    TestOutput(action, false, e.what());
  }
  ///
  action = "CompleteMultipartUpload";
  const bool LAST = true;
  try {
    S3Client s3(cfg.access, cfg.secret, cfg.url);
    s3.CompleteMultipartUpload(uid, bucket, key, etags);
    TestOutput(action, true, "", LAST);
  } catch (const exception &e) {
    TestOutput(action, false, e.what(), LAST);
  }

  try {
    S3Client s3(cfg.access, cfg.secret, cfg.url);
    // delete object
    s3.DeleteObject(bucket, key);
  } catch (...) {
    cerr << "Error deleting object " << bucket << "/" << key << endl;
    exit(EXIT_FAILURE);
  }
  // delete bucket
  try {
    S3Client s3(cfg.access, cfg.secret, cfg.url);
    s3.DeleteBucket(bucket);
  } catch (...) {
    cerr << "Error deleting bucket " << bucket;

    exit(EXIT_FAILURE);
  }
  return 0;
}
