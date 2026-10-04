#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/*
    AoS = Array of Structures

    Нэг particle-ийн бүх мэдээлэл хамт байна:
    x, y, z, mass
*/

struct ParticleAoS {
    double x;
    double y;
    double z;
    double mass;
};


/*
    SoA = Structure of Arrays

    x, y, z, mass тус бүр өөрийн array-тай.
*/

struct ParticlesSoA {
    double *x;
    double *y;
    double *z;
    double *mass;
};


/* =========================================================
   RANDOM INDEX
   Fisher-Yates algorithm
   ========================================================= */

int *make_random_indices(int n)
{
    int *idx = malloc(n * sizeof(int));

    if (idx == NULL) {
        printf("Memory allocation error!\n");
        exit(1);
    }

    /* 0, 1, 2, ..., n-1 */
    for (int i = 0; i < n; i++) {
        idx[i] = i;
    }

    /* Fisher-Yates shuffle */
    for (int i = n - 1; i > 0; i--) {
        int j = rand() % (i + 1);

        int temp = idx[i];
        idx[i] = idx[j];
        idx[j] = temp;
    }

    return idx;
}


/* =========================================================
   AoS - 1
   Sequential + бүх талбар
   ========================================================= */

double sum_aos_seq_all(struct ParticleAoS *p, int n)
{
    double sum = 0.0;

    for (int i = 0; i < n; i++) {
        sum += p[i].x;
        sum += p[i].y;
        sum += p[i].z;
        sum += p[i].mass;
    }

    return sum;
}


/* =========================================================
   AoS - 2
   Sequential + зөвхөн mass
   ========================================================= */

double sum_aos_seq_one(struct ParticleAoS *p, int n)
{
    double sum = 0.0;

    for (int i = 0; i < n; i++) {
        sum += p[i].mass;
    }

    return sum;
}


/* =========================================================
   AoS - 3
   Random + бүх талбар
   ========================================================= */

double sum_aos_rand_all(struct ParticleAoS *p, const int *idx, int n)
{
    double sum = 0.0;

    for (int i = 0; i < n; i++) {

        int j = idx[i];

        sum += p[j].x;
        sum += p[j].y;
        sum += p[j].z;
        sum += p[j].mass;
    }

    return sum;
}


/* =========================================================
   AoS - 4
   Random + зөвхөн mass
   ========================================================= */

double sum_aos_rand_one(struct ParticleAoS *p, const int *idx, int n)
{
    double sum = 0.0;

    for (int i = 0; i < n; i++) {

        int j = idx[i];

        sum += p[j].mass;
    }

    return sum;
}


/* =========================================================
   SoA - 5
   Sequential + бүх талбар
   ========================================================= */

double sum_soa_seq_all(struct ParticlesSoA *p, int n)
{
    double sum = 0.0;

    for (int i = 0; i < n; i++) {
        sum += p->x[i];
        sum += p->y[i];
        sum += p->z[i];
        sum += p->mass[i];
    }

    return sum;
}


/* =========================================================
   SoA - 6
   Sequential + зөвхөн mass
   ========================================================= */

double sum_soa_seq_one(struct ParticlesSoA *p, int n)
{
    double sum = 0.0;

    for (int i = 0; i < n; i++) {
        sum += p->mass[i];
    }

    return sum;
}


/* =========================================================
   SoA - 7
   Random + бүх талбар
   ========================================================= */

double sum_soa_rand_all(struct ParticlesSoA *p, const int *idx, int n)
{
    double sum = 0.0;

    for (int i = 0; i < n; i++) {

        int j = idx[i];

        sum += p->x[j];
        sum += p->y[j];
        sum += p->z[j];
        sum += p->mass[j];
    }

    return sum;
}


/* =========================================================
   SoA - 8
   Random + зөвхөн mass
   ========================================================= */

double sum_soa_rand_one(struct ParticlesSoA *p, const int *idx, int n)
{
    double sum = 0.0;

    for (int i = 0; i < n; i++) {

        int j = idx[i];

        sum += p->mass[j];
    }

    return sum;
}


/* =========================================================
   TIME MEASUREMENT
   ========================================================= */

double get_time()
{
    return (double)clock() / CLOCKS_PER_SEC;
}


/* =========================================================
   MAIN
   ========================================================= */

