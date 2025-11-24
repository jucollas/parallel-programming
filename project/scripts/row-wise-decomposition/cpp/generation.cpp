#include "../include/generation.h"

using Row = vector<Cell>;
using Mat = vector<Row>;

Param::Param(int proc_rabbits, 
             int proc_foxes, 
             int food_foxes, 
             int n_gen, 
             int r,
             int c ) : 
    GEN_PROC_RABBITS(proc_rabbits), 
    GEN_PROC_FOXES(proc_foxes),
    GEN_FOOD_FOXES(food_foxes),
    N_GEN(n_gen),
    R(r),
    C(c) {}

Generation::Generation(const Mat& _ecosystem,
                       const Param& _param)
    : simulation_parameters(_param),
      ecosystem(_ecosystem),
      n_gen_local(0),
      eco_compress(sz(_ecosystem))
{   
    for(int i = 0; i < sz(ecosystem); ++i){
        for(int j = 0; j < sz(ecosystem[i]); ++j){
            EntityType type = ecosystem[i][j].get_type();
            if(type == EntityType::RABBIT || type == EntityType::FOX){
                eco_compress[i].push_back(j);
            }
        }
    }
}

Generation::Generation(int n_row, int n_col, const Mat& _ecosystem,
                       const Param& _param)
    : simulation_parameters(_param),
      ecosystem(n_row, Row(n_col)),
      n_gen_local(0),
      eco_compress(n_row)
{   
    for(int i = 0; i < sz(_ecosystem); ++i){
        for(int j = 0; j < sz(_ecosystem[i]); ++j){
            EntityType type = _ecosystem[i][j].get_type(); 
            if(type == EntityType::ROCK){
                ecosystem[i][j] = Cell(EntityType::ROCK);
            }
        }
    }
}

Mat& Generation::get_ecosystem() { return ecosystem; }

int Generation::get_n_gen() const { return n_gen_local; }

void Generation::print(){
    cout << simulation_parameters.GEN_PROC_RABBITS << " ";
    cout << simulation_parameters.GEN_PROC_FOXES << " ";
    cout << simulation_parameters.GEN_FOOD_FOXES << " ";
    cout << simulation_parameters.N_GEN - n_gen_local << " ";
    cout << simulation_parameters.R  << " ";
    cout << simulation_parameters.C << " ";

    int count_entity = 0;
    for(int i = 0; i < sz(ecosystem); ++i){
        for(int j = 0; j < sz(ecosystem[i]); ++j){
            EntityType type = ecosystem[i][j].get_type();
            if (type != EntityType::EMPTY)
                count_entity += 1;
        }
    }


    cout << count_entity << "\n";

    for (int i = 0; i < simulation_parameters.R; ++i) {
        for (int j = 0; j < simulation_parameters.C; ++j) {
            EntityType type = ecosystem[i][j].get_type();
            if (type != EntityType::EMPTY)
                cout << WORD[(uint8_t)type] << " " << i << " " << j << "\n";
        }
    }
}

void Generation::print_grid(){
    cout << "Gen " << n_gen_local << "\n";
    for (int i = 0; i < simulation_parameters.C + 2; ++i) cout << '-';
    cout << '\n';

    for (int i = 0; i < simulation_parameters.R; ++i) {
        cout << '|';
        for (int j = 0; j < simulation_parameters.C; ++j) {
            char letter = LETTER[(uint8_t)ecosystem[i][j].get_type()];
            cout << letter;
        }
        cout << "|\n"; 
    }

    for (int i = 0; i < simulation_parameters.C + 2; ++i) cout << '-';
    cout << '\n';
}

Param Generation::get_param() const { return simulation_parameters; }
vector<vector<int>>& Generation::get_eco_compress() { return eco_compress; }
void Generation::increase_generation() { n_gen_local += 1; }
