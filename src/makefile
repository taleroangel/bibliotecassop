CC	= @gcc
FLAGS = -Wall

all: client server

server: server.o
	$(CC) $(FLAGS) -o $@ $<

server.o: server.c server.h common.h data.h
	$(CC) $(FLAGS) -o $@ $<

client: client.o
	$(CC) $(FLAGS) -o $@ $<

client.o: client.c client.h common.h data.h
	$(CC) $(FLAGS) -o $@ $<
