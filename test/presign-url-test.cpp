#include "aws_sign.h"
#include "s3-client.h"
#include <cassert>
#include <iostream>

using namespace std;
using namespace sss;

int main(int, char **) {
  /// [Pre-sign example]
  const string presignedUrl =
      "http://127.0.0.1:9000/bucket1/"
      "key1?X-Amz-Algorithm=AWS4-HMAC-SHA256&X-Amz-Credential=7PJRLUIHCX%2B%"
      "2F1O63TN%2F20230418%2Fus-east%2Fs3%2Faws4_request&X-Amz-Date="
      "20230418T153022Z&X-Amz-Expires=1000&X-Amz-SignedHeaders=host&X-Amz-"
      "Signature="
      "e48f7576e8978074bb747f4cfed31230da726cce9074ef577a9739149c4d342a";
  const S3SignUrlConfig cfg{.access = "7PJRLUIHCX+/1O63TN",
                            .secret = "bTDYuxv+0teEVY9gUYWM7p3B3x=GuiFAtO+4",
                            .endpoint = "http://127.0.0.1:9000",
                            .expiration = 1000,
                            .method = "PUT",
                            .bucket = "bucket1",
                            .key = "key1",
                            .dates = {"20230418T153022Z", "20230418"}};
  cout << "Sign,"
       << "Presign URL," << (presignedUrl == SignedURL(cfg)) << endl;
  /// [Pre-sign example]
  return 0;
}
