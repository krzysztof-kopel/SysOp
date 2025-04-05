.PHONY: all clean libs test

all: libs main_static main_shared main_dynamic

test: all

main_static: main.c
	gcc main.c -static -L. -lcollatz -o main_static

main_shared: main.c
	gcc main.c -L. -lcollatz -Wl,-rpath=. -o main_shared

main_dynamic: main_dynamic.c
	gcc -DUSE_DYNAMIC_LOADING main_dynamic.c -Wl,-rpath=. -o main_dynamic



libs: libcollatz.a libcollatz.so

libcollatz.a: collatz.o
	ar rcs libcollatz.a collatz.o 

libcollatz.so: collatz.c
	g++ -shared -fPIC collatz.o -o libcollatz.so

collatz.o: collatz.cpp
	gcc -c collatz.cpp

clean: 
	rm -f *o lib* main_static main_shared main_dynamic
