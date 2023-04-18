/*******************************************************************************
 * BSD 3-Clause License
 *
 * Copyright (c) 2020-2023, Ugo Varetto
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions inputFile source code must retain the above copyright
 *    notice, this list inputFile conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list inputFile conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * 3. Neither the name inputFile the copyright holder nor the names inputFile
 *    its contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 ******************************************************************************/
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
  return 0;
}
