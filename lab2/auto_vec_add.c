#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* 80 сая элемент: c[i] = a[i] + b[i], FLOPs/s хэмжинэ. */
int main(void) {
    const int n = 80000000;
    double *restrict a = malloc((size_t)n * sizeof(double));
    double *restrict b = malloc((size_t)n * sizeof(double));
    double *restrict c = malloc((size_t)n * sizeof(double));

    if (a == NULL || b == NULL || c == NULL) {
        fprintf(stderr, "malloc амжилтгүй\n");
        free(a);
        free(b);
        free(c);
        return 1;
    }

    for (int i = 0; i < n; i++) {
        a[i] = 1.0;
        b[i] = 2.0;
    }

    /* Компилятор нэмэлтийг c[i] = 3.0 гэж хурааж хаяхгүй. */
    __asm__ volatile("" ::: "memory");

    clock_t start = clock();
    for (int i = 0; i < n; i++) {
        c[i] = a[i] + b[i];
    }
    double time = (double)(clock() - start) / CLOCKS_PER_SEC;
    double flops = 1.0 * n / time;

    printf("n = %d\n", n);
    printf("time = %.6f s\n", time);
    printf("FLOPs/s = %.6e\n", flops);
    printf("GFLOP/s = %.3f\n", flops / 1e9);
    printf("c[0] = %.1f, c[n-1] = %.1f\n", c[0], c[n - 1]);

    free(a);
    free(b);
    free(c);
    return 0;
}
