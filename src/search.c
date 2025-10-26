#include <stdio.h>

#include "attacks.h"
#include "eval.h"
#include "movegen.h"
#include "search.h"
#include "types.h"

long g_nodes;

int negamax(Position *P, int depth, int ply, int alpha, int beta,
            OrderTables *ord)
{
    ord->pv_len[ply] = ply;
    
    if (depth == 0) {
        ord->pv_len[ply] = ply;
        return quiesce(P, alpha, beta);
    }

    if (ply >= MAX_PLY)
        return main_eval(P);

    g_nodes++;

    bool in_check = checksquare(P, (Color)(P->side ^ 1),
                                lsb(P->pcbb[P->side ? B_KING : W_KING]));
    bool gameover = true;

    if (depth >= R_FACTOR + 1 && in_check == false && ply)
    {
        Position prev = *P;
        P->side ^= 1;
        P->enpassant = none;
        int eval = -negamax(P, depth - 1 - R_FACTOR, ply + 1,
                            -beta, -beta + 1, ord);
        *P = prev;
        if (eval >= beta)
            return beta;
    }

    int nsearched = 0;

    MoveList ml = (MoveList){0};
    gen_all(P, &ml, GEN_ALL);

    if (ord->follow_pv[ply])
        enable_pv_scoring(ord, &ml, ply);

    score_all(&ml, ord, ply);

    for (int i = 0; i < ml.nmoves; i++) {
        Position prev = *P;
        get_ordered_move(&ml, i);
        if (!make_move(P, ml.moves[i])) continue;
        gameover = false;

        ord->follow_pv[ply + 1] = ord->follow_pv[ply] &&
                                  (ml.moves[i] == ord->pv_table[0][ply]);

        int eval;

        if (nsearched == 0) {
            eval = -negamax(P, depth - 1, ply + 1, -beta, -alpha, ord);
        }
        else {
            if (
                nsearched >= NFULL_DEPTHS &&
                depth >= LMR_LIMIT &&
                in_check == false &&
                !dcdcap(ml.moves[i]) &&
                !dcdpromo(ml.moves[i])
            )
                eval = -negamax(P, depth - 2, ply + 1, -alpha - 1, -alpha, ord);
            else
                eval = alpha + 1;

            if (eval > alpha) {
                eval = -negamax(P, depth - 1, ply + 1, -alpha - 1, -alpha, ord);
                if ((eval > alpha) && (eval < beta)) {
                    eval = -negamax(P, depth - 1, ply + 1, -beta, -alpha, ord);
                }
            }
        }

        *P = prev;
        nsearched++;

        if (eval >= beta) {
            if (dcdcap(ml.moves[i]) == false) {
                ord->klr_table[1][ply] = ord->klr_table[0][ply];
                ord->klr_table[0][ply] = ml.moves[i];
            }
            return beta;
        }

        if (eval > alpha) {
            if (dcdcap(ml.moves[i]) == false) {
                Piece pc = dcdpc(ml.moves[i]);
                int dst = dcddst(ml.moves[i]);
                ord->hist_table[pc][dst] += depth;
            }

            alpha = eval;

            ord->pv_table[ply][ply] = ml.moves[i];

            for (int j = ply + 1; j < ord->pv_len[ply + 1]; j++)
                ord->pv_table[ply][j] = ord->pv_table[ply + 1][j];
            ord->pv_len[ply] = ord->pv_len[ply + 1];
        }
    }

    if (gameover) {
        ord->pv_len[ply] = ply;
        return in_check ? (-MATE + ply) : 0;
    }

    return alpha;
}

int quiesce(Position *P, int alpha, int beta)
{
    g_nodes++;

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

        int eval = -quiesce(P, -beta, -alpha);

        *P = prev;

        if (eval >= beta)
            return beta;

        alpha = MAX(alpha, eval);
    }

    return alpha;
}