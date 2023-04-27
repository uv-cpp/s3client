#pragma once
#include "utility.h"
#include <stdint.h>
#include <stdlib.h>
namespace sha256 {
void init_hash(uint32_t h[8]);
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
void hash_to_text(uint32_t hash[8], char *text);
} // namespace sha256
