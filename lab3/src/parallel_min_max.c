#include <ctype.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <getopt.h>
#include "find_min_max.h"
#include "utils.h"

int main(int argc, char **argv) {
  int seed = -1;
  int array_size = -1;
  int pnum = -1;
  bool with_files = false;

  while (true) {
    int current_optind = optind ? optind : 1;

    static struct option options[] = {{"seed", required_argument, 0, 0},
                                      {"array_size", required_argument, 0, 0},
                                      {"pnum", required_argument, 0, 0},
                                      {"by_files", no_argument, 0, 'f'},
                                      {0, 0, 0, 0}};

    int option_index = 0;
    int c = getopt_long(argc, argv, "f", options, &option_index);

    if (c == -1) break;

    switch (c) {
      case 0:
        switch (option_index) {
          case 0:
            seed = atoi(optarg);
            if (seed <= 0) {
              printf("seed must be a positive number\n");
              return 1;
            }
            break;
          case 1:
            array_size = atoi(optarg);
            if (array_size <= 0) {
              printf("array_size must be a positive number\n");
              return 1;
            }
            break;
          case 2:
            pnum = atoi(optarg);
            if (pnum <= 0) {
              printf("pnum must be a positive number\n");
              return 1;
            }
            if (pnum > array_size) {
              printf("pnum cannot be larger than array_size\n");
              return 1;
            }
            break;
          case 3:
            with_files = true;
            break;
          default:
            printf("Index %d is out of options\n", option_index);
        }
        break;
      case 'f':
        with_files = true;
        break;
      case '?':
        break;
      default:
        printf("getopt returned character code 0%o?\n", c);
    }
  }

  if (optind < argc) {
    printf("Has at least one no option argument\n");
    return 1;
  }

  if (seed == -1 || array_size == -1 || pnum == -1) {
    printf("Usage: %s --seed \"num\" --array_size \"num\" --pnum \"num\" [--by_files]\n",
           argv[0]);
    return 1;
  }

  int *array = malloc(sizeof(int) * array_size);
  GenerateArray(array, array_size, seed);

  // Создание массива каналов для каждого дочернего процесса
  int pipes[pnum][2];
  if (!with_files) {
    for (int i = 0; i < pnum; i++) {
      if (pipe(pipes[i]) == -1) {
        printf("Pipe creation failed\n");
        free(array);
        return 1;
      }
    }
  }

  struct timeval start_time;
  gettimeofday(&start_time, NULL);

  int active_child_processes = 0;

  // Разделение массива на pnum частей
  int chunk_size = array_size / pnum;
  int remainder = array_size % pnum;

  for (int i = 0; i < pnum; i++) {
    pid_t child_pid = fork();
    if (child_pid >= 0) {
        // successful fork
      active_child_processes += 1;
      if (child_pid == 0) {
        // child process
        // Вычисляем диапазон [begin, end) для текущего процесса
        unsigned int begin = i * chunk_size + (i < remainder ? i : remainder);
        unsigned int end = begin + chunk_size + (i < remainder ? 1 : 0);

        // Находим min и max в своей части массива
        struct MinMax local_min_max = GetMinMax(array, begin, end);

        if (with_files) {
          // use files here
          char filename[32];
          snprintf(filename, sizeof(filename), "min_max_%d.txt", i);
          FILE *file = fopen(filename, "w");
          if (file == NULL) {
            printf("Failed to open file %s\n", filename);
            free(array);
            exit(1);
          }
          fprintf(file, "%d %d\n", local_min_max.min, local_min_max.max);
          fclose(file);
        } else {
          // use pipe here
          close(pipes[i][0]); // Закрываем конец для чтения
          write(pipes[i][1], &local_min_max, sizeof(struct MinMax));
          close(pipes[i][1]); // Закрываем конец для записи
        }

        free(array); // Освобождаем память в дочернем процессе
        exit(0);
      }
    } else {
      printf("Fork failed!\n");
      free(array);
      return 1;
    }
  }

  // Родительский процесс ждёт завершения всех дочерних
  while (active_child_processes > 0) {
    wait(NULL);
    active_child_processes -= 1;
  }

  struct MinMax min_max;
  min_max.min = INT_MAX;
  min_max.max = INT_MIN;

  for (int i = 0; i < pnum; i++) {
    int min = INT_MAX;
    int max = INT_MIN;

    if (with_files) {
      // read from files
      char filename[32];
      snprintf(filename, sizeof(filename), "min_max_%d.txt", i);
      FILE *file = fopen(filename, "r");
      if (file == NULL) {
        printf("Failed to open file %s\n", filename);
        free(array);
        return 1;
      }
      fscanf(file, "%d %d", &min, &max);
      fclose(file);
      remove(filename); // Удаляем файл после чтения
    } else {
      // read from pipes
      close(pipes[i][1]); // Закрываем конец для записи
      struct MinMax local_min_max;
      read(pipes[i][0], &local_min_max, sizeof(struct MinMax));
      close(pipes[i][0]); // Закрываем конец для чтения
      min = local_min_max.min;
      max = local_min_max.max;
    }

    if (min < min_max.min) min_max.min = min;
    if (max > min_max.max) min_max.max = max;
  }

  struct timeval finish_time;
  gettimeofday(&finish_time, NULL);

  double elapsed_time = (finish_time.tv_sec - start_time.tv_sec) * 1000.0;
  elapsed_time += (finish_time.tv_usec - start_time.tv_usec) / 1000.0;

  free(array);

  printf("Min: %d\n", min_max.min);
  printf("Max: %d\n", min_max.max);
  printf("Elapsed time: %fms\n", elapsed_time);
  fflush(NULL);
  return 0;
}