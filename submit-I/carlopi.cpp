#include<iostream>
#include<vector>
#include<random>
#include<pthread.h>

#include<chrono>

using namespace std;
#define rep(i,strt,end) for(int i=strt;i<int(end);++i)


std::mt19937 mrandom( std::chrono::steady_clock::now().time_since_epoch().count() );

class point{
	public:
	double x,y;
	point()=default;
	double operator *(const point&o)const{return x*o.x+y*o.y;}
};

int montepi(int iter){
	point pt;
	std::uniform_real_distribution<> dis(-1.0, 1.0);
	auto shuffle=[&](){ pt.x=dis(mrandom); pt.y=dis(mrandom);};
	
	int res=0;
	rep(i,0,iter){
		shuffle();
		if(pt*pt<=1.0)++res;
	}
	return res;
}

struct tdata{
	vector<int>*result;
	int ind,iter;
};

void *  work_carlopi(void * arg){
	tdata *dat=(tdata*)arg;
	(*dat->result)[dat->ind] = montepi(dat->iter);
	pthread_exit(NULL);
}
int carlopi(int n_threads,int iter){	
	pthread_t * threads = new pthread_t[n_threads];
	tdata *thr_dat = new tdata[n_threads];
	
	vector<int> res(n_threads,0);
	int rw_th=iter/n_threads, rw_ex=iter%n_threads;
	int prv=0;
	rep(i,0,n_threads){
		int act=rw_th;
		if(rw_ex)++act,--rw_ex;

		thr_dat[i]={&res,i,act};
		
		pthread_create(&threads[i],NULL,work_carlopi,(void*)&thr_dat[i]);
	}
	
	rep(i,0,n_threads)pthread_join(threads[i],NULL);
	int tres=0;for(int act:res)tres+=act;
	delete threads; delete thr_dat;
	return tres;
}


int main(int argc, char*argv[]){
	int thrd,iter;cin >>thrd>>iter;
	
	auto get_tm=[](){return chrono::high_resolution_clock::now();};
	
	auto strt=get_tm();
	int p_res=carlopi(thrd,iter);
	double res=4.0*double(p_res)/double(iter);
	
	int time = (get_tm()-strt).count();
	cout << "RESPUESTA:::::::::: " << res << endl;

	cout << "TIME (in miliseconds)\n";
	cout << time << '\n';

	return 0;
}
