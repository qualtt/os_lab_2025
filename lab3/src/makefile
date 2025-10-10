CC=gcc
CFLAGS=-I.

all: sequential_min_max parallel_min_max exec_sequential_min_max

sequential_min_max : utils.o find_min_max.o utils.h find_min_max.h
	$(CC) -o sequential_min_max find_min_max.o utils.o sequential_min_max.c $(CFLAGS)

parallel_min_max : utils.o find_min_max.o utils.h find_min_max.h
	$(CC) -o parallel_min_max utils.o find_min_max.o parallel_min_max.c $(CFLAGS)

exec_sequential_min_max: exec_sequential_min_max.o
	$(CC) $(CFLAGS) -o exec_sequential_min_max exec_sequential_min_max.o

utils.o : utils.h
	$(CC) -o utils.o -c utils.c $(CFLAGS)

find_min_max.o : utils.h find_min_max.h
	$(CC) -o find_min_max.o -c find_min_max.c $(CFLAGS)

exec_sequential_min_max.o: exec_sequential_min_max.c
	$(CC) $(CFLAGS) -c exec_sequential_min_max.c

clean :
	rm -f *.o parallel_min_max sequential_min_max exec_sequential_min_max
