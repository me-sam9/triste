# triste - a tetris clone
# See LICENSE file for copyright and license details.

# customize below to fit your system

BIN = triste
CC = cc
LIBS = -lncurses -ltinfo
DESTDIR = /usr/local/bin/

CFLAGS = -pedantic -Wall -Werror
LDFLAGS = $(LIBS)

SRC = triste.c
OBJ = $(SRC:.c=.o)

all: $(BIN)

$(BIN): $(OBJ)
	$(CC) -o $@ $^ $(LDFLAGS)

run: all
	@./$(BIN)

install: all
	cp $(BIN) $(DESTDIR)

uninstall:
	rm -f $(DESTDIR)$(BIN)

clean:
	rm -f $(BIN) $(OBJ)

.PHONY: all run clean
