#include "multModulo.h"
#include "pthread.h"
#include <errno.h>
#include <getopt.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

struct Server {
  char ip[255];
  int port;
};

bool ConvertStringToUI64(const char *str, uint64_t *val) {
  char *end = NULL;
  unsigned long long i = strtoull(str, &end, 10);
  if (errno == ERANGE) {
    fprintf(stderr, "Out of uint64_t range: %s\n", str);
    return false;
  }

  if (errno != 0)
    return false;

  *val = i;
  return true;
}

uint64_t WaitForResponse(void *args) {
  int sck = *((int *)args);
  char response[sizeof(uint64_t)];
  if (recv(sck, response, sizeof(response), 0) < 0) {
    fprintf(stderr, "Recieve failed\n");
    exit(1);
  }
  close(sck);
  uint64_t ans = 0;
  memcpy(&ans, response, sizeof(uint64_t));
}

int main(int argc, char **argv) {
  uint64_t k = -1;
  uint64_t mod = -1;
  char servers[255] = {
      '\0'}; // TODO: explain why 255
  FILE *a;

  while (true) {
    int current_optind = optind ? optind : 1;

    static struct option options[] = {{"k", required_argument, 0, 0},
                                      {"mod", required_argument, 0, 0},
                                      {"servers", required_argument, 0, 0},
                                      {0, 0, 0, 0}};

    int option_index = 0;
    int c = getopt_long(argc, argv, "", options, &option_index);

    if (c == -1)
      break;

    switch (c) {
    case 0: {
      switch (option_index) {
      case 0:
        ConvertStringToUI64(optarg, &k);
        if (k <= 0) {
          printf("Invalid k value: must be positive\n");
          return 1;
        }
        break;
      case 1:
        ConvertStringToUI64(optarg, &mod);
        if (mod <= 0) {
          printf("Invalid mod value: must be positive\n");
          return 1;
        }
        break;
      case 2:
        a = fopen(optarg, "r");
        if (a == 0) {
          printf("Failed to open file!\n");
          return 1;
        } else
          fclose(a);
        memcpy(servers, optarg, strlen(optarg));
        break;
      default:
        printf("Index %d is out of options\n", option_index);
      }
    } break;

    case '?':
      printf("Arguments error\n");
      break;
    default:
      fprintf(stderr, "getopt returned character code 0%o?\n", c);
    }
  }

  if (k == -1 || mod == -1 || !strlen(servers)) {
    fprintf(stderr, "Using: %s --k 1000 --mod 5 --servers /path/to/file\n",
            argv[0]);
    return 1;
  }

  unsigned int servers_num = 0;
  int size = 10;
  struct Server *to = malloc(sizeof(struct Server) * size);
  a = fopen(servers, "r");
  char *cadress;
  char tmp[255] = {'\0'};
  while (!feof(a)) {
    if (servers_num == size) {
      size = size + 10;
      to = realloc(to, sizeof(struct Server) * size);
    }
    fscanf(a, "%s\n", tmp);
    char *marker = strtok(tmp, ":");
    memcpy(to[servers_num].ip, tmp, sizeof(tmp));
    to[servers_num].port = atoi(strtok(NULL, ":"));
    servers_num++;
  }
  if (to[servers_num - 1].port == 0)
    servers_num--;
  fclose(a);
  if (servers_num > k) {
    servers_num = k;
  }

  int step = k / servers_num;
  int remainder = k % servers_num;
  int sck[servers_num];
  pthread_t threads[servers_num];
  for (int i = 0; i < servers_num; i++) {
    struct hostent *hostname = gethostbyname(to[i].ip);
    if (hostname == NULL) {
      fprintf(stderr, "gethostbyname failed with %s\n", to[i].ip);
      exit(1);
    }

    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_port = htons(to[i].port);
    server.sin_addr.s_addr = *((unsigned long *)hostname->h_addr_list[0]);

    sck[i] = socket(AF_INET, SOCK_STREAM, 0);
    if (sck < 0) {
      fprintf(stderr, "Socket creation failed!\n");
      exit(1);
    }

    if (connect(sck[i], (struct sockaddr *)&server, sizeof(server)) < 0) {
      fprintf(stderr, "Connection failed\n");
      exit(1);
    }

    // parallel between servers
    uint64_t begin = step * i + (remainder < i ? remainder : i) + 1;
    uint64_t end = step * (i + 1) + (remainder < i + 1 ? remainder : i + 1) + 1;

    char task[sizeof(uint64_t) * 3];
    memcpy(task, &begin, sizeof(uint64_t));
    memcpy(task + sizeof(uint64_t), &end, sizeof(uint64_t));
    memcpy(task + 2 * sizeof(uint64_t), &mod, sizeof(uint64_t));

    if (send(sck[i], task, sizeof(task), 0) < 0) {
      fprintf(stderr, "Send failed\n");
      exit(1);
    }
    if (pthread_create(&threads[i], NULL, (void *)WaitForResponse,
                       (void *)&(sck[i]))) {
      printf("Error: pthread_create failed!\n");
      return 1;
    }
  }

  uint64_t answer = 1;
  uint64_t response = 0;
  for (int i = 0; i < servers_num; i++) {
    char buff[sizeof(uint64_t)];
    pthread_join(threads[i], (void **)&response);
    answer = MultModulo(response, answer, mod);
  }

  free(to);

  printf("Factorial %llu by mod %llu = %llu\n", k, mod, answer);
  return 0;
}