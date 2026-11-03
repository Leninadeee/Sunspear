#include <iostream>

#include "attacks.h"
#include "bitboard.h"
#include "position.h"
#include "types.h"
#include "utils/util.h"

int main() {

    std::string a("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - ");

    Position po(a);
    util::prettyBoard(po);

    std::cout << po.toFen();
}