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
 * \file sha256.h
 * \brief declaration of SHA256 hash functions.
 */
#pragma once
#include "utility.h"
#include <cstdint>
#include <cstdlib>
namespace sha256 {
/**
 * \addtogroup Hash
 * @{
 */
/**
 * \brief Initialize SHA256 hash.
 *
 * First 32 bits of the fractional parts of the square roots of the first 8
 * primes 2..19
 * \param[out] hash returned hash code
 */
inline void init_hash(uint32_t hash[8]) {
  hash[0] = 0x6a09e667;
  hash[1] = 0xbb67ae85;
  hash[2] = 0x3c6ef372;
  hash[3] = 0xa54ff53a;
  hash[4] = 0x510e527f;
  hash[5] = 0x9b05688c;
  hash[6] = 0x1f83d9ab;
  hash[7] = 0x5be0cd19;
}
/**
 * \brief Return SHA256 hash
 *
 * \param[in] data data to hash
 * \param[in] length size of data buffer
 * \param[out] hash returned hash value
 */
void sha256(const uint8_t data[], size_t length, uint32_t hash[8]);
/**
 * \brief Compute SHA256 hash of chunks of data, using preallocated temporary
 * buffer to store padded data.
 *
 * The SHA256 algorithm requires the *total size* of the hashed data to be added
 * to the data itself as the last 8 bytes of the input buffer.
 * When encoding the intermediate chunks \c length contains the size of the
 * chunk and \c total_length can be either zero or equal to \c length. When
 * encoding the last chunk, \c length needs to be equal to the chunk size and \c
 * total_length equal to the total length of the buffer i.e. the sum of the
 * sizes of the individual chunks.
 *
 * \param[in] data data to hash
 * \param[in] length size of chunk
 * \param[in, out] hash SHA256 hash
 * \param[in] total_length total length of data buffer
 */
void sha256_next(const uint8_t data[], uint32_t length, uint32_t hash[8],
                 size_t total_length, uint8_t *tmpbuf);
/**
 * \brief Compute SHA256 hash, updating hash value at every invocation.
 *
 * \param[in.out] hash SH256 hash
 * \param[in] data data to hash
 * \param[in] length size of data buffer
 */
void sha256_stream(uint32_t hash[8], const uint8_t data[], uint64_t length);
/**
 * \brief Convert hash value to little endian.
 *
 * \param[in,out] hahs SHA256 hash
 */
inline void to_little(uint32_t hash[8]) {
  for (size_t i = 0; i != 8; i++)
    hash[i] = to_little_endian(hash[i]);
}
/**
 * \brief Convert SHA256 hash to text.
 *
 * \param[in] hash SHA256 hash
 * \param[out] text null-terminated output string
 */
inline void hash_to_text(uint32_t hash[8], char *text) {
  const unsigned char *h = (unsigned char *)hash;
  char hash_text[65];
  for (size_t i = 0; i != 32; ++i) {
    snprintf(text + 2 * i, 3, "%02x", h[i]);
  }
}
/**
 * \brief Print SHA256 to standard output.
 *
 * \param[in] hash SHA256 hash
 */
void print_hash(uint32_t hash[8]);
} // namespace sha256
/**
 * @}
 */
