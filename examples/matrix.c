#include <stdio.h>
#include <stdlib.h>

#define M 4
#define N 5
#define L 3

struct vector {
  int *line;
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
        sizeof(int) * M * N +
        sizeof(int) * N * L);

  struct matrix *ptr = (struct matrix *)memory_block;
  struct vector *vector = (struct vector *)(ptr + 2);
  int* data = (int *)((struct vector*)vector + M + N);

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

  for (int i = 0; i < M; i++) {
    for (int j = 0; j < N; j++) {
      x.vector[i].line[j] = i + j;
      printf("%d ", x.vector[i].line[j]);
    }
    printf("\n");
  }

  for (int i = 0; i < N; i++) {
    for (int j = 0; j < L; j++) {
      y.vector[i].line[j] = M - i + N - j;
      printf("%d ", y.vector[i].line[j]);
    }
    printf("\n");
  }


  free(memory_block);
  return 0;
}
