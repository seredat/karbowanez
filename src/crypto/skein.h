#ifndef _SKEIN_H_
#define _SKEIN_H_     1
/**************************************************************************
**
** Interface declarations and internal definitions for Skein hashing.
**
** Source code author: Doug Whiting, 2008.
**
** This algorithm and source code is released to the public domain.
**
***************************************************************************
** 
** The following compile-time switches may be defined to control some
** tradeoffs between speed, code size, error checking, and security.
**
** The "default" note explains what happens when the switch is not defined.
**
**  SKEIN_DEBUG            -- make callouts from inside Skein code
**                            to examine/display intermediate values.
**                            [default: no callouts (no overhead)]
**
**  SKEIN_ERR_CHECK        -- how error checking is handled inside Skein
**                            code. If not defined, most error checking 
**                            is disabled (for performance). Otherwise, 
**                            the switch value is interpreted as:
**                                0: use assert()      to flag errors
**                                1: return SKEIN_FAIL to flag errors
**
***************************************************************************/
#include "skein_port.h"                      /* get platform-specific definitions */
#include <stddef.h>                          /* get size_t definition */
#include <string.h>      /* get the memcpy/memset functions */

#define  SKEIN_PORT_CODE /* instantiate any code in skein_port.h */

typedef enum
{
  SKEIN_SUCCESS         =      0,          /* return codes from Skein calls */
  SKEIN_FAIL            =      1,
  SKEIN_BAD_HASHLEN     =      2
}
HashReturn;

typedef size_t   DataLength;                /* bit count  type */
typedef u08b_t   BitSequence;               /* bit stream type */

#define DISABLE_UNUSED 0

#ifndef SKEIN_256_NIST_MAX_HASHBITS
#define SKEIN_256_NIST_MAX_HASHBITS (0)
#endif

#ifndef SKEIN_512_NIST_MAX_HASHBITS
#define SKEIN_512_NIST_MAX_HASHBITS (512)
#endif

#define  SKEIN_MODIFIER_WORDS  ( 2)          /* number of modifier (tweak) words */

#define  SKEIN_256_STATE_WORDS ( 4)
#define  SKEIN_512_STATE_WORDS ( 8)
#define  SKEIN1024_STATE_WORDS (16)
#define  SKEIN_MAX_STATE_WORDS (16)

#define  SKEIN_256_STATE_BYTES ( 8*SKEIN_256_STATE_WORDS)
#define  SKEIN_512_STATE_BYTES ( 8*SKEIN_512_STATE_WORDS)
#define  SKEIN1024_STATE_BYTES ( 8*SKEIN1024_STATE_WORDS)

#define  SKEIN_256_STATE_BITS  (64*SKEIN_256_STATE_WORDS)
#define  SKEIN_512_STATE_BITS  (64*SKEIN_512_STATE_WORDS)
#define  SKEIN1024_STATE_BITS  (64*SKEIN1024_STATE_WORDS)

#define  SKEIN_256_BLOCK_BYTES ( 8*SKEIN_256_STATE_WORDS)
#define  SKEIN_512_BLOCK_BYTES ( 8*SKEIN_512_STATE_WORDS)
#define  SKEIN1024_BLOCK_BYTES ( 8*SKEIN1024_STATE_WORDS)

#define SKEIN_RND_SPECIAL       (1000u)
#define SKEIN_RND_KEY_INITIAL   (SKEIN_RND_SPECIAL+0u)
#define SKEIN_RND_KEY_INJECT    (SKEIN_RND_SPECIAL+1u)
#define SKEIN_RND_FEED_FWD      (SKEIN_RND_SPECIAL+2u)

typedef struct
{
  size_t  hashBitLen;                      /* size of hash result, in bits */
  size_t  bCnt;                            /* current byte count in buffer b[] */
  u64b_t  T[SKEIN_MODIFIER_WORDS];         /* tweak words: T[0]=byte cnt, T[1]=flags */
} Skein_Ctxt_Hdr_t;

typedef struct                               /*  256-bit Skein hash context structure */
{
  Skein_Ctxt_Hdr_t h;                      /* common header context variables */
  u64b_t  X[SKEIN_256_STATE_WORDS];        /* chaining variables */
  u08b_t  b[SKEIN_256_BLOCK_BYTES];        /* partial block buffer (8-byte aligned) */
} Skein_256_Ctxt_t;

typedef struct                               /*  512-bit Skein hash context structure */
{
  Skein_Ctxt_Hdr_t h;                      /* common header context variables */
  u64b_t  X[SKEIN_512_STATE_WORDS];        /* chaining variables */
  u08b_t  b[SKEIN_512_BLOCK_BYTES];        /* partial block buffer (8-byte aligned) */
} Skein_512_Ctxt_t;

typedef struct                               /* 1024-bit Skein hash context structure */
{
  Skein_Ctxt_Hdr_t h;                      /* common header context variables */
  u64b_t  X[SKEIN1024_STATE_WORDS];        /* chaining variables */
  u08b_t  b[SKEIN1024_BLOCK_BYTES];        /* partial block buffer (8-byte aligned) */
} Skein1024_Ctxt_t;

typedef struct
{
  uint_t  statebits;                      /* 256, 512, or 1024 */
  union
  {
    Skein_Ctxt_Hdr_t h;                 /* common header "overlay" */
    Skein_256_Ctxt_t ctx_256;
    Skein_512_Ctxt_t ctx_512;
    Skein1024_Ctxt_t ctx1024;
  } u;
}
hashState;                         /* bit stream type */

/* "incremental" hashing API */
HashReturn S_Init(hashState *state, int hashbitlen);
HashReturn S_Update(hashState *state, const BitSequence *data, DataLength databitlen);
HashReturn S_Final(hashState *state, BitSequence *hashval);

/* "all-in-one" call */
HashReturn skein_hash(int hashbitlen,   const BitSequence *data, 
                      DataLength databitlen,  BitSequence *hashval);

#endif  /* ifndef _SKEIN_H_ */
