/**
 * @autor: JuCollas 
 * @date: 17 nov 2025
 */

#ifndef GENERATION_H
#define GENERATION_H

#include <bits/stdc++.h>
#include "cell.h"
#include "entity.h"
#define sz(x) (int) x.size()

using namespace std;

const array<char, 4> LETTER = {' ', '*', 'R', 'F'};
const array<string, 4> WORD = {"EMPTY", "ROCK", "RABBIT", "FOX"};

using Row = vector<Cell>;
using Mat = vector<Row>;

class Param {
public:
    const int GEN_PROC_RABBITS;
    const int GEN_PROC_FOXES;
    const int GEN_FOOD_FOXES;
    const int N_GEN;
    const int R;
    const int C;

    Param(int proc_rabbits, int proc_foxes, int food_foxes,
          int n_gen, int r, int c);
};


class Generation {
private:
    const Param simulation_parameters;
    vector<vector<int>> eco_compress;
    Mat ecosystem;
    int n_gen_local = 0;

public:
    Generation(const Mat& _ecosystem, const Param& _param);
    Generation(int n_row, int n_col, const Mat& _ecosystem,
                       const Param& _param);

    Mat& get_ecosystem();
    int get_n_gen() const;
    Param get_param() const;
    void increase_generation();

    vector<vector<int>>& get_eco_compress();


    void print();
    void print_grid();
};
#endif