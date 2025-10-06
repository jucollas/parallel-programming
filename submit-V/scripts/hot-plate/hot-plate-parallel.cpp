#include <bits/stdc++.h>
#include <pthread.h>

#include "random_mat.h"

using namespace std;
using Row = vector<double>; 
using M = vector<Row>;

const vector<pair<int,int>> cord = {{-1, 0}, {0, -1}, {1, 0}, {0, 1}};
const int N_THREADS = 10;

class  tdata{
public:
	const M *current;
	M *next;
	int start;
	int end;
	bool* all_converged;
	tdata(): current(NULL), next(NULL), start(-1), end(-1), all_converged(NULL) {}
	tdata(const M* current, M* next, bool* all_converged,  int start, int end): current(current), next(next), start(start), end(end), all_converged(all_converged) {}
};

double calculate_hot(const M& current, int i, int j){
	double ans = 0.0;

	for(const pair<int, int> &dir : cord){
		ans += current[i + dir.first][j + dir.second];
	}
	return ans; 
}

void* calculeRow(void * args){
	tdata* data = (tdata*) args;
	int m = (int) (*data->next)[0].size();
	for(int i = data->start; i < data->end; ++i){
		for(int j = 1; j < m - 1; ++j){
			double adj = calculate_hot(*data->current, i, j);
			(*data->next)[i][j] = (adj + (*data->current)[i][j] * 4) / 8;
			if(fabs((*data->current)[i][j] - (adj / 4)) > 0.1)
				(*data->all_converged) = false;
		}
	}
	return NULL;
}

bool update(const M& current, M& next) {
	int n = (int) current.size() - 2;

	int num_threads = min(n, N_THREADS);
	pthread_t threads[num_threads];
	tdata thr_data[num_threads];

	int len_chunck = n / num_threads;
	int remaider = n % num_threads;
	bool all_converged = true;

	int act = 1;
	for(int i = 0; i < num_threads; ++i){
		int mv = (i < remaider) ? 1 : 0;
		int to = act + len_chunck + mv;
		thr_data[i] = tdata(&current, &next, &all_converged, act, to);
		pthread_create(&threads[i], NULL, calculeRow, (void*) &thr_data[i]);
		act = to;
	}

	for(int i = 0; i < num_threads; ++i){
		pthread_join(threads[i], NULL);
	}
    return all_converged;
}

M simulation(M mat, int max_it, bool& converged) {
    int n = mat.size(), m = mat[0].size();
    M current = mat;
    M next(n, Row(m));
    converged = false;
    int i = 0;
    while (i < max_it && !converged) {
        converged = update(current, next);
        current.swap(next);
        i++;
    }
    return current;
}

int main(int argc, char const *argv[]){
	if(argc < 3){
		cout << "Use <number_rows> <number_coloums>" << endl;
		return 1;
	}

	int n = stoi(argv[1]);
	int m = stoi(argv[2]);
	int max_iter = 1000;
	bool convert;

	M mat = create_rand_matrix(n, m);
	M result = simulation(mat,max_iter, convert);
	cout << "Mat init:\n";
	show_mat(mat);
	cout << "Mat end:\n";
	show_mat(result);
	cout << "The simulation ";
	if(!convert) cout << "did not converge ";
	else cout << "converged"; 
	cout << "after " << max_iter << " iterations "<< endl;

	return 0;
}



