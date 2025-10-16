#include <stdio.h>

#include "attacks.h"
#include "movegen.h"

/* Debug symbols array */
const char sym[] = {'P','N','B','R','Q','K','p','n','b','r','q','k'};

static inline void emit_quiet_move(Piece pc, int src, int dst)
{
    char s1[3], s2[3]; idxtosq(src, s1); idxtosq(dst, s2);
    printf("%c  %s%s\n", sym[pc], s1, s2);
}

static inline void emit_capture_move(Piece pc, int src, int dst)
{
    char s1[3], s2[3]; idxtosq(src,s1); idxtosq(dst,s2);
    printf("%cx %s%s\n", sym[pc], s1, s2);
}

static inline void emit_from_mask(int src, bb64 mask, Piece pc, bb64 occ,
                                  bb64 them)
{
    bb64 quiets = mask & ~occ;
    bb64 caps   = mask &  them;

    while (quiets) {
        int dst = poplsb(&quiets);
        emit_quiet_move(pc, src, dst);
    }

    while (caps) {
        int dst = poplsb(&caps);
        emit_capture_move(pc, src, dst);
    }
}

static inline void emit_promo_quiet(Piece pc, int src, int dst)
{
    char s1[3], s2[3]; idxtosq(src, s1); idxtosq(dst, s2);
    char P = sym[pc];
    printf("%c=Q %s%s\n%c=R %s%s\n%c=B %s%s\n%c=N %s%s\n",
           P,s1,s2, P,s1,s2, P,s1,s2, P,s1,s2);
}
static inline void emit_promo_capture(Piece pc, int src, int dst)
{
    char s1[3], s2[3]; idxtosq(src, s1); idxtosq(dst, s2);
    char P = sym[pc];
    printf("%cx %s%s %c=Q\n%cx %s%s %c=R\n%cx %s%s %c=B\n%cx %s%s %c=N\n",
           P,s1,s2,P, P,s1,s2,P, P,s1,s2,P, P,s1,s2,P);
}
static inline void emit_en_passant(Piece pc, int src, int dst)
{
    char s1[3], s2[3]; idxtosq(src,s1); idxtosq(dst,s2);
    printf("%ce %s%s\n", sym[pc], s1, s2);
}

static inline void gen_pawn_pushes(const Position *P, Color c)
{
    const bb64  occ   = P->both;
    const bb64  empty = ~occ;
    const bb64  pawns = P->pcbb[(c == WHITE) ? W_PAWN : B_PAWN];
    const Piece pc    = (c == WHITE) ? W_PAWN : B_PAWN;

    /* Single square push mask */
    bb64 single = (c == WHITE) ? ((pawns >> 8) & empty) :
                                 ((pawns << 8) & empty);

    /* Quiet promotions */
    bb64 promo = (c == WHITE) ? (single & RANK_8) : (single & RANK_1);
    for (bb64 p = promo; p;)
    {
        int dst = poplsb(&p);
        int src = (c == WHITE) ? dst + 8 : dst - 8;
        emit_promo_quiet(pc, src, dst);
    }

    /* Non-promotion single pawn pushes*/
    single &= ~promo;
    for (bb64 p = single; p;)
    {
        int dst = poplsb(&p);
        int src = (c == WHITE) ? dst + 8 : dst - 8;
        emit_quiet_move(pc, src, dst);
    }

    /* Double pawn pushes */
    bb64 dbl;
    if (c == WHITE) { bb64 step1 = ((pawns & RANK_2) >> 8) & empty;
                      dbl = ((step1 >> 8) & empty) & RANK_4; }
    else            { bb64 step1 = ((pawns & RANK_7) << 8) & empty;
                      dbl = ((step1 << 8) & empty) & RANK_5; }

    for (bb64 p = dbl; p;)
    {
        int dst = poplsb(&p);
        int src = (c == WHITE) ? dst + 16 : dst - 16;
        emit_quiet_move(pc, src, dst);
    }
}

