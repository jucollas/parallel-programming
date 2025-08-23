#include <bits/stdc++.h>
#include <pthread.h>

using namespace std;
using column = vector<int>;
using matrix = vector<column>;

#define rep(i, n) for(int i = 0; i < (n); ++i)
#define sz(x) (int) x.size()

int N_THREADS = 32;

class tdata {
private:
	int start;
	int end;
	matrix  *A, *B;
	matrix *ans;
public: 
	tdata(int start, int end, matrix* A, matrix* B, matrix* ans) : 
		start(start), end(end), A(A), B(B), ans(ans) {}
	tdata() : start(0), end(0), ans(NULL), A(NULL), B(NULL) {}
	int getStart(){ return start;}
	int getEnd(){ return end;}
	matrix* getA(){ return A;}
	matrix* getB(){ return B;}
	matrix* getAns(){ return ans;}
};  

void* partial_mult(void* args){
	tdata* data = (tdata*) args;
	int start = data->getStart(), end = data->getEnd();
	matrix *A = data->getA(), *B = data->getB();
	matrix *ans = data->getAns();
	int m = sz((*B)), n = sz((*B)[0]);
	int part = 0;
	for(int i = start; i < end; ++i){
		for(int j = 0; j < m; ++j){
			int xij = 0;
			for(int k = 0; k < n; ++k){
				xij += (*A)[i][k] * (*B)[j][k];
			}
			(*ans)[i][j] = xij;
		}
	}
	pthread_exit(NULL);
	return NULL;
}

matrix transpose(matrix& x){
	int n = size(x), m = size(x[0]);
	matrix ans(m, column(n));
	rep(i, n) rep(j, m){
		ans[j][i] = x[i][j];
	}
	return ans;
}

matrix mult(matrix& A, matrix& B){
	int n = sz(A);
	int m = sz(B[0]);
	int num_threads = min(N_THREADS, n);
	pthread_t threads[num_threads];
	tdata thr_data[num_threads];

	int len_chunk = n / num_threads;
	int reminder = n % num_threads;
	int curret = 0;
	matrix ans(n, column(m));
	matrix BT = transpose(B);
	rep(i, num_threads){
		int mov = (i < reminder) ? 1 : 0;
		int next = curret + len_chunk + mov;
		thr_data[i] = tdata(curret, next, &A, &BT, &ans);
		pthread_create(&threads[i], NULL, partial_mult, (void *) &thr_data[i]);
		curret = next;
	}
	rep(i, num_threads){
		pthread_join(threads[i], NULL);
	}
	return ans;
}

int main(){
	int n, m; cin >> n >> m;
	matrix A(n, column(m));
	rep(i, n) rep(j, m){
		cin >> A[i][j];
	}

	int q, p; cin >> q >> p;
	matrix B(q, column(p));
	rep(i, q) rep(j, p){
		cin >> B[i][j];
	}

	if(q != m){
		fprintf(stderr, "Size A is diferent to B");
		exit(1);
	}

	clock_t start_cpu = clock();
	matrix ans = mult(A, B);
	clock_t end_cpu = clock();

	cout << n << " " << p << endl;
	rep(i, n){
		rep(j, p){
			if(j != 0) cout << ' ';
			cout << ans[i][j];
		} 
		cout << "\n";
	}    
	double cpu_time = double(end_cpu - start_cpu) / CLOCKS_PER_SEC;
	cout << "CPU time: " << cpu_time << " seconds" << endl;
	return 0;
}
