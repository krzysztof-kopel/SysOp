CC = gcc
CFLAGS = -g -Wall -o

.PHONY: all test clean

all: countdown

test: countdown countdown.o

countdown: countdown.c
	$(CC) countdown.c $(CFLAGS) countdown

countdown.o: countdown.c
	$(CC) countdown.c -c $(CFLAGS) countdown.o

clean:
	rm *o countdown
