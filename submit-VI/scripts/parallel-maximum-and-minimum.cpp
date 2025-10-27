#include <bits/stdc++.h>
#include <pthread.h>
#include <chrono>

using namespace std;
const int INF = 1 << 29;
const int N_THREAD = 2;

struct tdata{
  int start, end;
  int local_max, local_min;
  vector<int>* arr;
  tdata(){
    arr = NULL;
    start = -1;
    end = -1;
    local_max = -INF;
    local_min = INF;
  }
  tdata(vector<int>* _arr, int _start, int _end){
    arr = _arr;
    start = _start;
    end = _end;
    local_max = -INF;
    local_min = INF;
  }
};

void * partial_max_and_min(void * args){
  tdata* data = (tdata *) args;
  for(int i = data->start; i < data->end; ++i){
    data->local_max = max(data->local_max, (*data->arr)[i]);
    data->local_min = min(data->local_min, (*data->arr)[i]);
  }
  return NULL;
}

pair<int, int> max_and_min(vector<int>& A){
  int n = (int) A.size();
  int num_thread = min(n, N_THREAD);

  pthread_t threads[num_thread];
  tdata thr_data[num_thread];

  int len_chuck = n / num_thread;
  int remaider = n % num_thread;

  int act = 0;
  for(int i = 0; i < num_thread; ++i){
    int mv = (i < remaider) ? 1 : 0;
    int to = act + len_chuck + mv;
    thr_data[i] = tdata(&A, act, to);
    pthread_create(&threads[i], NULL, partial_max_and_min, (void *) &thr_data[i]);
    act = to;
  }

  int global_max = -INF;
  int global_min = INF;
  for(int i = 0; i < num_thread; ++i){
    pthread_join(threads[i], NULL);
    global_max = max(global_max, thr_data[i].local_max);
    global_min = min(global_min, thr_data[i].local_min);
  }
  return {global_max, global_min};
}


int main(){
  int n; cin >> n;
  vector<int> A(n);
  for(int i = 0; i < n; ++i){
    cin >> A[i];
  }
  auto start_time = chrono::high_resolution_clock::now();
  pair<int, int> ans = max_and_min(A);
  auto end_time = chrono::high_resolution_clock::now();

  chrono::duration<double> elapsed = end_time - start_time;
  cout << "max : " << ans.first << "\nmin : " << ans.second << '\n';
  cout << "Time : " << elapsed.count() << " s.\n";
  return 0;
}