#include <bits/stdc++.h>
#include <pthread.h>

#include "random_mat.h"

using namespace std;
using Row = vector<int>; 
using M = vector<Row>;

const int N_THREADS = 10;


class tdata{
public: 
	M *a_mat;
	Row *a_vector;
	Row *result;
	int start;
	int end;

	tdata() : a_mat(NULL), a_vector(NULL), result(NULL), start(-1), end(-1) {}
	tdata(M *a_mat, Row *a_vector, Row *result, int start, int end) :
		a_mat(a_mat), 
		a_vector(a_vector), 
		result(result), 
		start(start), 
		end(end) {}
};	

void* multRow(void * args){
	tdata* data = (tdata*) args;
	for(int i = data->start; i < data->end; ++i){
		(*data->result)[i] = 0;
		for(int j = 0; j < (int) data->a_vector->size(); ++j){
			(*data->result)[i] += (*data->a_mat)[i][j] * (*data->a_vector)[j];
		}
	}
	return NULL;
}

Row mult_matrix_vector(M & mat, Row& vec){
	if((int) vec.size() != (int)mat[0].size()){
		throw invalid_argument("The sizes aren't the same");
	}
	int m = (int) mat.size();
	int n = (int) vec.size();
	int num_threads = min(N_THREADS, m);

	pthread_t threads[num_threads];
	tdata thr_data[num_threads];

	int len_chunck = m / num_threads;
	int reminder = m % num_threads;

	int current = 0;
	Row ans(m, 0);

	for(int i = 0; i < num_threads; ++i){
		int mov = (i < reminder) ? 1 : 0;
		int next = current + len_chunck + mov;
		thr_data[i] = tdata(&mat, &vec, &ans, current, next);
		pthread_create(&threads[i], NULL, multRow, (void*) &thr_data[i]);
		current = next;
	}

	for(int i = 0; i < num_threads; ++i){
		pthread_join(threads[i], NULL);
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



