#include <stdio.h>

#include "attacks.h"
#include "types.h"
#include "uci.h"

int main(void)
{
    printf("Initializing engine...\n\n");

    Position pos = {0};
    init_leapers();
    init_sliders();

    uci_loop(&pos);

    return 0;
}