#include <stdio.h>
#include <stdlib.h>

#define M 4
#define N 5
#define L 3

struct vector {
  double *line;
};

struct matrix {
  struct vector *vector;
};

int main() {
  struct matrix x;
  struct matrix y;

  void* memory_block = calloc(1,
        sizeof(struct matrix) * 2 +
        sizeof(struct vector) * (M + N) +
        sizeof(double) * M * N +
        sizeof(double) * N * L);

  struct matrix *ptr = (struct matrix *)memory_block;
  struct vector *vector = (struct vector *)((struct matrix*)ptr + 2);
  double* data = (double *)((struct vector*)ptr + M + N);

  x = *(ptr + 0); // MxN
  y = *(ptr + 1); // NxL
  x.vector = vector + 0; // M
  y.vector = vector + M; // N

  for (int i = 0; i < M; i++) {
    x.vector[i].line = data + i * N;
  }
  for (int i = 0; i < N; i++) {
    y.vector[i].line = data + M*N + i * L;
  }

  free(memory_block);
  return 0;
}
