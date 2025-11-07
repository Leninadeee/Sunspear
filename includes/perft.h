#ifndef PERFT_H
#define PERFT_H

#include <windows.h>

#include "types.h"

#define  POS1  "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"
#define  POS2  "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R " \
               "w KQkq - 0 1 "
#define  POS3  "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1 "
#define  POS4  "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 " \
               "w kq - 0 1"
#define  POS5  "rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8  "
#define  POS6  "r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 " \
               "w - - 0 10 "

static const ull pos1[] = {
    1ULL,
    20ULL,
    400ULL,
    8902ULL,
    197281ULL,
    4865609ULL,
    119060324ULL,
    3195901860ULL
};

static const ull pos2[] = {
    1ULL,
    48ULL,
    2039ULL,
    97862ULL,
    4085603ULL,
    193690690ULL,
    8031647685ULL,
};

static const ull pos3[] = {
    1ULL,
    14ULL,
    191ULL,
    2812ULL,
    43238ULL,
    674624ULL,
    11030083ULL,
    178633661ULL
};

static const ull pos4[] = {
    1ULL,
    6ULL,
    264ULL,
    9467ULL,
    422333ULL,
    15833292ULL,
    706045033ULL,
};

static const ull pos5[] = {
    1ULL,
    44ULL,
    1486ULL,
    62379ULL,
    2103487ULL,
    89941194ULL
};

static const ull pos6[] = {
    1ULL,
    46ULL,
    2079ULL,
    89890ULL,
    3894594ULL,
    164075551ULL,
    6923051137ULL
};

// d=depth
extern  ull   perft(Position *P, int d);
extern  void  perft_driver(int d);

static inline ull gettime(void)
{
    return GetTickCount64();
}

#endif /* PERFT_H */