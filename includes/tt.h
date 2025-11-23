#ifndef TT_H
#define TT_H

#include "types.h"

#define  INIT_SEED  1804289383

#define  TT_EXACT   0
#define  TT_ALPHA   1
#define  TT_BETA    2
#define  TT_UNKNOWN 60000
#define  TT_SIZE    0x400000
#define  MATE_BOUND 48000

/* Zobrist Hashing Schema */
extern  uint32_t  seed;
extern  uint64_t  Z_PSQ[12][64];
extern  uint64_t  Z_CSTL[16];
extern  uint64_t  Z_EP[64];
extern  uint64_t  Z_SIDE;

extern  TTentry   TTable[TT_SIZE];

// d=depth a=alpha b=beta
extern  void      init_zobrist(void);
extern  uint64_t  gen_key(Position P);
extern  int       tt_read(uint64_t zob, int d, int ply, int a, int b);
extern  void      tt_write(uint64_t zob, int d, int ply, int eval, int flag);
extern  void      tt_clear(void);

static inline uint32_t prng32()
{
    uint32_t rand32 = seed;

    rand32 ^= rand32 << 13;
    rand32 ^= rand32 >> 17;
    rand32 ^= rand32 << 5;

    seed = rand32;
    return rand32;
}

static inline uint64_t prng64()
{
    bb64 n1, n2, n3, n4;

    n1 = (bb64)(prng32()) & 0xFFFF;
    n2 = (bb64)(prng32()) & 0xFFFF;
    n3 = (bb64)(prng32()) & 0xFFFF;
    n4 = (bb64)(prng32()) & 0xFFFF;
    
    return n1 | (n2 << 16) | (n3 << 32) | (n4 << 48);
}

#endif /* TT_H */