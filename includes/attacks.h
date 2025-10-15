#ifndef ATTACKS_H
#define ATTACKS_H

#include <immintrin.h>
#include <stdbool.h>

#include "types.h"

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

#endif /* ATTACKS_H */