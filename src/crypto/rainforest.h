#ifndef RAINFOREST_H
#define RAINFOREST_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

void rf256_hash(void *out, const void *in, size_t len);
static inline void rainforest_hash(const char* input, char* output, uint32_t len) {
  uint8_t state[32];
  rf256_hash(state, input, len);
  rf256_hash(output, state, sizeof(state));
}

#ifdef __cplusplus
}
#endif

#endif