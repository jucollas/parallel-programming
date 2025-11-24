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

Generation::Generation(int n, int m, const vector<Entity>& _rocks, const Param& _param)
    : simulation_parameters(_param),
      ecosystem(n, Row(m)),
      rabbits(),
      foxes(),
      rocks(_rocks),
      n_gen_local(0)
{
    for (int i = 0; i < sz(_rocks); ++i) {
        ecosystem[_rocks[i].get_x()][_rocks[i].get_y()] = Cell(EntityType::ROCK, i);
    }
}


Generation::Generation(int _gen, const Mat& _ecosystem,
                       const vector<Entity>& _rocks,
                       const vector<Entity>& _rabbits,
                       const vector<Entity>& _foxes,
                       const Param& _param)
    : simulation_parameters(_param),
      ecosystem(_ecosystem),
      rabbits(_rabbits),
      foxes(_foxes),
      rocks(_rocks),
      n_gen_local(_gen)
{}


const vector<Entity>& Generation::get_rocks() const { return rocks; }

vector<Entity>& Generation::get_rabbits() { return rabbits; }

vector<Entity>& Generation::get_foxes() { return foxes; }

Mat& Generation::get_ecosystem() { return ecosystem; }

int Generation::get_n_gen() const { return n_gen_local; }
void Generation::set_n_gen(int _n_gen) { n_gen_local = _n_gen; }

void Generation::clean(){
    for (int i = 0; i < sz(rabbits); ++i) {
        Entity& act = rabbits[i];
        ecosystem[act.get_x()][act.get_y()] = Cell();
    }
    rabbits.clear();

    for (int i = 0; i < sz(foxes); ++i) {
        Entity& act = foxes[i];
        ecosystem[act.get_x()][act.get_y()] = Cell();
    }
    foxes.clear();
}

void Generation::print(){
    cout << simulation_parameters.GEN_PROC_RABBITS << " ";
    cout << simulation_parameters.GEN_PROC_FOXES << " ";
    cout << simulation_parameters.GEN_FOOD_FOXES << " ";
    cout << simulation_parameters.N_GEN - n_gen_local << " ";
    cout << simulation_parameters.R  << " ";
    cout << simulation_parameters.C << " "; 
    cout << sz(rabbits) + sz(foxes) + sz(rocks) << "\n";

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

int Generation::get_param_food_foxes() const { return simulation_parameters.GEN_FOOD_FOXES; }
int Generation::get_param_proc_rabbits() const { return simulation_parameters.GEN_PROC_RABBITS; }
int Generation::get_param_proc_foxes() const { return simulation_parameters.GEN_PROC_FOXES; }
Param Generation::get_param() const { return simulation_parameters; }
