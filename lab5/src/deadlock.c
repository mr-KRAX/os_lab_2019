
#include <pthread.h>
#include <stdio.h>

pthread_mutex_t a_lock;
pthread_mutex_t b_lock;
static int a;
static int b;

void *taskA(void *args) {
  printf("[taskA]: start\n");
  pthread_mutex_lock(&a_lock);
  printf("[taskA]: lock a\n");
  sleep(1);
  a++;
  pthread_mutex_lock(&b_lock);
  printf("[taskA]: lock b\n");
  b += a;
  pthread_mutex_unlock(&b_lock);
  printf("[taskA]: unlock b\n");
  pthread_mutex_unlock(&a_lock);
  printf("[taskA]: unlock a\n");
  printf("[taskA]: end\n");
}

void *taskB(void *args) {
  printf("[taskB]: start\n");
  pthread_mutex_lock(&b_lock);
  printf("[taskB]: lock b\n");
  sleep(1);
  b++;
  pthread_mutex_lock(&a_lock);
  printf("[taskB]: lock a\n");
  a += b;
  pthread_mutex_unlock(&b_lock);
  printf("[taskB]: unlock b\n");
  pthread_mutex_unlock(&a_lock);
  printf("[taskB]: unlock a\n");
  printf("[taskB]: end\n");
}

int main() {
  pthread_t threads[2];
  pthread_create(&threads[0], NULL, taskA, NULL);
  pthread_create(&threads[1], NULL, taskB, NULL);
  pthread_join(threads[0], NULL);
  pthread_join(threads[1], NULL);
  printf("[main]: a - %i\n[main]: b - %i\n", a, b);
  return 0;
}