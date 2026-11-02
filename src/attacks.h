#ifndef ATTACKS_H
#define ATTACKS_H

#include <array>

#include "bitboard.h"

namespace attacks {

    using namespace boards;

    constexpr auto RookDirections
            = {offsets::Up,     offsets::Down,    offsets::Left,     offsets::Right};

    constexpr auto BishopDirections
            = {offsets::UpLeft, offsets::UpRight, offsets::DownLeft, offsets::DownRight};

    constexpr Bitboard generatePawnAttack(Square src, Color c) {
        
        Bitboard singular = Bitboard::point(src);
        Bitboard white, black, mask;

        white  = ((singular & NotA) << shifts::DiagonalLR);
        black  = ((singular & NotA) >> shifts::DiagonalRL);
        white |= ((singular & NotH) << shifts::DiagonalRL);
        black |= ((singular & NotH) >> shifts::DiagonalLR);
        mask  = -(static_cast<u64>(c) & 1);

        return (white & ~mask) | (black & mask);      
    }

    constexpr Bitboard generateKnightAttack(Square src) {

        Bitboard res(0);
        Bitboard singular = Bitboard::point(src);

        res |= (singular & NotA)  << shifts::DiagonalL1R2;
        res |= (singular & NotA)  >> shifts::DiagonalR1L2;

        res |= (singular & NotH)  << shifts::DiagonalR1L2;
        res |= (singular & NotH)  >> shifts::DiagonalL1R2;

        res |= (singular & NotAB) << shifts::DiagonalL2R1;
        res |= (singular & NotAB) >> shifts::DiagonalR2L1;

        res |= (singular & NotGH) << shifts::DiagonalR2L1;
        res |= (singular & NotGH) >> shifts::DiagonalL2R1;
        
        return res;
    }

    constexpr Bitboard generateKingAttack(Square src) {

        Bitboard res(0);
        Bitboard singular = Bitboard::point(src);

        res  = (singular >> shifts::Vertical);
        res |= (singular << shifts::Vertical);

        res |= ((singular & NotA) >> shifts::Horizontal);
        res |= ((singular & NotH) << shifts::Horizontal);

        res |= ((singular & NotA) << shifts::DiagonalLR);
        res |= ((singular & NotA) >> shifts::DiagonalRL);
        res |= ((singular & NotH) << shifts::DiagonalRL);
        res |= ((singular & NotH) >> shifts::DiagonalLR);

        return res;
    }

    constexpr Bitboard generateBishopRays(Square src) {
        
        Bitboard fill{};

        for (auto dir : BishopDirections)
            fill |= Bitboard::fillRay(src, dir);

        return fill;
    }

     constexpr Bitboard generateRookRays(Square src) {
        
        Bitboard fill{};

        for (auto dir : RookDirections)
            fill |= Bitboard::fillRay(src, dir);

        return fill;
    }

    constexpr Bitboard generateOccupancyBishopRays(Square src, Bitboard occ) {
        
        Bitboard fill{};

        for (auto dir : BishopDirections)
            fill |= Bitboard::fillRay(src, dir, occ);

        return fill;
    }

     constexpr Bitboard generateOccupancyRookRays(Square src, Bitboard occ) {
        
        Bitboard fill{};

        for (auto dir : RookDirections)
            fill |= Bitboard::fillRay(src, dir, occ);

        return fill;
    }

    constexpr std::array<Bitboard, 64> generatePawnAttackTable(Color c) {

        std::array<Bitboard, 64> table{};

        for (size_t i = 0; i < table.size(); i++) {
            table[i] = generatePawnAttack(static_cast<Square>(i), c);
        }

        return table;
    }

    constexpr std::array<Bitboard, 64> generateKnightAttackTable() {

        std::array<Bitboard, 64> table{};

        for (size_t i = 0; i < table.size(); i++) {
            table[i] = generateKnightAttack(static_cast<Square>(i));
        }

        return table;
    }

    constexpr std::array<Bitboard, 64> generateKingAttackTable() {

        std::array<Bitboard, 64> table{};

        for (size_t i = 0; i < table.size(); i++) {
            table[i] = generateKingAttack(static_cast<Square>(i));
        }

        return table;
    }

