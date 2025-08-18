#include<iostream>
#include<vector>
#include<random>

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

int main(int argc, char*argv[]){
	int iter;cin>>iter;
	
	auto get_tm=[](){return chrono::high_resolution_clock::now();};
	
	auto strt=get_tm();
	int p_res=montepi(iter);
	double res=4.0*double(p_res)/double(iter);
	
	int time = (get_tm()-strt).count();
	cout << "RESPUESTA:::::::::: " << res << endl;

	cout << "TIME (in miliseconds)\n";
	cout << time << '\n';

	return 0;
}
