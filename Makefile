CC=gcc
CFLAGS=-Wall -Wextra -std=c99 -O2
LDLIBS=-lm

kmeans: main.o kmeans.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDLIBS)

main.o: main.c
kmeans.o: kmeans.c

clean:
	rm -f kmeans main.o kmeans.o

.PHONY: clean
