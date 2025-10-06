#include "random_mat.h"
const double MAX_VALUE = 1.0;
const double MIN_VALUE = 0.0;

double rand_double(double a = 0.0, double b = 1.0) {
    static random_device rd;
    static mt19937 gen(rd());
    uniform_real_distribution<double> dist(a, b);
    return dist(gen);
}

M create_rand_matrix(int n, int m){
	M mat(n, Row(m));
	for(int i = 0; i < n; ++i){
		for(int j = 0; j < m; ++j){
			mat[i][j] = rand_double(MIN_VALUE, MAX_VALUE);
		}
	}
	return mat;
}

void show_mat(const M& mat){
	int n = (int) mat.size();
	int m = (int) mat[0].size();
	for(int i = 0; i < n; ++i){
		for(int j = 0; j < m; ++j){
			printf("%6.2f",  mat[i][j]);
		}
		printf("\n");
	}
}
