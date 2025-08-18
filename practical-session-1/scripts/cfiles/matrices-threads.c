#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define SIZE 40

int A[SIZE][SIZE], B[SIZE][SIZE];
int SUM[SIZE][SIZE], DIFF[SIZE][SIZE], RESULT[SIZE][SIZE];

// Thread function to compute sum
void* compute_sum(void* arg) {
    for(int i = 0; i < SIZE; i++) {
        for(int j = 0; j < SIZE; j++) {
            SUM[i][j] = A[i][j] + B[i][j];
        }
    }
    pthread_exit(NULL);
}

// Thread function to compute difference
void* compute_diff(void* arg) {
    for(int i = 0; i < SIZE; i++) {
        for(int j = 0; j < SIZE; j++) {
            DIFF[i][j] = A[i][j] - B[i][j];
        }
    }
    pthread_exit(NULL);
}

// Multiplication done by main thread
void multiply_matrices() {
    for(int i = 0; i < SIZE; i++) {
        for(int j = 0; j < SIZE; j++) {
            RESULT[i][j] = 0;
            for(int k = 0; k < SIZE; k++) {
                RESULT[i][j] += SUM[i][k] * DIFF[k][j];
            }
        }
    }
}

int main() {
    pthread_t t1, t2;

    // Initialize matrices with 0s and 1s randomly
    for(int i = 0; i < SIZE; i++) {
        for(int j = 0; j < SIZE; j++) {
            A[i][j] = rand() % 2;
            B[i][j] = rand() % 2;
        }
    }

    // Create threads
    pthread_create(&t1, NULL, compute_sum, NULL);
    pthread_create(&t2, NULL, compute_diff, NULL);

    // Wait for threads to finish
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);

    // Main thread multiplies SUM and DIFF
    multiply_matrices();

    // Print final result
    printf("Final Result (SUM * DIFF):\n");
    for(int i = 0; i < SIZE; i++) {
        for(int j = 0; j < SIZE; j++) {
            printf("%4d", RESULT[i][j]);
        }
        printf("\n");
    }

    return 0;
}
