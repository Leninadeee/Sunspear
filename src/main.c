#include <stdio.h>

#include "attacks.h"
#include "tt.h"
#include "types.h"
#include "uci.h"

int main(void)
{
    SearchCtx Ctx = {0};
    init_leapers();
    init_sliders();
    init_zobrist();
    tt_clear();

    uci_loop(&Ctx);

    return 0;
}