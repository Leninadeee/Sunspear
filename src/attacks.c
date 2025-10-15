#include <string.h>

#include "attacks.h"asdfasdf
#include "bitboard.h"

bb64 bmask[64];     /* All bishop attack masks  */
bb64 rmask[64];     /* All rook attack masks    */

bb64 ptable[2][64]; /* All pawn attack tables   */
bb64 ntable[64];    /* All knight attack tables */
bb64 ktable[64];    /* All king attack tables   */
bb64 *btable[64];   /* All bishop attack tables */
bb64 *rtable[64];   /* All rook attack tables   */

static bb64 pawn_attack_mask(Color c, Square sq)
{
    bb64 bb = 0ULL;
    bb64 white = 0ULL;
    bb64 black = 0ULL;
    bb64 mask = 0ULL;

    setbit(bb, sq);

    white = ((bb & NOT_A) >> 9) | ((bb & NOT_H) >> 7);
    black = ((bb & NOT_A) << 7) | ((bb & NOT_H) << 9);
    mask = -(bb64)(c & 1);

    return (white & ~mask) | (black & mask);
}

static bb64 knight_attack_mask(Square sq)
{
    bb64 kn_attacks = 0ULL;
    bb64 bb = 0ULL;

    setbit(bb, sq);

    /* Attack downstream */
    kn_attacks |= (bb & NOT_A)  << 15;
    kn_attacks |= (bb & NOT_AB) << 6;
    kn_attacks |= (bb & NOT_H)  << 17;
    kn_attacks |= (bb & NOT_GH) << 10;

    /* Attack upstream */
    kn_attacks |= (bb & NOT_A)  >> 17;
    kn_attacks |= (bb & NOT_AB) >> 10;
    kn_attacks |= (bb & NOT_H)  >> 15;
    kn_attacks |= (bb & NOT_GH) >> 6;
    
    return kn_attacks;
}

static bb64 king_attack_mask(Square sq)
{
    bb64 K_attacks = 0ULL;
    bb64 bb = 0ULL;

    setbit(bb, sq);

    /* X,Y Attacks */
    K_attacks |= (bb << 8);
    K_attacks |= (bb >> 8);
    K_attacks |= ((bb & NOT_H) << 1);
    K_attacks |= ((bb & NOT_A) >> 1);

    /* Attack downstream */
    K_attacks |= ((bb & NOT_H) << 9);
    K_attacks |= ((bb & NOT_A) << 7);

    /* Attack upstream */
    K_attacks |= ((bb & NOT_H) >> 7);
    K_attacks |= ((bb & NOT_A) >> 9);

    return K_attacks;
}

static bb64 bishop_attack_mask(Square sq)
{
    bb64 attacks = 0ULL;

    int r, f;
    int tr = sq >> 3;
    int tf = sq & 7;

    for (r = tr + 1, f = tf + 1; r <= 6 && f <= 6; r++, f++) attacks |= (1ULL << (idx(r, f)));
    for (r = tr - 1, f = tf + 1; r >= 1 && f <= 6; r--, f++) attacks |= (1ULL << (idx(r, f)));
    for (r = tr + 1, f = tf - 1; r <= 6 && f >= 1; r++, f--) attacks |= (1ULL << (idx(r, f)));
    for (r = tr - 1, f = tf - 1; r >= 1 && f >= 1; r--, f--) attacks |= (1ULL << (idx(r, f)));

    return attacks;
}

static bb64 rook_attack_mask(Square sq)
{
    bb64 attacks = 0ULL;

    int r, f;
    int tr = sq >> 3;
    int tf = sq & 7;

    for (r = tr + 1; r <= 6; r++) attacks |= (1ULL << (idx(r, tf)));
    for (r = tr - 1; r >= 1; r--) attacks |= (1ULL << (idx(r, tf)));
    for (f = tf + 1; f <= 6; f++) attacks |= (1ULL << (idx(tr, f)));
    for (f = tf - 1; f >= 1; f--) attacks |= (1ULL << (idx(tr, f)));

    return attacks;
}

