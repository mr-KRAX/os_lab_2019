#include <ctype.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <getopt.h>

#include "find_min_max.h"
#include "utils.h"

void exit_invalid_syntaxis(const char* arg) {
  printf("Usage: %s --seed \"num\" --array_size \"num\" --pnum \"num\" --kill 10 \"num\" \n", arg);
  exit(1);
}

int main(int argc, char **argv) {
  int seed = -1;
  int array_size = -1;
  int pnum = -1;
  int kill_timeout = -1;
  bool with_files = false;
  while (true) {
    int current_optind = optind ? optind : 1;

    static struct option options[] = {{"seed", required_argument, 0, 0},
                                      {"array_size", required_argument, 0, 0},
                                      {"pnum", required_argument, 0, 0},
                                      {"by_files", no_argument, 0, 'f'},
                                      {"kill", required_argument, 0, 0},
                                      {0, 0, 0, 0}};

    int option_index = 0;
    int c = getopt_long(argc, argv, "f", options, &option_index);

    if (c == -1) break;

    switch (c) { 
      case 0:
        switch (option_index) {
          case 0:
            seed = atoi(optarg);
            break;
          case 1:
            array_size = atoi(optarg);
            break;
          case 2:
            pnum = atoi(optarg);
            break;
          case 3:
            with_files = true;
            break;
          case 4:
            kill_timeout = atoi(optarg);
            if (kill_timeout <= 0)
              exit_invalid_syntaxis(argv[0]);
            break;
          defalut:
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
  printf("log: 2 kill_timeout %d\n", kill_timeout);
  if (seed <= 0 || array_size <= 0 || pnum <= 0 )
    exit_invalid_syntaxis(argv[0]);

  int *array = malloc(sizeof(int) * array_size);
  GenerateArray(array, array_size, seed);
  
  int active_child_processes = 0;
  int array_process_step = pnum < array_size ? (array_size / pnum) : 1;
  // for (int i = 0; i <array_size; i++){
  //   printf("| %i ", array[i]);
  // }
  // printf("\n");
  int **pipes = malloc(sizeof(int*) * pnum);
  FILE *sync_file;
  int pipefd[2];
  if (with_files) {
    sync_file = fopen("test",  "wb+");
  }
  else {
    for (int i = 0; i < pnum; i++){
      pipes[i] = malloc(sizeof(int) * 2);
      int* pipe_fd = pipes[i];
      if (pipe(pipe_fd) == -1) {
          perror("pipe");
          exit(EXIT_FAILURE);
      }
    }
  }
  int *pids;
  pids = (int*)malloc(sizeof(int)+pnum);
  struct timeval start_time;
  gettimeofday(&start_time, NULL);
  for (int i = 0; i < pnum; i++) {
    int* pipe_fd = pipes[i];
    pid_t child_pid = fork();
    if (child_pid >= 0) {
      // successful fork
      active_child_processes += 1;
      if (child_pid == 0) {
        // child process
        // parallel somehow
        unsigned int begin = array_process_step * (active_child_processes - 1);
        unsigned int end = begin + array_process_step;
        begin = begin > array_size ? array_size : begin;
        end = end > array_size ? array_size : end;
        if (active_child_processes == pnum) end = array_size;
        struct MinMax min_max = GetMinMax(array, begin, end);
        if (with_files) {
          // use files here
          fwrite(&min_max, sizeof(struct MinMax), 1, sync_file);
        } else {
          // use pipe here
          close(pipe_fd[0]);
          write(pipe_fd[1], &min_max, sizeof(struct MinMax));
          close(pipe_fd[1]);
        }
        if (kill_timeout > 0){
          sleep(2);
          printf("active process %i: before SIGKILL\n", active_child_processes);
          sleep(kill_timeout);
          printf("active process %i: after SIGKILL\n", active_child_processes);
        }
        return 0;
      }
      else 
      {
        pids[i] = child_pid;
      }

    } else {
      printf("Fork failed!\n");
      return 1;
    }
  }
  int status[pnum];
  int res_status;
  struct timeval curr;
  if (kill_timeout > 0) {
    for (int i = 0; i < pnum; i++) {
      while (true) {
        gettimeofday(&curr, NULL);
        double elapsed_time = (curr.tv_sec - start_time.tv_sec);
        if (elapsed_time > kill_timeout) {
          for (int i = 0; i < pnum; i++) {
            waitpid(pids[i], &status[i], WNOHANG);
            if (WEXITSTATUS(status[i]) > 1)
              kill(pids[i], SIGKILL);
          }
          break;
        }
      }
    }
  } 
  else {
    int *status;
    while (active_child_processes > 0) {
      // your code here
      wait(status);
      active_child_processes -= 1;
    }
  }
  if (with_files) {
    fclose(sync_file);
    sync_file = fopen("test",  "rb");
  }

  struct MinMax min_max;
  min_max.min = INT_MAX;
  min_max.max = INT_MIN;

  for (int i = 0; i < pnum; i++) {
    int* pipe_fd = pipes[i];
    int min = INT_MAX;
    int max = INT_MIN;
    struct MinMax tmpmm;

    if (with_files) {
      // read from files
      fread(&tmpmm, sizeof(struct MinMax), 1, sync_file);
    }
    else {
      // read from pipes
      close(pipe_fd[1]);
      read(pipe_fd[0], &tmpmm, sizeof(struct MinMax));
      close(pipe_fd[0]);
    }
    min = tmpmm.min;
    max = tmpmm.max;
    // printf("---------------\nmin_max= %i - %i\n",min, max);

    if (min < min_max.min) min_max.min = min;
    if (max > min_max.max) min_max.max = max;
  }

  struct timeval finish_time;
  gettimeofday(&finish_time, NULL);

  double elapsed_time = (finish_time.tv_sec - start_time.tv_sec) * 1000.0;
  elapsed_time += (finish_time.tv_usec - start_time.tv_usec) / 1000.0;

  free(array);
  free(pids);

  printf("Min: %d\n", min_max.min);
  printf("Max: %d\n", min_max.max);
  printf("Elapsed time: %fms\n", elapsed_time);
  fflush(NULL);
  return 0;
}
