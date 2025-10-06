#include <bits/stdc++.h>
#include <pthread.h>

#include "random_mat.h"

using namespace std;
using Row = vector<int>; 
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
	vector<pair<int, int>>* ones;
	tdata(): current(NULL), next(NULL), start(-1), end(-1), all_converged(NULL), ones(NULL) {}
	tdata(const M* current, M* next, bool* all_converged, vector<pair<int,int>>* ones,  int start, int end): current(current), next(next), start(start), end(end), all_converged(all_converged), ones(ones) {}
};

double calculate_min(const M& current, pair<int, int>& u){
	int ans = current[u.first][u.second];
	int n = (int) current.size(), m = (int) current[0].size();
	for(const pair<int, int> &dir : cord){
		int x = u.first + dir.first, y = u.second + dir.second;
		if(0 <= x && x < n && 0 <= y && y < m){
			if(current[x][y] != 0)
				ans = min(ans, current[x][y]);
		}
	}
	return ans; 
}

void* improve_label(void * args){
	tdata* data = (tdata*) args;
	for(int i = data->start; i < data->end; ++i){
		pair<int, int> u = (*data->ones)[i];
		double mini = calculate_min(*data->current, u);
		(*data->next)[u.first][u.second] = mini;
		if(mini != (*data->current)[u.first][u.second])
			(*data->all_converged) = false;
	}
	return NULL;
}

bool update(const M& current, M& next, vector<pair<int, int>>& ones) {
	int n = (int) ones.size();

	int num_threads = min(n, N_THREADS);
	pthread_t threads[num_threads];
	tdata thr_data[num_threads];

	int len_chunck = n / num_threads;
	int remaider = n % num_threads;
	bool all_converged = true;

	int act = 0;
	for(int i = 0; i < num_threads; ++i){
		int mv = (i < remaider) ? 1 : 0;
		int to = act + len_chunck + mv;
		thr_data[i] = tdata(&current, &next, &all_converged, &ones, act, to);
		pthread_create(&threads[i], NULL, improve_label, (void*) &thr_data[i]);
		act = to;
	}

	for(int i = 0; i < num_threads; ++i){
		pthread_join(threads[i], NULL);
	}
    return all_converged;
}

M labeling(const M& mat, int max_it) {
    int n = mat.size(), m = mat[0].size();
	M label(n, Row(m));
	vector<pair<int, int>> ones;
	int act = 1;
	for(int i = 0; i < n; ++i){
		for(int j = 0; j < m; ++j){
			if(mat[i][j] == 1){
				ones.push_back({i, j});
				label[i][j] = act;
				act++;
			}
		}
	}
    M current = label;
    M next(n, Row(m));
    bool converged = false;
    int i = 0;
    while (i < max_it && !converged) {
        converged = update(current, next, ones);
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
	M result = labeling(mat, max_iter);
	cout << "Mat one-zero:\n";
	show_mat(mat);
	cout << "Mat labeling:\n";
	show_mat(result);
	cout << "iterations : " << max_iter<< "\n";
	return 0;
}



