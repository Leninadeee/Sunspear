#ifndef BITBOARD_H
#define BITBOARD_H

#include "types.h"

/* Bit manipulation macros */
#define  getbit(bb, offset)  ((bb) & (1ULL << (offset)))
#define  setbit(bb, offset)  ((bb) |= (1ULL << (offset)))
#define  clrbit(bb, offset)  ((bb) &= (~(1ULL << (offset))))
#define  idx(r, f)           (((r) << 3) | (f))

extern  void  print_bitboard(bb64 bb);
extern  void  print_board(const Position *P);
extern  void  reset_board(Position *P);
extern  bool  parse_fen(const char *fen, Position *P);

/* Return the number of set bits */
static inline int popcnt64(bb64 bb)
{
    return __builtin_popcountll(bb);
}

/* Return the offset of the least significant set bit */
static inline int lsb(bb64 bb)
{
    if (!bb) return -1;
    return __builtin_ctzll(bb);
}

static inline int poplsb(bb64 * bb)
{
    int i = lsb(*bb);
    *bb &= *bb - 1;
    return i;
}

/* Converts an index to its tile string representation */
static inline void idxtosq(int sq, char buf[3])
{
    buf[0] = 'a' + (sq & 7);
    buf[1] = '8' - (sq >> 3);
    buf[2] = '\0';
}

/* Converts a tile string to its index representation */
static inline int sqtoidx(const char *str)
{
    int f = str[0] - 'a';
    int r = '8' - str[1];
    return (r << 3) | f;
}

/* Converts a character piece to its 'Piece' index */
static inline int pctoidx(char c)
{
    switch (c) {
        case 'P': return W_PAWN;
        case 'N': return W_KNIGHT;
        case 'B': return W_BISHOP;
        case 'R': return W_ROOK;
        case 'Q': return W_QUEEN;
        case 'K': return W_KING;
        case 'p': return B_PAWN;
        case 'n': return B_KNIGHT;
        case 'b': return B_BISHOP;
        case 'r': return B_ROOK;
        case 'q': return B_QUEEN; 
        case 'k': return B_KING;
        default:  return -1;
    }
}

/* Converts a 'Piece' index to its character representation */
static inline char idxtopc(Piece pc) {
    switch (pc) {
        case W_PAWN:   return 'P';
        case W_KNIGHT: return 'N';
        case W_BISHOP: return 'B';
        case W_ROOK:   return 'R';
        case W_QUEEN:  return 'Q';
        case W_KING:   return 'K';
        case B_PAWN:   return 'p';
        case B_KNIGHT: return 'n';
        case B_BISHOP: return 'b';
        case B_ROOK:   return 'r';
        case B_QUEEN:  return 'q';
        case B_KING:   return 'k';
        default:       return '?';
    }
}


#endif /* BITBOARD_H*/