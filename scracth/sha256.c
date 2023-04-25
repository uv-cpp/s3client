// sha256.c - SHA reference implementation
// Copied from Jeffrey Walton's version
//
// Testing with non-empty message, refactored code, more similar to Wikipedia
// version Removed bitshifts on final hash, works on little-endian architectures
// only
//-----------------------------------------------------------------------------
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
#define RIGHT_ROTATE(x, y) (((x) >> (y)) | ((x) << (32 - (y))))
#define S0(x)                                                                  \
  (RIGHT_ROTATE((x), 2) ^ RIGHT_ROTATE((x), 13) ^ RIGHT_ROTATE((x), 22))
#define S1(x)                                                                  \
  (RIGHT_ROTATE((x), 6) ^ RIGHT_ROTATE((x), 11) ^ RIGHT_ROTATE((x), 25))

#define CH(x, y, z) (((x) & (y)) ^ ((~(x)) & (z)))
#define MAJ(x, y, z) (((x) & (y)) ^ ((x) & (z)) ^ ((y) & (z)))

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
void sha256_stream(uint32_t hash[8], const uint8_t data[], uint32_t length) {
  uint32_t h0, h1, h2, h3, h4, h5, h6, h7;
  uint32_t s0, s1;
  uint32_t tmp1, tmp2;
  uint32_t w[16];

  uint64_t blocks = length / 64;
  while (blocks--) {
    h0 = hash[0];
    h1 = hash[1];
    h2 = hash[2];
    h3 = hash[3];
    h4 = hash[4];
    h5 = hash[5];
    h6 = hash[6];
    h7 = hash[7];

    for (int i = 0; i != 16; ++i) {
      w[i] = lshift(data[0], 24) | lshift(data[1], 16) | lshift(data[2], 8) |
             lshift(data[3], 0);
      data += 4;

      tmp1 = h7;
      tmp1 += S1(h4);
      tmp1 += CH(h4, h5, h6);
      tmp1 += K[i];
      tmp1 += w[i];

      tmp2 = S0(h0);
      tmp2 += MAJ(h0, h1, h2);

      h7 = h6;
      h6 = h5;
      h5 = h4;
      h4 = h3 + tmp1;
      h3 = h2;
      h2 = h1;
      h1 = h0;
      h0 = tmp1 + tmp2;
    }

    for (int i = 16; i != 64; ++i) {
      s0 = w[(i + 1) & 0x0f];
      s0 = RIGHT_ROTATE(s0, 7) ^ RIGHT_ROTATE(s0, 18) ^ ((s0) >> 3);
      s1 = w[(i + 14) & 0x0f];
      s1 = RIGHT_ROTATE(s1, 17) ^ RIGHT_ROTATE(s1, 19) ^ (s1 >> 10);

      tmp1 = w[i & 0xf] += s0 + s1 + w[(i + 9) & 0xf];
      tmp1 += h7 + S1(h4) + CH(h4, h5, h6) + K[i];
      tmp2 = S0(h0) + MAJ(h0, h1, h2);
      h7 = h6;
      h6 = h5;
      h5 = h4;
      h4 = h3 + tmp1;
      h3 = h2;
      h2 = h1;
      h1 = h0;
      h0 = tmp1 + tmp2;
    }

    hash[0] += h0;
    hash[1] += h1;
    hash[2] += h2;
    hash[3] += h3;
    hash[4] += h4;
    hash[5] += h5;
    hash[6] += h6;
    hash[7] += h7;
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
  const size_t BUFSIZE = 0x100000; // 1 MiB
  char buf[BUFSIZE];
  memset(buf, 0, BUFSIZE);
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
      uint8_t message[message_size];
      memset(message, 0, message_size);
      memcpy(message, buf, bytes);
      // pad with '1' and zeros (array already zeroed out)
      message[bytes] = 0x80; // 100..
      // pad with length in big endian format
      const uint64_t data_bit_size = 8 * (length + bytes);
      const uint64_t size = to_big_endian(data_bit_size);
      memcpy(&message[message_size - 8], &size, sizeof(uint64_t));
      sha256_stream(hash, (uint8_t *)message, message_size);
      break;
    } else {

      sha256_stream(hash, (uint8_t *)buf, (uint32_t)BUFSIZE);
    }
    memset(buf, 0, BUFSIZE);
    length += bytes;
  }
  to_little(hash);
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
  const char c[] = "12345678"
                   "12345678"
                   "12345678"
                   "12345678"
                   "12345678"
                   "12345678";
  // write to file so that it can be fed to sha256sum or equivalent
  // command to test correctness
  FILE *out = fopen(file_name, "wb");
  if (!out) {
    fprintf(stderr, "Error opening file %s\n", file_name);
    exit(EXIT_FAILURE);
  }
  const uint64_t data_size = strlen(c);
  fwrite(c, data_size, 1, out);
  fclose(out);
  uint8_t m[data_size];
  // read from file to make sure it's the same input sent to
  // sha256 hashing application
  FILE *in = fopen(file_name, "rb");
  if (!fread(&m, data_size, 1, in)) {
    fprintf(stderr, "Error reading from file %s\n", file_name);
    exit(EXIT_FAILURE);
  }
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
  const char c[] = "12345678"
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
  // write to file so that it can be fed to sha256sum or equivalent
  // command to test correctness
  FILE *out = fopen(file_name, "wb");
  if (!out) {
    fprintf(stderr, "Error opening file %s\n", file_name);
    exit(EXIT_FAILURE);
  }
  const uint64_t data_size = strlen(c);
  fwrite(c, data_size, 1, out);
  fclose(out);
  uint8_t m[data_size];
  // read from file to make sure it's the same input sent to
  // sha256 hashing application
  FILE *in = fopen(file_name, "rb");
  if (!fread(&m, data_size, 1, in)) {
    fprintf(stderr, "Error reading from file %s\n", file_name);
    exit(EXIT_FAILURE);
  }
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
  const char c[] = "12345678"
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
  // write to file so that it can be fed to sha256sum or equivalent
  // command to test correctness
  FILE *out = fopen(file_name, "wb");
  if (!out) {
    fprintf(stderr, "Error opening file %s\n", file_name);
    exit(EXIT_FAILURE);
  }
  const uint64_t data_size = strlen(c);
  fwrite(c, data_size, 1, out);
  fclose(out);
  uint8_t m[data_size];
  // read from file to make sure it's the same input sent to
  // sha256 hashing application
  FILE *in = fopen(file_name, "rb");
  if (!fread(&m, data_size, 1, in)) {
    fprintf(stderr, "Error reading from file %s\n", file_name);
    exit(EXIT_FAILURE);
  }
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
  char hash_text[64];
  hash_to_text(hash, hash_text);
  assert(strncmp(hash_text, test_hash, 64) == 0 && "test 4");
  printf("Test 4 passed\n");
  // print_hash(hash);
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
