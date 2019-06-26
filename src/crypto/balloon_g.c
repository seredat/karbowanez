// Copyright (c) 2016, Dan Boneh, Henry Corrigan-Gibbs, Stuart Schechter
// Copyright (c) 2019, itwysgsl
// Copyright (c) 2019, The Karbo developers
//
// This file is part of Karbo.
//
// Karbo is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Karbo is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with Karbo.  If not, see <http://www.gnu.org/licenses/>.

// https://github.com/itwysgsl/balloon/


#include "groestl.h"
#include "groestl.c"

#include <stdlib.h>
#include <stdint.h>
#include <string.h>

inline uint64_t u8tou64(uint8_t const* u8){
	uint64_t u64;
	memcpy(&u64, u8, sizeof(u64));
	return u64;
}

void balloon_g(const unsigned char* input, char* output, int length, const unsigned char* salt, int salt_length)
{
	const uint64_t s_cost = 64;
	const uint64_t t_cost = 2;
	const int delta = 3;
	
	hashState ctx_groestl;
	uint8_t blocks[64][64];
	
	// Step 1: Expand input into buffer
	uint64_t cnt = 0;
	Init(&ctx_groestl);
  Update(&ctx_groestl, (uint8_t *)&cnt, sizeof((uint8_t *)&cnt));
  Update(&ctx_groestl, input, length);
  Update(&ctx_groestl, salt, salt_length);
  Final(&ctx_groestl, blocks[0]);
	cnt++;

	for (int m = 1; m < s_cost; m++) {
    Update(&ctx_groestl, (uint8_t *)&cnt, sizeof((uint8_t *)&cnt));
    Update(&ctx_groestl, blocks[m - 1], 64);
		Final(&ctx_groestl, blocks[m]);
		cnt++;
	}

	// Step 2: Mix buffer contents
	for (uint64_t t = 0; t < t_cost; t++) {
		for (uint64_t m = 0; m < s_cost; m++) {
			// Step 2a: Hash last and current blocks
      Update(&ctx_groestl, (uint8_t *)&cnt, sizeof((uint8_t *)&cnt));
      Update(&ctx_groestl, blocks[(m - 1) % s_cost], 64);
      Update(&ctx_groestl, blocks[m], 64);
			Final(&ctx_groestl, blocks[m]);
			cnt++;

			for (uint64_t i = 0; i < delta; i++) {
				uint8_t index[64];
        Update(&ctx_groestl, (uint8_t *)&t, sizeof((uint8_t *)&t));
        Update(&ctx_groestl, (uint8_t *)&cnt, sizeof((uint8_t *)&cnt));
        Update(&ctx_groestl, (uint8_t *)&m, sizeof((uint8_t *)&m));
        Update(&ctx_groestl, salt, salt_length);
        Update(&ctx_groestl, (uint8_t *)&i, sizeof((uint8_t *)&i));
				Final(&ctx_groestl, index);
				cnt++;

				uint64_t other = u8tou64(index) % s_cost;
				cnt++;

        Update(&ctx_groestl, (uint8_t *)&cnt, sizeof((uint8_t *)&cnt));
        Update(&ctx_groestl, blocks[m], 64);
        Update(&ctx_groestl, blocks[other], 64);
				Final(&ctx_groestl, blocks[m]);
				cnt++;
			}
		}
	}
	
	memcpy(output, blocks[s_cost - 1], 32);
}
