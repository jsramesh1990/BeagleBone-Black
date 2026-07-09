CC = gcc
CFLAGS = -Wall -Wextra -O2
TARGETS = char_app block_app unified_app

all: $(TARGETS)

char_app: char_app.c
	$(CC) $(CFLAGS) -o char_app char_app.c

block_app: block_app.c
	$(CC) $(CFLAGS) -o block_app block_app.c

unified_app: unified_app.c
	$(CC) $(CFLAGS) -o unified_app unified_app.c -lpthread

clean:
	rm -f $(TARGETS)

install:
	sudo cp char_app /usr/local/bin/
	sudo cp block_app /usr/local/bin/
	sudo cp unified_app /usr/local/bin/

uninstall:
	sudo rm -f /usr/local/bin/char_app
	sudo rm -f /usr/local/bin/block_app
	sudo rm -f /usr/local/bin/unified_app

.PHONY: all clean install uninstall
