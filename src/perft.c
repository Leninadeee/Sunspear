#include <stdio.h>

#include "movegen.h"
#include "perft.h"

#define  MAX_DEPTH  6   // Higher takes too long

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
        if (!make_move(P, mov.moves[i])) continue;
        nodes += perft(P, depth - 1);
        *P = prev;
    }

    return nodes;
}

void perft_test(const char *fen, const ull *expected, int depth)
{
    int start = gettime();
    Position pos = {0};
    parse_fen(fen, &pos);

    for (int d = 1; d <= depth; d++)
    {
        ull actual = perft(&pos, d);
        ull time_elapsed = gettime() - start;
        char *res  = (actual == expected[d]) ? "PASS" : "FAIL";
        printf("%s  Depth=%d  Actual=%-10llu  Expected=%-10llu  Time: %6llums\n",
                res, d, actual, expected[d], time_elapsed);
    }
    printf("\n");
}

void perft_driver(int depth)
{
    depth = (depth > MAX_DEPTH) ? MAX_DEPTH : depth;
    printf("Beginning perft tests...\n");
    perft_test(POS1, pos1, depth);
    perft_test(POS2, pos2, depth);
    perft_test(POS3, pos3, depth);
    perft_test(POS4, pos4, depth);
    perft_test(POS5, pos5, depth);
    perft_test(POS6, pos6, depth);
    printf("Perft tests complete.\n");
}