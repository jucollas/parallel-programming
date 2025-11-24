#include <bits/stdc++.h>
#include <thread>
#include <chrono>

#include "./include/cell.h"
#include "./include/entity.h"
#include "./include/generation.h"

#define sz(x) (int) x.size()
using namespace std;

const vector<pair<int, int>> directions = {{-1, 0}, {0, 1}, {1, 0}, {0, -1}};

const map<string, EntityType> str_to_type = {
  {"RABBIT", EntityType::RABBIT},
  {"FOX",    EntityType::FOX},
  {"ROCK",   EntityType::ROCK},
};

int N_ROW, N_COL;

void delete_single_row(const int id_row, Mat& ecosystem){
  for(int i = 0; i < sz(ecosystem[id_row]); ++i){
    Cell& cell = ecosystem[id_row][i];
    EntityType type = cell.get_type(); 
    if(type == EntityType::RABBIT || type == EntityType::FOX){
      delete(cell.get_entity());
      cell.set_entity(nullptr);
      cell.set_type(EntityType::EMPTY);
    }
  }
}

void delete_entities(Mat& ecosystem){
  const int n = sz(ecosystem);
  for(int row = 0; row < n; ++row){
    delete_single_row(row, ecosystem);
  }
}

void modify_ecosystem(const EntityType type, Entity* entity, Mat& ecosystem){
  Cell& dest = ecosystem[entity->get_x()][entity->get_y()];

  if(dest.get_type() == type){
    Entity* occupant = dest.get_entity();
    const int proc_occup = occupant->get_local_proc();
    const int food_occup = occupant->get_local_food();
    const int proc_new   = entity->get_local_proc();
    const int food_new   = entity->get_local_food();

    bool replace = false;

    if(proc_new > proc_occup) replace = true;
    else if(proc_new == proc_occup && food_new < food_occup) replace = true;

    if(replace){
      delete occupant;
      dest.set_entity(entity);
      dest.set_type(type);
    }else{
      delete entity;
    }
  }else if(dest.get_type() == EntityType::EMPTY){
    dest.set_entity(entity);
    dest.set_type(type);
  }else{
    delete entity;
  }
}

void modify_ecosystem_copy(const EntityType type, Entity* entity, Mat& ecosystem){
  Cell& dest = ecosystem[entity->get_x()][entity->get_y()];
  if(dest.get_type() == EntityType::EMPTY){
    dest.set_entity(entity);
    dest.set_type(type);
  }else{
    delete entity;
  }
}

bool isValid(const int x, const int y){
    return 0 <= x && x < N_ROW && 0 <= y && y < N_COL;
}

pair<int, int> step_fox(const int x, const int y, const Mat& ecosystem, const int G){
  vector<pair<int, int>> possibles;
  for(int i = 0; i < sz(directions); ++i){
    int u = x + directions[i].first;
    int v = y + directions[i].second;
    if(isValid(u, v) && ecosystem[u][v].get_type() == EntityType::RABBIT){
      possibles.push_back({u, v});
    }
  }

  if(possibles.empty()){
    for(int i = 0; i < sz(directions); ++i){
      int u = x + directions[i].first;
      int v = y + directions[i].second;
      if(isValid(u, v) && ecosystem[u][v].get_type() == EntityType::EMPTY){
        possibles.push_back({u, v});
      }
    }
  }
  if(possibles.empty()) return {x, y};

  const int P = sz(possibles);
  const int ind = (G + x + y) % P;
  return possibles[ind];
}

