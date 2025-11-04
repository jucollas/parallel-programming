 #include <bits/stdc++.h>
using namespace std;

const int MAX_VALUE = 100;
const int NUM_THREADS = 8;

struct data_t{
  int start, end;
  vector<int>* sumed;
  vector<vector<int>>* arrs;
  data_t(){
    start = -1;
    end = -1;
    sumed = nullptr;
    arrs = nullptr;
  }
  data_t(vector<vector<int>>* _arrs, vector<int>* _acc, int _start, int _end){
    start = _start;
    end = _end;
    sumed = _acc;
    arrs = _arrs;
  }
};

vector<vector<int>> generate_arrs(int n_arrs, int len_arrs){
  vector<vector<int>> arrs(n_arrs, vector<int>(len_arrs));
  for(int i = 0; i < n_arrs; ++i ){
    for( int j =0; j < len_arrs; ++j){
      arrs[i][j] = rand() % MAX_VALUE;
    }
  }
  return arrs;
}

void sum_single_vector(int id, vector<vector<int>>* arrs, vector<int>* sumed){
  for(int i = 0; i < (*arrs)[id].size(); ++i){
    (*sumed)[id] += (*arrs)[id][i];
  }
}

void* sum_mult_vector(void * args){
  data_t* data = (data_t*) args;
  for(int i = data->start; i < data->end; ++i){
    sum_single_vector(i, data->arrs, data->sumed);
  }
  return NULL;
}

vector<int> sum_arrs(vector<vector<int>>& arrs){
  int n = arrs.size();
  int num_threads = min(n, NUM_THREADS);

  pthread_t threads[num_threads];
  data_t data[num_threads];
  
  vector<int> ans(n);
  int len_chunck = n / num_threads;
  int rem = n % num_threads;
  int curr = 0;
  for(int i = 0; i < num_threads; ++i){
    int mv = (i < rem) ? 1 : 0;
    int to = curr + len_chunck + mv;
    data[i] = data_t(&arrs, &ans, curr, to);
    pthread_create(&threads[i], NULL, sum_mult_vector, &data[i]);
    curr = to;
  } 
  for(int i = 0; i < num_threads; ++i){
    pthread_join(threads[i], NULL);
  }
  return ans;
}


int main(){
  int n_arrs, len_arrs; cin >> n_arrs >> len_arrs; 
  vector<vector<int>> arrs = generate_arrs(n_arrs, len_arrs);
  for(int i = 0; i < (int) arrs.size(); ++i){
    for(int j = 0; j < (int) arrs[i].size(); ++j){
      cout << arrs[i][j] << " ";
    }
    cout << endl;
  }
  vector<int> sumed = sum_arrs(arrs);
  for(int i = 0; i < n_arrs; ++i){
    cout << sumed[i] << ' '; 
  }
  cout << endl;
  return 0;
}