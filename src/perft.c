#include <stdio.h>

#include "movegen.h"
#include "perft.h"

#define MAX_DEPTH   5

ull perft(Position *P, int depth)
{
    ull nodes = 0;
    MoveList mov = {0};
    Position prev;

    if (depth <= 0) return 1ULL;

    prev = *P;

    gen_all(P, &mov, GEN_ALL);

    for (int i = 0; i < mov.nmoves; i++)
    {
        if (!move(P, mov.moves[i])) {*P = prev; continue; }
        nodes += perft(P, depth - 1);
        *P = prev;
    }

    return nodes;
}

void perft_test(const char *fen, const ull *expected)
{
    int start = gettime();
    Position pos = {0};
    parse_fen(fen, &pos);

    for (int depth = 1; depth <= MAX_DEPTH; depth++)
    {
        ull actual = perft(&pos, depth);
        ull time_elapsed = gettime() - start;
        char *res  = (actual == expected[depth]) ? "PASS" : "FAIL";
        printf("%s  Depth=%d  Actual=%-9llu  Expected=%-9llu  Time: %5llums\n",
                res, depth, actual, expected[depth], time_elapsed);
    }
    printf("\n");
}

void perft_driver()
{
    printf("Beginning perft tests...\n");
    perft_test(POS1, pos1);
    perft_test(POS2, pos2);
    perft_test(POS3, pos3);
    perft_test(POS4, pos4);
    perft_test(POS5, pos5);
    perft_test(POS6, pos6);
    printf("Perft tests complete.\n");
}