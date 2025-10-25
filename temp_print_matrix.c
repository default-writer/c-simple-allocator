#include <stdio.h>

struct vector {
  double *line;
};

struct matrix {
  struct vector *vector;
};

void print_matrix(struct matrix* m, int rows, int cols, const char* name) {
    printf("\nMatrix %s (%dx%d):\n", name, rows, cols);
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            printf("%8.2f ", m->vector[i].line[j]);
        }
        printf("\n");
    }
}

