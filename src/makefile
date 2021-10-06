CC	= @gcc
FLAGS = -Wall -Wno-sizeof-pointer-memaccess

all: client server

server: server.o
	$(CC) $(FLAGS) -o $@ $<

server.o: server.c server.h common.h data.h
	$(CC) -c $(FLAGS) -o $@ $<

client: client.o
	$(CC) $(FLAGS) -o $@ $<

client.o: client.c client.h common.h data.h
	$(CC) -c $(FLAGS) -o $@ $<

.PHONY: clean
clean:
	rm -f server.o server client.c client