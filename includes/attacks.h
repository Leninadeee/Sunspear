#ifndef ATTACKS_H
#define ATTACKS_H

#include <immintrin.h>
#include <stdbool.h>

#include "bitboard.h"

#define HAS_PEXT    1

extern  bb64    ptable[2][64];  /* All pawn attack tables   */
extern  bb64    ntable[64];     /* All knight attack tables */
extern  bb64    ktable[64];     /* All king attack tables   */
extern  bb64    bmask[64];      /* All bishop attack masks  */
extern  bb64    rmask[64];      /* All rook attack masks    */
extern  bb64    *btable[64];    /* All bishop attack tables */
extern  bb64    *rtable[64];    /* All rook attack tables   */

extern  void    bishop_pext_tables(void);
extern  void    rook_pext_tables(void);
extern  void    init_leapers(void);
extern  void    init_sliders(void);

/* Returns a bishop attack configuration */
static inline bb64 bishop_attacks_pext(int sq, bb64 occ) {
    bb64 mask = bmask[sq];
    uint64_t key = _pext_u64(occ, mask);
    return btable[sq][key];
}

/* Returns a rook attack configuration */
static inline bb64 rook_attacks_pext(int sq, bb64 occ) {
    bb64 mask = rmask[sq];
    uint64_t key = _pext_u64(occ, mask);
    return rtable[sq][key];
}

/* Returns a queen attack configuration */
static inline bb64 queen_attacks_pext(int sq, bb64 occ) {
    return bishop_attacks_pext(sq, occ) | rook_attacks_pext(sq, occ);
}

/* Check if a square sq is attacked by color c */
static inline bool checksquare(Color c, Square sq)
{
    const bb64 occ = pos.both;

    const int P = (c == WHITE) ? W_PAWN   : B_PAWN;
    const int N = (c == WHITE) ? W_KNIGHT : B_KNIGHT;
    const int B = (c == WHITE) ? W_BISHOP : B_BISHOP;
    const int R = (c == WHITE) ? W_ROOK   : B_ROOK;
    const int Q = (c == WHITE) ? W_QUEEN  : B_QUEEN;
    const int K = (c == WHITE) ? W_KING   : B_KING;

    if (ptable[c ^ 1][sq] & pos.pcbb[P]) return true;

    if (ntable[sq] & pos.pcbb[N]) return true;

    if (bishop_attacks_pext(sq, occ) & (pos.pcbb[B] | pos.pcbb[Q])) return true;

    if (rook_attacks_pext(sq, occ) & (pos.pcbb[R] | pos.pcbb[Q])) return true;

    if (ktable[sq] & pos.pcbb[K]) return true;

    return false;
}

#endif /* ATTACKS_H */