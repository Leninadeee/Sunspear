#include <stdio.h>

#include "attacks.h"
#include "eval.h"
#include "movegen.h"
#include "search.h"
#include "tt.h"

static inline bool end_or_quiesce(SearchCtx *Ctx, int depth, int ply,
                                  int alpha, int beta, int *out_eval)
{
    Ctx->Ord.pv_len[ply] = ply;
    if (depth == 0) {
        *out_eval = quiesce(&Ctx->Pos, ply + 1, alpha, beta, &Ctx->nodecnt);
        return true;
    }
    if (ply >= MAX_PLY) {
        *out_eval = main_eval(&Ctx->Pos);
        return true;
    }
    return false;
}

static inline bool try_null_move(SearchCtx *Ctx, int depth, int ply,
                                 bool in_check, int beta, int *out_eval)
{
    if (depth >= R_FACTOR + 1 && in_check == false && ply) {
        Position prev = Ctx->Pos;

        Ctx->Pos.side ^= 1;
        Ctx->Pos.zobrist ^= Z_SIDE;

        if (Ctx->Pos.enpassant != none)
            Ctx->Pos.zobrist ^= Z_EP[Ctx->Pos.enpassant];
        Ctx->Pos.enpassant = none;

        int eval = -negamax(Ctx, depth - 1 - R_FACTOR, ply + 1,
                            -beta, -beta + 1, false);
        Ctx->Pos = prev;
        if (eval >= beta) {
            *out_eval = beta;
            return true;
        }
    }
    return false;
}

static inline void fill_movelist(SearchCtx *Ctx, MoveList *ml, int ply)
{
    *ml = (MoveList){0};
    gen_all(&Ctx->Pos, ml, GEN_ALL);
    if (Ctx->Ord.follow_pv[ply])
        enable_pv_scoring(&Ctx->Ord, ml, ply);
    score_all(ml, &Ctx->Ord, ply);
}

static inline int pvs_lmr_search(SearchCtx *Ctx, int depth, int ply,
                                 int alpha, int beta, bool in_check,
                                 uint32_t move, int nsearched)
{
    if (nsearched == 0) {
        return -negamax(Ctx, depth - 1, ply + 1, -beta, -alpha, true);
    }
    else {
        int eval;
        if (
            nsearched >= NFULL_DEPTHS &&
            depth >= LMR_LIMIT &&
            in_check == false &&
            !dcdcap(move) &&
            !dcdpromo(move)
           )
        {
            eval = -negamax(Ctx, depth - 2, ply + 1, -alpha - 1, -alpha, true);
        }
        else {
            eval = alpha + 1;
        }

        if (eval > alpha) {
            eval = -negamax(Ctx, depth - 1, ply + 1, -alpha - 1, -alpha, true);
            if (eval > alpha && eval < beta) {
                eval = -negamax(Ctx, depth - 1, ply + 1, -beta, -alpha, true);
            }
        }
        return eval;
    }
}

static inline void update_klr(SearchCtx *Ctx, int ply, uint32_t mv)
{
    if (!dcdcap(mv)) {
        Ctx->Ord.klr_table[1][ply] = Ctx->Ord.klr_table[0][ply];
        Ctx->Ord.klr_table[0][ply] = mv;
    }
}

static inline void update_hist(SearchCtx *Ctx, int depth, uint32_t mv)
{
    if (!dcdcap(mv)) {
        Piece pc = dcdpc(mv);
        int dst  = dcddst(mv);
        Ctx->Ord.hist_table[pc][dst] += depth;
    }
}

static inline void update_pv(SearchCtx *Ctx, int ply, uint32_t mv)
{
    Ctx->Ord.pv_table[ply][ply] = mv;
    for (int j = ply + 1; j < Ctx->Ord.pv_len[ply + 1]; j++)
        Ctx->Ord.pv_table[ply][j] = Ctx->Ord.pv_table[ply + 1][j];
    Ctx->Ord.pv_len[ply] = Ctx->Ord.pv_len[ply + 1];
}

static inline bool test_king(const Position *P)
{
    Color us = (Color) P->side;
    int ksq  = lsb(P->pcbb[us ? B_KING : W_KING]);
    return checksquare(P, (Color)(us ^ 1), ksq);
}

int negamax(SearchCtx *Ctx, int depth, int ply, int alpha, int beta,
            bool f_null)
{
    Ctx->nodecnt++;
    time_check(Ctx->nodecnt);
    if (g_tc.stop_now)
        return alpha;

    int val;
    bool is_pv_node = (beta - alpha) > 1;
    if (ply && !is_pv_node && (val = tt_read(Ctx->Pos.zobrist, &Ctx->Ord.tt_moves[ply], depth, ply, alpha, beta))
        != TT_UNKNOWN)
    {
       return val;
    }

    int tt_flag = TT_ALPHA;

    int out_eval;
    if (end_or_quiesce(Ctx, depth, ply, alpha, beta, &out_eval))
        return out_eval;

    bool in_check = test_king(&Ctx->Pos);
    bool gameover = true;

    if (f_null && try_null_move(Ctx, depth, ply, in_check, beta, &out_eval))
        return out_eval;

    int nsearched = 0;

    uint32_t best;
    MoveList ml;
    fill_movelist(Ctx, &ml, ply);

    for (int i = 0; i < ml.nmoves; i++) {
        Position prev = Ctx->Pos;

        get_ordered_move(&ml, i);
        if (!make_move(&Ctx->Pos, ml.moves[i])) continue;
        gameover = false;

        Ctx->Ord.follow_pv[ply + 1] = Ctx->Ord.follow_pv[ply] &&
                                    (ml.moves[i] == Ctx->Ord.pv_table[0][ply]);

        int eval = pvs_lmr_search(Ctx, depth, ply, alpha, beta,
                                  in_check, ml.moves[i], nsearched);

        Ctx->Pos = prev;
        if (g_tc.stop_now) return alpha;
        nsearched++;

        if (eval > alpha) {
            update_hist(Ctx, depth, ml.moves[i]);
            alpha = eval;
            update_pv(Ctx, ply, ml.moves[i]);
            tt_flag = TT_EXACT;
            best = ml.moves[i];

            if (eval >= beta) {
                tt_write(Ctx->Pos.zobrist, best, depth, ply, beta, TT_BETA);
                update_klr(Ctx, ply, ml.moves[i]);
                return beta;
            }
        }
    }

    if (gameover) {
        Ctx->Ord.pv_len[ply] = ply;
        return in_check ? (-MATE + ply) : 0;
    }

    tt_write(Ctx->Pos.zobrist, best, depth, ply, alpha, tt_flag);
    return alpha;
}

int quiesce(Position *P, int ply, int alpha, int beta, uint64_t *nodecnt)
{
    (*nodecnt)++;
    time_check(*nodecnt);
    if (g_tc.stop_now)
        return alpha;

    int static_eval = main_eval(P);

    if (ply >= MAX_PLY)
        return main_eval(P);

    if (static_eval >= beta)
        return beta;
    
    alpha = MAX(alpha, static_eval);

    MoveList ml = (MoveList){0};
    gen_all(P, &ml, GEN_CAPTURES);
    score_all(&ml, NULL, -1);

    for (int i = 0; i < ml.nmoves; i++) {
        Position prev = *P;
        get_ordered_move(&ml, i);
        if (!make_move(P, ml.moves[i])) continue;

        int eval = -quiesce(P, ply + 1, -beta, -alpha, nodecnt);

        *P = prev;
        if (g_tc.stop_now)
            return alpha;

        if (eval >= beta)
            return beta;

        alpha = MAX(alpha, eval);
    }

    return alpha;
}