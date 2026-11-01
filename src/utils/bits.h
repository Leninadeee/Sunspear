#ifndef BITS_H
#define BITS_H

#include <immintrin.h>
#include <type_traits>

#include "../types.h"

namespace util {

    constexpr int popcount(u64 u) {
        return __builtin_popcountll(u);
    }

    constexpr int lsb(u64 u) {
        if (!u) return -1;
        return __builtin_ctzll(u);
    }

    constexpr int poplsb(u64& u) {
        int i = lsb(u);
        u &= u - 1;
        return i;
    }

    constexpr u64 pext(u64 val, u64 mask) {

        if (std::is_constant_evaluated()) {

            u64 res = 0;
            for (u64 bb = 1; mask; mask &= mask - 1, bb <<= 1) {
                if (val & (mask & -mask)) {
                    res |= bb;
                }
            }

            return res;
        }
        else {
            return _pext_u64(val, mask);
        }
    }

    constexpr u64 pdep(u64 val, u64 mask) {

        if (std::is_constant_evaluated()) {

            u64 res = 0;

            for (u64 bb = 1; mask; mask &= mask - 1, bb <<= 1) {
                if (val & bb) {
                    res |= (mask & -mask);
                }
            }
            return res;
        }
        else {
            return _pdep_u64(val, mask);
        }
    }
}

#endif