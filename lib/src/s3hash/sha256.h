#pragma once
#include <stdint.h>
#include <stdlib.h>

void init_hash(uint32_t h[8]);
void sha256(const uint8_t data[], uint32_t length, uint32_t hash[8]);
void sha256_next(const uint8_t data[], uint32_t length, uint32_t hash[8],
                 size_t total_length, uint8_t *tmpbuf);
void sha256_stream(uint32_t hash[8], const uint8_t data[], uint64_t length);
uint8_t *alloc_padded(uint64_t size, uint64_t buffer_size, size_t *sz,
                      uint8_t *tmpbuf);
void to_little(uint32_t hash[8]);
void print_hash(uint32_t hash[8]);
void hash_to_text(uint32_t hash[8], char *text);
