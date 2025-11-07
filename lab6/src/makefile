CC = gcc
CFLAGS = -pthread

PROGRAMS = client server
SERVERS_FILE = servers.txt

all: $(PROGRAMS)

client: client.c utils.o
	$(CC) $(CFLAGS) -o client client.c utils.o

server: server.c utils.o
	$(CC) $(CFLAGS) -o server server.c utils.o

utils.o: utils.c utils.h
	$(CC) $(CFLAGS) -c utils.c

run-servers:
	@echo "Запуск серверов из $(SERVERS_FILE)..."
	@while IFS=':' read -r ip port; do \
		if [ -n "$$ip" ] && [ -n "$$port" ]; then \
			echo "Запуск сервера: $$ip:$$port (tnum=4)"; \
			./server --port $$port --tnum 4 & \
		fi; \
	done < $(SERVERS_FILE)

run-client:
	@echo "Запуск клиента..."
	./client --k 1000 --mod 10003 --servers $(SERVERS_FILE)


clean:
	rm -f $(PROGRAMS) utils.o
	killall server 2>/dev/null || true

.PHONY: all clean run-servers run-client test