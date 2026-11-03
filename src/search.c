#include <stdio.h>

#include "attacks.h"
#include "eval.h"
#include "movegen.h"
#include "pesto.h"
#include "search.h"
#include "tt.h"

int negamax(SearchCtx *Ctx, int depth, int ply, int alpha, int beta,
            bool f_null)
{
    time_check(++Ctx->nodecnt);
    if (g_tc.stop_now)
        return alpha;

    // Transposition table proving
    int tt_eval = TT_UNKNOWN;
    bool is_pv_node = (beta - alpha) > 1;
    if (ply) {
        tt_eval = tt_read(Ctx->Pos.zobrist, &Ctx->Ord.tt_moves[ply],
                          depth, ply, alpha, beta);

        if (!is_pv_node && tt_eval != TT_UNKNOWN) {
            return tt_eval;
        }
    }

    int tt_flag = TT_ALPHA;

    // Check detection and extension
    Color us = (Color) Ctx->Pos.side;
    int ksq  = lsb(Ctx->Pos.pcbb[us ? B_KING : W_KING]);
    bool in_check = checksquare(&Ctx->Pos, (Color)(us ^ 1), ksq);

    if (in_check && depth == 0)
        depth = 1;

    Ctx->Ord.pv_len[ply] = ply;

    // Quiescense
    if (depth == 0) {
        return quiesce(&Ctx->Pos, ply + 1, alpha, beta, &Ctx->nodecnt);
    }

    // Eval
    if (ply >= MAX_PLY) {
        return pesto(&Ctx->Pos);
    }

    // Null move pruning
    if (f_null && depth >= R_FACTOR + 1 && !in_check && ply) {
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
            return beta;
        }
    }

    MoveList ml = (MoveList){0};
    gen_all(&Ctx->Pos, &ml, GEN_ALL);

    if (Ctx->Ord.follow_pv[ply])
        enable_pv_scoring(&Ctx->Ord, &ml, ply);
    score_all(&ml, &Ctx->Ord, ply);

    bool has_legal = false;
    int nsearched = 0;
    uint32_t best = 0;

    for (int i = 0; i < ml.nmoves; i++) {
        get_ordered_move(&ml, i);
        uint32_t mv = ml.moves[i];

        Position prev = Ctx->Pos;

        if (!make_move(&Ctx->Pos, mv))
            continue;
        has_legal = true;

        bool quiet = !dcdcap(mv) && !dcdpromo(mv);

        // Late move pruning
        if (!is_pv_node && depth <= LMP_DEPTH && !in_check && quiet && i >= LMP_LIMIT) {
            int hist = Ctx->Ord.hist_table[dcdpc(mv)][dcddst(mv)];
            if (hist < 3000) {
                Ctx->Pos = prev;
                continue;
            }
        }

        if (nsearched == 0) {
            best = mv;
        }

        Ctx->Ord.follow_pv[ply + 1] = Ctx->Ord.follow_pv[ply] &&
                                      (mv == Ctx->Ord.pv_table[0][ply]);

        int eval;

        // Principal variation search
        if (nsearched == 0) {
            // First move (PV move): Search with full window at full depth
            eval = -negamax(Ctx, depth - 1, ply + 1, -beta, -alpha, true);
        }
        else {
            // Late move reudctions
            int R = 0;
            if (!in_check && quiet) {
                if (depth > 2 && nsearched > 3) {
                    R = 1;
                    if (depth >= 5 && nsearched >= 8)  R++;
                    if (depth >= 8 && nsearched >= 16) R++;
                }
            }

            if (R > 0) {
                eval = -negamax(Ctx, depth - 1 - R, ply + 1,
                                -alpha - 1, -alpha, true);
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
        }

        Ctx->Pos = prev;
        if (g_tc.stop_now) return alpha;
        nsearched++;

        if (eval > alpha) {
            // History heuristic
            if (!dcdcap(mv)) {
                Piece pc = dcdpc(mv);
                int dst  = dcddst(mv);
                Ctx->Ord.hist_table[pc][dst] += depth;
            }

            alpha = eval;
            tt_flag = TT_EXACT;
            best = mv;

            // PV table update
            Ctx->Ord.pv_table[ply][ply] = mv;
            for (int j = ply + 1; j < Ctx->Ord.pv_len[ply + 1]; j++)
                Ctx->Ord.pv_table[ply][j] = Ctx->Ord.pv_table[ply + 1][j];
            Ctx->Ord.pv_len[ply] = Ctx->Ord.pv_len[ply + 1];

            if (eval >= beta) {
                tt_write(Ctx->Pos.zobrist, best, depth, ply, beta, TT_BETA);

                // Killer heuristic
                if (!dcdcap(mv)) {
                    Ctx->Ord.klr_table[1][ply] = Ctx->Ord.klr_table[0][ply];
                    Ctx->Ord.klr_table[0][ply] = mv;
                }
                return beta;
            }
        }
    }

    if (has_legal == false) {
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

    int static_eval = pesto(P);

    if (ply >= MAX_PLY)
        return static_eval;

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
