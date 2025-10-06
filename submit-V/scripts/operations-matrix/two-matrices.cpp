#include <bits/stdc++.h>
#include "random_mat.h"
#include "operations-matrix.h"

using namespace std;
using Row = vector<int>;
using M = vector<Row>;

int main(int argc, char** argv){
	if(argc < 5){
		cout << "Use <n> <m> <p> <q> : A(n * m), B(p * q)" << endl;
	}

	int n = stoi(argv[1]);
	int m = stoi(argv[2]);
	int p = stoi(argv[3]);
	int q = stoi(argv[4]);

	if(m != p){
		throw invalid_argument("m and n aren't the same");
	}

	M A = create_rand_matrix(n, m);
	M B = create_rand_matrix(p, q);
	M AT = transpose(A);
	M AB = addition(A, B);
	M AxB = multiplication(A, B);

	cout << "A:\n";
	show_mat(A);
	cout << "B:\n";
	show_mat(B);
	cout << "AT:\n";
	show_mat(AT);
	cout << "AB:\n";
	show_mat(AB);
	cout << "AxB:\n";
	show_mat(AxB);
	return 0;
}
