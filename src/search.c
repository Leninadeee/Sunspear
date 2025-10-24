#include <stdio.h>

#include "attacks.h"
#include "eval.h"
#include "movegen.h"
#include "search.h"
#include "types.h"

long g_nodes;

int negamax(Position *P, int depth, int ply, int alpha, int beta,
            QuietTable *qt, uint32_t *root)
{   
    if (depth == 0)
        return quiesce(P, alpha, beta);

    g_nodes++;

    bool in_check = checksquare(P, (Color)(P->side ^ 1),
                                lsb(P->pcbb[P->side ? B_KING : W_KING]));
    bool gameover = true;

    MoveList ml = (MoveList){0};
    gen_all(P, &ml, GEN_ALL);
    score_all(&ml, qt, ply);

    uint32_t bestmv = 0;

    for (int i = 0; i < ml.nmoves; i++) {
        Position prev = *P;
        get_ordered_move(&ml, i);
        if (!make_move(P, ml.moves[i])) continue;
        gameover = false;

        int eval = -negamax(P, depth - 1, ply + 1, -beta, -alpha, qt, NULL);

        *P = prev;

        if (eval >= beta) {
            if (dcdcap(ml.moves[i]) == false) {
                qt->klr_table[1][ply] = qt->klr_table[0][ply];
                qt->klr_table[0][ply] = ml.moves[i];
            }
            return beta;
        }

        if (eval > alpha) {
            if (dcdcap(ml.moves[i]) == false)
                qt->hist_table[dcdpc(ml.moves[i])][dcddst(ml.moves[i])] += depth;

            alpha = eval;

            if (ply == 0)
                bestmv = ml.moves[i];
        }
    }

    if (gameover) {
        return in_check ? (-MATE + ply) : 0;
    }

    if (ply == 0 && root) *root = bestmv;

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