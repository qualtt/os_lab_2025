#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main() {
    // Создание дочернего процесса
    pid_t pid = fork();

    if (pid < 0) {
        printf("Fork failed!\n");
        return 1;
    } else if (pid == 0) {
        // Дочерний процесс
        printf("Child process (PID %d) is running and will exit immediately.\n", getpid());
        exit(0);
    } else {
        // Родительский процесс
        printf("Parent process (PID %d) created child (PID %d).\n", getpid(), pid);     
        printf("Parent will sleep for 60 seconds. Use 'ps aux | grep Z' to see the zombie process.\n");
        sleep(60); // Родитель засыпает, не вызывая wait()
        printf("Parent waking up and exiting.\n");
    }

    return 0;
}