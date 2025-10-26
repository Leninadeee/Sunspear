#ifndef UCI_H
#define UCI_H

#define  MAXLEN  2048
#define  MAXFEN  256

extern  void      uci_print_move(uint32_t);
extern  uint32_t  parse_move(const Position *, const char *);
extern  void      parse_position(Position *, char *);
extern  void      parse_go(SearchCtx *, char *);
extern  void      uci_loop(SearchCtx *);

#endif /* UCI_H */