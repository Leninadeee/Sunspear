#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bitboard.h"
#include "movegen.h"
#include "search.h"
#include "types.h"
#include "uci.h"

uint32_t parse_move(const Position *P, const char *mvstr)
{
    if (!mvstr || !mvstr[0] || !mvstr[1] || !mvstr[2] || !mvstr[3]) return 0;

    const int src = (mvstr[0] - 'a') + (8 - (mvstr[1] - '0')) * 8;
    const int dst = (mvstr[2] - 'a') + (8 - (mvstr[3] - '0')) * 8;

    Piece pc = 0;
    if (mvstr[4]) {
        char c = mvstr[4];
        
        if      (c == 'q') pc = (P->side == WHITE) ? W_QUEEN  : B_QUEEN;
        else if (c == 'r') pc = (P->side == WHITE) ? W_ROOK   : B_ROOK;
        else if (c == 'b') pc = (P->side == WHITE) ? W_BISHOP : B_BISHOP;
        else if (c == 'n') pc = (P->side == WHITE) ? W_KNIGHT : B_KNIGHT;
    }

    MoveList mv = (MoveList){0};
    gen_all(P, &mv, GEN_ALL);

    for (int i = 0; i < mv.nmoves; i++) {
        const uint32_t m = mv.moves[i];
        if (dcdsrc(m) == src && dcddst(m) == dst && dcdpromo(m) == pc) {
            return m; }
    }

    return 0;
}


void parse_position(Position *P, char *cmd) {
    char *ptr = cmd + 9;

    if (!strncmp(ptr, "startpos", 8))
    {
        memset(P, 0, sizeof *P);
        parse_fen(START_POSITION, P);
    } 
    else
    {
        char *fen = strstr(cmd, "fen");

        if (!fen) {
            memset(P, 0, sizeof *P);
            parse_fen(START_POSITION, P);
        }
        else {
            fen += 4;
            char *mvstart = strstr(fen, " moves");
            size_t len = mvstart ? (size_t)(mvstart - fen) : strlen(fen);
            char fenbuf[256];
            if (len >= sizeof fenbuf) len = sizeof(fenbuf) - 1;
            memcpy(fenbuf, fen, len);
            fenbuf[len] = '\0';

            memset(P, 0, sizeof *P);
            assert(parse_fen(fenbuf, P));
        }
    }

    char *mv = strstr(cmd, "moves");
    if (mv) {
        mv += 6;
        while (*mv) {
            uint32_t m = parse_move(P, mv);
            if (!m) break;
            make_move(P, m);
            while (*mv && *mv != ' ') mv++;
            if (*mv == ' ') mv++;
        }
    }
}


void search(Position *P, int depth)
{
    uint32_t mv = 0;

    int eval = negamax(P, depth, 0, -INF, INF, &mv);

    if (mv == 0) {
        printf("info score cp %d depth %d nodes %ld\n", eval, depth, g_nodes);
        g_nodes = 0;
        printf("bestmove 0000\n");
        return;
    }

    char buf1[3]; idxtosq(dcdsrc(mv), buf1);
    char buf2[3]; idxtosq(dcddst(mv), buf2);

    printf("info score cp %d depth %d nodes %ld\n", eval, depth, g_nodes);
    g_nodes = 0;
    
    Piece promo = dcdpromo(mv);
    if (promo) printf("bestmove %s%s%c\n", buf1, buf2, (idxtopc(promo) | 32));
    else       printf("bestmove %s%s\n", buf1, buf2);
}

void parse_go(Position *P, char *cmd)
{
    char *ptr;
    int depth = 6;

    if ((ptr = strstr(cmd, "depth")))
        depth = atoi(ptr + 6);

    /* Temporary set depth */
    if (!depth) depth = 6;
    
    search(P, 1);
}

void uci_loop(Position *P)
{
    setbuf(stdin, NULL);
    setbuf(stdout, NULL);

    char buf[MAXLEN];

    while (1) {
        memset(buf, 0, MAXLEN);
        fflush(stdout);

        if (!fgets(buf, MAXLEN, stdin)) continue;

        if (buf[0] == '\n') continue;

        if (!strncmp(buf, "isready", 7)) {
            printf("readyok\n");
            continue;
        }
        else if (!strncmp(buf, "position", 8)) {
            parse_position(P, buf);
        }
        else if (!strncmp(buf, "ucinewgame", 10)) {
            parse_position(P, "position startpos");
        }
        else if (!strncmp(buf, "go", 2)) {
            parse_go(P, buf);
        }
        else if (!strncmp(buf, "uci", 3)) {
            printf("id name stackphish\n");
            printf("id author Leninadeee\n");
            printf("uciok\n");
        }
        else if (!strncmp(buf, "quit", 4)) {
            break;
        }
    }
}