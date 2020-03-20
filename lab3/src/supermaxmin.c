#include <stdio.h>
#include <unistd.h>
int main(int argc, int *argv[])
{
  printf("Program %s is running...\n\n",
         argv[0]);  
  execv("parallel_min_max",argv);
  return 0;
}