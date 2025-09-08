#include <stdio.h>
#include <omp.h>
#include <stdlib.h>
#include <time.h>

int main(int argc, char* argv[]) {
    int nthreads = (argc > 1) ? atoi(argv[1]) : 32;

    omp_set_dynamic(0);
    omp_set_num_threads(nthreads);

    long long num_steps = 1000000000;   
    double step, pi, sum = 0.0;
    int i;

    step = 1.0 / (double) num_steps;

    double start = omp_get_wtime();

    #pragma omp parallel for private(i) reduction(+:sum)
    for (i = 0; i < num_steps; i++) {
        double x = (i + 0.5) * step;
        sum = sum + 4.0 / (1.0 + x * x);
    }

    pi = sum * step;

    double end = omp_get_wtime();

    printf("The value of PI is %15.12f\n", pi);
    printf("Elapsed time (seconds) with %d threads: %.3f\n",
           nthreads, end - start);

    return 0;
}
