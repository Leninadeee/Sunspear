CC      := gcc
CFLAGS  := -O3 -Wall -Wextra -std=c23 -mbmi2
INCLUDE := -I./includes -I./deps
SRC     := src/* deps/tinycthread.c
BIN     := engine

all: $(BIN)

$(BIN): $(SRC)
	$(CC) $(CFLAGS) $(INCLUDE) -o $@ $(SRC)

clean:
	rm -f $(BIN)