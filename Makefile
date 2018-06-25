CC= gcc
CFLAGS= -Wall -g
OBJ= main.o util.o db.o

all: $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) -o $@  #$@=target

main.o: main.c db.h
	$(CC) $(CFLAGS) -c $<  #$< first dependency

db.o: db.c util.h
	$(CC) $(CFLAGS) -c $<

util.o: util.c
	$(CC) $(CFLAGS) -c $<