#include <stdio.h>

#include "attacks.h"
#include "eval.h"
#include "movegen.h"
#include "search.h"

int negamax(SearchCtx *Ctx, int depth, int ply, int alpha, int beta,
            bool f_null)
{
    time_check(Ctx->nodecnt);
    if (g_tc.stop_now)
        return alpha;

    Ctx->Ord.pv_len[ply] = ply;
    
    if (depth == 0) {
        Ctx->Ord.pv_len[ply] = ply;
        return quiesce(&Ctx->Pos, alpha, beta, &Ctx->nodecnt);
    }

    if (ply >= MAX_PLY)
        return main_eval(&Ctx->Pos);

    Ctx->nodecnt++;

    bool in_check = checksquare(&Ctx->Pos, (Color)(Ctx->Pos.side ^ 1),
                        lsb(Ctx->Pos.pcbb[Ctx->Pos.side ? B_KING : W_KING]));
    bool gameover = true;

    if (f_null == true && depth >= R_FACTOR + 1 && in_check == false && ply)
    {
        Position prev = Ctx->Pos;
        Ctx->Pos.side ^= 1;
        Ctx->Pos.enpassant = none;
        int eval = -negamax(Ctx, depth - 1 - R_FACTOR, ply + 1,
                            -beta, -beta + 1, false);
        *(&Ctx->Pos) = prev;
        if (eval >= beta)
            return beta;
    }

    int nsearched = 0;

    MoveList ml = (MoveList){0};
    gen_all(&Ctx->Pos, &ml, GEN_ALL);

    if (Ctx->Ord.follow_pv[ply])
        enable_pv_scoring(&Ctx->Ord, &ml, ply);

    score_all(&ml, &Ctx->Ord, ply);

    for (int i = 0; i < ml.nmoves; i++) {
        Position prev = Ctx->Pos;
        get_ordered_move(&ml, i);
        if (!make_move(&Ctx->Pos, ml.moves[i])) continue;
        gameover = false;

        Ctx->Ord.follow_pv[ply + 1] = Ctx->Ord.follow_pv[ply] &&
                                  (ml.moves[i] == Ctx->Ord.pv_table[0][ply]);

        int eval;

        if (nsearched == 0) {
            eval = -negamax(Ctx, depth - 1, ply + 1, -beta, -alpha, true);
        }
        else {
            if (
                nsearched >= NFULL_DEPTHS &&
                depth >= LMR_LIMIT &&
                in_check == false &&
                !dcdcap(ml.moves[i]) &&
                !dcdpromo(ml.moves[i])
            )
                eval = -negamax(Ctx, depth - 2, ply + 1,
                                -alpha - 1, -alpha, true);
            else
                eval = alpha + 1;

            if (eval > alpha)
            {
                eval = -negamax(Ctx, depth - 1, ply + 1,
                                -alpha - 1, -alpha, true);
                                
                if ((eval > alpha) && (eval < beta))
                {
                    eval = -negamax(Ctx, depth - 1, ply + 1,
                                    -beta, -alpha, true);
                }
            }
        }

        *(&Ctx->Pos) = prev;
        nsearched++;

        if (eval >= beta) {
            if (dcdcap(ml.moves[i]) == false) {
                Ctx->Ord.klr_table[1][ply] = Ctx->Ord.klr_table[0][ply];
                Ctx->Ord.klr_table[0][ply] = ml.moves[i];
            }
            return beta;
        }

        if (eval > alpha) {
            if (dcdcap(ml.moves[i]) == false) {
                Piece pc = dcdpc(ml.moves[i]);
                int dst = dcddst(ml.moves[i]);
                Ctx->Ord.hist_table[pc][dst] += depth;
            }

            alpha = eval;

            Ctx->Ord.pv_table[ply][ply] = ml.moves[i];

            for (int j = ply + 1; j < Ctx->Ord.pv_len[ply + 1]; j++)
                Ctx->Ord.pv_table[ply][j] = Ctx->Ord.pv_table[ply + 1][j];
            Ctx->Ord.pv_len[ply] = Ctx->Ord.pv_len[ply + 1];
        }
    }

    if (gameover) {
        Ctx->Ord.pv_len[ply] = ply;
        return in_check ? (-MATE + ply) : 0;
    }

    return alpha;
}

int quiesce(Position *P, int alpha, int beta, uint64_t *nodecnt)
{
    (*nodecnt)++;

    time_check(*nodecnt);
    if (g_tc.stop_now)
        return alpha;

    int static_eval = main_eval(P);

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

        int eval = -quiesce(P, -beta, -alpha, nodecnt);

        *P = prev;

        if (eval >= beta)
            return beta;

        alpha = MAX(alpha, eval);
    }

    return alpha;
}