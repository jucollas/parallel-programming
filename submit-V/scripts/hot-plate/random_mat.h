#ifndef RANDOM_MAT_H
#define RANDOM_MAT_H

#include <bits/stdc++.h>
using namespace std;
using Row = vector<double>;
using M = vector<Row>;

double rand_double(double a, double b);
M create_rand_matrix(int n, int m);
void show_mat(const M& mat);

#endif