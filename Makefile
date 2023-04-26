CC=clang
#CFLAGS=-Wall -Wextra -Werror -std=c11 -pedantic -Iusr/include/SDL2 -lSDL2 -lpthread -lm -D_REENTRANT 
CFLAGS=-Wall -Wextra -std=c11 -pedantic -Iusr/include/SDL2 -lSDL2 -lpthread -lm -D_REENTRANT 

all: jul

jul: main.c
	$(CC) $(CFLAGS) $< -o jul

debug: main.c
	$(CC) $(CFLAGS) -g $< -o jul

.PHONY: clean
clean:
	rm jul
