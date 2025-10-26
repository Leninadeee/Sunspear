#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bitboard.h"
#include "movegen.h"
#include "search.h"
#include "types.h"
#include "uci.h"

void uci_print_move(uint32_t mv)
{
    char buf1[3]; idxtosq(dcdsrc(mv), buf1);
    char buf2[3]; idxtosq(dcddst(mv), buf2);
    Piece promo = dcdpromo(mv);
    if (promo) printf("%s%s%c", buf1, buf2, (idxtopc(promo) | 32));
    else       printf("%s%s", buf1, buf2);
}

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


void search(Position *P, OrderTables *ord, int depth)
{
    assert(depth > 0);

    int eval;
    uint32_t mv;
    //g_nodes = 0;
    
    memset(ord, 0, sizeof(OrderTables));

    for (int curr_depth = 1; curr_depth <= depth; curr_depth++)
    {
        g_nodes = 0;
        memset(ord->follow_pv, 0, MAX_PLY);
        memset(ord->score_pv, 0, MAX_PLY);
        ord->follow_pv[0] = true;

        eval = negamax(P, curr_depth, 0, -INF, INF, ord);

        printf("info score cp %d depth %d nodes %ld pv ", eval, curr_depth,
                                                          g_nodes);

        for (int i = 0; i < ord->pv_len[0]; i++) {
            uci_print_move(ord->pv_table[0][i]); printf(" ");
        }

        printf("\n");
    }
    
    mv = ord->pv_table[0][0];

    if (mv == 0) {
        printf("info score cp %d depth %d nodes %ld\n", eval, depth, g_nodes);
        printf("bestmove 0000\n");
        return;
    }

    char buf1[3]; idxtosq(dcdsrc(mv), buf1);
    char buf2[3]; idxtosq(dcddst(mv), buf2);
    
    printf("bestmove "); uci_print_move(mv); printf("\n");
}

void parse_go(Position *P, OrderTables *ord, char *cmd)
{
    char *ptr;
    int depth = 10;

    if ((ptr = strstr(cmd, "depth")))
        depth = atoi(ptr + 6);

    /* Temporary set depth */
    if (!depth) depth = 6;
    
    search(P, ord, depth);
}

void uci_loop(Position *P, OrderTables *ord)
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
            parse_go(P, ord, buf);
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