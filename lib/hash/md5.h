#pragma once
#include "utility.h"
#include <cstdint>
#include <cstdlib>
namespace md5 {
inline void init_hash(uint32_t h[4]) {
  h[0] = 0x67452301;
  h[1] = 0xefcdab89;
  h[2] = 0x98badcfe;
  h[3] = 0x10325476;
}
//void md5(const uint8_t data[], size_t length, uint32_t hash[4]);
//void md5_next(const uint8_t data[], uint32_t length, uint32_t hash[4],
//                 size_t total_length, uint8_t *tmpbuf);
void md5_stream(uint32_t hash[4], const uint8_t data[], uint64_t length);
void hash_to_text(uint32_t hash[4], char *text) {
  const unsigned char *h = (unsigned char *)hash;
  for (size_t i = 0; i != 16; ++i) {
    snprintf(text + 2 * i, 3, "%02x", h[i]);
  }
}
void print_hash(uint32_t hash[4]);
} // namespace sha256