static inline void gen_pawn_captures(const Position *P, Color c)
{
    const bb64  them  = (c == WHITE) ? P->black : P->white;
    const bb64  pawns = P->pcbb[(c == WHITE) ? W_PAWN : B_PAWN];
    const Piece pc    = (c == WHITE) ? W_PAWN : B_PAWN;

    for (bb64 x = pawns; x;)
    {
        int from = poplsb(&x);
        bb64 caps = ptable[c][from] & them;

        /* Captures to promotion*/
        bb64 promo = (c==WHITE) ? (caps & RANK_8) : (caps & RANK_1);
        for (bb64 p = promo; p;) {
            int dst = poplsb(&p);
            emit_promo_capture(pc, from, dst);
        }

        /* Generic captures */
        caps &= ~promo;
        for (bb64 p = caps; p;) {
            int dst = poplsb(&p);
            emit_capture_move(pc, from, dst);
        }

        /* En passant captures */
        if (P->enpassant != none)
        {
            bb64 epmask = ptable[c][from] & (1ULL << P->enpassant);
            if (epmask) {
                int dst = lsb(epmask);
                emit_en_passant(pc, from, dst);
            }
        }
    }
}

static inline void gen_king_moves(const Position* P, Color c)
{
    bb64  us   = (c == WHITE)? P->white : P->black;
    bb64  them = (c == WHITE)? P->black : P->white;
    bb64  occ  = P->both;
    Piece pc   = (c == WHITE) ? W_KING : B_KING;
    int   src  = lsb(P->pcbb[(c == WHITE) ? W_KING : B_KING]);

    bb64  mask = ktable[src] & ~us;
    emit_from_mask(src, mask, pc, occ, them);
}

static inline void gen_castles(const Position* P, Color c)
{
    bb64  occ = P->both;
    Piece pc  = (c == WHITE) ? W_KING : B_KING;

    if (c == WHITE)
    {
        if (P->castling & CASTLE_WK)
        {
            if (!(occ & ((1ULL << f1) | (1ULL << g1))) &&
                !checksquare(P, BLACK, e1) &&
                !checksquare(P, BLACK, f1))
            {
                emit_quiet_move(pc, e1, g1);
            }
        }
        if (P->castling & CASTLE_WQ)
        {
            if (!(occ & ((1ULL << b1) | (1ULL << c1) | (1ULL << d1))) &&
                !checksquare(P, BLACK, e1) &&
                !checksquare(P, BLACK, d1))
            {
                emit_quiet_move(pc, e1, c1);
            }
        }
    }
    else
    {
        if (P->castling & CASTLE_BK)
        {
            if (!(occ & ((1ULL << f8) | (1ULL << g8))) &&
                !checksquare(P, WHITE, e8) &&
                !checksquare(P, WHITE, f8))
            {
                emit_quiet_move(pc, e8, g8);
            }
        }
        if (P->castling & CASTLE_BQ)
        {
            if (!(occ & ((1ULL << b8)|(1ULL << c8) | (1ULL << d8))) &&
                !checksquare(P, WHITE, e8) &&
                !checksquare(P, WHITE, d8))
            {
                emit_quiet_move(pc, e8, c8);
            }
        }
    }
}

static inline void gen_knight_moves(const Position *P, Color c)
{   
    Piece pc   = (c == WHITE) ? W_KNIGHT : B_KNIGHT;
    bb64  us   = (c == WHITE) ? P->white : P->black;
    bb64  them = (c == WHITE) ? P->black : P->white;
    bb64  occ  = P->both;
    bb64  nn   = P->pcbb[pc];

    for (bb64 x = nn; x;)
    {
        int src = poplsb(&x);
        bb64 mask = ntable[src] & ~us;
        emit_from_mask(src, mask, pc, occ, them);
    }

}

void gen_all(const Position* P)
{
    const Color side = (Color)P->side;

    gen_knight_moves(P, side);
    gen_knight_moves(P, side^1);

}