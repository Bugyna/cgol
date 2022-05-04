CC=gcc
CFLAGS=-Wall -Wextra -pedantic
CLIBS=-lSDL2 -lm
main: *.c
	$(CC) main.c -o main $(CFLAGS) $(CLIBS)

run:
	./main