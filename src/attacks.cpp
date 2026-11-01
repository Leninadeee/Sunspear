#include <array>

#include "attacks.h"
#include "types.h"

namespace attacks {
    
    std::array<u16, RookData.size> generateRookAttacks() {

        std::array<u16, RookData.size> table;

        for (int sq = 0; sq < 64; sq++) {

            auto& data = RookData.data[sq];
            u32 entries = 1 << util::popcount(data.block);
            
            for (int idx = 0; idx < entries; idx++) {

                Bitboard occ = static_cast<Bitboard>(util::pdep(idx, data.block));
                Bitboard attacks = generateRookRays(static_cast<Square>(sq));

                table[idx + data.offset] = static_cast<u16>(util::pext(attacks, occ));
            }
        }

        return table;
    }

    std::array<Bitboard, BishopData.size> generateBishopAttacks() {

        std::array<Bitboard, BishopData.size> table{};

        for (int sq = 0; sq < 64; sq++) {

            auto& data = BishopData.data[sq];
            u32 entries = 1 << util::popcount(data.block);

            for (int idx = 0; idx < entries; idx++) {

                Bitboard occ = util::pdep(idx, data.block);
                table[idx + data.offset] = generateBishopRays(static_cast<Square>(sq));
            }

        }

        return table;
    }
}