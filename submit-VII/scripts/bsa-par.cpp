#include <bits/stdc++.h>
#include <pthread.h>
using namespace std;

#define rep(i, n) for (int i = 0; i < (n); ++i)
#define sz(x) (int)(x).size()

const int MAX_VALUE = 100;
const int N_THREADS = 4;
const int LIMIT = 30;

struct ThreadData {
    vector<int>* arr;
    int start;
    int end;
    int target;
    int id;
    bool* found;
    int* found_index;
    pthread_mutex_t* lock;
};

vector<int> generate_arr(int n) {
    vector<int> arr(n);
    rep(i, n) arr[i] = rand() % MAX_VALUE;
    return arr;
}

void print_arr(vector<int>& arr) {
    rep(i, sz(arr)) printf("%4d", arr[i]);
    printf("\n");
}

int binarySearch(vector<int>& arr, int l, int h, int x) {
    int low = l, high = h;
    while (low <= high) {
        int mid = low + ((high - low) >> 1);
        if (arr[mid] == x) return mid;
        if (arr[mid] < x) low = mid + 1;
        else high = mid - 1;
    }
    return -1;
}

void* parallel_search(void* arg) {
    ThreadData* data = (ThreadData*)arg;
    auto& arr = *data->arr;
    int start = data->start;
    int end = data->end;
    int x = data->target;

    if (*(data->found) || start > end) pthread_exit(nullptr);

    if (arr[start] == x) {
        pthread_mutex_lock(data->lock);
        if (!*(data->found)) {
            *(data->found) = true;
            *(data->found_index) = start;
        }
        pthread_mutex_unlock(data->lock);
        pthread_exit(nullptr);
    }

    if (arr[end] == x) {
        pthread_mutex_lock(data->lock);
        if (!*(data->found)) {
            *(data->found) = true;
            *(data->found_index) = end;
        }
        pthread_mutex_unlock(data->lock);
        pthread_exit(nullptr);
    }

    if (x < arr[start] || x > arr[end]) pthread_exit(nullptr);

    int n = end - start + 1;
    if (n <= LIMIT) {
        int idx = binarySearch(arr, start, end, x);
        if (idx != -1) {
            pthread_mutex_lock(data->lock);
            if (!*(data->found)) {
                *(data->found) = true;
                *(data->found_index) = idx;
            }
            pthread_mutex_unlock(data->lock);
        }
        pthread_exit(nullptr);
    }

    int num_threads = min(n, N_THREADS);
    int segment_size = n / num_threads;
    int rem = n % num_threads;
    pthread_t threads[num_threads];
    ThreadData subdata[num_threads];
    int act = start;

    rep(i, num_threads) {
        int mv = i < rem ? 1 : 0;
        int next = act + segment_size + mv - 1;
        subdata[i] = {&arr, act, next, x, i, data->found, data->found_index, data->lock};
        pthread_create(&threads[i], nullptr, parallel_search, &subdata[i]);
        act = next + 1;
    }

    rep(i, num_threads) pthread_join(threads[i], nullptr);
    pthread_exit(nullptr);
}

// ---------- controlador principal ----------
int parallel_binary_search(vector<int>& arr, int x) {
    int n = sz(arr);
    int num_threads = min(n, N_THREADS);
    bool found = false;
    int found_index = -1;
    pthread_mutex_t lock;
    pthread_mutex_init(&lock, nullptr);

    pthread_t threads[num_threads];
    ThreadData data[num_threads];

    int segment_size = n / num_threads;
    int rem = n % num_threads;
    int act = 0;

    rep(i, num_threads) {
        int mv = i < rem ? 1 : 0;
        int next = act + segment_size + mv - 1;
        data[i] = {&arr, act, next, x, i, &found, &found_index, &lock};
        pthread_create(&threads[i], nullptr, parallel_search, &data[i]);
        act = next + 1;
    }

    rep(i, num_threads) pthread_join(threads[i], nullptr);
    pthread_mutex_destroy(&lock);
    return found_index;
}

int main() {
    srand(time(0));
    int t = 0;
    int times = 8;
    int n = 10;
    while (t < times){
        vector<int> arr = generate_arr(n);
        sort(arr.begin(), arr.end());
        int x = rand() % MAX_VALUE;
    
        /*printf("Arreglo ordenado:\n");
        print_arr(arr);
        printf("Buscando el elemento %d...\n", x);
        */
        auto start_time = chrono::high_resolution_clock::now();
        int found_index = parallel_binary_search(arr, x);
        auto end_time = chrono::high_resolution_clock::now();

        chrono::duration<double> elapsed = end_time - start_time;
        cout << "10**" << t + 1 << " : " << elapsed.count() << "s.\n";
        
        /*if (found_index != -1)
            printf("✅ Elemento %d encontrado en la posición %d\n", x, found_index);
        else
            printf("❌ Elemento %d no encontrado en el arreglo\n", x);*/
        t++;
        n *= 10;
    }
    return 0;
}
