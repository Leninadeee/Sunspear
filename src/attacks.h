#ifndef ATTACKS_H
#define ATTACKS_H

#include <array>

#include "bitboard.h"

namespace attacks {

    constexpr u64 NotA  = 0xFEFEFEFEFEFEFEFEULL;  // Not in file A
    constexpr u64 NotAB = 0xFCFCFCFCFCFCFCFCULL;  // Not in file A and B
    constexpr u64 NotH  = 0x7F7F7F7F7F7F7F7FULL;  // Not in file H
    constexpr u64 NotGH = 0x3F3F3F3F3F3F3F3FULL;  // Not in file G and H

    constexpr auto RookDirs = {offsets::Up, offsets::Down, offsets::Left, offsets::Right};
    constexpr auto BishopDirs = {offsets::UpLeft, offsets::UpRight, offsets::DownLeft, offsets::DownRight};

    constexpr Bitboard generatePawnAttack(Square sq, Color c) {
        
        Bitboard singular = Bitboard::point(sq);
        Bitboard white, black, mask;

        white = ((singular & NotA) << shifts::DiagonalLR);
        black = ((singular & NotA) >> shifts::DiagonalRL);
        white |= ((singular & NotH) << shifts::DiagonalRL);
        black |= ((singular & NotH) >> shifts::DiagonalLR);
        mask  = -(static_cast<u64>(c) & 1);

        return (white & ~mask) | (black & mask);      
    }

    constexpr Bitboard generateKnightAttack(Square sq) {

        Bitboard res(0);
        Bitboard singular = Bitboard::point(sq);

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

    constexpr Bitboard generateKingAttack(Square sq) {

        Bitboard res(0);
        Bitboard singular = Bitboard::point(sq);

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

    constexpr Bitboard generateBishopRays(Square sq) {
        
        Bitboard fill{};

        for (auto dir : BishopDirs)
            fill |= Bitboard::fillRay(sq, dir);

        return fill;
    }

     constexpr Bitboard generateRookRays(Square sq) {
        
        Bitboard fill{};

        for (auto dir : RookDirs)
            fill |= Bitboard::fillRay(sq, dir);

        return fill;
    }

    constexpr Bitboard generateOccupancyBishopRays(Square sq, Bitboard occ) {
        
        Bitboard fill{};

        for (auto dir : BishopDirs)
            fill |= Bitboard::fillRay(sq, dir, occ);

        return fill;
    }

     constexpr Bitboard generateOccupancyRookRays(Square sq, Bitboard occ) {
        
        Bitboard fill{};

        for (auto dir : RookDirs)
            fill |= Bitboard::fillRay(sq, dir, occ);

        return fill;
    }

    constexpr std::array<Bitboard, 64> generatePawnAttackTable(Color c) {

        std::array<Bitboard, 64> Ptable{};

        for (size_t i = 0; i < Ptable.size(); i++) {
            Ptable[i] = generatePawnAttack(static_cast<Square>(i), c);
        }

        return Ptable;
    }

    constexpr std::array<Bitboard, 64> generateKnightAttackTable() {

        std::array<Bitboard, 64> Kntable{};

        for (size_t i = 0; i < Kntable.size(); i++) {
            Kntable[i] = generateKnightAttack(static_cast<Square>(i));
        }

        return Kntable;
    }

    constexpr std::array<Bitboard, 64> generateKingAttackTable() {

        std::array<Bitboard, 64> Ktable{};

        for (size_t i = 0; i < Ktable.size(); i++) {
            Ktable[i] = generateKingAttack(static_cast<Square>(i));
        }

        return Ktable;
    }

    auto constexpr WhitePawnAttacks = generatePawnAttackTable(Color::White);
    auto constexpr BlackPawnAttacks = generatePawnAttackTable(Color::Black);
    auto constexpr KnightAttacks = generateKnightAttackTable();
    auto constexpr KingAttacks   = generateKingAttackTable();

    struct RookDataEntry {
        Bitboard block; // all possible blocker bits set
        Bitboard rays;  // all possible dest bits set
        u32 offset;
    };

    struct RookData_ {
        std::array<RookDataEntry, 64> data;
        u32 tableSize;
    };

    struct BishopDataEntry {
        Bitboard block;
        u32 offset;
    };

    struct BishopData_ {
        std::array<BishopDataEntry, 64> data;
        u32 tableSize;
    };

    constexpr struct RookData_ generateRookData() {

        RookData_ tab{};

        for (u32 i = 0; i < 64; i++) {

            Square sq = static_cast<Square>(i);

            for (auto dir : {offsets::Up, offsets::Down, offsets::Left, offsets::Right}) {

                tab.data[i].block |= Bitboard::fillRay(sq, dir) & ~Bitboard::border(dir);
                tab.data[i].rays  |= Bitboard::fillRay(sq, dir);
            }

            tab.data[i].offset = tab.tableSize;
            tab.tableSize += 1 << util::popcount(tab.data[i].block);
        }

        return tab;
    }

    constexpr struct BishopData_ generateBishopData() {

        BishopData_ tab{};

        for (u32 i = 0; i < 64; i++) {

            Square sq = static_cast<Square>(i);

            for (auto dir : BishopDirs) {
                tab.data[i].block |= (Bitboard::fillRay(sq, dir) & ~Bitboard::border(dir));
            }

            tab.data[i].offset = tab.tableSize;
            tab.tableSize += 1 << util::popcount(tab.data[i].block);
        }

        return tab;
    }

    constexpr auto RookData = generateRookData();
    constexpr auto BishopData = generateBishopData();

    extern std::array<u16, RookData.tableSize> getRookAttacks();
    extern std::array<Bitboard, BishopData.tableSize> getBishopAttacks();

    const std::array<u16, RookData.tableSize> RookAttacks = getRookAttacks();
    const std::array<Bitboard, BishopData.tableSize> BishopAttacks = getBishopAttacks();

};

#endif