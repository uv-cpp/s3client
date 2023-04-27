

// H(K xor opad) concat H( (K xor ipad ) concat message))
// K = K if length(K) <=block size, H(K) otherwise
// opad = 0x5c times 64
// ipad = 0x36 times 64
// H = hash function
//

void hmac256(uint8_t *data, size_t length, uint8_t *key, size_t key_length) {
  uint32_t message_hash[8];
  uint32_t ipad_message_hash[8];
  uint32_t opad_ipad_message_hash[8];
}
