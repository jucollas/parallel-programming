#include <bits/stdc++.h>
#include <pthread.h>
using namespace std;

#define N_THREAD 4

struct ThreadData {
    int id;
    vector<int>* A;
    int n;
};

pthread_barrier_t barrier;

void* prefixSum(void* arg) {
    ThreadData* data = (ThreadData*) arg;
    int tid = data->id;
    int n = data->n;
    vector<int>& A = *(data->A);

    for (int d = 0; (1 << d) < n; d++) {
        int step = 1 << (d + 1);
        for (int i = tid * step; i + step - 1 < n; i += N_THREAD * step) {
            A[i + step - 1] += A[i + (step / 2) - 1];
        }
        pthread_barrier_wait(&barrier);
    }

    if (tid == 0) A[n - 1] = 0;
    pthread_barrier_wait(&barrier);

    for (int d = (int)log2(n) - 1; d >= 0; d--) {
        int step = 1 << (d + 1);
        for (int i = tid * step; i + step - 1 < n; i += N_THREAD * step) {
            int t = A[i + (step / 2) - 1];
            A[i + (step / 2) - 1] = A[i + step - 1];
            A[i + step - 1] += t;
        }
        pthread_barrier_wait(&barrier);
    }

    return NULL;
}

int main() {
    vector<int> A = {2, 4, 1, 7, 3, 0, 4, 2};
    int n = A.size();

    pthread_t threads[N_THREAD];
    ThreadData tdata[N_THREAD];
    pthread_barrier_init(&barrier, NULL, N_THREAD);

    for (int i = 0; i < N_THREAD; i++) {
        tdata[i] = {i, &A, n};
        pthread_create(&threads[i], NULL, prefixSum, &tdata[i]);
    }

    for (int i = 0; i < N_THREAD; i++)
        pthread_join(threads[i], NULL);

    pthread_barrier_destroy(&barrier);

    cout << "Resultado del Prefix Sum:\n";
    for (int i = 0; i < n; i++)
        cout << A[i] << " ";
    cout << endl;

    return 0;
}
