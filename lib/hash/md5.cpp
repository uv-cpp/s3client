/*******************************************************************************
 * BSD 3-Clause License
 *
 * Copyright (c) 2020-2023, Ugo Varetto
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived from
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
 * \file md5.cpp
 * \brief Implementation of MD5 algorithm,
 */

// md5.c MD5 reference implementation
//
//-----------------------------------------------------------------------------
#include "md5.h"
#include "utility.h"
#include <cassert>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>

namespace md5 {
//-----------------------------------------------------------------------------
uint32_t r[] = {7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22,
                5, 9,  14, 20, 5, 9,  14, 20, 5, 9,  14, 20, 5, 9,  14, 20,
                4, 11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23,
                6, 10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21};

// Use binary integer part of the sines of integers (in radians) as constants
// Initialize variables:
uint32_t k[] = {
    0xd76aa478, 0xe8c7b756, 0x242070db, 0xc1bdceee, 0xf57c0faf, 0x4787c62a,
    0xa8304613, 0xfd469501, 0x698098d8, 0x8b44f7af, 0xffff5bb1, 0x895cd7be,
    0x6b901122, 0xfd987193, 0xa679438e, 0x49b40821, 0xf61e2562, 0xc040b340,
    0x265e5a51, 0xe9b6c7aa, 0xd62f105d, 0x02441453, 0xd8a1e681, 0xe7d3fbc8,
    0x21e1cde6, 0xc33707d6, 0xf4d50d87, 0x455a14ed, 0xa9e3e905, 0xfcefa3f8,
    0x676f02d9, 0x8d2a4c8a, 0xfffa3942, 0x8771f681, 0x6d9d6122, 0xfde5380c,
    0xa4beea44, 0x4bdecfa9, 0xf6bb4b60, 0xbebfbc70, 0x289b7ec6, 0xeaa127fa,
    0xd4ef3085, 0x04881d05, 0xd9d4d039, 0xe6db99e5, 0x1fa27cf8, 0xc4ac5665,
    0xf4292244, 0x432aff97, 0xab9423a7, 0xfc93a039, 0x655b59c3, 0x8f0ccc92,
    0xffeff47d, 0x85845dd1, 0x6fa87e4f, 0xfe2ce6e0, 0xa3014314, 0x4e0811a1,
    0xf7537e82, 0xbd3af235, 0x2ad7d2bb, 0xeb86d391};

void md5_stream(uint32_t hash[4], const uint8_t data[], size_t length) {
  for (size_t offset = 0; offset < length; offset += 64) {

    // break chunk into sixteen 32-bit words w[j], 0 ≤ j ≤ 15
    uint32_t *w = (uint32_t *)(data + offset);

    // Initialize hash value for this chunk:
    uint32_t a = hash[0];
    uint32_t b = hash[1];
    uint32_t c = hash[2];
    uint32_t d = hash[3];

    // Main loop:
    for (uint32_t i = 0; i < 64; i++) {

      uint32_t f, g;

      if (i < 16) {
        f = (b & c) | ((~b) & d);
        g = i;
      } else if (i < 32) {
        f = (d & b) | ((~d) & c);
        g = (5 * i + 1) % 16;
      } else if (i < 48) {
        f = b ^ c ^ d;
        g = (3 * i + 5) % 16;
      } else {
        f = c ^ (b | (~d));
        g = (7 * i) % 16;
      }

      uint32_t temp = d;
      d = c;
      c = b;
      b = b + left_rotate(a + f + k[i] + w[g], r[i]);
      a = temp;
    }

    // Add this chunk's hash to result so far:

    hash[0] += a;
    hash[1] += b;
    hash[2] += c;
    hash[3] += d;
  }
}

// MD5 on single fixed size buffer
void md5(const uint8_t data[], size_t length, uint32_t hash[4]) {
  init_hash(hash);
  md5_stream(hash, data, length);
}

void print_hash(uint32_t hash[4]) {
  const unsigned char *ph = (unsigned char *)hash;
  for (size_t i = 0; i != 16; ++i)
    printf("%02x", ph[i]);
  printf("\n");
}

// calculate file MD5
void md5_file(const char *fname, uint32_t hash[4]) {
  FILE *f = fopen(fname, "rb");
  if (!f) {
    fprintf(stderr, "Error opening file %s\n", fname);
    exit(EXIT_FAILURE);
  }
  const size_t BUFSIZE = 0x1000000; // 16 MiB
  char *buf = (char *)calloc(BUFSIZE, sizeof(char));
  assert(buf);
  init_hash(hash);
  size_t length = 0;
  while (1) {
    size_t bytes = fread(buf, 1, BUFSIZE, f);
    if (!bytes) {
      perror("Error reading from file");
      exit(EXIT_FAILURE);
    }
    int eof = 0;
    if (bytes < BUFSIZE)
      eof = 1;
    else {
      int c = getc(f);
      ungetc(c, f);
      if (c == EOF)
        eof = 1;
    }
    if (eof) {
      const uint64_t message_size = next_div_by(bytes + 1 + 8, 64);
      // allocate and zero out
      uint8_t *message = (uint8_t *)calloc(message_size, sizeof(char));
      assert(buf);
      memcpy(message, buf, bytes);
      // pad with '1' and zeros (array already zeroed out)
      message[bytes] = 0x80; // 100..
      // pad with length
      const uint64_t data_bit_size = 8 * (length + bytes);
      const uint64_t size = data_bit_size;
      memcpy(&message[message_size - 8], &size, sizeof(uint64_t));
      md5_stream(hash, (uint8_t *)message, message_size);
      free(message);
      break;
    } else {
      md5_stream(hash, (uint8_t *)buf, BUFSIZE);
    }
    memset(buf, 0, BUFSIZE);
    length += bytes;
  }
  free(buf);
}
} // namespace md5

//-----------------------------------------------------------------------------
#ifdef TEST
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cstring>

using namespace md5;

void test(const char *fname, const char *test_hash) {
  uint32_t hash[4];
  md5_file(fname, hash);
  char hash_text[33];
  hash_to_text(hash, hash_text);
  printf("%s\n", hash_text);
  assert(strncmp(hash_text, test_hash, 32) == 0);
  printf("Test passed\n");
}

//
int main(int argc, char *argv[]) {
  if (argc != 3) {
    fprintf(stderr, "Usage: %s <test file> <test file hash>\n", argv[0]);
    exit(EXIT_FAILURE);
  }
  test(argv[1], argv[2]);
  return 0;
}
#endif
