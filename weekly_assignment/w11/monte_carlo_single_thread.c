#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define N_DATA 3300000

int main() {
  long long count;
  int i;
  double x, y;

  // srand(time(NULL));
  srand(0);

  count = 0;
  for (i = 0; i < N_DATA; i++) {
    x = (double)rand() / RAND_MAX;
    y = (double)rand() / RAND_MAX;
    if (x * x + y * y <= 1) {
      count++;
    }
  }

  printf("PI = %.2lf\n", (double)count / N_DATA * 4);
  return 0;
}