#include <stdio.h>
#include <string.h>

#include "bitboard.h"
#include "tt.h"

uint32_t  seed = INIT_SEED;

uint64_t  Z_PSQ[12][64];
uint64_t  Z_CSTL[16];
uint64_t  Z_EP[64];
uint64_t  Z_SIDE;
TTentry   TTable[0x400000];

void init_zobrist(void)
{
    for (int pc = W_PAWN; pc <= B_KING; pc++) {
        for (int sq = 0; sq <= 63; sq++)
            Z_PSQ[pc][sq] = prng64();
    }

    for (int sq = 0; sq < 64; sq++)
        Z_EP[sq] = prng64();

    for (int i = 0; i < 16; i++)
        Z_CSTL[i] = prng64();

    Z_SIDE = prng64();
}

extern uint64_t gen_key(Position pos)
{
    uint64_t key = 0;

    for (int pc = W_PAWN; pc <= B_KING; pc++)
    {
        while (pos.pcbb[pc]) {
            int sq = poplsb(&pos.pcbb[pc]);
            key ^= Z_PSQ[pc][sq];
        }
    }

    if (pos.enpassant != none)
        key ^= Z_EP[pos.enpassant];

    key ^= Z_CSTL[pos.castling];

    if (pos.side == BLACK)
        key ^= Z_SIDE;

    return key;
}

int tt_read(uint64_t zobrist, int depth, int alpha, int beta)
{
    const TTentry *E = &TTable[zobrist % TT_SIZE];

    if (E->key == zobrist) {
        if (E->depth >= depth) {
            if (E->flag == TT_EXACT)
                return E->eval;
            if (E->flag == TT_ALPHA && E->eval <= alpha)
                return alpha;
            if (E->flag == TT_BETA && E->eval >= beta)
                return beta;
        }
    }

    return TT_UNKNOWN;
}

void tt_write(uint64_t zobrist, int depth, int eval, int flag)
{
    TTentry *E = &TTable[zobrist % TT_SIZE];

    E->key   = zobrist;
    E->depth = depth;
    E->eval  = eval;
    E->flag  = flag;
}

void tt_clear(void)
{
    memset(TTable, 0, sizeof(TTable));
}
