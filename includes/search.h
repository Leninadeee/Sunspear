#ifndef SEARCH_H
#define SEARCH_H

#include <assert.h>

#include "types.h"
#include "uci.h"

#define MAX(a, b)   ((a) > (b) ? (a) : (b))
#define MIN(a, b)   ((a) < (b) ? (a) : (b))
#define PTYPE(p)    ((p) >= 6) ? ((p) - 6) : (p)

#define INF     50000
#define MATE    49000

#define NFULL_DEPTHS    4
#define LMR_LIMIT       3

#define PV_OFFSET   (1 << 30)
#define CAP_OFFSET  (1 << 29)
#define KLR_OFFSET  (1 << 28)

extern long g_nodes;

extern int negamax(Position *, int, int , int, int, OrderTables *);
extern int quiesce(Position *, int, int);

static inline void enable_pv_scoring(OrderTables *ord, MoveList *ml, int ply)
{
    ord->follow_pv[ply] = false;
    ord->score_pv[ply]  = false;

    uint32_t target = ord->pv_table[0][ply];
    if (!target) return;

    for (int i = 0; i < ml->nmoves; i++)
    {
        if (ml->moves[i] == target) {
            ord->follow_pv[ply] = true;
            ord->score_pv[ply]  = true;
            break;
        }
    }
}


static inline uint32_t score_move(OrderTables *ord, int ply, uint32_t mv)
{
    if (ply >= 0 && ord->score_pv[ply] == true) {
        if (ord->pv_table[0][ply] == mv) {
            ord->score_pv[ply] = false;
            return PV_OFFSET;
        }
    }

    if (dcdcap(mv) == true) {
        int a = PTYPE(dcdpc(mv));
        int v = PTYPE(dcdtarget(mv));
        return 100 * (v + 1) + (5 - a) + CAP_OFFSET;
    }
    
    if (ply >= 0 && ord) {
        if (ord->klr_table[0][ply] == mv)
            return KLR_OFFSET;
        else if (ord->klr_table[1][ply] == mv)
            return KLR_OFFSET - 1;
        else
            return ord->hist_table[dcdpc(mv)][dcddst(mv)];
    }

    // Normal flow should never reach here
    assert(0);
    return 0;
}

static inline void score_all(MoveList *ml, OrderTables *ord, int ply)
{
    for (int i = 0; i < ml->nmoves; i++)
        ml->scores[i] = score_move(ord, ply, ml->moves[i]);
}

static inline void get_ordered_move(MoveList *ml, int i)
{
    int bestidx = i;

    for (int j = i + 1; j < ml->nmoves; j++)
        if (ml->scores[j] > ml->scores[bestidx]) bestidx = j;

    if (bestidx != i) {
        uint32_t tempmv = ml->moves[bestidx];
        ml->moves[bestidx] = ml->moves[i];
        ml->moves[i] = tempmv;

        uint32_t tempsc = ml->scores[bestidx];
        ml->scores[bestidx] = ml->scores[i];
        ml->scores[i] = tempsc;
    }
}

#endif /* SEARCH_H */