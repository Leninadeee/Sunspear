#include <array>

#include "attacks.h"
#include "types.h"

namespace attacks {
    
    std::array<u16, RookData.tableSize> generateRookAttacks() {

        std::array<u16, RookData.tableSize> tab;

        for (u32 i = 0; i < 64; i++) {

            auto& data = RookData.data[i];
            u32 entries = 1 << util::popcount(data.block);
            
            for (u64 j = 0; j < entries; j++) {

                Bitboard occ = static_cast<Bitboard>(util::pdep(j, data.block));

                Bitboard attacks{};

                for (auto dir : RookDirs) attacks |= generateRookRays(static_cast<Square>(i));

                tab[j + data.offset] = static_cast<u16>(util::pext(attacks, occ));
            }
        }

        return tab;
    }

    std::array<Bitboard, BishopData.tableSize> generateBishopAttacks() {

        std::array<Bitboard, BishopData.tableSize> tab{};

        for (u32 i = 0; i < 64; i++) {

            Square sq = static_cast<Square>(i);
            auto& data = BishopData.data[i];
            u32 entries = util::popcount(data.block);

            for (u32 j = 0; j < entries; j++) {

                Bitboard occ = util::pdep(i, data.block);

                Bitboard attacks{};

                for (auto dir : RookDirs) attacks |= generateBishopRays(static_cast<Square>(i));

                tab[j + data.offset] = static_cast<u64>(util::pext(attacks, occ));
            }

        }

        return tab;
    }
}