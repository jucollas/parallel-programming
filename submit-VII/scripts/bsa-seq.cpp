#include <bits/stdc++.h>
#include <chrono>

#define rep(i, n) for(int (i) = 0; i < n; ++i)
#define sz(x) (int) x.size()
using namespace std;
const int MAX_VALUE = 1000;

vector<int> generate_arr(int n){
  vector<int> arr(n);
  rep(i, n) arr[i] = rand() % MAX_VALUE;
  return arr;
}

void print_arr(vector<int>& arr){
  rep(i, sz(arr)) printf("%4d", arr[i]);
  printf("\n");
}

int binarySearch(vector<int> &arr, int x) {
    int low = 0, high = sz(arr) - 1;
    int ans = -1;
    while (low <= high) {
        int mid = low + ((high - low) >> 1);
        if (arr[mid] == x)
            ans = mid;
        if (arr[mid] < x)
            low = mid + 1;
        else
            high = mid - 1;
    }
    return ans;
}

int main(){
  srand(time(0));
  int t = 0;
  int times = 8;
  int n = 10;
  while (t < times){
    vector<int> arr = generate_arr(n);
    sort(arr.begin(), arr.end());
    int x = rand() % MAX_VALUE;
    auto start_time = chrono::high_resolution_clock::now();
    int ind = binarySearch(arr, x);
    auto end_time = chrono::high_resolution_clock::now();
    chrono::duration<double> elapsed = end_time - start_time;
    cout << "10**" << t + 1 << " : " << elapsed.count() << "s.\n";
    n *= 10;
    t++;
    //print_arr(arr);
    /*if (ind != -1){
      printf("El elemento %d se encuentra en la posicion %d", x, ind);
      }else{
        printf("El elemento %d no se encuentra en el arreglo", x);
        }*/
  }
  return 0;
}