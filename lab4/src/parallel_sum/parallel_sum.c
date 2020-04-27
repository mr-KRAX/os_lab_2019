#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

#include <pthread.h>
#include <getopt.h>
#include <sys/time.h>
#include "sum.h"
#include "utils.h"

#define DEBUG true
#define LOG_DEBUG(msg, ...) do { \
  if (DEBUG) { \
    printf("[DEBUG]: "); \
    printf(msg, ##__VA_ARGS__ );\
    printf("\n"); \
    } \
  } while (0)

void *ThreadSum(void *args) {
  struct SumArgs *sum_args = (struct SumArgs *)args;
  return (void *)(size_t)Sum(sum_args);
}

void exit_invalid_syntaxis(const char* arg) {
  printf("Usage: %s --threads_num \"num\" --seed \"num\" --array_size \"num\"\n", arg);
  exit(1);
}

int main(int argc, char **argv) {
  uint32_t threads_num = 0;
  uint32_t array_size = 0;
  uint32_t seed = 0;
  pthread_t threads[threads_num];
  while (true) {
    int current_optind = optind ? optind : 1;

    static struct option options[] = {{"threads_num", required_argument, 0, 0},
                                      {"seed", required_argument, 0, 0},
                                      {"array_size", required_argument, 0, 0},
                                      {0, 0, 0, 0}};

    int option_index = 0;
    int c = getopt_long(argc, argv, "f", options, &option_index);

    if (c == -1) break;

    switch (c) { 
      case 0:
        switch (option_index) {
          case 0:
            threads_num = atoi(optarg);
            break;
          case 1:
            seed = atoi(optarg);
            break;
          case 2:
            array_size = atoi(optarg);
            break;
          defalut:
            printf("Index %d is out of options\n", option_index);
        }
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
  if (threads_num <= 0 || seed <= 0 || array_size <= 0 )
    exit_invalid_syntaxis(argv[0]);

  LOG_DEBUG("threads_num = %d; seed = %d; array_size = %d", threads_num, seed, array_size);
  /*
   * TODO:
   * your code here
   * Generate array here
   */
  int *array = malloc(sizeof(int) * array_size);
  GenerateArray(array, array_size, seed);
  LOG_DEBUG("Array generated");

  struct SumArgs *args;
  args = (struct SumArgs*) malloc(sizeof(struct SumArgs) * threads_num);
  int array_process_step = threads_num < array_size ? (array_size / threads_num) : 1;

  struct timeval start_time;
  gettimeofday(&start_time, NULL);
  int arr_size = array_size;
  int t_num = threads_num;

  for (int  i = 0; i < t_num; i++) {
    args[i].array = array;
    int begin = array_process_step * i;
    args[i].begin = begin > arr_size ? arr_size : begin;
    int end = begin + array_process_step;
    args[i].end = end > arr_size ? arr_size : end;
  }
  for (int  i = 0; i < t_num; i++) {
    struct SumArgs *arg;
    arg = (struct SumArgs*) malloc(sizeof(struct SumArgs));
    *arg = args[i];
    if (pthread_create(&threads[i], NULL, ThreadSum, (void *)arg)) {
      printf("Error: pthread_create failed!\n");
      return 1;
    }
  }

  int total_sum = 0;
  for (uint32_t i = 0; i < t_num; i++) {
    int sum = 0;
    pthread_join(threads[i], (void **)&sum);
    total_sum += sum;
  }

  struct timeval finish_time;
  gettimeofday(&finish_time, NULL);
  double elapsed_time = (finish_time.tv_sec - start_time.tv_sec) * 1000.0;
  elapsed_time += (finish_time.tv_usec - start_time.tv_usec) / 1000.0;

  printf("Total: %d\n", total_sum);
  printf("Elapsed time: %fms\n", elapsed_time);
  free(array);
  return 0;
}
