#ifndef SEARCH_H
#define SEARCH_H

#include <assert.h>

#include "types.h"

#define MAX(a, b)   ((a) > (b) ? (a) : (b))
#define MIN(a, b)   ((a) < (b) ? (a) : (b))
#define PTYPE(p)    ((p) >= 6) ? ((p) - 6) : (p)

#define INF     50000
#define MATE    49000

#define CAP_OFFSET   (1 << 30)
#define KLR_OFFSET   (1 << 29)

extern long g_nodes;

extern int negamax(Position *, int, int , int, int, QuietTable *, uint32_t *);
extern int quiesce(Position *, int, int);

static inline int score_move(QuietTable *qt, int ply, uint32_t mv)
{
    if (dcdcap(mv) == true) {
        int a = PTYPE(dcdpc(mv));
        int v = PTYPE(dcdtarget(mv));
        return 100 * (v + 1) + (5 - a) + CAP_OFFSET;
    }
    else if (ply >= 0 && qt) {
        return 0;
        if (qt->klr_table[0][ply] == mv)
            return KLR_OFFSET;
        else if (qt->klr_table[1][ply] == mv)
            return KLR_OFFSET - 1;
        else
            return qt->hist_table[dcdpc(mv)][dcddst(mv)];
    }

    // Normal flow should never reach here
    assert(0);
    return 0;
}

static inline void score_all(MoveList *ml, QuietTable *qt, int ply)
{
    for (int i = 0; i < ml->nmoves; i++)
        ml->scores[i] = score_move(qt, ply, ml->moves[i]);
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