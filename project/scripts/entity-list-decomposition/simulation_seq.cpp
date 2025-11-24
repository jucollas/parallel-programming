/**
 * @autor: JuCollas 
 * @version: 3.0
 * @date: 16 nov 2025
 * Project Parallel-Programming
 * 
 * Descripción general:
 * --------------------
 * Versión secuencial del simulador de ecosistema con:
 *  - RABBIT (conejos)
 *  - FOX (zorros)
 *  - ROCKS (rocas)
 *
 * El ecosistema evoluciona por generaciones, usando reglas de:
 *  - Movimiento
 *  - Procreación
 *  - Cacería
 *  - Muerte por inanición
 *
 * Se usa un sistema de doble buffer (current / next) para evitar
 * efectos de actualización simultánea. Esta versión es la base
 * para paralelización posterior.
 */

#include <bits/stdc++.h>
#include <filesystem>
#include <chrono>

#include "./include/cell.h"
#include "./include/entity.h"
#include "./include/generation.h"

#define sz(x) (int) x.size()
using namespace std;
namespace fs = std::filesystem;


/** Direcciones cardinales de movimiento */
const vector<pair<int, int>> directions = {{-1, 0}, {0, 1}, {1, 0}, {0, -1}};

int N_ROW, N_COL;

/**
 * @brief Verifica si una posición está dentro de la matriz.
 * @param x Fila
 * @param y Columna
 * @return true si (x, y) es válida; false de lo contrario
 */
bool isValid(int x, int y){
    return 0 <= x && x < N_ROW && 0 <= y && y < N_COL;
}

/**
 * @brief Calcula el siguiente movimiento del zorro.
 * 
 * Reglas:
 *  - Prioridad 1: moverse hacia un conejo adyacente.
 *  - Prioridad 2: moverse hacia una celda vacía.
 *  - Si no hay movimientos posibles, se queda quieto.
 *
 * @param act Zorro actual
 * @param ecosystem Matriz del ecosistema
 * @param G Generación global actual
 * @return Par (fila, columna) representando el nuevo movimiento
 */
pair<int, int> step_fox(Entity& act, Mat& ecosystem, const int G){
  const int x = act.get_x();
  const int y = act.get_y();
  vector<pair<int, int>> possibles;
  for(int i = 0; i < sz(directions); ++i){
    const int dx = directions[i].first; 
    const int dy = directions[i].second;
    const int u = x + dx;
    const int v = y + dy;
    if(isValid(u, v) && ecosystem[u][v].get_type() == EntityType::RABBIT){
      possibles.push_back({u, v});
    }
  }

  if(possibles.empty()){
    for(int i = 0; i < sz(directions); ++i){
      int dx = directions[i].first; 
      int dy = directions[i].second;
      int u = x + dx;
      int v = y + dy;
      if(isValid(u, v) && ecosystem[u][v].get_type() == EntityType::EMPTY){
        possibles.push_back({u, v});
      }
    }
  }
  if(possibles.empty()) return {x, y};
  int ind = (G + x + y) % sz(possibles);
  return possibles[ind];
}

/**
 * @brief Mueve todos los zorros en la generación actual.
 * 
 * Funciones principales:
 *  - Mover zorros según step_fox()
 *  - Comer conejos
 *  - Incrementar hambre y matar si llegan al límite
 *  - Procrear si cumplen condiciones
 *  - Resolver conflictos entre zorros en la misma celda
 *
 * @param current Generación actual
 * @param next Generación siguiente
 * @param current_gen Número de generación global
 */
