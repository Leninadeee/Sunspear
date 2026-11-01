#include <array>

#include "attacks.h"
#include "types.h"

namespace attacks {

    std::array<Bitboard, RookData.size> generateRookAttacks() {

        std::array<Bitboard, RookData.size> table{};

        for (int sq = 0; sq < 64; sq++) {

            auto& data = RookData.data[sq];
            int entries = 1 << util::popcount(data.blocker);

            for (int idx = 0; idx < entries; idx++) {

                Bitboard occ = util::pdep(idx, data.blocker);
                table[idx + data.offset] = generateOccupancyRookRays(static_cast<Square>(sq), occ);
            }

        }

        return table;
    }

    std::array<Bitboard, BishopData.size> generateBishopAttacks() {

        std::array<Bitboard, BishopData.size> table{};

        for (int sq = 0; sq < 64; sq++) {

            auto& data = BishopData.data[sq];
            int entries = 1 << util::popcount(data.blocker);

            for (int idx = 0; idx < entries; idx++) {

                Bitboard occ = util::pdep(idx, data.blocker);
                table[idx + data.offset] = generateOccupancyBishopRays(static_cast<Square>(sq), occ);
            }

        }

        return table;
    }

    #if 0 // save for later
    // approx 800kb->200kb compressed table, unnecessary for modern CPUs
    std::array<u16, RookData.size> generateRookAttacks() {

        std::array<u16, RookData.size> table;

        for (int sq = 0; sq < 64; sq++) {

            auto& data = RookData.data[sq];
            u32 entries = 1 << util::popcount(data.blocker);
            
            for (int idx = 0; idx < entries; idx++) {

                Bitboard occ = static_cast<Bitboard>(util::pdep(idx, data.blocker));
                Bitboard attacks = generateRookRays(static_cast<Square>(sq));

                table[idx + data.offset] = static_cast<u16>(util::pext(attacks, occ));
            }
        }

        return table;
    }
    #endif
}