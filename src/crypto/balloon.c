// https://github.com/itwysgsl/balloon/

#include "balloon.h"
#include "blake256.h"

#include <stdlib.h>
#include <stdint.h>
#include <string.h>

uint64_t u8tou64(uint8_t const* u8){
  uint64_t u64;
  memcpy(&u64, u8, sizeof(u64));
  return u64;
}

void balloon_blake(const unsigned char* input, char* output, size_t length, const unsigned char* salt, size_t salt_length)
{
  state ctx;
  uint8_t blocks[S_COST][64];

  // Step 1: Expand input into buffer
  uint64_t cnt = 0;
  blake256_init(&ctx);
  blake256_update(&ctx, (uint8_t *)&cnt, sizeof((uint8_t *)&cnt));
  blake256_update(&ctx, input, length);
  blake256_update(&ctx, salt, salt_length);
  blake256_final(&ctx, blocks[0]);
  cnt++;

  for (uint64_t m = 1; m < S_COST; m++) {
    blake256_update(&ctx, (uint8_t *)&cnt, sizeof((uint8_t *)&cnt));
    blake256_update(&ctx, blocks[m - 1], 64);
    blake256_final(&ctx, blocks[m]);
    cnt++;
  }

  // Step 2: Mix buffer contents
  for (uint64_t t = 0; t < T_COST; t++) {
    for (uint64_t m = 0; m < S_COST; m++) {
      // Step 2a: Hash last and current blocks
      blake256_update(&ctx, (uint8_t *)&cnt, sizeof((uint8_t *)&cnt));
      blake256_update(&ctx, blocks[(m - 1) % S_COST], 64);
      blake256_update(&ctx, blocks[m], 64);
      blake256_final(&ctx, blocks[m]);
      cnt++;

      for (uint64_t i = 0; i < DELTA; i++) {
        uint8_t index[64];
        blake256_update(&ctx, (uint8_t *)&t, sizeof((uint8_t *)&t));
        blake256_update(&ctx, (uint8_t *)&cnt, sizeof((uint8_t *)&cnt));
        blake256_update(&ctx, (uint8_t *)&m, sizeof((uint8_t *)&m));
        blake256_update(&ctx, salt, salt_length);
        blake256_update(&ctx, (uint8_t *)&i, sizeof((uint8_t *)&i));
        blake256_final(&ctx, index);
        cnt++;

        uint64_t other = u8tou64(index) % S_COST;
        cnt++;

        blake256_update(&ctx, (uint8_t *)&cnt, sizeof((uint8_t *)&cnt));
        blake256_update(&ctx, blocks[m], 64);
        blake256_update(&ctx, blocks[other], 64);
        blake256_final(&ctx, blocks[m]);
        cnt++;
      }
    }
  }

  memcpy(output, blocks[S_COST - 1], 32);
}
