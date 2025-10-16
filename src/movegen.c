#include <stdio.h>

#include "attacks.h"
#include "movegen.h"

/* Debug symbols array */
const char sym[] = {'P','N','B','R','Q','K','p','n','b','r','q','k'};

MoveList mvs;

static inline void emit_move(MoveList *M, int src, int dst, Piece pc,
                             Piece promo, bool cap, bool dbl, bool enp,
                             bool cstl)
{
    add_move(M, encode(src, dst, pc, promo, cap, dbl, enp, cstl));
}

static inline void emit_from_mask(MoveList *M, int src, bb64 mask, Piece pc,
                                  bb64 occ, bb64 them)
{
    bb64 quiets = mask & ~occ;
    bb64 caps   = mask &  them;

    while (quiets) {
        int dst = poplsb(&quiets);
        emit_move(M, src, dst, pc, 0, 0, 0, 0, 0);
    }

    while (caps) {
        int dst = poplsb(&caps);
        emit_move(M, src, dst, pc, 0, 1, 0, 0, 0);
    }
}

static inline void gen_pawn_pushes(const Position *P, MoveList *M, Color c)
{
    const bb64  occ    = P->both;
    const bb64  empty  = ~occ;
    const bb64  pawns  = P->pcbb[(c == WHITE) ? W_PAWN : B_PAWN];
    const Piece pc     = (c == WHITE) ? W_PAWN : B_PAWN;

    /* Single square push mask */
    bb64 single = (c == WHITE) ? ((pawns >> 8) & empty) :
                                 ((pawns << 8) & empty);

    /* Quiet promotions */
    bb64 promo = (c == WHITE) ? (single & RANK_8) : (single & RANK_1);
    for (bb64 p = promo; p;)
    {
        int dst = poplsb(&p);
        int src = (c == WHITE) ? dst + 8 : dst - 8;

        /* UCI mandates lowercase promotions */
        emit_move(M, src, dst, pc, B_KNIGHT, 0, 0, 0, 0);
        emit_move(M, src, dst, pc, B_BISHOP, 0, 0, 0, 0);
        emit_move(M, src, dst, pc, B_ROOK,   0, 0, 0, 0);
        emit_move(M, src, dst, pc, B_QUEEN,  0, 0, 0, 0);
    }

    /* Non-promotion single pawn pushes*/
    single &= ~promo;
    for (bb64 p = single; p;)
    {
        int dst = poplsb(&p);
        int src = (c == WHITE) ? dst + 8 : dst - 8;
        emit_move(M, src, dst, pc, 0, 0, 0, 0, 0);
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
        emit_move(M, src, dst, pc, 0, 0, 0, 0, 0);
    }
}

static inline void gen_pawn_captures(const Position *P, MoveList *M, Color c)
{
    const bb64  them  = (c == WHITE) ? P->black : P->white;
    const bb64  pawns = P->pcbb[(c == WHITE) ? W_PAWN : B_PAWN];
    const Piece pc    = (c == WHITE) ? W_PAWN : B_PAWN;

    for (bb64 x = pawns; x;)
    {
        int src = poplsb(&x);
        bb64 caps = ptable[c][src] & them;

        /* Captures to promotion*/
        bb64 promo = (c==WHITE) ? (caps & RANK_8) : (caps & RANK_1);
        for (bb64 p = promo; p;) {
            int dst = poplsb(&p);
            emit_move(M, src, dst, pc, B_KNIGHT, 1, 0, 0, 0);
            emit_move(M, src, dst, pc, B_BISHOP, 1, 0, 0, 0);
            emit_move(M, src, dst, pc, B_ROOK,   1, 0, 0, 0);
            emit_move(M, src, dst, pc, B_QUEEN,  1, 0, 0, 0);
            
        }

        /* Generic captures */
        caps &= ~promo;
        for (bb64 p = caps; p;) {
            int dst = poplsb(&p);
            emit_move(M, src, dst, pc, 0, 1, 0, 0, 0);
        }

        /* En passant captures */
        if (P->enpassant != none)
        {
            bb64 epmask = ptable[c][src] & (1ULL << P->enpassant);
            if (epmask) {
                int dst = lsb(epmask);
                emit_move(M, src, dst, pc, 0, 1, 0, 1, 0);
            }
        }
    }
}

static inline void gen_king_moves(const Position* P, MoveList *M, Color c)
{
    bb64  us   = (c == WHITE)? P->white : P->black;
    bb64  them = (c == WHITE)? P->black : P->white;
    bb64  occ  = P->both;
    Piece pc   = (c == WHITE) ? W_KING : B_KING;
    int   src  = lsb(P->pcbb[(c == WHITE) ? W_KING : B_KING]);

    bb64  mask = ktable[src] & ~us;
    emit_from_mask(M, src, mask, pc, occ, them);
}

