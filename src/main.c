#include <stdio.h>

#include "attacks.h"
#include "bitboard.h"
#include "movegen.h"

int main(void)
{
    printf("Initializing engine...\n\n");

    bb64 bb = 0x0000FF0000000000ULL;
    
    print_bitboard(bb);

    return 0;
}