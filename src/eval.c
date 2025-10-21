#include "bitboard.h"
#include "eval.h"

int main_eval(const Position *P)
{
    int eval = 0;

    eval += material_eval(P);

    return (P->side == WHITE) ? eval : -eval;
}

int material_eval(const Position *P)
{
    int eval = 0;

    for (int pc = W_PAWN; pc <= B_KING; pc++)
    {
        int sq;
        bb64 temp = P->pcbb[pc];

        while (temp) {
            sq = poplsb(&temp);
            eval += pcval[pc];

            switch (pc) {
                case W_PAWN:   eval += psqtable[sq]; break;
                case W_KNIGHT: eval += nsqtable[sq]; break;
                case W_BISHOP: eval += bsqtable[sq]; break;
                case W_ROOK:   eval += rsqtable[sq]; break;
                case W_QUEEN:  eval += qsqtable[sq]; break;
                case W_KING:   eval += ksqtable[sq]; break;
                case B_PAWN:   eval -= psqtable[sq ^ 56]; break;
                case B_KNIGHT: eval -= nsqtable[sq ^ 56]; break;
                case B_BISHOP: eval -= bsqtable[sq ^ 56]; break;
                case B_ROOK:   eval -= rsqtable[sq ^ 56]; break;
                case B_QUEEN:  eval -= qsqtable[sq ^ 56]; break;
                case B_KING:   eval -= ksqtable[sq ^ 56]; break;
            }
        }
    }

    return eval;
}