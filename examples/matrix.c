#include <stdlib.h>
#include <sys/mman.h>

#define M 4
#define N 5
#define L 3
#define MEMORY_SIZE sizeof(struct matrix) * 3 + sizeof(struct vector) * (M + N + M) + sizeof(double) * (M * N + N * L + M * L)
#define STD_ALLOC

struct vector {
  double *line;
};

struct matrix {
  struct vector *vector;
};

void print_matrix(struct matrix *m, int rows, int cols, const char *name);

int main() {
  struct matrix x, y, z;

#ifdef STD_ALLOC
  void *memory_block = calloc(1, MEMORY_SIZE);
#else
#ifdef _WIN32
  void *memory_block = memory_block = VirtualAlloc(NULL, MEMORY_SIZE, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
#else
  void *memory_block = mmap(NULL, MEMORY_SIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
#endif
#endif

  struct matrix *ptr = (struct matrix *)memory_block;
  struct vector *vector_ptr = (struct vector *)((struct matrix *)ptr + 3);
  double *data_ptr = (double *)((struct vector *)vector_ptr + M + N + M);

  x = *(ptr + 0);
  y = *(ptr + 1);
  z = *(ptr + 2);
  x.vector = vector_ptr + 0;
  y.vector = vector_ptr + M;
  z.vector = vector_ptr + M + N;

  double *x_data_start = data_ptr;
  double *y_data_start = data_ptr + M * N;
  double *z_data_start = y_data_start + N * L;

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

#ifdef STD_ALLOC
  free(memory_block);
#else
#ifdef _WIN32
  VirtualFree(memory_block, 0, MEM_RELEASE);
#else
  munmap(memory_block, MEMORY_SIZE);
#endif
#endif
  return 0;
}
