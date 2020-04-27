#ifndef SUM_H_
#define SUM_H_


struct SumArgs {
  int *array;
  int begin;
  int end;
};

int Sum(const struct SumArgs *args);

#endif
