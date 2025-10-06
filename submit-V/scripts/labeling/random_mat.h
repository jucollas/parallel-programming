#ifndef RANDOM_MAT_H
#define RANDOM_MAT_H

#include <bits/stdc++.h>
using namespace std;
using Row = vector<int>;
using M = vector<Row>;

M create_rand_matrix(int n, int m);
void show_mat(const M& mat);

#endif