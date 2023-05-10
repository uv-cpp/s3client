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
 * \file utility.h
 * \brief Declaration of utility functions for SHA256 hashing.
 */
#pragma once
#include <cstdint>
#include <cstdio>
#include <cstdlib>
/**
 * \addtogroup Hash_Utility
 * \brief Utility function called from hashing functions.
 * @{
 */
//-----------------------------------------------------------------------------
/**
 * \brief Convert number from little endian to big endian.
 *
 * \param[in] n number to convert in little endian format.
 * \return number in big endian format.
 */
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

/**
 * \brief Convert from big endian to little endian.
 *
 * \param[in] n number in big endian format.
 * \return number in little endian format.
 */
inline uint32_t to_little_endian(uint32_t n) {
  const uint64_t b0 = (n & 0xff000000) >> 24u;
  const uint64_t b1 = (n & 0x00ff0000) >> 8u;
  const uint64_t b2 = (n & 0x0000ff00) << 8u;
  const uint64_t b3 = (n & 0x000000ff) << 24u;
  return b0 | b1 | b2 | b3;
}

/**
 * \brief Return next number evenly divisible by specified number.
 *
 * \param[in] n input number.
 * \param[in] d divisor
 * \return number evenly divisible by \c d.
 */
inline uint64_t next_div_by(uint64_t n, uint64_t d) {
  for (; n % d; n++)
    ;
  return n;
}

//-----------------------------------------------------------------------------
/**
 * \brief Right rotate bits of 32 bit integer numbers.
 *
 * \param[in] x number to rotate.
 * \param[in] n number of right shifts.
 * \return right rotated number.
 */
inline uint32_t right_rotate(uint32_t x, uint32_t n) {
  return (x >> n) | (x << (32 - n));
}

/**
 * \brief Left rotate bits of 32 bit integer numbers.
 *
 * \param[in] x number to rotate.
 * \param[in] n number of bitshifts.
 */
inline uint32_t left_rotate(uint32_t x, uint32_t n) {
  return (x << n) | (x >> (32 - n));
}

/**
 * \brief Left shift of unsigned 8 bit int and conversion to 32 bit.
 *
 * \param[in] n number to shift.
 * \param[in] nshifts number of shifts.
 * \return left shifted 32 bit number.
 */
inline uint32_t lshift(uint8_t n, uint8_t nshifts) {
  return (uint32_t)n << nshifts;
}

/**
 * \brief Allocate buffer of correct size for computing hash code.
 *
 * The input buffer needs to be padded with a \c 1 bit and 8 bytes
 * containing the buffer size in big endian format. The total size of
 * the buffer size must therefore be greater or equal to:
 * \code
 * buffer size + 1 (1...) + 8 (length) = buffer size + 9 bytes
 * \endcode
 */
uint8_t *alloc_padded(uint64_t size, uint64_t buffer_size, size_t *sz,
                      uint8_t *tmpbuf);
/**
 * @}
 */