void move_single_fox(const int x, const int y, const int G,
                     const Mat& current, Mat& next, const Param& param ){    
  const Entity& act = *current[x][y].get_entity();
  const pair<int, int> step = step_fox(x, y, current, G);

  const bool moved = !(act.get_x() == step.first && act.get_y() == step.second);
  const bool abj_rabbit = current[step.first][step.second].get_type() == EntityType::RABBIT;

  int update_proc = act.get_local_proc() + 1;
  int update_food = act.get_local_food() + 1;
  
  if(update_food >= param.GEN_FOOD_FOXES && !abj_rabbit)
    return;

  const bool can_procreate = (act.get_local_proc() >= param.GEN_PROC_FOXES) && moved;
  if(can_procreate){
    update_proc = 0;
    Entity* baby = new Entity(act.get_x(), act.get_y());
    modify_ecosystem(EntityType::FOX, baby, next); 
  }

  if(abj_rabbit) update_food = 0;

  Entity* act_mov = new Entity(step.first, step.second, update_proc, update_food);
  modify_ecosystem(EntityType::FOX, act_mov, next); 
}

pair<int,int> step_rabbit(const int x, const int y, const Mat& ecosystem, const int G){
  vector<pair<int,int>> possibles;
  for (int i = 0; i < sz(directions); ++i) {
    const int u = x + directions[i].first;
    const int v = y + directions[i].second;
    if (isValid(u, v) && ecosystem[u][v].get_type() == EntityType::EMPTY) {
      possibles.push_back({u, v});
    }
  }

  if (possibles.empty()) return {x, y};

  const int P = sz(possibles);
  const int idx = (G + x + y) % P;
  return possibles[idx];
}

void move_single_rabbit(int x, int y, int G,
                        const Mat& current, Mat& next, const Param& param){
  const Entity& act = *current[x][y].get_entity();
  const pair<int,int> step = step_rabbit(x, y, current, G);
  const bool moved = !(step.first == act.get_x() && step.second == act.get_y());

  int update_proc = act.get_local_proc() + 1;

  const bool can_procreate = (act.get_local_proc() >= param.GEN_PROC_RABBITS) && moved;
  if (can_procreate) {
    update_proc = 0;
    Entity* baby = new Entity(act.get_x(), act.get_y());
    modify_ecosystem(EntityType::RABBIT, baby, next); 
  }

  Entity* act_mov = new Entity(step.first, step.second, update_proc);
  modify_ecosystem(EntityType::RABBIT, act_mov, next); 
}

void copy_entity(const int x, const int y,
                 const Mat& current, Mat& next){
  Entity* entity = new Entity(*current[x][y].get_entity());
  EntityType type = current[x][y].get_type();
  modify_ecosystem_copy(type, entity, next);
}

void move_sigle_entity(const int x, const int y, const int G,
                       const EntityType turn, const Mat& current,
                       Mat& next, const Param& param){
  EntityType type = current[x][y].get_type();

  if(type == EntityType::ROCK || type == EntityType::EMPTY) return;
  
  if(turn == EntityType::RABBIT){
    if(type == EntityType::RABBIT){
      move_single_rabbit(x, y, G, current, next, param);
    }else if(type == EntityType::FOX){
      copy_entity(x, y, current, next);
    }
  }else if(turn == EntityType::FOX){
    if(type == EntityType::FOX){
      move_single_fox(x, y, G, current, next, param);
    }else if(type == EntityType::RABBIT){
      copy_entity(x, y, current, next);
    }
  }
}

void move_single_row(const int id_row, const int G, const EntityType turn,
                     const Mat& current, Mat& next, const Param& param){
  for(int i = 0; i < sz(current[id_row]); ++i){
    move_sigle_entity(id_row, i, G, turn, current, next, param);
  }
}

void move_entities(Generation* current, Generation* next, const EntityType turn){
  Mat& current_ecosystem = current->get_ecosystem();
  Mat& next_ecosystem    = next->get_ecosystem();
  
  const Param& param = current->get_param();
  const int current_gen = current->get_n_gen();

  for(int row = 0; row < sz(current_ecosystem); ++row){
    move_single_row(row, current_gen, turn, current_ecosystem, next_ecosystem, param);
  }

  delete_entities(current_ecosystem);
}

void step_once(Generation*& current, Generation*& next){
  move_entities(current, next, EntityType::RABBIT);
  swap(current, next);
  move_entities(current, next, EntityType::FOX);
  swap(current, next);
  next->increase_generation();
  current->increase_generation();
}