    auto constexpr WhitePawnAttacks = generatePawnAttackTable(Color::White);
    auto constexpr BlackPawnAttacks = generatePawnAttackTable(Color::Black);
    auto constexpr KnightAttacks = generateKnightAttackTable();
    auto constexpr KingAttacks   = generateKingAttackTable();

    template<Color C>
    constexpr Bitboard getPawnAttack(Square src) {
        if constexpr(C == Color::White)
            return WhitePawnAttacks[src];
        else:
            return BlackPawnAttacks[src];
    }

    constexpr Bitboard getKnightAttack(Square src) {
        return KnightAttacks[static_cast<int>(src)];
    }

    constexpr Bitboard getKingAttack(Square src) {
        return KingAttacks[static_cast<int>(src)];
    }

    #if 1 // save for magics
    struct RookDataEntry {
        Bitboard blocker; // all possible blocker bits set
        u32 offset;
    };

    struct RookData_ {
        std::array<RookDataEntry, 64> data;
        u32 size;
    };

    struct BishopDataEntry {
        Bitboard blocker;
        u32 offset;
    };

    struct BishopData_ {
        std::array<BishopDataEntry, 64> data;
        u32 size;
    };

    constexpr struct RookData_ generateRookData() {

        RookData_ table{};

        for (int i = 0; i < 64; i++) {

            Square sq = static_cast<Square>(i);

            for (auto dir : RookDirections)
                table.data[i].blocker |= Bitboard::fillRay(sq, dir) & ~Bitboard::border(dir);

            table.data[i].offset = table.size;
            table.size += 1 << util::popcount(table.data[i].blocker);
        }

        return table;
    }

    constexpr struct BishopData_ generateBishopData() {

        BishopData_ table{};

        for (int i = 0; i < 64; i++) {

            Square sq = static_cast<Square>(i);

            for (auto dir : BishopDirections)
                table.data[i].blocker |= (Bitboard::fillRay(sq, dir) & ~Bitboard::border(dir));

            table.data[i].offset = table.size;
            table.size += 1 << util::popcount(table.data[i].blocker);
        }

        return table;
    }

    constexpr auto RookData = generateRookData();
    constexpr auto BishopData = generateBishopData();

    extern std::array<Bitboard, RookData.size  > generateRookAttacks();
    extern std::array<Bitboard, BishopData.size> generateBishopAttacks();

    const std::array<Bitboard, RookData.size  > RookAttacks   = generateRookAttacks();
    const std::array<Bitboard, BishopData.size> BishopAttacks = generateBishopAttacks();


    inline Bitboard getRookAttacks(Square src, Bitboard occ) {

		const auto s = static_cast<i32>(src);

		const auto &data = RookData.data[s];
		const auto idx = util::pext(occ, data.blocker);

		return RookAttacks[data.offset + idx];
	}

	inline Bitboard getBishopAttacks(Square src, Bitboard occ) {

		const auto s = static_cast<i32>(src);

		const auto &data = BishopData.data[s];
		const auto idx = util::pext(occ, data.blocker);

		return BishopAttacks[data.offset + idx];
	}

    #endif
    
#if 0 // save for later
    struct RookDataEntry {
        Bitboard blocker; // all possible blocker bits set
        Bitboard rays;  // all possible dest bits set
        u32 offset;
    };

    struct RookData_ {
        std::array<RookDataEntry, 64> data;
        u32 size;
    };
    inline Bitboard getRookAttacks(Square src, Bitboard occ) {

		const auto i = static_cast<i32>(src);

		const auto &data = RookData.data[i];

		const auto idx = util::pext(occ, data.blocker);
		const auto attacks = util::pdep(RookAttacks[data.offset + idx], data.rays);

		return attacks;
	}

    constexpr auto RookData = generateRookData();
    extern std::array<u16, RookData.size> generateRookAttacks();
    const std::array<u16, RookData.size> RookAttacks = generateRookAttacks();

#endif

};

#endif