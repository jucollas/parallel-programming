#include <stdio.h>
#include <omp.h>

#define NTHREADS 4
#define COUNT 100

int main() {
    int i;
    int sum = 0;
    omp_set_num_threads(NTHREADS);

    #pragma omp parallel for reduction(+:sum) 
    for (i = 0; i < COUNT; i++) {
        sum = sum + i;
        printf("Thread number: %d Iteration: %d Local Sum: %d\n",
               omp_get_thread_num(), i, sum);
    }

    printf("\nAll Threads Done – Final Global Sum: %d\n", sum);
}
