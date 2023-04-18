#include "aws_sign.h"
#include "s3-client.h"
#include <cassert>
#include <iostream>

using namespace std;
using namespace sss;

int main(int, char **) {
  const string presignedUrl =
      "http://127.0.0.1:9000/bucket1/"
      "key1?X-Amz-Algorithm=AWS4-HMAC-SHA256&X-Amz-Credential=7PJRLUIHCX%2B%"
      "2F1O63TN%2FT153022Z%2Fus-east%2Fs3%2Faws4_request&X-Amz-Date="
      "20230418T153022Z&X-Amz-Expires=1000&X-Amz-SignedHeaders=host&X-Amz-"
      "Signature="
      "85bc483adcdfdf9c866e8a0ae2b9cc56330a61fedf11eac9d46683b59e754da9";
  const S3SignUrlConfig cfg{.access = "7PJRLUIHCX+/1O63TN",
                            .secret = "bTDYuxv+0teEVY9gUYWM7p3B3x=GuiFAtO+4",
                            .endpoint = "http://127.0.0.1:9000",
                            .expiration = 1000,
                            .method = "PUT",
                            .bucket = "bucket1",
                            .key = "key1",
                            .dates = {"20230418T153022Z", "20230418"}};
  cout << SignedURL(cfg) << endl;
  return 0;
}