void move_foxes(Generation* current, Generation* next, int current_gen){
  vector<Entity>& current_foxes = current->get_foxes();
  vector<Entity>& next_foxes = next->get_foxes();

  Mat& current_ecosystem = current->get_ecosystem();
  Mat& next_ecosystem = next->get_ecosystem();

  set<int> index_rabbits_no_include;

  for(int i = 0; i < sz(current_foxes); ++i){
    Entity& act = current_foxes[i];
    
    pair<int, int> mv = step_fox(act, current_ecosystem, current_gen);
    bool moved = !(act.get_x() == mv.first && act.get_y() == mv.second);
    int new_proc = act.get_local_proc() + 1;
    bool abj_rabbit = current_ecosystem[mv.first][mv.second].get_type() == EntityType::RABBIT;
    
    if(act.get_local_food() + 1 >= current->get_param_food_foxes() && !abj_rabbit)
      continue;

    bool can_procreate = (act.get_local_proc() >= current->get_param_proc_foxes()) && moved;
    if(can_procreate){
      new_proc = 0;
      Entity baby(
        act.get_x(),
        act.get_y(),
        current_gen + 1,
        0,
        0
      );
      next_foxes.push_back(baby);
      next_ecosystem[act.get_x()][act.get_y()] = Cell(
        EntityType::FOX, sz(next_foxes) - 1
      );
    }

    Entity new_step(
      mv.first,
      mv.second, 
      current_gen + 1,
      new_proc,
      act.get_local_food() + 1
    );
    Cell& dest = next_ecosystem[new_step.get_x()][new_step.get_y()];

    if(dest.get_type() == EntityType::FOX){
      const int ind = dest.get_index();
      const int local_proc = next_foxes[ind].get_local_proc();
      const int local_food = next_foxes[ind].get_local_food();
      if(new_step.get_local_proc() > local_proc){
        next_foxes[ind] = new_step;
      }else if(new_step.get_local_proc() == local_proc 
            && new_step.get_local_food() < local_food){
        next_foxes[ind] = new_step;
      }
    }else{
      Cell& current_dest = current_ecosystem[new_step.get_x()][new_step.get_y()];
      if(current_dest.get_type() == EntityType::RABBIT){
        const int ind = current_dest.get_index();
        index_rabbits_no_include.insert(ind);
        new_step.set_local_food(0);
      }
      next_foxes.push_back(new_step);
      dest = Cell(EntityType::FOX, sz(next_foxes) - 1);
    }
  }

  vector<Entity>& current_rabbits = current->get_rabbits();
  vector<Entity>& next_rabbits = next->get_rabbits();

  for(int i = 0; i < sz(current_rabbits); ++i){
    Entity& act = current_rabbits[i];
    if (!index_rabbits_no_include.count(i)) {
      next_rabbits.push_back(act);
      next_ecosystem[act.get_x()][act.get_y()] = Cell(EntityType::RABBIT, sz(next_rabbits) - 1);
    }
  }
  current->clean();
}

/**
 * @brief Calcula el movimiento del conejo.
 * 
 * Reglas:
 *  - Solo puede moverse a una celda vacía.
 *  - Si múltiples opciones, se elige determinísticamente usando (G + x + y).
 *  - Si no hay movimiento posible, permanece quieto.
 *
 * @param act Conejo actual
 * @param eco Matriz del ecosistema
 * @param G Generación global
 * @return Nuevo par (fila, columna)
 */
pair<int,int> step_rabbit(Entity& act, Mat& eco, int G){
  const int x = act.get_x();
  const int y = act.get_y();

  vector<pair<int,int>> possibles;
  for (int i = 0; i < sz(directions); ++i) {
    int dx = directions[i].first;
    int dy = directions[i].second;
    int u = x + dx;
    int v = y + dy;
    if (isValid(u, v) && eco[u][v].get_type() == EntityType::EMPTY) {
      possibles.push_back({u, v});
    }
  }

  if (possibles.empty()) return {x, y};

  int P = sz(possibles);
  int idx = (G + x + y) % P;
  return possibles[idx];
}

/**
 * @brief Mueve todos los conejos en la generación actual.
 * 
 * Funciones principales:
 *  - Mover conejos según step_rabbit()
 *  - Procrear si cumplen condiciones
 *  - Resolver conflictos entre conejos en la misma celda
 *  - Mantener a los zorros sin modificar
 *
 * @param current Generación actual
 * @param next Generación siguiente
 * @param current_gen Número de generación global
 */
