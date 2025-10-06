#ifndef OPERATIONS_MATRIX
#define OPERATIONS_MATRIX

#include <bits/stdc++.h>
using namespace std;
using Row = vector<int>;
using M = vector<Row>;

M transpose(const M& A);
M multiplication(const M& A, const M& B);
M addition(const M& A, const M& B);

#endif