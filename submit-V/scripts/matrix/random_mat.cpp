#include "random_mat.h"

Row create_rand_vector(int n){
	Row r(n);
	for(int i = 0; i < n; ++i){
		r[i] = rand() % MAX_VALUE;
	}
	return r;
}

M create_rand_matrix(int m, int n){
	M mat;
	for(int i = 0; i < m; ++i){
		mat.push_back(create_rand_vector(n));
	}
	return mat;
}

void show_vec(const Row& r){
	int n = (int) r.size();
	for(int i = 0; i < n; ++i){
		printf("%5d ", r[i]);
	}
	printf("\n");
}

void show_mat(const M& mat){
	int m = (int) mat.size();
	for(int i = 0; i < m; ++i){
		show_vec(mat[i]);
	}
}