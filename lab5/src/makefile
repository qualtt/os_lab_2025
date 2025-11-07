CC = gcc
CFLAGS = -pthread
PROGRAMS = mutex_with mutex_without factorial deadlock

all: $(PROGRAMS)

mutex_with: mutex.c
	$(CC) $(CFLAGS) -DUSE_MUTEX -o mutex_with mutex.c 

mutex_without: mutex.c
	$(CC) $(CFLAGS) -o mutex_without mutex.c 

factorial: factorial.c
	$(CC) $(CFLAGS) -o factorial factorial.c

deadlock: deadlock.c
	$(CC) $(CFLAGS) -o deadlock deadlock.c

clean:
	rm -f $(PROGRAMS)

.PHONY: all clean

