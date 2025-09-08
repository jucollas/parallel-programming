#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

int main(int argc, char** argv) {
    int COUNT   = (argc > 1) ? atoi(argv[1]) : 16; 
    int nthreads = (argc > 2) ? atoi(argv[2]) : 2;
    int N_RUNS  = (argc > 3) ? atoi(argv[3]) : 10;

    omp_set_dynamic(0);
    omp_set_num_threads(nthreads);

    int correct = 0;
    int expected = (COUNT * (COUNT - 1)) / 2;

    for (int r = 0; r < N_RUNS; r++) {
        int sum = 0;

        #pragma omp parallel for
        for (int i = 0; i < COUNT; i++) {
            sum = sum + i;
        }

        if (sum == expected) {
            correct++;
        }

        printf("Run %d -> Sum: %d [%s]\n", 
               r, sum, (sum == expected) ? "OK" : "WRONG");
    }

    printf("\nDe %d ejecuciones, %d fueron correctas y %d incorrectas.\n",
           N_RUNS, correct, N_RUNS - correct);

    return 0;
}
