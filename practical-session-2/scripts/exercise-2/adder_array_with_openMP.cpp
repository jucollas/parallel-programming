// suma_arreglo_openmp.cpp
#include <bits/stdc++.h>
#include <pthread.h>
using namespace std;

#define rep(i, n) for(int i = 0; i < (n); ++i)
#define sz(x) (int) x.size()

typedef long long lint;


lint sum_arr(vector<int>& arr) {
	int n = sz(arr);
	lint ans = 0;
	#pragma omp parallel for reduction(+:ans)
	for (int i = 0; i < n; i++) {
		ans += arr[i];
	}
	return ans;
}

int main(){
	int n; cin >> n;
    vector<int> arr(n);
    rep(i, n) cin >> arr[i];

	clock_t start_cpu = clock();
	lint ans = sum_arr(arr);
	clock_t end_cpu = clock();
	cout << ans << endl;
	double cpu_time = double(end_cpu - start_cpu) / CLOCKS_PER_SEC;
	cout << "CPU time: " << cpu_time << " seconds" << endl;
	return 0;
}
