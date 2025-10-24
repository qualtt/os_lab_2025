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
#include <fcntl.h>
#include <signal.h>

#include "find_min_max.h"
#include "utils.h"

pid_t *child_pids = NULL;
int child_pids_size = 0;

bool timeout_triggered = false;

void alarm_handler(int sig)
{
  printf("Timeout occurred! Terminating child processes.\n");
  timeout_triggered = true;
  for (int i = 0; i < child_pids_size; i++)
  {
    if (child_pids[i] > 0)
    {
      kill(child_pids[i], SIGKILL);
    }
  }
  free(child_pids);
  child_pids = NULL;
  child_pids_size = 0;
}

int main(int argc, char **argv)
{
  int seed = -1;
  int array_size = -1;
  int pnum = -1;
  int timeout = -1;
  bool with_files = false;

  while (true)
  {
    int current_optind = optind ? optind : 1;

    static struct option options[] = {
        {"seed", required_argument, 0, 0},
        {"array_size", required_argument, 0, 0},
        {"pnum", required_argument, 0, 0},
        {"by_files", no_argument, 0, 'f'},
        {"timeout", required_argument, 0, 0},
        {0, 0, 0, 0}};

    int option_index = 0;
    int c = getopt_long(argc, argv, "f", options, &option_index);

    if (c == -1)
      break;

    switch (c)
    {
    case 0:
      switch (option_index)
      {
      case 0:
        seed = atoi(optarg);
        if (seed <= 0)
        {
          printf("seed must be a positive number\n");
          return 1;
        }
        break;
      case 1:
        array_size = atoi(optarg);
        if (array_size <= 0)
        {
          printf("array_size must be a positive number\n");
          return 1;
        }
        break;
      case 2:
        pnum = atoi(optarg);
        if (pnum <= 0)
        {
          printf("pnum must be a positive number\n");
          return 1;
        }
        break;
      case 3:
        with_files = true;
        break;
      case 4:
        timeout = atoi(optarg);
        if (timeout <= 0)
        {
          printf("timeout must be a positive number\n");
          return 1;
        }
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

  if (optind < argc)
  {
    printf("Has at least one no option argument\n");
    return 1;
  }

  if (seed == -1 || array_size == -1 || pnum == -1)
  {
    printf("Usage: %s --seed \"num\" --array_size \"num\" --pnum \"num\" [--timeout \"num\"]\n",
           argv[0]);
    return 1;
  }

  if (pnum > array_size)
  {
    printf("pnum cannot be larger than array_size\n");
    return 1;
  }

  child_pids = malloc(sizeof(pid_t) * pnum);
  child_pids_size = pnum;
  for (int i = 0; i < pnum; i++)
  {
    child_pids[i] = 0;
  }

  int *array = malloc(sizeof(int) * array_size);
  if (array == NULL)
  {
    printf("Failed to allocate memory for array\n");
    free(child_pids);
    return 1;
  }
  GenerateArray(array, array_size, seed);

  int pipes[pnum][2];
  if (!with_files)
  {
    for (int i = 0; i < pnum; i++)
    {
      if (pipe(pipes[i]) == -1)
      {
        printf("Pipe creation failed\n");
        free(array);
        free(child_pids);
        return 1;
      }
    }
  }

  struct timeval start_time;
  gettimeofday(&start_time, NULL);

  int chunk_size = array_size / pnum;
  int remainder = array_size % pnum;

  int active_child_processes = 0;

  for (int i = 0; i < pnum; i++)
  {
    pid_t child_pid = fork();
    if (child_pid >= 0)
    {
      // Успешный fork
      active_child_processes += 1;
      if (child_pid == 0)
      {
        // Дочерний процесс
        unsigned int begin = i * chunk_size + (i < remainder ? i : remainder);
        unsigned int end = begin + chunk_size + (i < remainder ? 1 : 0);

        struct MinMax local_min_max = GetMinMax(array, begin, end);

        if (with_files)
        {
          // файлы
          char filename[32];
          snprintf(filename, sizeof(filename), "min_max_%d.txt", i);
          FILE *file = fopen(filename, "w");
          if (file == NULL)
          {
            printf("Failed to open file %s\n", filename);
            free(array);
            exit(1);
          }
          fprintf(file, "%d %d\n", local_min_max.min, local_min_max.max);
          fclose(file);
          printf("Child %d wrote to file %s\n", i, filename);
        }
        else
        {
          // каналы
          close(pipes[i][0]); 
          write(pipes[i][1], &local_min_max, sizeof(struct MinMax));
          close(pipes[i][1]);
        }

        free(array);
        exit(0);
      }
      else
      {
        // Родительский процесс
        child_pids[i] = child_pid;
      }
    }
    else
    {
      printf("Fork failed!\n");
      free(array);
      free(child_pids);
      return 1;
    }
  }

  // Установка таймера, если задан таймаут
  if (timeout > 0)
  {
    signal(SIGALRM, alarm_handler);
    alarm(timeout);
  }

  while (active_child_processes > 0)
  {
    wait(NULL);
    active_child_processes -= 1;
  }

  if (timeout > 0)
  {
    alarm(0);
    if (child_pids != NULL)
    {
      free(child_pids);
      child_pids = NULL;
      child_pids_size = 0;
    }
  }

  if (timeout_triggered)
  {
    printf("Warning: Results may be incomplete due to timeout.\n");
  }

  struct MinMax min_max;
  min_max.min = INT_MAX;
  min_max.max = INT_MIN;

  for (int i = 0; i < pnum; i++)
  {
    int min = INT_MAX;
    int max = INT_MIN;

    if (with_files)
    {
      // Чтение из файлов
      char filename[32];
      snprintf(filename, sizeof(filename), "min_max_%d.txt", i);
      FILE *file = fopen(filename, "r");
      if (file == NULL)
      {
        printf("Failed to open file %s, possibly due to timeout\n", filename);
        continue;
      }
      if (fscanf(file, "%d %d", &min, &max) != 2)
      {
        printf("Failed to read valid data from file %s\n", filename);
        fclose(file);
        remove(filename);
        continue;
      }
      fclose(file);
      remove(filename);
    }
    else
    {
      // Чтение из каналов
      close(pipes[i][1]);
      struct MinMax local_min_max;
      ssize_t bytes_read = read(pipes[i][0], &local_min_max, sizeof(struct MinMax));
      close(pipes[i][0]);
      if (bytes_read != sizeof(struct MinMax))
      {
        printf("Failed to read valid data from pipe %d, possibly due to timeout\n", i);
        continue;
      }
      min = local_min_max.min;
      max = local_min_max.max;
    }

    if (min < min_max.min)
      min_max.min = min;
    if (max > min_max.max)
      min_max.max = max;
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
