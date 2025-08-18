// taylorpi.cpp
#include <bits/stdc++.h>
#include <pthread.h>
#include <ctime>
using namespace std;
int const N_THREADS = 8;

class tdata{
private:
	int start;
	int end;
	double part;
public: 
	tdata(int start, int end) : start(start), end(end), part(0) {}
	tdata() : start(0), end(0), part(0) {}
	int getStart(){ return start; }
	int getEnd(){ return end; }
	double getPart(){ return part; }
	void setPart(double x){ part = x; }
};

void* fraction(void * args){
	tdata* data = (tdata*) args;
	int start = data->getStart(), end = data->getEnd();
	double ans = 0;
	for(int i = start; i < end; ++i){
		ans += ((i % 2 == 0) ? 1.0 : -1.0) / (2.0 * i + 1.0);
	}
	data->setPart(ans);
  pthread_exit(NULL);
	return NULL;
}

double taylor_pi(int n){
	int num_threads = min(N_THREADS, n);
	pthread_t threads[num_threads];
	tdata thr_data[num_threads];

	int len_chunk = n / num_threads;
	int reminder = n % num_threads;
	int curret = 0;
	for(int i = 0; i < num_threads; ++i){
		int mov = (i < reminder) ? 1 : 0;
		int next = curret + len_chunk + mov;
		thr_data[i] = tdata(curret, next);
		pthread_create(&threads[i], NULL, fraction, (void*) &thr_data[i]);
		curret = next;
	}
	
	double ans = 0;
	for(int i = 0; i < num_threads; ++i){
		pthread_join(threads[i], NULL);
		ans += thr_data[i].getPart();
	}
	return ans * 4.0;
}

int main(int argc, char const *argv[]){
	if(argc < 2){
		printf("%s : Use <n_iter> \n", argv[0]);
	}
	int n_iter = stoi(argv[1]);
	clock_t start = clock();
	double pi = taylor_pi(n_iter);
	clock_t end = clock();
	double cpu_time = double(end - start) / CLOCKS_PER_SEC;
	cout << setprecision(15) << pi << endl;
	cout << "CPU time: " << cpu_time << " seconds" << endl;
	return 0;
}