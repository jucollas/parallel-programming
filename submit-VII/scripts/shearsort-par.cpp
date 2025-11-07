#include <bits/stdc++.h>
#include <pthread.h>

#define rep(i, n) for(int (i) = 0; i < n; ++i)
#define sz(x) (int) x.size()
using namespace std;
using Row = vector<int>;
using Mat = vector<Row>; 

const int MAX_NUMBER = 100;
const int N_THREADS = 8;

class data_t{
  public:
    int start, end;
    bool is_row;
    Mat* A;
    data_t() : A(NULL), start(-1), end(-1), is_row(false) {}
    data_t(Mat* A, int start, int end, bool is_row): A(A), start(start), end(end), is_row(is_row) {}
};

Mat generated_random_mat(int  n, int m){
  Mat ans(n, Row(m));
  rep(i, n){
    rep(j, m){
      ans[i][j] = rand() % MAX_NUMBER;
    }
  }
  return ans;
}

void print_mat(Mat& A){
  rep(i, sz(A)){
    rep(j, sz(A[i])){
      printf("%2d ", A[i][j]);
    }
    printf("\n");
  }
  printf("\n");
}
  

int log_2(int x){
  int ans = 0;
  while(x > 1){
    x >>= 1;
    ans++;
  }
  return ans;
}


void sort_row(Mat* A, int tid, bool asc){
  if(asc) sort((*A)[tid].begin(), (*A)[tid].end());
  else sort((*A)[tid].begin(), (*A)[tid].end(), [](int a, int b){
    return a > b;
  });
}

void sort_column(Mat* A, int tid){
  int n = sz((*A));
  Row r(n);
  rep(i, n){
    r[i] = (*A)[i][tid];
  }
  sort(r.begin(), r.end());
  rep(i, n){
    (*A)[i][tid] = r[i];
  }
}

void* sort_mult(void* arg){
  data_t* data = (data_t*) arg;
  for(int i = data->start; i < data->end; ++i){
    if(data->is_row) sort_row(data->A, i, i % 2 == 0);
    else sort_column(data->A, i);
  }
  return NULL;
}

void sort_parall(Mat& A, bool is_row){
  int n = sz(A);
  int num_threads = min(n, N_THREADS);
  
  pthread_t threads[num_threads];
  data_t data[num_threads];

  int len_chunk = n / num_threads;
  int rem = n % num_threads;
  int act = 0;

  for( int i = 0; i < num_threads; ++i){
    int mv = i < rem ? 1 : 0;
    int next = act + len_chunk + mv;
    data[i] = data_t(&A, act, next, is_row);
    pthread_create(&threads[i], NULL, sort_mult, (void *) &data[i]);
    act = next;
  }
  for(int i = 0; i < num_threads; ++i){
    pthread_join(threads[i], NULL);
  }
}

void shear_sort(Mat& A){
  int n = sz(A);
  int times = log_2(n) + 1;
  rep(i, times){
    sort_parall(A, true);
    sort_parall(A, false);
  }
}

int main(){
  srand(time(0));
  int n = 5;
  int t = 0;
  int times = 10;
  while (t < times){
    Mat A = generated_random_mat(n, n);
    //print_mat(A);
    auto start_time = chrono::high_resolution_clock::now();
    shear_sort(A);
    auto end_time = chrono::high_resolution_clock::now();
    chrono::duration<double> elapsed = end_time - start_time;
    cout << n * n << " : " << elapsed.count() << "s.\n";
    
    //print_mat(A);
    n *= 2;
    t++;
  }
  return 0;
}