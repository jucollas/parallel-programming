#include <stdio.h>
#include <time.h>

int main(int argc, char* argv[]) {
    long long num_steps = 1000000000; 
    double step, x, pi, sum = 0.0;
    int i;

    step = 1.0 / (double) num_steps;

    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);

    for (i = 0; i < num_steps; i++) {
        x = (i + 0.5) * step;
        sum = sum + 4.0 / (1.0 + x * x);
    }

    pi = sum * step;

    clock_gettime(CLOCK_MONOTONIC, &end);

    double elapsed = (end.tv_sec - start.tv_sec) +
                     (end.tv_nsec - start.tv_nsec) / 1e9;

    printf("The value of PI is %15.12f\n", pi);
    printf("Elapsed time (seconds): %.3f\n", elapsed);

    return 0;
}
