#ifndef RANDOM_MAT_H
#define RANDOM_MAT_H

#include <vector>
#include <cstdio>
#include <cstdlib>
using namespace std;

using Row = vector<int>;
using M = vector<Row>;

const int MAX_VALUE = 100; 

M create_rand_matrix(int m, int n);
Row create_rand_vector(int n);
void show_mat(const M &mat);
void show_vec(const Row &vec);

#endif
