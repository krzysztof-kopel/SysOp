all: zad1 catcher sender

.PHONY: all test clean

test: all

catcher: catcher.c 
	gcc catcher.c -o catcher

sender: sender.c 
	gcc sender.c -o sender

zad1: zad1.c
	gcc zad1.c -o zad1

clean:
	find . -type f -executable -delete
