#ifndef BITS_H
#define BITS_H

#include "../types.h"

namespace util {

    static inline int popcnt(u64 u) {
        return __builtin_popcountll(u);
    }

    /* Return the offset of the least significant set bit */
    static inline int lsb(u64 u) {
        if (!u) return -1;
        return __builtin_ctzll(u);
    }

    static inline int poplsb(u64* u) {
        int i = lsb(*u);
        *u &= *u - 1;
        return i;
    }
}

#endif