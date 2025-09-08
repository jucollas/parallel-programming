#include <stdio.h>
#include <omp.h>

int main(int argc, char** argv) {
    int COUNT = (argc > 1) ? atoi(argv[1]) : 16; 
    int nthreads = (argc > 2) ? atoi(argv[2]) : 2;
    omp_set_dynamic(0);
    omp_set_num_threads(nthreads);
    int sum = 0;
    #pragma omp parallel for
    for (int i = 0; i < COUNT; i++) {
        sum = sum + i;
        printf("Thread number: %d Iteration: %d Local Sum: %d\n",
               omp_get_thread_num(), i, sum);
    }

    printf("\nAll Threads Done – Final Global Sum: %d\n", sum);
}