void compute_stats(Generation& g, int& rabbits, int& foxes, int& rocks){
  rabbits = foxes = rocks = 0;
  Mat& eco = g.get_ecosystem();
  for(int i = 0; i < sz(eco); ++i){
    for(int j = 0; j < sz(eco[i]); ++j){
      EntityType t = eco[i][j].get_type();
      if      (t == EntityType::RABBIT) ++rabbits;
      else if (t == EntityType::FOX)    ++foxes;
      else if (t == EntityType::ROCK)   ++rocks;
    }
  }
}

void render_step(Generation& g, int step,
                 int rabbits, int foxes, int rocks, int delay_ms){
  system("clear");
  cout << "Generacion global: " << g.get_n_gen()
       << "  (step local: " << step << ")\n";
  g.print_grid();
  cout << "Rabbits: " << rabbits
       << " | Foxes: "  << foxes
       << " | Rocks: "  << rocks << "\n";
  cout << "Velocidad actual: " << delay_ms << " ms entre pasos\n";
  cout << "Comando [Enter=seguir, q=salir, +=mas rapido, -=mas lento]: ";
}

void dump_stats_csv(const vector<int>& vr,
                    const vector<int>& vf,
                    const vector<int>& vk,
                    const string& filename){
  ofstream out(filename);
  if(!out.is_open()) return;
  out << "step,rabbits,foxes,rocks\n";
  for(int i = 0; i < sz(vr); ++i){
    out << i << "," << vr[i] << "," << vf[i] << "," << vk[i] << "\n";
  }
}

void run_viewer(Generation& start, int N_GEN){
  Generation first(start);
  Generation second(N_ROW, N_COL, start.get_ecosystem(), start.get_param());
  Generation* current = &first;
  Generation* next    = &second;

  int delay_ms = 500;
  vector<int> hist_r, hist_f, hist_k;

  cin.ignore(numeric_limits<streamsize>::max(), '\n');

  for(int step = 0; step < N_GEN; ++step){
    int rabbits, foxes, rocks;
    compute_stats(*current, rabbits, foxes, rocks);
    hist_r.push_back(rabbits);
    hist_f.push_back(foxes);
    hist_k.push_back(rocks);

    render_step(*current, step, rabbits, foxes, rocks, delay_ms);

    string cmd;
    getline(cin, cmd);
    if(cmd == "q") break;
    if(cmd == "+") delay_ms = max(10, delay_ms / 2);
    if(cmd == "-") delay_ms = min(2000, delay_ms * 2);

    std::this_thread::sleep_for(std::chrono::milliseconds(delay_ms));
    step_once(current, next);
  }

  dump_stats_csv(hist_r, hist_f, hist_k, "stats_viewer.csv");

  cout << "\nSimulacion terminada. Estadisticas guardadas en stats_viewer.csv\n";
}

int main(){
  int GEN_PROC_RABBITS; cin >> GEN_PROC_RABBITS;
  int GEN_PROC_FOXES;   cin >> GEN_PROC_FOXES;
  int GEN_FOOD_FOXES;   cin >> GEN_FOOD_FOXES;
  int N_GEN;            cin >> N_GEN;
  cin >> N_ROW >> N_COL;
  int N; cin >> N;
  
  Param param(GEN_PROC_RABBITS, GEN_PROC_FOXES, GEN_FOOD_FOXES,
              N_GEN, N_ROW, N_COL);
  Mat ecosystem(N_ROW, Row(N_COL));
  
  for (int i = 0; i < N; ++i) {
    string name_type; cin >> name_type; 
    int x, y; cin >> x >> y;
    EntityType type = str_to_type.at(name_type);
    Entity* entity = nullptr;
    if(type != EntityType::ROCK)
      entity = new Entity(x, y);
    ecosystem[x][y] = Cell(type, entity);
  }
  
  Generation start(ecosystem, param);
  run_viewer(start, N_GEN);
  return 0;
}
