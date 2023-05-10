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
  /// [Create signature example]
  const string signature =
      "2ff4da4766da392b60b3278d2993398ee3f05fbf45aae378a66b489d266a4e87";
  const ComputeSignatureConfig cfg{.access = "08XW32=0H=G7=HBLCG",
                                   .secret =
                                       "y8a=4KnHBxTtOuH5zduTxjfFIjBXfwfBWfjF",
                                   .endpoint = "http://localhost:9000",
                                   .method = "GET",
                                   .bucket = "bucket1",
                                   .key = "key1",
                                   .headers = {{"x-amz-meta-mymeta", "123"}},
                                   .dates = {"20230418T153022Z", "20230418"}};
  cout << "Sign,"
       << "Sign request," << (signature == ComputeSignature(cfg).signature)
       << endl;
  /// [Create signature example]
  return 0;
}
