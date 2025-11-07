#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <getopt.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <pthread.h>
#include <arpa/inet.h>
#include "utils.h"

struct Server {
    char ip[255];
    int port;
    uint64_t begin;
    uint64_t end;
    uint64_t mod;
};

// подключение к одному серверу, отправка задачи, получение результата
void *server_task(void *args) {
    struct Server *srv = (struct Server *)args;
    uint64_t *result = malloc(sizeof(uint64_t));
    *result = 1;

    struct addrinfo hints = {0}, *res;
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    char port_str[10];
    snprintf(port_str, sizeof(port_str), "%d", srv->port);

    if (getaddrinfo(srv->ip, port_str, &hints, &res) != 0) {
        fprintf(stderr, "getaddrinfo failed: %s:%d\n", srv->ip, srv->port);
        return result;
    }

    int sck = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (sck < 0) {
        perror("socket");
        freeaddrinfo(res);
        return result;
    }

    if (connect(sck, res->ai_addr, res->ai_addrlen) < 0) {
        fprintf(stderr, "connect failed: %s:%d\n", srv->ip, srv->port);
        close(sck);
        freeaddrinfo(res);
        return result;
    }

    freeaddrinfo(res);

    char task[sizeof(uint64_t) * 3];
    memcpy(task, &srv->begin, sizeof(uint64_t));
    memcpy(task + sizeof(uint64_t), &srv->end, sizeof(uint64_t));
    memcpy(task + 2 * sizeof(uint64_t), &srv->mod, sizeof(uint64_t));

    if (send(sck, task, sizeof(task), 0) != sizeof(task)) {
        fprintf(stderr, "send failed: %s:%d\n", srv->ip, srv->port);
        close(sck);
        return result;
    }

    char buffer[sizeof(uint64_t)];
    if (recv(sck, buffer, sizeof(buffer), 0) != sizeof(buffer)) {
        fprintf(stderr, "recv failed: %s:%d\n", srv->ip, srv->port);
        close(sck);
        return result;
    }

    memcpy(result, buffer, sizeof(uint64_t));
    close(sck);
    return result;  // вместо pthread_exit
}

int main(int argc, char **argv) {
    uint64_t k = -1, mod = -1;
    char servers_path[255] = {0};

    while (true) {
        int current_optind = optind ? optind : 1;
        static struct option options[] = {
            {"k", required_argument, 0, 0},
            {"mod", required_argument, 0, 0},
            {"servers", required_argument, 0, 0},
            {0, 0, 0, 0}
        };
        int option_index = 0;
        int c = getopt_long(argc, argv, "", options, &option_index);
        if (c == -1) break;

        switch (c) {
            case 0:
                switch (option_index) {
                    case 0: ConvertStringToUI64(optarg, &k); break;
                    case 1: ConvertStringToUI64(optarg, &mod); break;
                    case 2: strncpy(servers_path, optarg, 254); break;
                }
                break;
            case '?':
                fprintf(stderr, "Invalid argument\n");
                return 1;
        }
    }

    if (k == 0 || mod <= 1 || !servers_path[0]) {
        fprintf(stderr, "Usage: %s --k 1000 --mod 13 --servers servers.txt\n", argv[0]);
        return 1;
    }

    // чтение серверов из файла
    FILE *file = fopen(servers_path, "r");
    if (!file) {
        perror("fopen");
        return 1;
    }

    struct Server *servers = NULL;
    unsigned int server_count = 0;
    char line[300];

    while (fgets(line, sizeof(line), file)) {
        line[strcspn(line, "\n")] = '\0';
        if (!strlen(line)) continue;

        servers = realloc(servers, (server_count + 1) * sizeof(struct Server));
        if (!servers) { perror("realloc"); return 1; }

        char *colon = strchr(line, ':');
        if (!colon) {
            fprintf(stderr, "Invalid format: %s (expected ip:port)\n", line);
            continue;
        }

        *colon = '\0';
        strncpy(servers[server_count].ip, line, 254);
        servers[server_count].port = atoi(colon + 1);

        if (servers[server_count].port <= 0 || servers[server_count].port > 65535) {
            fprintf(stderr, "Invalid port: %s\n", colon + 1);
            continue;
        }

        server_count++;
    }
    fclose(file);

    if (server_count == 0) {
        fprintf(stderr, "No valid servers found\n");
        return 1;
    }

    uint64_t chunk = k / server_count;
    uint64_t remainder = k % server_count;
    uint64_t current = 1;

    pthread_t *threads = malloc(server_count * sizeof(pthread_t));
    if (!threads) { perror("malloc"); return 1; }

    for (unsigned int i = 0; i < server_count; i++) {
        servers[i].begin = current;
        servers[i].end = current + chunk - 1;
        if (i < remainder) servers[i].end++;
        if (servers[i].end > k) servers[i].end = k;
        servers[i].mod = mod;

        current = servers[i].end + 1;

        if (pthread_create(&threads[i], NULL, server_task, &servers[i]) != 0) {
            perror("pthread_create");
            return 1;
        }
    }

    // Объединение результатов
    uint64_t final_result = 1;
    for (unsigned int i = 0; i < server_count; i++) {
        void *res_ptr;
        pthread_join(threads[i], &res_ptr);
        uint64_t partial = *(uint64_t *)res_ptr;
        final_result = MultModulo(final_result, partial, mod);
        free(res_ptr);

        printf("Server %s:%d: [%lu, %lu] → %lu\n",
               servers[i].ip, servers[i].port, servers[i].begin, servers[i].end, partial);
    }

    printf("Final result: %lu! mod %lu = %lu\n", k, mod, final_result);

    free(threads);
    free(servers);
    return 0;
}