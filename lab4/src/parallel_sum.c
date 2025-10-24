#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <pthread.h>
#include <sys/time.h>
#include "utils.h"
#include "sum.h"
#include <stdbool.h>

void *ThreadSum(void *args)
{
  struct SumArgs *sum_args = (struct SumArgs *)args;
  return (void *)(size_t)Sum(sum_args);
}

int main(int argc, char **argv)
{
  uint32_t threads_num = 0;
  uint32_t array_size = 0;
  uint32_t seed = 0;

  while (true)
  {
    int current_optind = optind ? optind : 1;

    static struct option options[] = {
        {"threads_num", required_argument, 0, 0},
        {"seed", required_argument, 0, 0},
        {"array_size", required_argument, 0, 0},
        {0, 0, 0, 0}};

    int option_index = 0;
    int c = getopt_long(argc, argv, "", options, &option_index);

    if (c == -1)
      break;

    switch (c)
    {
    case 0:
      switch (option_index)
      {
      case 0:
        threads_num = atoi(optarg);
        if (threads_num <= 0)
        {
          printf("threads_num должен быть положительным числом\n");
          return 1;
        }
        break;
      case 1:
        seed = atoi(optarg);
        if (seed <= 0)
        {
          printf("seed должен быть положительным числом\n");
          return 1;
        }
        break;
      case 2:
        array_size = atoi(optarg);
        if (array_size <= 0)
        {
          printf("array_size должен быть положительным числом\n");
          return 1;
        }
        if (threads_num > array_size)
        {
          printf("threads_num не может быть больше array_size\n");
          return 1;
        }
        break;
      default:
        printf("Индекс %d вне диапазона опций\n", option_index);
      }
      break;
    case '?':
      break;
    default:
      printf("getopt вернул код символа 0%o?\n", c);
    }
  }

  if (optind < argc)
  {
    printf("Обнаружен хотя бы один неопциональный аргумент\n");
    return 1;
  }

  if (threads_num == 0 || seed == 0 || array_size == 0)
  {
    printf("Использование: %s --threads_num \"num\" --seed \"num\" --array_size \"num\"\n", argv[0]);
    return 1;
  }

  int *array = malloc(sizeof(int) * array_size);
  if (array == NULL)
  {
    printf("Ошибка выделения памяти для массива\n");
    return 1;
  }
  GenerateArray(array, array_size, seed);

  struct timeval start_time;
  gettimeofday(&start_time, NULL);

  // Инициализация потоков
  pthread_t threads[threads_num];
  struct SumArgs args[threads_num];
  int chunk_size = array_size / threads_num;
  int remainder = array_size % threads_num;

  for (uint32_t i = 0; i < threads_num; i++)
  {
    args[i].array = array;
    args[i].begin = i * chunk_size + (i < remainder ? i : remainder);
    args[i].end = args[i].begin + chunk_size + (i < remainder ? 1 : 0);
    if (pthread_create(&threads[i], NULL, ThreadSum, (void *)&args[i]))
    {
      printf("Ошибка: не удалось создать поток!\n");
      free(array);
      return 1;
    }
  }

  int total_sum = 0;
  for (uint32_t i = 0; i < threads_num; i++)
  {
    void *sum;
    pthread_join(threads[i], &sum);
    total_sum += (int)(size_t)sum;
  }

  struct timeval finish_time;
  gettimeofday(&finish_time, NULL);

  double elapsed_time = (finish_time.tv_sec - start_time.tv_sec) * 1000.0;
  elapsed_time += (finish_time.tv_usec - start_time.tv_usec) / 1000.0;

  free(array);
  printf("Итого: %d\n", total_sum);
  printf("Время выполнения: %f мс\n", elapsed_time);
  return 0;
}
