#include <bits/stdc++.h>
#include <omp.h>
using namespace std;

#define rep(i, n) for(int i = 0; i < (n); ++i)
#define sz(x) (int) x.size()

typedef long long lint;

int N_THREADS;

lint sum_arr(vector<int>& arr) {
	int n = sz(arr);
	lint ans = 0;
	#pragma omp parallel for reduction(+:ans)
	for (int i = 0; i < n; i++) {
		ans += arr[i];
	}
	return ans;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Use: %s <num_thread>\n", argv[0]);
        exit(1);
    }

    N_THREADS = stoi(argv[1]);

	omp_set_num_threads(N_THREADS);
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
