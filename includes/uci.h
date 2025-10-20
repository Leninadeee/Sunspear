#ifndef UCI_H
#define UCI_H

#define MAXLEN  2048

extern uint32_t parse_move(const Position *, const char *);
extern void parse_position(Position *, char *);
extern void parse_go(Position *, char *);
extern void uci_loop(Position *);

#endif