

#include <csignal>
#include <iostream>
#include <unistd.h>

int pids[3];
using namespace std;
void hdl(int sig) {
  cout << "get signal: " << sig << endl;
  switch (sig) {
  case SIGTERM:
    exit(0);
    for (int i = 0; i < 3; i++)
      kill(pids[i], SIGKILL);
    break;
  default:
    break;
  }
}

int main(void) {
  signal(SIGTERM, hdl);
  //Установка будильника на 10 секунд
  cout << "Zombie parant started" << endl;
  for (int i = 0; i < 3; i++) {
    pid_t child_pid = fork();
    if (child_pid >= 0) {
      // successful fork
      if (child_pid == 0) {
        cout << " i'm a zombeee" << endl;
        return 0;
      } else {
        pids[i] = child_pid;
      }

    } else {
      printf("Fork failed!\n");
      return 1;
    }
  }
  while (1)
    ;
  return 0;
}