int main()
{
    /*
        Туршилтын хэмжээ.

        Эхлээд жижиг N-ээр шалгаж болно.
        Дараа нь багшийн өгсөн:
        1,000,000
        10,000,000
        50,000,000
    */

    int sizes[] = {
        1000000,
        10000000,
        50000000
    };

    int number_of_sizes = 3;

    /* Random seed */
    srand(42);


    /* =====================================================
       N бүр дээр туршилт хийнэ
       ===================================================== */

    for (int s = 0; s < number_of_sizes; s++) {

        int n = sizes[s];

        printf("\n========================================\n");
        printf("N = %d\n", n);
        printf("========================================\n");


        /* =================================================
           AoS memory
           ================================================= */

        struct ParticleAoS *aos;

        aos = malloc(n * sizeof(struct ParticleAoS));

        if (aos == NULL) {
            printf("AoS memory allocation error!\n");
            return 1;
        }


        /* =================================================
           SoA memory
           ================================================= */

        struct ParticlesSoA soa;

        soa.x = malloc(n * sizeof(double));
        soa.y = malloc(n * sizeof(double));
        soa.z = malloc(n * sizeof(double));
        soa.mass = malloc(n * sizeof(double));


        if (soa.x == NULL ||
            soa.y == NULL ||
            soa.z == NULL ||
            soa.mass == NULL) {

            printf("SoA memory allocation error!\n");

            free(aos);
            free(soa.x);
            free(soa.y);
            free(soa.z);
            free(soa.mass);

            return 1;
        }


        /* =================================================
           Data initialize
           ================================================= */

        for (int i = 0; i < n; i++) {

            /* AoS */
            aos[i].x = 1.0;
            aos[i].y = 2.0;
            aos[i].z = 3.0;
            aos[i].mass = 1.5;


            /* SoA */
            soa.x[i] = 1.0;
            soa.y[i] = 2.0;
            soa.z[i] = 3.0;
            soa.mass[i] = 1.5;
        }


        /* =================================================
           Random index
           Нэг л удаа үүсгэнэ.
           Бүх 8 function ижил idx ашиглана.
           ================================================= */

        int *idx = make_random_indices(n);


        /* =================================================
           1. AoS sequential all
           ================================================= */

        double start = get_time();

        double result1 = sum_aos_seq_all(aos, n);

        double end = get_time();

        printf("\n1. AoS sequential all\n");
        printf("   Sum  = %f\n", result1);
        printf("   Time = %f seconds\n", end - start);


        /* =================================================
           2. AoS sequential mass
           ================================================= */

        start = get_time();

        double result2 = sum_aos_seq_one(aos, n);

        end = get_time();

        printf("\n2. AoS sequential mass\n");
        printf("   Sum  = %f\n", result2);
        printf("   Time = %f seconds\n", end - start);


        /* =================================================
           3. AoS random all
           ================================================= */

        start = get_time();

        double result3 = sum_aos_rand_all(aos, idx, n);

        end = get_time();

        printf("\n3. AoS random all\n");
        printf("   Sum  = %f\n", result3);
        printf("   Time = %f seconds\n", end - start);


        /* =================================================
           4. AoS random mass
           ================================================= */

        start = get_time();

        double result4 = sum_aos_rand_one(aos, idx, n);

        end = get_time();

        printf("\n4. AoS random mass\n");
        printf("   Sum  = %f\n", result4);
        printf("   Time = %f seconds\n", end - start);


        /* =================================================
           5. SoA sequential all
           ================================================= */

        start = get_time();

        double result5 = sum_soa_seq_all(&soa, n);

        end = get_time();

        printf("\n5. SoA sequential all\n");
        printf("   Sum  = %f\n", result5);
        printf("   Time = %f seconds\n", end - start);


        /* =================================================
           6. SoA sequential mass
           ================================================= */

        start = get_time();

        double result6 = sum_soa_seq_one(&soa, n);

        end = get_time();

        printf("\n6. SoA sequential mass\n");
        printf("   Sum  = %f\n", result6);
        printf("   Time = %f seconds\n", end - start);


        /* =================================================
           7. SoA random all
           ================================================= */

        start = get_time();

        double result7 = sum_soa_rand_all(&soa, idx, n);

        end = get_time();

        printf("\n7. SoA random all\n");
        printf("   Sum  = %f\n", result7);
        printf("   Time = %f seconds\n", end - start);


        /* =================================================
           8. SoA random mass
           ================================================= */

        start = get_time();

        double result8 = sum_soa_rand_one(&soa, idx, n);

        end = get_time();

        printf("\n8. SoA random mass\n");
        printf("   Sum  = %f\n", result8);
        printf("   Time = %f seconds\n", end - start);


        /* =================================================
           Memory free
           ================================================= */

        free(aos);

        free(soa.x);
        free(soa.y);
        free(soa.z);
        free(soa.mass);

        free(idx);

        printf("\nMemory freed.\n");
    }


    return 0;
}
