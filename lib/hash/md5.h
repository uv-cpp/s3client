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
 * \file md5.h
 * \brief declaration of MD5 hashing functions.
 */
#pragma once
#include "utility.h"
#include <cstdint>
#include <cstdlib>
namespace md5 {
/**
 * \addtogroup Hash
 * \brief Hashing functions.
 * @{
 */
/**
 * \brief Initialize MD5 hash
 * \param[out] h hash code
 */
inline void init_hash(uint32_t h[4]) {
  h[0] = 0x67452301;
  h[1] = 0xefcdab89;
  h[2] = 0x98badcfe;
  h[3] = 0x10325476;
}
// void md5(const uint8_t data[], size_t length, uint32_t hash[4]);
// void md5_next(const uint8_t data[], uint32_t length, uint32_t hash[4],
//                  size_t total_length, uint8_t *tmpbuf);
/**
 * \brief MD5 hash calculation, streaming version: hash is updated with new
 * hash at each call.
 * \param[in,out] MD5 hash updated at each call, initialize with init_hash
 * \param[in] data data to hash
 * \param[in] length size of data buffer
 */
void md5_stream(uint32_t hash[4], const uint8_t data[], uint64_t length);
/**
 * \brief Print hash code to text.
 * \param[in] hash hash code
 * \param[out] text null-terminated out buffer
 */
inline void hash_to_text(uint32_t hash[4], char *text) {
  const unsigned char *h = (unsigned char *)hash;
  for (size_t i = 0; i != 16; ++i) {
    snprintf(text + 2 * i, 3, "%02x", h[i]);
  }
}
/**
 * \brief Print hash code to stdout
 * \param[in] hash hash code
 */
void print_hash(uint32_t hash[4]);
/**
 * @}
 */
} // namespace md5
