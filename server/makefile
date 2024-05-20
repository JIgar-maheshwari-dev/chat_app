CC = gcc
CFLAGS = -Wall -g

CFILES=$(wildcard ./*.c);
HEADER=-I ../header

all:server

server:$(CFILES)
	$(CC) -o $@ $^ $(CFLAGS) $(HEADER)
