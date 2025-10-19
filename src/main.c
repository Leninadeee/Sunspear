#include <stdio.h>

#include "attacks.h"
#include "bitboard.h"
#include "movegen.h"
#include "perft.h"

int main(void)
{
    printf("Initializing engine...\n\n");

    bb64 bb = 0ULL;
    // print_bitboard(bb);
    init_leapers();
    init_sliders();

    perft_driver();

    return 0;
}