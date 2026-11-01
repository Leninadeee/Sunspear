#include <array>

#include "attacks.h"
#include "types.h"
#include "util.h"

namespace attacks {

    std::array<u16, RookData.tableSize> RookAttacks;
    
    std::array<u16, RookData.tableSize> generateRookAttacks() {

        std::array<u16, RookData.tableSize> table;

        for (u32 i = 0; i < 64; i++) {

            auto& data = RookData.data[i];
            u32 entries = 1 << util::popcount(data.block);
            
            for (u64 j = 0; j < entries; j++) {

                Bitboard occ = static_cast<Bitboard>(util::pdep(j, data.block));

                Bitboard attacks{};

                for (auto dir : RookDirs) attacks |= generateRookRays(static_cast<Square>(i));

                RookAttacks[j + data.offset] = static_cast<u16>(util::pext(attacks, occ));
            }
        }
    }
}