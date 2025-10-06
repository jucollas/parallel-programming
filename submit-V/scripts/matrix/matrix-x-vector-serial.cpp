#include <bits/stdc++.h>
#include "random_mat.h"
using namespace std;
using Row = vector<int>; 
using M = vector<Row>;

int mult_vec(Row v, Row u){
	int ans = 0;
	int n = (int) v.size();
	for(int i = 0; i < n; ++i){
		ans += v[i] * u[i];
	}
	return ans;
}

Row mult_matrix_vector(M mat, Row vec){
	if((int) vec.size() != (int)mat[0].size()){
		throw invalid_argument("The sizes aren't the same");
	}
	int m = (int) mat.size();
	int n = (int) vec.size();
	Row ans(m);
	for(int i = 0; i < m; ++i){
		ans[i] = mult_vec(mat[i], vec);
	}
	return ans;
}


int main(int argc, char const *argv[]){
	if(argc < 3){
		cout << "Use <number_rows> <number_coloums>" << endl;
		return 1;
	}
	srand (time(NULL));
	int m = stoi(argv[1]);
	int n = stoi(argv[2]);
	M mat = create_rand_matrix(m, n);
	Row vec = create_rand_vector(n);

	Row ans = mult_matrix_vector(mat, vec);
	cout << "MAT:" << endl;
	show_mat(mat);
	cout << "\n";
	cout << "VEC:" << endl;
	show_vec(vec);
	cout << "\n";
	cout << "MAT x VEC:" << endl;
	show_vec(ans);
	return 0;
}



