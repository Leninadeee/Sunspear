#ifndef SEARCH_H
#define SEARCH_H

#include "types.h"

#define MAX(a, b)   ((a) > (b) ? (a) : (b))
#define MIN(a, b)   ((a) < (b) ? (a) : (b))
#define PTYPE(p)    ((p) >= 6) ? ((p) - 6) : (p)

#define INF     50000
#define MATE    49000

extern long g_nodes;

extern int negamax(Position *, int, int , int, int, uint32_t *);
extern int quiesce(Position *, int, int);

static inline void get_mvvlva_move(MoveList *ml, int i)
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