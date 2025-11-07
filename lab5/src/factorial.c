#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>

typedef struct
{
    int start;
    int end;
    long long mod;
} ThreadData;

long long result = 1;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void *partial_factorial(void *arg)
{
    ThreadData *data = (ThreadData *)arg;
    long long partial_result = 1;
   
    for (int i = data->start; i <= data->end; i++)
    {
        partial_result = (partial_result * i) % data->mod;
    }
   
    pthread_mutex_lock(&mutex);
    result = (result * partial_result) % data->mod;
    pthread_mutex_unlock(&mutex);
    return NULL;
}

int main(int argc, char *argv[])
{
    int k = 0, pnum = 0;
    long long mod = 0;
    
    for (int i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "-k") == 0 && i + 1 < argc)
        {
            k = atoi(argv[++i]);
        }
        else if (strcmp(argv[i], "--pnum") == 0 && i + 1 < argc)
        {
            pnum = atoi(argv[++i]);
        }
        else if (strcmp(argv[i], "--mod") == 0 && i + 1 < argc)
        {
            mod = atoll(argv[++i]);
        }
    }
   
    if (k < 0 || pnum <= 0 || mod <= 0)
    {
        printf("Ошибка: некорректные параметры. Использование: ./factorial -k <число> --pnum=<потоки> --mod=<модуль>\n");
        return 1;
    }
    
    if (pnum > k)
    {
        pnum = k;
        printf("Предупреждение: pnum больше k, установлено pnum = %d\n", pnum);
    }
   
    pthread_t *threads = (pthread_t *)malloc(pnum * sizeof(pthread_t));
    ThreadData *thread_args = (ThreadData *)malloc(pnum * sizeof(ThreadData));
    if (threads == NULL || thread_args == NULL)
    {
        printf("Ошибка выделения памяти\n");
        return 1;
    }

    
    int chunk_size = k / pnum;
    int remainder = k % pnum;
    int current_position = 1;

    for (int i = 0; i < pnum; i++)
    {
        thread_args[i].start = current_position;
        thread_args[i].end = current_position + chunk_size - 1;
        if (i < remainder)
        {
            thread_args[i].end++;
        }
        thread_args[i].mod = mod;
        current_position = thread_args[i].end + 1;
        
        if (pthread_create(&threads[i], NULL, partial_factorial, &thread_args[i]) != 0)
        {
            printf("Ошибка создания потока %d\n", i);
            free(threads);
            free(thread_args);
            return 1;
        }
    }
   
    for (int i = 0; i < pnum; i++)
    {
        if (pthread_join(threads[i], NULL)!= 0)
        {
            printf("Ошибка ожидания потока %d\n", i);
            free(threads);
            free(thread_args);
            return 1;
        }
    }

    
    printf("Факториал %d! по модулю %lld = %lld\n", k, mod, result);
   
    free(threads);
    free(thread_args);
    pthread_mutex_destroy(&mutex);
    return 0;
}