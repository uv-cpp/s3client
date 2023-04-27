// sha256.c - SHA256 reference implementation
//
//-----------------------------------------------------------------------------
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//-----------------------------------------------------------------------------
static inline uint64_t to_big_endian(uint64_t n) {
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
static inline uint32_t to_little_endian(uint32_t n) {
  const uint64_t b0 = (n & 0xff000000) >> 24u;
  const uint64_t b1 = (n & 0x00ff0000) >> 8u;
  const uint64_t b2 = (n & 0x0000ff00) << 8u;
  const uint64_t b3 = (n & 0x000000ff) << 24u;
  return b0 | b1 | b2 | b3;
}

static inline void to_little(uint32_t hash[8]) {
  for (size_t i = 0; i != 8; i++)
    hash[i] = to_little_endian(hash[i]);
}

uint64_t next_div_by(uint64_t n, uint64_t d) {
  for (; n % d; n++)
    ;
  return n;
}

void hash_to_text(uint32_t hash[8], char *text) {
  const unsigned char *h = (unsigned char *)hash;
  char hash_text[65];
  for (size_t i = 0; i != 32; ++i) {
    snprintf(text + 2 * i, 3, "%02x", h[i]);
  }
}
//-----------------------------------------------------------------------------
// right rotate
static inline uint32_t right_rotate(uint32_t x, uint32_t n) {
  return (x >> n) | (x << (32 - n));
}

static inline uint32_t Sigma0(uint32_t x) {
  return right_rotate(x, 2) ^ right_rotate(x, 13) ^ right_rotate(x, 22);
}
static inline uint32_t Sigma1(uint32_t x) {
  return right_rotate(x, 6) ^ right_rotate(x, 11) ^ right_rotate(x, 25);
}

// Choose: if c bit == 0 select bit from y else select bit from x
static inline uint32_t Ch(uint32_t c, uint32_t x, uint32_t y) {
  return (c & x) | ((~c) & y);
}

// Majority: if at least two zeroes return 0 else return 1
static inline uint32_t Maj(uint32_t x, uint32_t y, uint32_t z) {
  return (x & y) ^ (x & z) ^ (y & z);
}
// Initialize array of round constants:
// first 32 bits of the fractional parts of the cube roots of the first 64
// primes 2..311:
static const uint32_t K[] = {
    0x428A2F98, 0x71374491, 0xB5C0FBCF, 0xE9B5DBA5, 0x3956C25B, 0x59F111F1,
    0x923F82A4, 0xAB1C5ED5, 0xD807AA98, 0x12835B01, 0x243185BE, 0x550C7DC3,
    0x72BE5D74, 0x80DEB1FE, 0x9BDC06A7, 0xC19BF174, 0xE49B69C1, 0xEFBE4786,
    0x0FC19DC6, 0x240CA1CC, 0x2DE92C6F, 0x4A7484AA, 0x5CB0A9DC, 0x76F988DA,
    0x983E5152, 0xA831C66D, 0xB00327C8, 0xBF597FC7, 0xC6E00BF3, 0xD5A79147,
    0x06CA6351, 0x14292967, 0x27B70A85, 0x2E1B2138, 0x4D2C6DFC, 0x53380D13,
    0x650A7354, 0x766A0ABB, 0x81C2C92E, 0x92722C85, 0xA2BFE8A1, 0xA81A664B,
    0xC24B8B70, 0xC76C51A3, 0xD192E819, 0xD6990624, 0xF40E3585, 0x106AA070,
    0x19A4C116, 0x1E376C08, 0x2748774C, 0x34B0BCB5, 0x391C0CB3, 0x4ED8AA4A,
    0x5B9CCA4F, 0x682E6FF3, 0x748F82EE, 0x78A5636F, 0x84C87814, 0x8CC70208,
    0x90BEFFFA, 0xA4506CEB, 0xBEF9A3F7, 0xC67178F2};

// left shift of unsigned 8 bit int and conversion to 32 bit
static inline uint32_t lshift(uint8_t n, uint8_t nbits) {
  return (uint32_t)n << nbits;
}
// First 32 bits of the fractional parts of the square roots of the first 8
// primes 2..19
void init_with_square_roots(uint32_t hash[8]) {

  uint32_t h[8] = {0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a,
                   0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19};
  memcpy(hash, h, 8 * sizeof(uint32_t));
}

// sha256 algorithm, streaming version: receives and updates hash
// data is unsigned byte, all other variables are usigned 32 bit int
void sha256_stream(uint32_t hash[8], const uint8_t data[], uint64_t length) {
  uint32_t a, b, c, d, e, f, g, h;
  uint32_t w[16];
  uint64_t blocks = length / 64;
  while (blocks--) {
    a = hash[0];
    b = hash[1];
    c = hash[2];
    d = hash[3];
    e = hash[4];
    f = hash[5];
    g = hash[6];
    h = hash[7];

    for (int i = 0; i != 16; ++i) {
      w[i] = lshift(data[0], 24) | lshift(data[1], 16) | lshift(data[2], 8) |
             lshift(data[3], 0);
      data += 4;

      const uint32_t tmp1 = h + Sigma1(e) + Ch(e, f, g) + K[i] + w[i];
      const uint32_t tmp2 = Sigma0(a) + Maj(a, b, c);
      h = g;
      g = f;
      f = e;
      e = d + tmp1;
      d = c;
      c = b;
      b = a;
      a = tmp1 + tmp2;
    }

    for (int i = 16; i != 64; ++i) {
      const uint32_t s0 = w[(i + 1) & 0x0f];
      const uint32_t sigma0 =
          right_rotate(s0, 7) ^ right_rotate(s0, 18) ^ (s0 >> 3);
      const uint32_t s1 = w[(i + 14) & 0x0f];
      const uint32_t sigma1 =
          right_rotate(s1, 17) ^ right_rotate(s1, 19) ^ (s1 >> 10);
      w[i & 0xf] += sigma0 + sigma1 + w[(i + 9) & 0xf];
      const uint32_t tmp1 = w[i & 0xf] + h + Sigma1(e) + Ch(e, f, g) + K[i];
      const uint32_t tmp2 = Sigma0(a) + Maj(a, b, c);
      h = g;
      g = f;
      f = e;
      e = d + tmp1;
      d = c;
      c = b;
      b = a;
      a = tmp1 + tmp2;
    }

    hash[0] += a;
    hash[1] += b;
    hash[2] += c;
    hash[3] += d;
    hash[4] += e;
    hash[5] += f;
    hash[6] += g;
    hash[7] += h;
  }
}

// sha256 on single fixed size buffer
void sha256(const uint8_t data[], uint32_t length, uint32_t hash[8]) {
  init_with_square_roots(hash);
  sha256_stream(hash, data, length);
  to_little(hash);
}

void print_hash(uint32_t hash[8]) {
  const unsigned char *ph = (unsigned char *)hash;
  for (size_t i = 0; i != 32; ++i)
    printf("%02x", ph[i]);
  printf("\n");
}

// calculate file SHA256
void sha256_file(const char *fname, uint32_t hash[8]) {
  FILE *f = fopen(fname, "rb");
  if (!f) {
    fprintf(stderr, "Error opening file %s\n", fname);
    exit(EXIT_FAILURE);
  }
  const size_t BUFSIZE = 0x1000000; // 16 MiB
  char *buf = (char *)calloc(BUFSIZE, sizeof(char));
  assert(buf);
  init_with_square_roots(hash);
  size_t length = 0;
  while (1) {
    size_t bytes = fread(buf, 1, BUFSIZE, f);
    if (!bytes) {
      perror("Error reading from file");
      exit(EXIT_FAILURE);
    }
    int eof = 0;
    if (bytes < BUFSIZE)
      eof = 1;
    else {
      int c = getc(f);
      ungetc(c, f);
      if (c == EOF)
        eof = 1;
    }
    if (eof) {
      const uint64_t message_size = next_div_by(bytes + 1 + 8, 64);
      // allocate and zero out
      uint8_t *message = (uint8_t *)calloc(message_size, sizeof(char));
      assert(buf);
      memcpy(message, buf, bytes);
      // pad with '1' and zeros (array already zeroed out)
      message[bytes] = 0x80; // 100..
      // pad with length in big endian format
      const uint64_t data_bit_size = 8 * (length + bytes);
      const uint64_t size = to_big_endian(data_bit_size);
      memcpy(&message[message_size - 8], &size, sizeof(uint64_t));
      sha256_stream(hash, (uint8_t *)message, message_size);
      free(message);
      break;
    } else {

      sha256_stream(hash, (uint8_t *)buf, (uint32_t)BUFSIZE);
    }
    memset(buf, 0, BUFSIZE);
    length += bytes;
  }
  to_little(hash);
  free(buf);
}

//-----------------------------------------------------------------------------
#ifdef TEST
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *file_name = "tmp-input";

// signgle block
void test1() {
  static const char *sha256sum_generated =
      "dd7f20ca4910f937c3e560427de36fea7c37eed94899b3a9bf286905860d17ae";
  // message
  const char m[] = "12345678"
                   "12345678"
                   "12345678"
                   "12345678"
                   "12345678"
                   "12345678";
  const uint64_t data_size = strlen(m);
  const uint64_t message_size = 64; // single 512 block
  uint8_t message[message_size];
  memset(message, 0, message_size);
  memcpy(message, m, data_size);
  // pad with '1' and zeros (array already zeroed out)
  message[data_size] = 0x80; // 1 + 0 x 63
  // pad with length in big endian format
  const uint64_t data_bit_size = 8 * data_size;
  const uint64_t size = to_big_endian(data_bit_size);
  memcpy(&message[56], &size, 8);

  uint32_t hash[8];
  sha256(message, message_size, hash);
  const unsigned char *h = (unsigned char *)hash;
  // 64 chars + null terminator
  char hash_text[65];
  for (size_t i = 0; i != 32; ++i) {
    snprintf(hash_text + 2 * i, 3, "%02x", h[i]);
  }
  // printf("%s\n", hash_text);
  assert(strncmp(hash_text, sha256sum_generated, 65) == 0 && "test 1");
  printf("Test 1 passed\n");
}

// multi-block
void test2() {
  static const char *sha256sum_generated =
      "0c65765f1b9fff74bb831fa24c63d9ab0513c881fc7b4919b43f72f5487a24fd";
  // message 14 x 8 + 7 bytes
  const char m[] = "12345678"
                   "12345678"
                   "12345678"
                   "12345678"
                   "12345678"
                   "12345678"
                   "12345678"
                   "12345678"
                   "12345678"
                   "12345678"
                   "12345678"
                   "12345678"
                   "12345678"
                   "12345678"
                   "1234567";
  const uint64_t data_size = strlen(m);
  // message size = next number divisable by 64
  const uint64_t message_size = next_div_by(data_size + 1 + 8, 64);
  // allocate and zero out
  uint8_t message[message_size];
  memset(message, 0, message_size);
  memcpy(message, m, data_size);
  // pad with '1' and zeros (array already zeroed out)
  message[data_size] = 0x80; // 100..
  // pad with length in big endian format
  const uint64_t data_bit_size = 8 * data_size;
  const uint64_t size = to_big_endian(data_bit_size);
  memcpy(&message[message_size - 8], &size, sizeof(uint64_t));

  uint32_t hash[8];
  sha256(message, message_size, hash);

  const unsigned char *h = (unsigned char *)hash;
  char hash_text[65];
  for (size_t i = 0; i != 32; ++i) {
    snprintf(hash_text + 2 * i, 3, "%02x", h[i]);
  }
  // printf("%s\n", hash_text);
  assert(strncmp(hash_text, sha256sum_generated, 64) == 0 && "test 2");
  printf("Test 2 passed\n");
}

// multi-block 2
void test3() {
  static const char *sha256sum_generated =
      "979e3016a670a5b1308dba2d715f75201eebcef0adc4a1ac99877fad91ce3ff6";
  // message 15 x 8 bytes
  const char m[] = "12345678"
                   "12345678"
                   "12345678"
                   "12345678"
                   "12345678"
                   "12345678"
                   "12345678"
                   "12345678"
                   "12345678"
                   "12345678"
                   "12345678"
                   "12345678"
                   "12345678"
                   "12345678"
                   "12345678";
  const uint64_t data_size = strlen(m);
  // message size = next number divisable by 64
  // need to always allocate at least 1 byte for 0x80 (1...) padding and 8 bytes
  // for length data
  const uint64_t message_size = next_div_by(data_size + 1 + 8, 64);
  // allocate and zero out
  uint8_t message[message_size];
  memset(message, 0, message_size);
  memcpy(message, m, data_size);
  // pad with '1' and zeros (array already zeroed out)
  message[data_size] = 0x80; // 100..
  // pad with length in big endian format
  const uint64_t data_bit_size = 8 * data_size;
  const uint64_t size = to_big_endian(data_bit_size);
  memcpy(&message[message_size - 8], &size, sizeof(uint64_t));

  uint32_t hash[8];
  sha256(message, message_size, hash);

  const unsigned char *h = (unsigned char *)hash;
  char hash_text[65];
  for (size_t i = 0; i != 32; ++i) {
    snprintf(hash_text + 2 * i, 3, "%02x", h[i]);
  }
  // printf("%s\n", hash_text);
  assert(strncmp(hash_text, sha256sum_generated, 64) == 0 && "test 3");
  printf("Test 3 passed\n");
}

void test4(const char *fname, const char *test_hash) {
  uint32_t hash[8];
  sha256_file(fname, hash);
  char hash_text[65];
  hash_to_text(hash, hash_text);
  assert(strncmp(hash_text, test_hash, 64) == 0 && "test 4");
  printf("Test 4 passed\n");
}

//
int main(int argc, char *argv[]) {
  test1();
  test2();
  test3();
  if (argc != 3) {
    fprintf(stderr, "Usage: %s <test file> <test file hash>\n", argv[0]);
    exit(EXIT_FAILURE);
  }
  test4(argv[1], argv[2]);
  return 0;
}
#endif
