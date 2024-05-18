CC = gcc
CFLAGS = -Wall -g

CFILES=$(wildcard ./*.c);
HEADER=-I ../header
LIB=-lpthread


all:server

server:$(CFILES)
	$(CC) -o $@ $^ $(CFLAGS) $(HEADER) $(LIB)
