#ifndef SEARCH_H
#define SEARCH_H

#include <assert.h>
#include <windows.h>

#include "types.h"
#include "uci.h"

#define  MAX(a, b)    ((a) > (b) ? (a) : (b))
#define  PTYPE(p)     ((p) >= 6) ? ((p) - 6) : (p)
#define  MVVLVA(a, v) (100 * ((v) + 1) + (5 - (a)))

#define  INF    50000
#define  MATE   49000
#define  ASP_WINDOW  50

#define  NFULL_DEPTHS   4
#define  LMR_LIMIT      3
#define  R_FACTOR       2

#define  TT_OFFSET   (1 << 30)
#define  PV_OFFSET   (1 << 29)
#define  CAP_OFFSET  (1 << 28)
#define  KLR_OFFSET  (1 << 27)

#define  TIME_CHECK_MASK  0x07FF

// d=depth a=alpha b=beta
extern int negamax(SearchCtx *Ctx, int d, int ply, int a, int b, bool f_null);
extern int quiesce(Position *P, int ply, int a, int b, uint64_t *nodecnt);

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
        return MVVLVA(a, v) + CAP_OFFSET;
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

static uint64_t time_ms(void)
{
    return GetTickCount64(); 
}

static inline void time_check(uint64_t nodes) {
    if ((!g_tc.enabled && !g_tc.node_limit) || g_tc.infinite || g_tc.ponder)
        return;

    if ((nodes & TIME_CHECK_MASK) != 0) return;

    if (g_tc.node_limit && nodes >= g_tc.node_limit) {
        g_tc.stop_now = true;
        return;
    }

    if (!g_tc.enabled) return;

    uint64_t elapsed = time_ms() - g_tc.start_ms;
    if (!g_tc.abort_iter && elapsed >= g_tc.soft_ms)
        g_tc.abort_iter = true;
    if (!g_tc.stop_now  && elapsed >= g_tc.hard_ms)
        g_tc.stop_now   = true;
}

#endif /* SEARCH_H */