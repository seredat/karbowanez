// RainForest hash algorithm
// Author: Bill Schneider
// Date: Feb 13th, 2018
//
// RainForest uses native integer operations which are extremely fast on
// modern 64-bit processors, significantly slower on 32-bit processors such
// as GPUs, and extremely slow if at all implementable on FPGAs and ASICs.
// It makes an intensive use of the L1 cache to maintain a heavy intermediary
// state favoring modern CPUs compared to GPUs (small L1 cache shared by many
// shaders) or FPGAs (very hard to implement the required low-latency cache)
// when scanning ranges for nonces. The purpose is to create a fair balance
// between all mining equipments, from mobile phones to extreme performance
// GPUs and to rule out farming factories relying on ASICs and FPGAs. The
// CRC32 instruction is used a lot as it is extremely fast on low-power ARM
// chips and allows such devices to rival high-end PCs mining performance.
//

#ifndef RAINFOREST
#define RAINFOREST

#define __attribute__(X)
#include <stdint.h>

// this seems necessary only for gcc, otherwise hash is bogus
typedef __attribute__((may_alias)) uint8_t  rf_u8;
typedef __attribute__((may_alias)) uint16_t rf_u16;
typedef __attribute__((may_alias)) uint32_t rf_u32;
typedef __attribute__((may_alias)) uint64_t rf_u64;

// 2048 entries for the rambox => 16kB
#define RAMBOX_SIZE 2048
#define RAMBOX_LOOPS 4

typedef union {
  rf_u8  b[32];
  rf_u16 w[16];
  rf_u32 d[8];
  rf_u64 q[4];
} hash256_t;

typedef struct __attribute__((aligned(16))) rf_ctx {
  uint64_t rambox[RAMBOX_SIZE];
  hash256_t hash;
  uint32_t crc;
  uint32_t word;  // LE pending message
  uint32_t len;   // total message length
} rf256_ctx_t;

void rf256_init(rf256_ctx_t *ctx);
void rf256_update(rf256_ctx_t *ctx, const void *msg, size_t len);
void rf256_final(void *out, rf256_ctx_t *ctx);
void rf256_hash(void *out, const void *in, size_t len);
void rf256_hash2(void *out, const void *in, size_t len, uint32_t seed);

#endif