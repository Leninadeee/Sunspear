#include <stdio.h>

#include "attacks.h"
#include "eval.h"
#include "movegen.h"
#include "search.h"
#include "types.h"

long g_nodes;

int negamax(Position *P, int depth, int ply, int alpha, int beta,
            uint32_t *root)
{
    bool in_check = checksquare(P, (Color)(P->side ^ 1),
                                lsb(P->pcbb[P->side ? B_KING : W_KING]));
    if (in_check) depth++;
    
    if (depth <= 0)
        return quiesce(P, alpha, beta);

    MoveList ml = {0};
    gen_all(P, &ml, GEN_ALL);

    bool gameover = true;
    int max_eval = -INF;
    uint32_t bestmv = 0;

    for (int i = 0; i < ml.nmoves; i++) {
        Position prev = *P;
        if (!make_move(P, ml.moves[i])) { *P = prev; continue; }
        gameover = false;
        g_nodes++;
        int eval = -negamax(P, depth - 1, ply + 1, -beta, -alpha, NULL);
        *P = prev;

        if (eval > max_eval) {
            max_eval = eval;
            alpha = MAX(alpha, eval);

            if (ply == 0)
                bestmv = ml.moves[i];
        }

        if (alpha >= beta) break;
    }

    if (gameover) {
        return in_check ? (-MATE + ply) : 0;
    }

    if (ply == 0 && root) *root = bestmv;
    return max_eval;
}

int quiesce(Position *P, int alpha, int beta)
{
    int static_eval = main_eval(P);

    int max_eval = static_eval;
    if (max_eval >= beta)
        return max_eval;
    alpha = MAX(alpha, max_eval);

    MoveList ml = (MoveList){0};
    gen_all(P, &ml, GEN_CAPTURES);

    for (int i = 0; i < ml.nmoves; i++) {
        Position prev = *P;
        if (!make_move(P, ml.moves[i])) { *P = prev; continue; }
        g_nodes++;
        int eval = -quiesce(P, -beta, -alpha);
        *P = prev;

        if (eval >= beta)
            return eval;
        max_eval = MAX(max_eval, eval);
        alpha = MAX(alpha, eval);
    }

    return max_eval;
}