void move_rabbits(Generation* current, Generation* next, int current_gen){
  Mat& current_ecosystem = current->get_ecosystem();
  Mat& next_ecosystem    = next->get_ecosystem();

  vector<Entity>& current_rabbits = current->get_rabbits();
  vector<Entity>& next_rabbits    = next->get_rabbits();

  for (int i = 0; i < sz(current_rabbits); ++i) {
    Entity act = current_rabbits[i];

    pair<int,int> mv = step_rabbit(act, current_ecosystem, current_gen);
    bool moved = !(mv.first == act.get_x() && mv.second == act.get_y());

    int new_proc = act.get_local_proc() + 1;

    bool can_procreate = (act.get_local_proc() >= current->get_param_proc_rabbits()) && moved;
    if (can_procreate) {
      new_proc = 0;

      Entity baby(
          act.get_x(),
          act.get_y(),
          current_gen + 1,
          0, 0);
      next_rabbits.push_back(baby);
      next_ecosystem[act.get_x()][act.get_y()] = Cell(
          EntityType::RABBIT, sz(next_rabbits) - 1
      );
    }

    Entity new_step(
        mv.first,
        mv.second,
        current_gen + 1,
        new_proc, 0);

    Cell& dest = next_ecosystem[new_step.get_x()][new_step.get_y()];
    if (dest.get_type() == EntityType::RABBIT) {
      int ind = dest.get_index();
      if (new_step.get_local_proc() > next_rabbits[ind].get_local_proc()) {
        next_rabbits[ind] = new_step;
      }
    } else if (dest.get_type() == EntityType::EMPTY) {
      next_rabbits.push_back(new_step);
      dest = Cell(EntityType::RABBIT, sz(next_rabbits) - 1);
    }
  }

  vector<Entity>& current_foxes = current->get_foxes();
  vector<Entity>& next_foxes    = next->get_foxes();

  for (int i = 0; i < sz(current_foxes); ++i) {
    Entity& act = current_foxes[i];
    next_foxes.push_back(act);
    next_ecosystem[act.get_x()][act.get_y()] =
        Cell(EntityType::FOX, sz(next_foxes) - 1);
  }
  current->clean();
}

/**
 * @brief Ejecuta la simulación completa.
 *
 * Internamente:
 *  - Alterna entre dos buffers (first, second)
 *  - Realiza los pasos:
 *      1. move_rabbits()
 *      2. swap()
 *      3. move_foxes()
 *      4. swap()
 *  - Actualiza número de generación
 *
 * @param N_GEN Total de generaciones a ejecutar
 * @param start Generación inicial
 * @return Generación final tras N_GEN pasos
 */
Generation simulation(int N_GEN, Generation& start){
  Generation first(start);
  Generation second(N_ROW, N_COL, start.get_rocks(), start.get_param());

  Generation* current = &first;
  Generation* next    = &second;

  int current_gen = start.get_n_gen();
  current->set_n_gen(current_gen);

  for (int step = 0; step < N_GEN; ++step) {
    move_rabbits(current, next, current_gen);
    swap(current, next);
    move_foxes(current, next, current_gen);
    swap(current, next);
    ++current_gen;
    next->set_n_gen(current_gen);
    current->set_n_gen(current_gen);
  }
  return *current;
}

/**
 * @brief Punto de entrada principal.
 *
 * Entrada:
 *  - GEN_PROC_RABBITS    generaciones para que un conejo procree
 *  - GEN_PROC_FOXES      generaciones para que un zorro procree
 *  - GEN_FOOD_FOXES      límite de hambre del zorro
 *  - N_GEN               número total de generaciones a simular
 *  - N_ROW, N_COL        tamaño de la matriz
 *  - N                   número de entidades iniciales
 *  - Luego lista de entidades (tipo, x, y)
 *
 * Salida:
 *  - Estado final del ecosistema tras la simulación
 *
 */
int main(){
  int GEN_PROC_RABBITS; cin >> GEN_PROC_RABBITS;
  int GEN_PROC_FOXES; cin >> GEN_PROC_FOXES;
  int GEN_FOOD_FOXES; cin >> GEN_FOOD_FOXES;
  int N_GEN; cin >> N_GEN;
  cin >> N_ROW >> N_COL;
  int N; cin >> N;
  
  Param param(GEN_PROC_RABBITS, GEN_PROC_FOXES, GEN_FOOD_FOXES, N_GEN, N_ROW, N_COL);
  Mat ecosystem(N_ROW, Row(N_COL));
  vector<Entity> rabbits, foxes, rocks;
  
  for (int i = 0; i < N; ++i) {
    string name_type; 
    int x, y;
    cin >> name_type >> x >> y;
    
    if (name_type == "RABBIT") {
      rabbits.emplace_back(x, y);
      ecosystem[x][y] = Cell(EntityType::RABBIT, sz(rabbits) - 1);
    } else if (name_type == "FOX") {
      foxes.emplace_back(x, y);
      ecosystem[x][y] = Cell(EntityType::FOX, sz(foxes) - 1);
    } else if (name_type == "ROCK") {
      rocks.emplace_back(x, y);
      ecosystem[x][y] = Cell(EntityType::ROCK, sz(rocks) - 1);
    }
  }
  
  Generation start(0, ecosystem, rocks, rabbits, foxes, param);
  Generation end_state = simulation(N_GEN, start);
  end_state.print();
    return 0;
}
