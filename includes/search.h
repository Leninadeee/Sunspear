#ifndef SEARCH_H
#define SEARCH_H

#include "types.h"

#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) < (b) ? (a) : (b))

#define INF     200000
#define MATE    100000

extern long g_nodes;

extern int negamax(Position *, int, int , int, int, uint32_t *);
extern int quiesce(Position *, int, int);

#endif /* SEARCH_H */