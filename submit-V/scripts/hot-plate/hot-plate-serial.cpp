#include <bits/stdc++.h>
#include "random_mat.h"
using namespace std;
using Row = vector<double>; 
using M = vector<Row>;
const vector<pair<int,int>> cord = {{-1, 0}, {0, -1}, {1, 0}, {0, 1}};

double calculate_hot(const M& current, int i, int j){
	double ans = 0.0;
	for(const pair<int, int> &dir : cord){
		ans += current[i + dir.first][j + dir.second];
	}
	return ans; 
}

bool update(const M& current, M& next) {
    int n = current.size();
    int m = current[0].size();
    bool all_converged = true;
    for (int i = 1; i < n - 1; ++i) {
        for (int j = 1; j < m - 1; ++j) {
            double adj = calculate_hot(current, i, j);
            next[i][j] = (adj + current[i][j] * 4) / 8;
            if (fabs(current[i][j] - (adj / 4)) > 0.1)
                all_converged = false;
        }
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



