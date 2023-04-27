#include "utility.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

uint8_t *alloc_padded(uint64_t size, uint64_t buffer_size, size_t *sz,
                      uint8_t *tmpbuf) {
  *sz = next_div_by(size + 1 + 8, 64);
  uint8_t *buf = NULL;
  if (tmpbuf) {
    memset(tmpbuf, 0, *sz);
    buf = tmpbuf;
  } else {
    buf = (uint8_t *)calloc(*sz, sizeof(uint8_t));
  }
  buf[size] = 0x80;
  const uint64_t bitsize = to_big_endian(8 * size);
  memcpy(&buf[*sz - 8], &bitsize, sizeof(bitsize));
  return buf;
}
