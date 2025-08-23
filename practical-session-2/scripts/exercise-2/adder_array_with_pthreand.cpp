#include <bits/stdc++.h>
#include <pthread.h>
using namespace std;

#define rep(i, n) for(int i = 0; i < (n); ++i)
#define sz(x) (int) x.size()
typedef long long lint;

int N_THREADS = 16;
pthread_mutex_t mux = PTHREAD_MUTEX_INITIALIZER;

class tdata {
private:
    int start;
    int end;
    lint* ans;
    vector<int>* arr;
public: 
    tdata(int start, int end, vector<int>* arr, lint* ans) 
        : start(start), end(end), ans(ans), arr(arr) {}
    tdata() : start(0), end(0), ans(nullptr), arr(nullptr) {}
    int getStart() { return start; }
    int getEnd() { return end; }
    vector<int>* getArr() { return arr; }
    lint* getAns() { return ans; }
};  

void* partial_sum(void* args) {
    tdata* data = (tdata*) args;
    int start = data->getStart(), end = data->getEnd();
    vector<int>* arr = data->getArr();

    lint part = 0;
    for(int i = start; i < end; ++i) {
        part += (*arr)[i];
    }

    pthread_mutex_lock(&mux);
    *(data->getAns()) += part;
    pthread_mutex_unlock(&mux);

    return nullptr;
}

lint sum_arr(vector<int>& arr) {
    int n = sz(arr);
    int num_threads = min(N_THREADS, n);
    pthread_t threads[num_threads];
    tdata thr_data[num_threads];
    lint ans = 0;

    int len_chunk = n / num_threads;
    int remainder = n % num_threads;
    int current = 0;

    rep(i, num_threads) {
        int mov = (i < remainder) ? 1 : 0;
        int next = current + len_chunk + mov;
        thr_data[i] = tdata(current, next, &arr, &ans);
        pthread_create(&threads[i], NULL, partial_sum, (void *) &thr_data[i]);
        current = next;
    }

    rep(i, num_threads) {
        pthread_join(threads[i], NULL);
    }
    return ans;
}

int main() {
    int n; cin >> n;
    vector<int> arr(n);
    rep(i, n) cin >> arr[i];

    clock_t start_cpu = clock();
    lint ans = sum_arr(arr);
    clock_t end_cpu = clock();

    cout << "ANS: " << ans << endl;
    double cpu_time = double(end_cpu - start_cpu) / CLOCKS_PER_SEC;
    cout << "CPU time: " << cpu_time << " seconds" << endl;
    return 0;
}
