CC=gcc
CFLAGS=-I.
LDFLAGS=-pthread

all: parallel_min_max zombie_demo process_memory parallel_sum

parallel_min_max: utils.o find_min_max.o utils.h find_min_max.h
	$(CC) -o parallel_min_max utils.o find_min_max.o parallel_min_max.c $(CFLAGS)

zombie_demo: zombie_demo.c
	$(CC) -o zombie_demo zombie_demo.c $(CFLAGS)

process_memory: process_memory.c
	$(CC) -o process_memory process_memory.c $(CFLAGS) -w

parallel_sum: parallel_sum.o utils.o libsum.a
	$(CC) -o parallel_sum parallel_sum.o utils.o -L. -lsum $(CFLAGS) $(LDFLAGS)

libsum.a: sum.o
	ar rcs libsum.a sum.o

sum.o: sum.c sum.h
	$(CC) -c sum.c -o sum.o $(CFLAGS)

parallel_sum.o: parallel_sum.c utils.h sum.h
	$(CC) -c parallel_sum.c -o parallel_sum.o $(CFLAGS)

utils.o: utils.h
	$(CC) -o utils.o -c utils.c $(CFLAGS)

find_min_max.o: utils.h find_min_max.h
	$(CC) -o find_min_max.o -c find_min_max.c $(CFLAGS)

clean:
	rm -f *.o parallel_min_max zombie_demo process_memory parallel_sum libsum.a

.PHONY: all clean