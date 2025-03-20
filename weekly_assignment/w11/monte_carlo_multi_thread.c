#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define N_DATA 3300000
#define N_THREAD 4
#define N_DATA_PER_THREAD (N_DATA / N_THREAD)

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
long long total_count = 0;

void *monte_carlo(void *arg) {
  long long count;
  int i;
  double x, y;

  count = 0;
  unsigned int seed = time(NULL) + pthread_self();
  for (i = 0; i < N_DATA_PER_THREAD; i++) {
    x = (double)rand_r(&seed) / RAND_MAX;
    y = (double)rand_r(&seed) / RAND_MAX;
    if (x * x + y * y <= 1) {
      count++;
    }
  }

  pthread_mutex_lock(&mutex);
  total_count += count;
  pthread_mutex_unlock(&mutex);

  return NULL;
}

int main() {
  pthread_t threads[N_THREAD];
  int i;

  srand(time(NULL));

  for (i = 0; i < N_THREAD; i++) {
    pthread_create(&threads[i], NULL, monte_carlo, NULL);
  }

  for (i = 0; i < N_THREAD; i++) {
    pthread_join(threads[i], NULL);
  }

  printf("PI = %.2lf\n", (double)total_count / N_DATA * 4);
  return 0;
}
