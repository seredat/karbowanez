// https://github.com/itwysgsl/balloon/

#ifndef BALLOON_H
#define BALLOON_H

#include <stddef.h>

#define S_COST (uint64_t)56
#define T_COST (uint64_t)8
#define DELTA  (uint64_t)7

#ifdef __cplusplus
extern "C" {
#endif

void balloon_blake(const unsigned char* input, char* output, size_t length, const unsigned char* salt, size_t salt_length);

#ifdef __cplusplus
}
#endif

#endif
