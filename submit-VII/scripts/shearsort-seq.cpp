#include <bits/stdc++.h>
#include <chrono>
#define rep(i, n) for(int (i) = 0; i < n; ++i)
#define sz(x) (int) x.size()
using namespace std;
using Row = vector<int>;
using Mat = vector<Row>; 

const int MAX_NUMBER = 100;

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


void sort_row(Mat& A, int tid, bool asc){
  if(asc) sort(A[tid].begin(), A[tid].end());
  else sort(A[tid].begin(), A[tid].end(), [](int a, int b){
    return a > b;
  });
}

void sort_column(Mat& A, int tid){
  int n = sz(A);
  Row r(n);
  rep(i, n){
    r[i] = A[i][tid];
  }
  sort(r.begin(), r.end());
  rep(i, n){
    A[i][tid] = r[i];
  }
}

void shear_sort(Mat& A){
  int n = sz(A);
  int times = log_2(n) + 1;
  rep(i, times){
    rep(tid, n){
      sort_row(A, tid, tid % 2 == 0);
    }
    rep(tid, n){
      sort_column(A, tid);
    }
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