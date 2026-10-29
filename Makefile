CC       := g++
CFLAGS   := -O3 -Wall -Wextra -std=c++20 -mbmi2
INCLUDE  := -I./src -I./src/utils
SRC      := $(wildcard src/*.cpp) $(wildcard src/utils/*.cpp)
BIN      := engine

all: $(BIN)

$(BIN): $(SRC)
	$(CC) $(CFLAGS) $(INCLUDE) -o $@ $(SRC)

clean:
	rm -f $(BIN)