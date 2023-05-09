#pragma once
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
//-----------------------------------------------------------------------------
inline uint64_t to_big_endian(uint64_t n) {
  const uint64_t b0 = (n & 0x00000000000000ff) << 56u;
  const uint64_t b1 = (n & 0x000000000000ff00) << 40u;
  const uint64_t b2 = (n & 0x0000000000ff0000) << 24u;
  const uint64_t b3 = (n & 0x00000000ff000000) << 8u;
  const uint64_t b4 = (n & 0x000000ff00000000) >> 8u;
  const uint64_t b5 = (n & 0x0000ff0000000000) >> 24u;
  const uint64_t b6 = (n & 0x00ff000000000000) >> 40u;
  const uint64_t b7 = (n & 0xff00000000000000) >> 56u;

  return b0 | b1 | b2 | b3 | b4 | b5 | b6 | b7;
}
inline uint32_t to_little_endian(uint32_t n) {
  const uint64_t b0 = (n & 0xff000000) >> 24u;
  const uint64_t b1 = (n & 0x00ff0000) >> 8u;
  const uint64_t b2 = (n & 0x0000ff00) << 8u;
  const uint64_t b3 = (n & 0x000000ff) << 24u;
  return b0 | b1 | b2 | b3;
}

inline uint64_t next_div_by(uint64_t n, uint64_t d) {
  for (; n % d; n++)
    ;
  return n;
}

//-----------------------------------------------------------------------------
// right rotate
inline uint32_t right_rotate(uint32_t x, uint32_t n) {
  return (x >> n) | (x << (32 - n));
}

// left shift of unsigned 8 bit int and conversion to 32 bit
inline uint32_t lshift(uint8_t n, uint8_t nbits) {
  return (uint32_t)n << nbits;
}

uint8_t *alloc_padded(uint64_t size, uint64_t buffer_size, size_t *sz,
                      uint8_t *tmpbuf);
