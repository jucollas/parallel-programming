#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

int main(int argc, char** argv) {
    int N = (argc > 1) ? atoi(argv[1]) : 16; 
    int nthreads = (argc > 2) ? atoi(argv[2]) : 2;
    omp_set_dynamic(0);
    omp_set_num_threads(nthreads);

    #pragma omp parallel for  
    for (int i = 0; i < N; ++i) {
        printf("thread number: %d, iteracion = %d \n", i, omp_get_thread_num());
    }
    printf("\nGoodBye -PUJPP- Exiting Program\n");
    return 0;
}

