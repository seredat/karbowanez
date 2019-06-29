#ifndef _JH_H_
#define _JH_H_     1

/*This program gives the 64-bit optimized bitslice implementation of JH using ANSI C

   --------------------------------
   Performance

   Microprocessor: Intel CORE 2 processor (Core 2 Duo Mobile T6600 2.2GHz)
   Operating System: 64-bit Ubuntu 10.04 (Linux kernel 2.6.32-22-generic)
   Speed for long message:
   1) 45.8 cycles/byte   compiler: Intel C++ Compiler 11.1   compilation option: icc -O2
   2) 56.8 cycles/byte   compiler: gcc 4.4.3                 compilation option: gcc -O3

   --------------------------------
   Last Modified: January 16, 2011
*/
#pragma once

#include <stdint.h>
#include <string.h>

/*typedef unsigned long long uint64;*/
typedef uint64_t uint64;

/*define data alignment for different C compilers*/
#if defined(__GNUC__)
#define DATA_ALIGN16(x) x __attribute__ ((aligned(16)))
#else
#define DATA_ALIGN16(x) __declspec(align(16)) x
#endif

typedef unsigned char BitSequence;
typedef unsigned long long DataLength;
typedef enum { SUCCESS = 0, FAIL = 1, BAD_HASHLEN = 2 } HashReturn;

typedef struct {
  int hashbitlen;	   	              /*the message digest size*/
  unsigned long long databitlen;    /*the message size in bits*/
  unsigned long long datasize_in_buffer;      /*the size of the message remained in buffer; assumed to be multiple of 8bits except for the last partial block at the end of the message*/
  DATA_ALIGN16(uint64 x[8][2]);     /*the 1024-bit state, ( x[i][0] || x[i][1] ) is the ith row of the state in the pseudocode*/
  unsigned char buffer[64];         /*the 512-bit message block to be hashed;*/
} hashState;

/*The API functions*/
HashReturn J_Init(hashState *state, int hashbitlen);
HashReturn J_Update(hashState *state, const BitSequence *data, DataLength databitlen);
HashReturn J_Final(hashState *state, BitSequence *hashval);

HashReturn jh_hash(int hashbitlen, const BitSequence *data, DataLength databitlen, BitSequence *hashval);

#endif  /* ifndef _JH_H_ */
