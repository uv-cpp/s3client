#pragma once
#include "utility.h"
#include <cstdint>
#include <cstdlib>
namespace sha256 {
// First 32 bits of the fractional parts of the square roots of the first 8
// primes 2..19
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
void sha256(const uint8_t data[], size_t length, uint32_t hash[8]);
void sha256_next(const uint8_t data[], uint32_t length, uint32_t hash[8],
                 size_t total_length, uint8_t *tmpbuf);
void sha256_stream(uint32_t hash[8], const uint8_t data[], uint64_t length);
inline void to_little(uint32_t hash[8]) {
  for (size_t i = 0; i != 8; i++)
    hash[i] = to_little_endian(hash[i]);
}
inline void hash_to_text(uint32_t hash[8], char *text) {
  const unsigned char *h = (unsigned char *)hash;
  char hash_text[65];
  for (size_t i = 0; i != 32; ++i) {
    snprintf(text + 2 * i, 3, "%02x", h[i]);
  }
}
void print_hash(uint32_t hash[8]);
} // namespace sha256
