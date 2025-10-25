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

void print_matrix(struct matrix* m, int rows, int cols, const char* name);

int main() {
  struct matrix x, y, z;

  void* memory_block = calloc(1,
        sizeof(struct matrix) * 3 +
        sizeof(struct vector) * (M + N + M) +
        sizeof(double) * (M * N + N * L + M * L));

  struct matrix *ptr = (struct matrix *)memory_block;
  struct vector *vector_ptr = (struct vector *)((struct matrix*)ptr + 3);
  double* data_ptr = (double *)((struct vector*)vector_ptr + M + N + M);

  x = *(ptr + 0);
  y = *(ptr + 1);
  z = *(ptr + 2);

  x.vector = vector_ptr + 0;
  y.vector = vector_ptr + M;
  z.vector = vector_ptr + M + N;

  double* x_data_start = data_ptr;
  double* y_data_start = data_ptr + M * N;
  double* z_data_start = y_data_start + N * L;

  for (int i = 0; i < M; i++) {
    x.vector[i].line = x_data_start + i * N;
  }

  for (int i = 0; i < N; i++) {
    y.vector[i].line = y_data_start + i * L;
  }

  for (int i = 0; i < M; i++) {
    z.vector[i].line = z_data_start + i * L;
  }

  for (int i = 0; i < M; i++) {
    for (int j = 0; j < N; j++) {
      x.vector[i].line[j] = i * N + j + 1;
    }
  }

  for (int i = 0; i < N; i++) {
    for (int j = 0; j < L; j++) {
      y.vector[i].line[j] = i * L + j + 1;
    }
  }

  for (int i = 0; i < M; i++) {
    for (int j = 0; j < L; j++) {
      z.vector[i].line[j] = 0;
      for (int k = 0; k < N; k++) {
        z.vector[i].line[j] += x.vector[i].line[k] * y.vector[k].line[j];
      }
    }
  }

  print_matrix(&x, M, N, "X");
  print_matrix(&y, N, L, "Y");
  print_matrix(&z, M, L, "Z=X*Y");

  free(memory_block);
  return 0;
}
