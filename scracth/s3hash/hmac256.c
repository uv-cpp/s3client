#include "sha256.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
// H((K xor opad) concat H( (K xor ipad ) concat message))
// if length(message) % 64 == 0
// == H(K xor opad) concat H'(H(K xor ipat) concat H'(message))
// where H' == H but with length bytes set to sum of individual items
// H( a concat b concat c) = H(a) concat H(b) concat H'(c) where H' is the
// same as H but with length bytes == length(a) + length(b) + length(c)
//
// K = K if length(K) <=block size, H(K) otherwise
// opad = 0x5c times 64
// ipad = 0x36 times 64
// H = hash function

void hmac256(uint8_t *data, size_t length, uint8_t *key, size_t key_length,
             uint8_t hmac_hash[64]) {
  uint32_t outer_hash[8];
  uint32_t inner_hash[8];
  uint8_t ipad[64];
  uint8_t opad[64];

  uint8_t K[64];
  if (key_length <= 64) {
    memcpy(K, key, 64);
  } else {
    sha256(key, length, (uint32_t *)K);
  }
  for (int i = 0; i != 64; ++i) {
    ipad[i] = K[i] ^ 0x5c;
    opad[i] = K[i] ^ 0x36;
  }
  size_t size = 64 /*ipad*/ + length /*message*/;
  uint8_t *hinner = alloc_padded(size, size, &size, NULL);
  memcpy(hinner, ipad, 64);
  memcpy(hinner + 64, data, length);
  init_hash(inner_hash);
  sha256_stream(inner_hash, hinner, size);
  free(hinner);

  size = 64 /*opad*/ + 64 /*hash(k xor ipad) concat message)*/;
  uint8_t *houter = alloc_padded(size, size, &size, NULL);
  memcpy(houter, opad, 64);
  memcpy(houter + 64, (uint8_t *)inner_hash, 64);
  init_hash((uint32_t *)hmac_hash);
  sha256_stream((uint32_t *)hmac_hash, houter, size);
  free(houter);
}
