all: clean printer user init

test: all

printer: printer.c libqueue.so
	gcc printer.c -Wall -L. -lqueue -Wl,-rpath=. -o printer

user: user.c libqueue.so
	gcc user.c -Wall -L. -lqueue -Wl,-rpath=. -o user

init: init.c libqueue.so
	gcc init.c -Wall -L. -lqueue -Wl,-rpath=. -o init

libqueue.so: queue.o
	gcc -shared -fPIC queue.o -o libqueue.so

queue.o: queue.c
	gcc -c queue.c -o queue.o

clean:
	rm -f printer user init libqueue.so queue.o
