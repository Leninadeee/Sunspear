#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bitboard.h"
#include "movegen.h"
#include "search.h"
#include "types.h"
#include "uci.h"

TimeCntl g_tc;

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
    if (!mvstr || !mvstr[0] || !mvstr[1] || !mvstr[2] || !mvstr[3])
        return 0;

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
    
    memset(ord, 0, sizeof(OrderTables));
    for (int d = 1; d <= depth; d++)
    {
        g_nodes = 0;
        memset(ord->follow_pv, 0, MAX_PLY);
        memset(ord->score_pv,  0, MAX_PLY);
        ord->follow_pv[0] = true;

        g_tc.abort_iter = false;

        eval = negamax(P, d, 0, -INF, INF, ord);

        printf("info score cp %d depth %d nodes %ld pv ", eval, d, g_nodes);
        for (int i = 0; i < ord->pv_len[0]; i++) {
            uci_print_move(ord->pv_table[0][i]); printf(" ");
        }
        printf("\n");

        if (g_tc.abort_iter || g_tc.stop_now) break;
    }

    uint32_t mv = ord->pv_table[0][0];
    if (!mv) {
        printf("bestmove 0000\n");
        return; 
    }
    printf("bestmove "); uci_print_move(mv); printf("\n");
}


void parse_go_tokens(const char *s, GoParams *gp)
{
    memset(gp, 0, sizeof *gp);

    while (*s) {
        int d;
        uint64_t u;
        if (sscanf(s, " wtime %llu", &u) == 1) {
            gp->wtime = u; gp->f_wtime = 1; 
        }
        else if (sscanf(s, " btime %llu", &u) == 1) {
            gp->btime = u; gp->f_btime = 1;
        }
        else if (sscanf(s, " winc %llu",  &u) == 1) {
            gp->winc = u; gp->f_winc = 1;
        }
        else if (sscanf(s, " binc %llu",  &u) == 1) {
            gp->binc = u; gp->f_binc = 1;
        }
        else if (sscanf(s, " movetime %llu", &u) == 1) {
            gp->movetime = u;
        }
        else if (sscanf(s, " nodes %llu", &u) == 1) {
            gp->nodes = u;
        }
        else if (sscanf(s, " depth %d", &d) == 1) {
            gp->depth = d;
        }
        else if (sscanf(s, " movestogo %d", &d) == 1) {
            gp->movestogo = d; gp->f_mtg = 1;
        }
        else if (strstr(s, " infinite") == s) {
            gp->infinite = true;
        }
        else if (strstr(s, " ponder") == s) {
            gp->ponder = true;
        }

        while (*s && *s!=' ') s++;
        while (*s==' ') s++;
    }
}

void time_setup(const GoParams *gp, const Position *P)
{
    memset(&g_tc, 0, sizeof g_tc);

    g_tc.node_limit = gp->nodes;
    g_tc.infinite   = gp->infinite;
    g_tc.ponder     = gp->ponder;
    g_tc.start_ms   = now_ms();

    if (gp->movetime) {
        g_tc.enabled = true;
        g_tc.soft_ms = gp->movetime;
        g_tc.hard_ms = gp->movetime;
        return;
    }

    if (g_tc.node_limit || g_tc.infinite || g_tc.ponder)
    {
        g_tc.enabled = false;
        return;
    }

    uint64_t time = (P->side == WHITE) ? gp->wtime : gp->btime;
    uint64_t incr  = (P->side == WHITE) ? gp->winc  : gp->binc;

    if (!time) {
        g_tc.enabled = true;
        g_tc.soft_ms = 5;
        g_tc.hard_ms = 10;
        return;
    }

    int mtg = gp->f_mtg ? gp->movestogo : 30;
    if (mtg < 1) mtg = 1;

    uint64_t base    = time / (uint64_t)(mtg + 1);
    uint64_t bonus   = incr ? (incr * 2 / 3) : 0;
    uint64_t softcap = base + bonus;

    if (softcap < 5) softcap = 5;
    uint64_t max_ms = time * 7 / 10;
    if (softcap > max_ms) softcap = max_ms;

    uint64_t hardcap = softcap + (softcap / 2) + 5;
    if (hardcap >= time) hardcap = (time > 10) ? time - 10 : softcap;

    g_tc.enabled = true;
    g_tc.soft_ms = softcap;
    g_tc.hard_ms = hardcap;
}

void parse_go(Position *P, OrderTables *ord, char *cmd)
{
    GoParams gp;
    parse_go_tokens(cmd + 2, &gp);
    time_setup(&gp, P);

    int max_depth = gp.depth > 0 ? gp.depth : 64;
    if (gp.depth > 0) {
        memset(&g_tc, 0, sizeof g_tc);
    }

    search(P, ord, max_depth);
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
        else if (!strncmp(buf, "stop", 4)) {
            g_tc.stop_now = true;
        }
        else if (!strncmp(buf, "quit", 4)) {
            break;
        }
    }
}