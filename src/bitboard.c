#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bitboard.h"

const char pc_ascii[12] = {
    'P', 'N', 'B', 'R', 'Q', 'K',
    'p', 'n', 'b', 'r', 'q', 'k'
};

const  bb64  NOT_A  = 0xFEFEFEFEFEFEFEFEULL;  // Not in file A
const  bb64  NOT_AB = 0xFCFCFCFCFCFCFCFCULL;  // Not in file A and B
const  bb64  NOT_H  = 0x7F7F7F7F7F7F7F7FULL;  // Not in file H
const  bb64  NOT_GH = 0x3F3F3F3F3F3F3F3FULL;  // Not in file G and H

/* Prints the bit representation of a position */
void print_bitboard(bb64 bb)
{
    for (int rank = 0; rank < 8; rank++)
    {
        for (int file = 0; file < 8; file++)
        {
            if (!file) printf(" %d  ", 8 - rank);

            int sq = rank * 8 + file;
            printf(" %d", getbit(bb, sq) ? 1 : 0);
        }
        printf("\n");
    }
    printf("\n     a b c d e f g h\n\n");
    printf("     Bitboard: %llu\n\n", bb);
}

/* Prints the ascii representation of a position */
void print_board(const Position *pos)
{
    for (int rank = 0; rank < 8; ++rank)
    {
        for (int file = 0; file < 8; ++file)
        {
            if (!file) printf(" %d  ", 8 - rank);
            int sq = idx(rank, file);

            int pc = -1;

            for (int pcidx = W_PAWN; pcidx <= B_KING; pcidx++)
                if (getbit(pos->pcbb[pcidx], sq)) { pc = pcidx; break; }

            putchar(' ');
            putchar(pc == -1 ? '.' : pc_ascii[pc]);
        }
        putchar('\n');
    }
    printf("\n     a b c d e f g h\n\n");

    printf("      Side: %s\n", pos->side == WHITE ? "white" : "black");

    char ep[3];
    if (pos->enpassant < 64) { idxtosq(pos->enpassant, ep); }
    printf(" Enpassant: %s\n", pos->enpassant < 64 ? ep : "N/A");

    printf("  Castling: %c%c%c%c\n",
           (pos->castling & CASTLE_WK) ? 'K' : '-',
           (pos->castling & CASTLE_WQ) ? 'Q' : '-',
           (pos->castling & CASTLE_BK) ? 'k' : '-',
           (pos->castling & CASTLE_BQ) ? 'q' : '-');

    printf("  HM clock: %d\n", pos->hmclock);

    printf("  FM count: %d\n\n", pos->fmcount);
}

void reset_board(Position * pos)
{
    memset(pos, 0, sizeof(Position));
    pos->side = 0;
    pos->enpassant = none;
    pos->castling = 0;
    pos->hmclock = 0;
    pos->fmcount = 0;
    pos->zobrist = 0;
}

/* Sets up a position given a fen string.
   Returns false if parsing fails */
bool parse_fen(const char *fen, Position *pos)
{
    reset_board(pos);

    const char *ptr = fen;

    /* Piece placement */
    for (int r = 0; r < 8; ++r)
    {
        int f = 0;
        while (f < 8 && *ptr)
        {
            if (*ptr == '/') return false;

            if (isdigit((unsigned char)*ptr))
            {
                int n = *ptr - '0';
                if (n < 1 || n > 8 || f + n > 8) return false;
                f += n;
                ptr++;
            }
            else
            {
                int pcidx = pctoidx(*ptr);
                if (pcidx < 0) return false;
                pos->pcbb[pcidx] |= 1ULL << idx(r, f);
                f++; ptr++;
            }
        }
        if (f != 8) return false;
        if (r != 7) { if (*ptr != '/') return false; ptr++; }
    }

    if (*ptr != ' ') return false;

    ptr++;

    /* Set the side to move */
    if (*ptr == 'w')
        pos->side = WHITE;
    else if (*ptr == 'b')
        pos->side = BLACK;
    else
        return false;

    ptr++;

    if (*ptr != ' ') return false;

    ptr++;

    /* Set castling rights */
    pos->castling = 0;
    if (*ptr == '-')
        ptr++;
    else
    {
        for (; *ptr && *ptr != ' '; ptr++)
        {
            switch (*ptr) {
                case 'K': pos->castling |= CASTLE_WK; break;
                case 'Q': pos->castling |= CASTLE_WQ; break;
                case 'k': pos->castling |= CASTLE_BK; break;
                case 'q': pos->castling |= CASTLE_BQ; break;
                default: return false;
            }
        }
    }

    if (*ptr != ' ') return false;

    ptr++;

    /* Set en passant square */
    if (*ptr == '-')
    {
        pos->enpassant = -1;
        ptr++;
    }
    else
    {
        if (!ptr[0] || !ptr[1]) return false;
        int sq = sqtoidx(ptr);
        if (sq < 0) return false;
        pos->enpassant = sq;
        ptr += 2;
    }

    if (*ptr != ' ') return false;

    ptr++;

    /* Set halfmove clock */
    char *endptr = NULL;
    long hm = strtol(ptr, &endptr, 10);
    if (endptr == ptr || hm < 0) return false;
    pos->hmclock = (int)hm;
    ptr = endptr;
    if (*ptr != ' ') return false;
    ptr++;

    /* Set fullmove number */
    long fm = strtol(ptr, &endptr, 10);
    if (endptr == ptr || fm <= 0) return false;
    pos->fmcount = (int)fm;
    ptr = endptr;

    /* Skip trailing spaces */
    while (isspace((unsigned char)*ptr)) ptr++;

    if (*ptr != '\0') return false;

    /* Aggregate occupancy boards */
    pos->white = pos->pcbb[W_PAWN] | pos->pcbb[W_KNIGHT] |
                 pos->pcbb[W_BISHOP] | pos->pcbb[W_ROOK] |
                 pos->pcbb[W_QUEEN] | pos->pcbb[W_KING];

    pos->black = pos->pcbb[B_PAWN] | pos->pcbb[B_KNIGHT] |
                 pos->pcbb[B_BISHOP] | pos->pcbb[B_ROOK] |
                 pos->pcbb[B_QUEEN] | pos->pcbb[B_KING];

    pos->both  = pos->white | pos->black;

    return true;
}