#include "random_mat.h"

M create_rand_matrix(int n, int m){
	M mat(n, Row(m));
	for(int i = 0; i < n; ++i){
		for(int j = 0; j < m; ++j){
			mat[i][j] = rand() % 2;
		}
	}
	return mat;
}

void show_mat(const M& mat){
	int n = (int) mat.size();
	int m = (int) mat[0].size();
	for(int i = 0; i < n; ++i){
		for(int j = 0; j < m; ++j){
			printf("%5d",  mat[i][j]);
		}
		printf("\n");
	}
}
