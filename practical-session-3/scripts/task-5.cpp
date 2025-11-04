#include <bits/stdc++.h>
#include <chrono>

using namespace std;


const int MAX_VALUE = 100;
const int NUM_THREADS = 8;
const double gravity = 9.81;

double lower = 0.0;
double upper = 50.0;
std::mt19937 gen(std::chrono::high_resolution_clock::now().time_since_epoch().count());
std::uniform_real_distribution<double> dis(lower, upper);

struct data_t{
  int start, end;
  vector<double>* heights;
  data_t(){
    start = -1;
    end = -1;
    heights = nullptr;
  }
  data_t(vector<double>* _heights, int _start, int _end){
    start = _start;
    end = _end;
    heights = _heights;
  }
};

vector<double> generate_heights(int n_heights){
  vector<double> heights(n_heights);
  for(int i = 0; i < n_heights; ++i ){
    heights[i] = dis(gen);
  }
  return heights;
}

double calculate_time_fall(double h){
  return sqrt((double(2) * h) / gravity);
}

void* time_falls_mult(void * args){
  data_t* data = (data_t*) args; 
  for(int i = data->start; i < data->end; ++i){
    double time = calculate_time_fall((*data->heights)[i]);
    printf("time %d : %f\n", i, time);
  }
  return NULL;
}

bool time_falls(vector<double>& heights){
  int n = heights.size();
  int num_threads = min(n, NUM_THREADS);

  pthread_t threads[num_threads];
  data_t data[num_threads];

  int len_chunck = n / num_threads;
  int rem = n % num_threads;
  int curr = 0;
  for(int i = 0; i < num_threads; ++i){
    int mv = (i < rem) ? 1 : 0;
    int to = curr + len_chunck + mv;
    data[i] = data_t(&heights, curr, to);
    pthread_create(&threads[i], NULL, time_falls_mult, &data[i]);
    curr = to;
  } 
  for(int i = 0; i < num_threads; ++i){
    pthread_join(threads[i], NULL);
  }
  return true;
}


int main(){
  int n_height; cin >> n_height; 
  vector<double> heights = generate_heights(n_height);
  for(int i = 0; i < (int) heights.size(); ++i){
      cout << "height " << i << " : " << heights[i] << endl;
  }
  if(time_falls(heights)){
    cout << " Succesfull" << endl;
  }
  return 0;
}