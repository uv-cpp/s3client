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
#include "sha256.h"
#include "utility.h"
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
// H((K xor opad) concat H( (K xor ipad ) concat message))
// K = K if length(K) <=block size, H(K) otherwise
// opad = 0x5c times 64
// ipad = 0x36 times 64
// H = hash function

using namespace sha256;

void hmac256(const uint8_t *data, size_t length, const uint8_t *key,
             size_t key_length, uint8_t hmac_hash[32]) {
  uint32_t outer_hash[8];
  uint32_t inner_hash[8];
  uint8_t ipad[64];
  uint8_t opad[64];

  uint8_t K[64];
  memset(K, 0, 64);
  if (key_length <= 64) {
    memcpy(K, key, key_length);
  } else {
    sha256::sha256(key, length, (uint32_t *)K);
  }
  for (int i = 0; i != 64; ++i) {
    opad[i] = K[i] ^ 0x5c;
    ipad[i] = K[i] ^ 0x36;
  }
  size_t size = 64 /*ipad*/ + length /*message*/;
  uint8_t *hinner = alloc_padded(size, size, &size, NULL);
  memcpy(hinner, ipad, 64);
  memcpy(hinner + 64, data, length);
  init_hash(inner_hash);
  sha256_stream(inner_hash, hinner, size);
  to_little(inner_hash);
  free(hinner);

  size = 64 /*opad*/ + 32 /*hash(k xor ipad) concat message)*/;
  uint8_t *houter = alloc_padded(size, size, &size, NULL);
  memcpy(houter, opad, 64);
  memcpy(houter + 64, (uint8_t *)inner_hash, 32);
  init_hash((uint32_t *)hmac_hash);
  sha256_stream((uint32_t *)hmac_hash, houter, size);
  to_little((uint32_t *)hmac_hash);
  free(houter);
}
