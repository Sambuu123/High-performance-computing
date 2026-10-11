/*
 * Лабораторийн ажил №3 — OpenMP: дундын өгөгдөл, race condition,
 * critical vs reduction, first touch.
 *
 * Хөрвүүлэх:   gcc -fopenmp -O3 -o lab3 lab3.c timer.c
 * Ажиллуулах:  OMP_NUM_THREADS=4 ./lab3            (бүх хувилбар)
 *              OMP_NUM_THREADS=4 ./lab3 B2 B4      (сонгосон хувилбарууд)
 *
 * Хувилбар бүрийн тооцооллын хэсгийг NREP удаа хэмжиж, дунджийг хэвлэнэ.
 * Инициализаци, malloc хугацаанд орохгүй.
 * "RESULT,..." мөрүүдийг run_benchmarks.sh скрипт CSV болгон цуглуулна.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>
#include "timer.h"

#ifndef N
#define N 80000000L
#endif

#ifndef NREP
#define NREP 3
#endif

#define EXPECTED_SUM (3.0 * (double)N)

/* Глобал заагч: хөрвүүлэгч давтан хэмжилтийн циклийг "илүүц" гэж үзэж
 * устгахгүй байх үүднээс static биш. */
double *a, *b, *c;

static int g_argc;
static char **g_argv;

/* Аргумент өгөөгүй бол бүх хувилбарыг, өгсөн бол зөвхөн нэрлэсэнийг ажиллуулна. */
static int want(const char *name)
{
    if (g_argc <= 1)
        return 1;
    for (int i = 1; i < g_argc; i++)
        if (strcmp(g_argv[i], name) == 0)
            return 1;
    return 0;
}

static double *alloc_array(const char *name)
{
    double *p = malloc((size_t)N * sizeof(double));
    if (p == NULL) {
        fprintf(stderr, "malloc амжилтгүй: %s (%.2f GB). N-ийг багасгана уу.\n",
                name, (double)N * sizeof(double) / 1e9);
        exit(EXIT_FAILURE);
    }
    return p;
}

static void alloc_all(void)
{
    a = alloc_array("a");
    b = alloc_array("b");
    c = alloc_array("c");
}

static void free_all(void)
{
    free(a);
    free(b);
    free(c);
    a = b = c = NULL;
}

/* Цуваа инициализаци: бүх хуудсыг мастер урсгал анх хүрнэ (first touch). */
static void init_serial(void)
{
    for (long i = 0; i < N; i++) {
        a[i] = 1.0;
        b[i] = 2.0;
        c[i] = 0.0;
    }
}

/* Параллель инициализаци: тооцоололтой ижил schedule(static) хуваарилалтаар
 * урсгал бүр өөрийн хэсгийн хуудсыг анх хүрнэ (first touch). */
static void init_parallel(void)
{
#pragma omp parallel for schedule(static)
    for (long i = 0; i < N; i++) {
        a[i] = 1.0;
        b[i] = 2.0;
        c[i] = 0.0;
    }
}

/* ---------------- Хэсэг А: Вектор нэмэх ---------------- */

/* A1 — Tseq */
static double vec_add_seq(void)
{
    for (long i = 0; i < N; i++)
        c[i] = a[i] + b[i];
    return 0.0;
}

/* A2 — Tprag (инициализаци цуваа хийгдсэн) */
static double vec_add_prag(void)
{
#pragma omp parallel for
    for (long i = 0; i < N; i++)
        c[i] = a[i] + b[i];
    return 0.0;
}

/* A3 — Tft (инициализаци параллель хийгдсэн) */
static double vec_add_ft(void)
{
#pragma omp parallel for schedule(static)
    for (long i = 0; i < N; i++)
        c[i] = a[i] + b[i];
    return 0.0;
}

/* ---------------- Хэсэг Б: Нийлбэр олох ---------------- */

/* B1 — Tseq */
static double sum_seq(void)
{
    double sum = 0.0;
    for (long i = 0; i < N; i++)
        sum += c[i];
    return sum;
}

/* B2 — Trace: sum нь бүх урсгалд дундын, хамгаалалтгүй → race condition */
static double sum_race(void)
{
    double sum = 0.0;
#pragma omp parallel for
    for (long i = 0; i < N; i++)
        sum += c[i];
    return sum;
}

