#include "operations-matrix.h"

M transpose(const M& A){
	int n = (int) A.size();
	int m = (int) A[0].size();
	M R(m, Row(n));
	for(int i = 0; i < n; ++i){
		for(int j = 0; j < m; ++j){
			R[j][i] = A[i][j];
		}
	}
	return R;
}

M addition(const M& A, const M& B){
	int n = (int) A.size(), m = (int) A[0].size();
	if(n != (int) B.size() || m != (int) B[0].size()){
		throw invalid_argument("The sizes aren't the same");
	}
	M R(n, Row(m));
	for(int i = 0; i < n; ++i){
		for(int j = 0; j < m; ++j){
			R[i][j] = A[i][j] + B[i][j];
		}
	}
	return R;
}

M multiplication(const M& A, const M& B){
	int n = (int) A.size(), m = (int) A[0].size();
	int p = (int) B.size(), q = (int) B[0].size();
	if(m != p){
		throw invalid_argument("The sizes aren't the same");
	}
	M R(n, Row(q, 0));
	for(int i = 0; i < n; ++i){
		for(int j = 0; j < q; ++j){
			for(int k = 0; k < m; ++k){
				R[i][j] += A[i][k] * B[k][j];
			}
		}
	}
	return R;
}

