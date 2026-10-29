#include <array>

#include "bitboard.h"

namespace attacks {

    constexpr u64 NotA  = 0xFEFEFEFEFEFEFEFEULL;  // Not in file A
    constexpr u64 NotAB = 0xFCFCFCFCFCFCFCFCULL;  // Not in file A and B
    constexpr u64 NotH  = 0x7F7F7F7F7F7F7F7FULL;  // Not in file H
    constexpr u64 NotGH = 0x3F3F3F3F3F3F3F3FULL;  // Not in file G and H


    constexpr Bitboard genPawnAttackSingular(Square sq, Color c) {
        
        Bitboard singular = Bitboard::point(sq);
        Bitboard white, black, mask;

        white = ((singular & NotA) << shifts::DiagonalLR);
        black = ((singular & NotA) >> shifts::DiagonalRL);
        white |= ((singular & NotH) << shifts::DiagonalRL);
        black |= ((singular & NotH) >> shifts::DiagonalLR);
        mask  = -(static_cast<u64>(c) & 1);

        return (white & ~mask) | (black & mask);      
    }

    constexpr Bitboard genKnightAttackSingular(Square sq) {

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

    constexpr Bitboard genKingAttackSingular(Square sq) {

        Bitboard res(0);
        Bitboard singular = Bitboard::point(sq);

        res  = (singular >> shifts::Vertical);
        res |= (singular << shifts::Vertical);

        res |=  ((singular & NotA) >> shifts::Horizontal);
        res |= ((singular & NotH) << shifts::Horizontal);

        res |= ((singular & NotA) << shifts::DiagonalLR);
        res |= ((singular & NotA) >> shifts::DiagonalRL);
        res |= ((singular & NotH) << shifts::DiagonalRL);
        res |= ((singular & NotH) >> shifts::DiagonalLR);

        return res;
    }

};