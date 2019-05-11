/*
 * Argon2 reference source code package - reference C implementations
 *
 * Copyright 2015
 * Daniel Dinu, Dmitry Khovratovich, Jean-Philippe Aumasson, and Samuel Neves
 *
 * You may use this work under the terms of a Creative Commons CC0 1.0
 * License/Waiver or the Apache Public License 2.0, at your option. The terms of
 * these licenses can be found at:
 *
 * - CC0 1.0 Universal : http://creativecommons.org/publicdomain/zero/1.0
 * - Apache 2.0        : http://www.apache.org/licenses/LICENSE-2.0
 *
 * You should have received a copy of both of these licenses along with this
 * software. If not, they may be obtained at the above URLs.
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#ifdef _MSC_VER
#include <intrin.h>
#endif

#include "argon2.h"

static uint64_t rdtsc(void) {
#ifdef _MSC_VER
    return __rdtsc();
#else
#if defined(__amd64__) || defined(__x86_64__)
    uint64_t rax, rdx;
    __asm__ __volatile__("rdtsc" : "=a"(rax), "=d"(rdx) : :);
    return (rdx << 32) | rax;
#elif defined(__i386__) || defined(__i386) || defined(__X86__)
    uint64_t rax;
    __asm__ __volatile__("rdtsc" : "=A"(rax) : :);
    return rax;
#elif defined(__aarch64__)
  // System timer of ARMv8 runs at a different frequency than the CPU's.
  // The frequency is fixed, typically in the range 1-50MHz.  It can be
  // read at CNTFRQ special register.  We assume the OS has set up
  // the virtual timer properly.
  int64_t virtual_timer_value;
  __asm__ volatile("mrs %0, cntvct_el0" : "=r"(virtual_timer_value));
  return virtual_timer_value;
#elif defined(__arm__) && (__ARM_ARCH >= 6)
  uint32_t pmccntr;
  uint32_t pmuseren;
  uint32_t pmcntenset;
  // Read the user mode perf monitor counter access permissions.
  __asm__ volatile("mrc p15, 0, %0, c9, c14, 0" : "=r"(pmuseren));
  if (pmuseren & 1) {  // Allows reading perfmon counters for user mode code.
    __asm__ volatile("mrc p15, 0, %0, c9, c12, 1" : "=r"(pmcntenset));
    if (pmcntenset & 0x80000000ul) {  // Is it counting?
      __asm__ volatile("mrc p15, 0, %0, c9, c13, 0" : "=r"(pmccntr));
      // The counter is set up to count every 64th cycle
      return ((int64_t) pmccntr) * 64;  // Should optimize to << 6
    }
  }
#else
#error "Not implemented!"
#endif
#endif
}

/*
 * Benchmarks Argon2 with salt length 16, password length 16, t_cost 3,
   and different m_cost and threads
 */
static void benchmark() {
#define BENCH_OUTLEN 16
#define BENCH_INLEN 16
    const uint32_t inlen = BENCH_INLEN;
    const unsigned outlen = BENCH_OUTLEN;
    unsigned char out[BENCH_OUTLEN];
    unsigned char pwd_array[BENCH_INLEN];
    unsigned char salt_array[BENCH_INLEN];
#undef BENCH_INLEN
#undef BENCH_OUTLEN

    uint32_t t_cost = 3;
    uint32_t m_cost;
    uint32_t thread_test[4] = {1, 2, 4,  8};
    argon2_type types[3] = {Argon2_i, Argon2_d, Argon2_id};

    memset(pwd_array, 0, inlen);
    memset(salt_array, 1, inlen);

    for (m_cost = (uint32_t)1 << 10; m_cost <= (uint32_t)1 << 22; m_cost *= 2) {
        unsigned i;
        for (i = 0; i < 4; ++i) {
            double run_time = 0;
            uint32_t thread_n = thread_test[i];

            unsigned j;
            for (j = 0; j < 3; ++j) {
                clock_t start_time, stop_time;
                uint64_t start_cycles, stop_cycles;
                uint64_t delta;
                double mcycles;

                argon2_type type = types[j];
                start_time = clock();
                start_cycles = rdtsc();

                argon2_hash(t_cost, m_cost, thread_n, pwd_array, inlen,
                            salt_array, inlen, out, outlen, NULL, 0, type,
                            ARGON2_VERSION_NUMBER);

                stop_cycles = rdtsc();
                stop_time = clock();

                delta = (stop_cycles - start_cycles) / (m_cost);
                mcycles = (double)(stop_cycles - start_cycles) / (1UL << 20);
                run_time += ((double)stop_time - start_time) / (CLOCKS_PER_SEC);

                printf("%s %d iterations  %d MiB %d threads:  %2.2f cpb %2.2f "
                       "Mcycles \n", argon2_type2string(type, 1), t_cost,
                       m_cost >> 10, thread_n, (float)delta / 1024, mcycles);
            }

            printf("%2.4f seconds\n\n", run_time);
        }
    }
}

int main() {
    benchmark();
    return ARGON2_OK;
}