static inline void gen_castles(const Position* P, MoveList *M, Color c)
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
                emit_move(M, e1, g1, pc, 0, 0, 0, 0, 1);
            }
        }
        if (P->castling & CASTLE_WQ)
        {
            if (!(occ & ((1ULL << b1) | (1ULL << c1) | (1ULL << d1))) &&
                !checksquare(P, BLACK, e1) &&
                !checksquare(P, BLACK, d1))
            {
                emit_move(M, e1, c1, pc, 0, 0, 0, 0, 1);
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
                emit_move(M, e8, g8, pc, 0, 0, 0, 0, 1);
            }
        }
        if (P->castling & CASTLE_BQ)
        {
            if (!(occ & ((1ULL << b8)|(1ULL << c8) | (1ULL << d8))) &&
                !checksquare(P, WHITE, e8) &&
                !checksquare(P, WHITE, d8))
            {
                emit_move(M, e8, c8, pc, 0, 0, 0, 0, 1);
            }
        }
    }
}

static inline void gen_knight_moves(const Position *P, MoveList *M, Color c)
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
        emit_from_mask(M, src, mask, pc, occ, them);
    }

}

static inline void gen_bishop_moves(const Position *P, MoveList *M, Color c)
{   
    Piece pc   = (c == WHITE) ? W_BISHOP : B_BISHOP;
    bb64  us   = (c == WHITE) ? P->white : P->black;
    bb64  them = (c == WHITE) ? P->black : P->white;
    bb64  occ  = P->both;
    bb64  bb   = P->pcbb[pc];

    for (bb64 x = bb; x;)
    {
        int src = poplsb(&x);
        bb64 mask = bishop_attacks_pext(src, occ) & ~us;
        emit_from_mask(M, src, mask, pc, occ, them);
    }

}

static inline void gen_rook_moves(const Position *P, MoveList *M, Color c)
{   
    Piece pc   = (c == WHITE) ? W_ROOK : B_ROOK;
    bb64  us   = (c == WHITE) ? P->white : P->black;
    bb64  them = (c == WHITE) ? P->black : P->white;
    bb64  occ  = P->both;
    bb64  rr   = P->pcbb[pc];

    for (bb64 x = rr; x;)
    {
        int src = poplsb(&x);
        bb64 mask = rook_attacks_pext(src, occ) & ~us;
        emit_from_mask(M, src, mask, pc, occ, them);
    }

}

static inline void gen_queen_moves(const Position *P, MoveList *M, Color c)
{   
    Piece pc   = (c == WHITE) ? W_QUEEN : B_QUEEN;
    bb64  us   = (c == WHITE) ? P->white : P->black;
    bb64  them = (c == WHITE) ? P->black : P->white;
    bb64  occ  = P->both;
    bb64  qq   = P->pcbb[pc];

    for (bb64 x = qq; x;)
    {
        int src = poplsb(&x);
        bb64 mask = queen_attacks_pext(src, occ) & ~us;
        emit_from_mask(M, src, mask, pc, occ, them);
    }

}

void gen_all(const Position* P, MoveList *M)
{
    const Color side = (Color)P->side;

    gen_pawn_pushes(P, M, side);
    gen_pawn_captures(P, M, side);
    gen_king_moves(P, M, side);
    gen_castles(P, M, side);
    gen_knight_moves(P, M, side);
    gen_bishop_moves(P, M, side);
    gen_rook_moves(P, M, side);
    gen_queen_moves(P, M, side);
}

void add_move(MoveList *M, uint32_t move)
{
    /* Assumes nmoves can never go above MAX_MOVES */
    M->moves[(M->nmoves)++] = move;
}

void print_moves(const MoveList *M)
{
    printf("    move   pc  cap dbl enp cstl\n\n");
    int i = 0;
    while (i < M->nmoves)
    {
        char src[3]; idxtosq(dcdsrc(M->moves[i]), src);
        char dst[3]; idxtosq(dcddst(M->moves[i]), dst);
        char c = idxtopc(dcdpromo(M->moves[i]));
        printf("    %s%s%c  %c   %d   %d   %d   %d\n", src, dst, (c == 'P') ? ' ' : c,
                                              idxtopc(dcdpc(M->moves[i])),
                                              (int)dcdcap(M->moves[i]),
                                              (int)dcddbl(M->moves[i]),
                                              (int)dcden(M->moves[i]),
                                              (int)dcdcstl(M->moves[i]));
        i++;
    }

    printf("\n    Total moves=%d\n", M->nmoves);
}
