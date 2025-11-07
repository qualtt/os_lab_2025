#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

pthread_mutex_t mutexA = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutexB = PTHREAD_MUTEX_INITIALIZER;

void* thread1_func(void* arg) {
    printf("[Поток 1] Захватываю mutexA...\n");
    pthread_mutex_lock(&mutexA);
    sleep(1);

    printf("[Поток 1] Пытаюсь захватить mutexB...\n");
    pthread_mutex_lock(&mutexB);  //  deadlock

    printf("[Поток 1] Успешно захватил оба мьютекса!\n");
    pthread_mutex_unlock(&mutexB);
    pthread_mutex_unlock(&mutexA);
    return NULL;
}

void* thread2_func(void* arg) {
    printf("[Поток 2] Захватываю mutexB...\n");
    pthread_mutex_lock(&mutexB);
    sleep(1);

    printf("[Поток 2] Пытаюсь захватить mutexA...\n");
    pthread_mutex_lock(&mutexA);  // deadlock

    printf("[Поток 2] Успешно захватил оба мьютекса\n");
    pthread_mutex_unlock(&mutexA);
    pthread_mutex_unlock(&mutexB);
    return NULL;
}

int main() {
    pthread_t threads[2];

    if (pthread_create(&threads[0], NULL, thread1_func, NULL) != 0) {
        perror("pthread_create thread1");
        return 1;
    }
    if (pthread_create(&threads[1], NULL, thread2_func, NULL) != 0) {
        perror("pthread_create thread2");
        return 1;
    }

    pthread_join(threads[0], NULL);
    pthread_join(threads[1], NULL);

    printf("Программа завершилась\n");
    return 0;
}