#ifndef RAINFOREST_H
#define RAINFOREST_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

void rf256_hash(void *out, const void *in, size_t len);
void rainforest_hash(const void* input, void* output, uint32_t len);

#ifdef __cplusplus
}
#endif

#endif