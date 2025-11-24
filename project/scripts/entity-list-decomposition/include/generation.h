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
    Mat ecosystem;
    vector<Entity> rabbits;
    vector<Entity> foxes;
    const vector<Entity> rocks;
    int n_gen_local = 0;

public:

    Generation(int n, int m, const vector<Entity>& _rocks, const Param& _param);

    Generation(int _gen, const Mat& _ecosystem,
               const vector<Entity>& _rocks,
               const vector<Entity>& _rabbits,
               const vector<Entity>& _foxes,
               const Param& _param);


    const vector<Entity>& get_rocks() const;
    vector<Entity>& get_rabbits();
    vector<Entity>& get_foxes();
    Mat& get_ecosystem();
    int get_n_gen() const ;
    void set_n_gen(int _n_gen);

    void clean();

    void print();

    void print_grid();


    int get_param_food_foxes() const ;
    int get_param_proc_rabbits() const ;
    int get_param_proc_foxes() const ;
    Param get_param() const ;
};
#endif