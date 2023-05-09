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
/**
 * \addtogroup Applications
 * \brief Command line applications.
 * @{
 */
/**
 * \file generate_s3_credentials.cpp
 * \brief Generates S3 access and secret keys.
 */
#include <iostream>
#include <random>
#include <string>

using namespace std;
static constexpr auto CHARS = "abcdefghijklmnopqrstuvwxyz"
                              "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                              "0123456789"
                              "+/=";
static constexpr auto CHARS_LENGTH = char_traits<char>::length(CHARS);

static constexpr size_t ACCESS_LENGTH = 18;
static constexpr size_t SECRET_LENGTH = 36;

int main(int argc, char **argv) {
  random_device rd;
  std::mt19937 gen(rd());
  uniform_int_distribution<size_t> distrSecret(0, CHARS_LENGTH - 1);
  uniform_int_distribution<size_t> distrAccess(26, CHARS_LENGTH - 1);
  bool genAccess = true;
  bool genSecret = true;
  if (argc > 2) {
    cerr << "ERROR - too many parameters, usage: " << argv[0]
         << " [access|secret]" << endl;
    exit(EXIT_FAILURE);
  }
  if (argc == 2) {
    if (string(argv[1]) == "access") {
      genSecret = false;
    } else if (string(argv[1]) == "secret") {
      genAccess = false;

    } else if (string(argv[1]) == "-h") {
      cout << "Generates S3 access and secret keys" << endl
           << "Usage: " << argv[0] << " [access|key]" << endl;
      exit(EXIT_SUCCESS);
    } else {
      cerr << "ERROR - unrecognized parameter, usage: " << argv[0]
           << " [access|secret]" << endl;
      exit(EXIT_FAILURE);
    }
  }
  if (genAccess) {
    string access;
    for (size_t i = 0; i != ACCESS_LENGTH; ++i) {
      access.push_back(CHARS[distrAccess(gen)]);
    }
    cout << access;
    if (genSecret == true)
      cout << endl;
  }
  if (genSecret) {
    string secret;
    for (size_t i = 0; i != SECRET_LENGTH; ++i) {
      secret.push_back(CHARS[distrSecret(gen)]);
    }
    cout << secret;
    if (genAccess == true)
      cout << endl;
  }
  return 0;
}
/**
 * @}
 */