/* B3 — Tcrit: нэмэх бүрт түгжээ авна → бүх нэмэлт цуваа болно */
static double sum_crit(void)
{
    double sum = 0.0;
#pragma omp parallel for
    for (long i = 0; i < N; i++) {
#pragma omp critical
        sum += c[i];
    }
    return sum;
}

/* B4 — Tred: урсгал бүр хувийн sum-д нэмж, төгсгөлд нь нэг удаа нэгтгэнэ */
static double sum_red(void)
{
    double sum = 0.0;
#pragma omp parallel for reduction(+:sum)
    for (long i = 0; i < N; i++)
        sum += c[i];
    return sum;
}

/* c[i] == 3.0 эсэхийг шалгана (хугацаанд орохгүй). */
static long count_bad_c(void)
{
    long bad = 0;
    for (long i = 0; i < N; i++)
        if (c[i] != 3.0)
            bad++;
    return bad;
}

/* Хувилбарыг NREP удаа хэмжээд дундаж хугацааг буцаана.
 * is_sum = 1 бол буцаасан нийлбэрийг, 0 бол c массивын зөв эсэхийг хэвлэнэ. */
static double run_variant(const char *name, const char *desc,
                          double (*kernel)(void), int is_sum, int nthreads)
{
    double total = 0.0;
    printf("%s — %s\n", name, desc);
    for (int r = 1; r <= NREP; r++) {
        struct timespec t0;
        cpu_timer_start(&t0);
        double value = kernel();
        double t = cpu_timer_stop(t0);
        total += t;

        if (is_sum) {
            printf("  ажиллуулалт %d: %10.6f сек   sum = %.4f%s\n", r, t, value,
                   value == EXPECTED_SUM ? "" : "   (БУРУУ)");
        } else {
            value = (double)count_bad_c();
            printf("  ажиллуулалт %d: %10.6f сек   буруу c[i] = %.0f\n", r, t, value);
        }
        printf("RESULT,%s,%d,%d,%.9f,%.4f\n", name, nthreads, r, t, value);
    }
    double avg = total / NREP;
    printf("  дундаж:        %10.6f сек\n\n", avg);
    return avg;
}

int main(int argc, char **argv)
{
    g_argc = argc;
    g_argv = argv;

    int nthreads = omp_get_max_threads();
    printf("N = %ld, OMP_NUM_THREADS = %d, давталт = %d\n", N, nthreads, NREP);
    printf("Санах ой: %.2f GB (3 массив)\n\n", 3.0 * (double)N * sizeof(double) / 1e9);

    /* --- A1, A2: цуваа инициализацитай массивууд --- */
    if (want("A1") || want("A2")) {
        alloc_all();
        init_serial();
        if (want("A1"))
            run_variant("A1", "Tseq: c[i] = a[i] + b[i] (дараалсан)",
                        vec_add_seq, 0, nthreads);
        if (want("A2"))
            run_variant("A2", "Tprag: parallel for, инициализаци цуваа",
                        vec_add_prag, 0, nthreads);
        free_all();
    }

    /* --- A3 ба Б хэсэг: параллель (first touch) инициализацитай массивууд --- */
    int need_b = want("B1") || want("B2") || want("B3") || want("B4");
    if (want("A3") || need_b) {
        alloc_all();

        struct timespec t0;
        cpu_timer_start(&t0);
        init_parallel();
        double tinit = cpu_timer_stop(t0);
        printf("Параллель инициализаци (first touch): %.6f сек\n\n", tinit);
        printf("RESULT,INIT_PAR,%d,1,%.9f,0\n", nthreads, tinit);

        if (want("A3"))
            run_variant("A3", "Tft: parallel for schedule(static), инициализаци параллель",
                        vec_add_ft, 0, nthreads);
        else
            vec_add_ft();

        if (want("B1"))
            run_variant("B1", "Tseq: sum += c[i] (дараалсан)", sum_seq, 1, nthreads);
        if (want("B2"))
            run_variant("B2", "Trace: хамгаалалтгүй (race condition)", sum_race, 1, nthreads);
        if (want("B3"))
            run_variant("B3", "Tcrit: omp critical", sum_crit, 1, nthreads);
        if (want("B4"))
            run_variant("B4", "Tred: reduction(+:sum)", sum_red, 1, nthreads);

        free_all();
    }

    printf("Зөв хариу: sum = %.1f\n", EXPECTED_SUM);
    return 0;
}