/* Returns one instance of a blocker configuration */
static bb64 set_occupancy(int idx, int nbits, bb64 mask)
{
    bb64 occ = 0ULL;

    for (int i = 0; i < nbits; i++)
    {
        int offset = lsb(mask);
        clrbit(mask, offset);

        if (idx & (1ULL << i)) occ |= (1ULL << offset);
    }

    return occ;
}

static inline bb64 bishop_scan(Square sq, bb64 occ)
{
    bb64 attacks = 0ULL;

    int r, f;
    int tr = sq >> 3;
    int tf = sq & 7;

    for (r = tr + 1, f = tf + 1; r <= 7 && f <= 7; r++, f++)
    {
        attacks |= (1ULL << (idx(r, f)));
        if (occ & (1ULL << idx(r, f))) break;
    }
    for (r = tr - 1, f = tf + 1; r >= 0 && f <= 7; r--, f++)
    {
        attacks |= (1ULL << (idx(r, f)));
        if (occ & (1ULL << idx(r, f))) break;
    }
    for (r = tr + 1, f = tf - 1; r <= 7 && f >= 0; r++, f--)
    {
        attacks |= (1ULL << (idx(r, f)));
        if (occ & (1ULL << idx(r, f))) break;
    }
    for (r = tr - 1, f = tf - 1; r >= 0 && f >= 0; r--, f--)
    {
        attacks |= (1ULL << (idx(r, f)));
        if (occ & (1ULL << idx(r, f))) break;
    }

    return attacks;
}

static inline bb64 rook_scan(Square sq, bb64 occ)
{
    bb64 attacks = 0ULL;

    int r, f;
    int tr = sq >> 3;
    int tf = sq & 7;

    for (r = tr + 1; r <= 7; r++)
    {
        attacks |= (1ULL << (idx(r, tf)));
        if (occ & (1ULL << idx(r, tf))) break;
    }
    for (r = tr - 1; r >= 0; r--)
    {
        attacks |= (1ULL << (idx(r, tf)));
        if (occ & (1ULL << idx(r, tf))) break;
    }
    for (f = tf + 1; f <= 7; f++)
    {
        attacks |= (1ULL << (idx(tr, f)));
        if (occ & (1ULL << idx(tr, f))) break;
    }
    for (f = tf - 1; f >= 0; f--)
    {
        attacks |= (1ULL << (idx(tr, f)));
        if (occ & (1ULL << idx(tr, f))) break;
    }

    return attacks;
}

/* Precompute all bishop attack configurations */
void build_bishop_pext_tables(void)
{
    for (int sq = 0; sq < 64; sq++)
    {
        bb64 mask = bmask[sq];
        int r = popcnt64(mask);
        int N = 1 << r;

        btable[sq] = (bb64 *)malloc(N * sizeof(bb64));
        memset(btable[sq], 0, N * sizeof(bb64));

        for (int idx = 0; idx < N; idx++)
        {
            bb64 occ = set_occupancy(idx, r, mask);
            uint64_t key = _pext_u64(occ, mask);
            btable[sq][key] = bishop_scan(sq, occ);
        }
    }
}

/* Precompute all rook attack configurations */
static void build_rook_pext_tables(void)
{
    for (int sq = 0; sq < 64; sq++)
    {
        bb64 mask = rmask[sq];
        int r = popcnt64(mask);
        int N = 1 << r;

        rtable[sq] = (bb64 *)malloc(N * sizeof(bb64));
        memset(rtable[sq], 0, N * sizeof(bb64));

        for (int idx = 0; idx < N; idx++) {
            bb64 occ = set_occupancy(idx, r, mask);
            uint64_t key = _pext_u64(occ, mask);
            rtable[sq][key] = rook_scan(sq, occ);
        }
    }
}

void init_leapers()
{
    for (int sq = 0; sq < 64; sq++)
    {
        ptable[WHITE][sq] = pawn_attack_mask(WHITE, sq);
        ptable[BLACK][sq] = pawn_attack_mask(BLACK, sq);

        ntable[sq] = knight_attack_mask(sq);

        ktable[sq] = king_attack_mask(sq);
    }
}

void init_sliders(void)
{
    for (int sq = 0; sq <= 63; sq++)
    {
        bmask[sq] = bishop_attack_mask(sq);
        rmask[sq] = rook_attack_mask(sq);
    }
    build_bishop_pext_tables();
    build_rook_pext_tables();
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
