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

    constexpr Bitboard generatePawnAttack(Square sq, Color c) {
        
        Bitboard singular = Bitboard::point(sq);
        Bitboard white, black, mask;

        white  = ((singular & NotA) << shifts::DiagonalLR);
        black  = ((singular & NotA) >> shifts::DiagonalRL);
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

        for (auto dir : BishopDirections)
            fill |= Bitboard::fillRay(sq, dir);

        return fill;
    }

     constexpr Bitboard generateRookRays(Square sq) {
        
        Bitboard fill{};

        for (auto dir : RookDirections)
            fill |= Bitboard::fillRay(sq, dir);

        return fill;
    }

    constexpr Bitboard generateOccupancyBishopRays(Square sq, Bitboard occ) {
        
        Bitboard fill{};

        for (auto dir : BishopDirections)
            fill |= Bitboard::fillRay(sq, dir, occ);

        return fill;
    }

     constexpr Bitboard generateOccupancyRookRays(Square sq, Bitboard occ) {
        
        Bitboard fill{};

        for (auto dir : RookDirections)
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

#if 1 // save for magics
    struct RookDataEntry {
        Bitboard block; // all possible blocker bits set
        Bitboard rays;  // all possible dest bits set
        u32 offset;
    };

    struct RookData_ {
        std::array<RookDataEntry, 64> data;
        u32 size;
    };

    struct BishopDataEntry {
        Bitboard block;
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

            for (auto dir : RookDirections) {

                table.data[i].block |= Bitboard::fillRay(sq, dir) & ~Bitboard::border(dir);
                table.data[i].rays  |= Bitboard::fillRay(sq, dir);
            }

            table.data[i].offset = table.size;
            table.size += 1 << util::popcount(table.data[i].block);
        }

        return table;
    }

    constexpr struct BishopData_ generateBishopData() {

        BishopData_ table{};

        for (int i = 0; i < 64; i++) {

            Square sq = static_cast<Square>(i);

            for (auto dir : BishopDirections) {
                table.data[i].block |= (Bitboard::fillRay(sq, dir) & ~Bitboard::border(dir));
            }

            table.data[i].offset = table.size;
            table.size += 1 << util::popcount(table.data[i].block);
        }

        return table;
    }

    constexpr auto RookData = generateRookData();
    constexpr auto BishopData = generateBishopData();

    extern std::array<u16, RookData.size> generateRookAttacks();
    extern std::array<Bitboard, BishopData.size> generateBishopAttacks();

    const std::array<u16, RookData.size> RookAttacks = generateRookAttacks();
    const std::array<Bitboard, BishopData.size> BishopAttacks = generateBishopAttacks();
#endif

};

#endif