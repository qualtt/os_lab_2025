CC = gcc
CFLAGS = -Wall -Wextra
PROGRAMS = tcpclient tcpserver udpclient udpserver

all: $(PROGRAMS)

tcpclient: tcpclient.c
	$(CC) $(CFLAGS) -o tcpclient tcpclient.c

tcpserver: tcpserver.c
	$(CC) $(CFLAGS) -o tcpserver tcpserver.c

udpclient: udpclient.c
	$(CC) $(CFLAGS) -o udpclient udpclient.c

udpserver: udpserver.c
	$(CC) $(CFLAGS) -o udpserver udpserver.c

clean:
	rm -f $(PROGRAMS)

.PHONY: all clean