#ifndef UCI_H
#define UCI_H

#define MAXLEN  2048

extern void uci_print_move(uint32_t);
extern uint32_t parse_move(const Position *, const char *);
extern void parse_position(Position *, char *);
extern void parse_go(Position *, OrderTables *, char *);
extern void uci_loop(Position *, OrderTables *);

#endif /* UCI_H */