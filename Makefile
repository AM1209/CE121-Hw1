CC= gcc
CFLAGS= -g -Wall

all: hw1.o db.o 
	$(CC) $(CFLAGS) hw1.o db.o -o all

db.o: db.c
	$(CC) $(CFLAGS) db.c -c

hw1.o: hw1.c
	$(CC) $(CFLAGS) hw1.c -c

.PHONY: clean
clean:
	rm *.o