#include <stdio.h>

long my_asm_function(long a, long b);

int main() {
    long a = 2;
    long b = 3;
    printf("%ld + %ld = %ld\n", a, b, my_asm_function(a, b));
    return 0;
}
