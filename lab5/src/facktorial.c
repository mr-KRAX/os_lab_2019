#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include <getopt.h>
#include <pthread.h>

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

struct FArg {
  unsigned long long *res;
  int begin;
  int end;
};

#define DEBUG false
#define LOG_DEBUG(msg, ...)       \
  do {                            \
    if (DEBUG) {                  \
      printf("[DEBUG]: ");        \
      printf(msg, ##__VA_ARGS__); \
      printf("\n");               \
    }                             \
  } while (0)

void exit_invalid_syntaxis(const char *arg) {
  printf("Usage: %s -k \"num\" --pnum \"num\" --mod \"num\"\n", arg);
  exit(1);
}

void *multTask(void *args) {
  struct FArg *fArgs = (struct FArg *)args;
  int beg = fArgs->begin, end = fArgs->end;
  unsigned long long tmp = 1;
  for (int i = beg + 1; i <= end; i++)
    tmp *= i;
  pthread_mutex_lock(&mutex);
  *(fArgs->res) *= tmp;
  LOG_DEBUG("beg = %d; end = %d; tmp = %d;", beg, end, tmp);
  LOG_DEBUG("res = %d", *(fArgs->res));
  pthread_mutex_unlock(&mutex);
}

int main(int argc, char **argv) {
  int k = -1;
  int pnum = -1;
  int mod = -1;
  while (true) {
    int current_optind = optind ? optind : 1;
    static struct option options[] = {{"k", required_argument, 0, 0},
                                      {"pnum", required_argument, 0, 0},
                                      {"mod", required_argument, 0, 0},
                                      {0, 0, 0, 0}};

    int option_index = 0;
    int c = getopt_long(argc, argv, "?", options, &option_index);

    if (c == -1)
      break;

    switch (c) {
    case 0:
      switch (option_index) {
      case 0:
        k = atoi(optarg);
        break;
      case 1:
        pnum = atoi(optarg);
        break;
      case 2:
        mod = atoi(optarg);
        break;
      defalut:
        printf("Index %d is out of options\n", option_index);
      }
      break;
    case '?':
      break;
    default:
      printf("getopt returned character code 0%\n", c);
    }
  }

  if (optind < argc) {
    printf("Has at least one no option argument\n");
    return 1;
  }
  if (k <= 0 || mod <= 0 || pnum <= 0)
    exit_invalid_syntaxis(argv[0]);
  unsigned long long ans = 1;

  int step = pnum < k ? (k / pnum) : 1;

  LOG_DEBUG("k = %d; pnum = %d; mod = %d;", k, pnum, mod);
  LOG_DEBUG("step = %d;", step);
  pthread_t *threads = (pthread_t *)malloc(sizeof(pthread_t) * pnum);
  struct FArg *fargs = (struct FArg *)malloc(sizeof(struct FArg) * pnum);
  for (int i = 0; i < pnum; i++) {
    fargs[i].res = &ans;
    int begin = step * i;
    fargs[i].begin = begin > k ? k : begin;
    int end = begin + step;
    fargs[i].end = end > k ? k : end;
  }
  for (int i = 0; i < pnum; i++) {
    if (pthread_create(&threads[i], NULL, multTask, (void *)(fargs+i))) {
      printf("Error: pthread_create failed!\n");
      return 1;
    }
  }

  for (int i = 0; i < pnum; i++) {
    pthread_join(threads[i], NULL);
  }
  printf("%d! mod %d == %d\n", k, mod, ans % mod);
  return 0;
}