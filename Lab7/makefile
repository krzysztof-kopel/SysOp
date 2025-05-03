all: client server

client: client.c
	gcc client.c -o client

server: server.c
	gcc server.c -o server

examples: client_example server_example

client_example: client_example.c
	gcc client_example.c -o client_example

server_example: server_example.c
	gcc server_example.c -o server_example

clean:
	find . -type f -executable -delete
