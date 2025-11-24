/**
 * @autor: JuCollas 
 * @version: 4.0
 * @date: 17 nov 2025
 * Project Parallel-Programming
 * 
 * Descripción general:
 * --------------------
 * Versión paralela del simulador de ecosistema con:
 *  - RABBIT (conejos)
 *  - FOX (zorros)
 *  - ROCKS (rocas)
 *
 * Esta versión extiende la lógica secuencial añadiendo paralelismo
 * mediante hilos POSIX (pthread), permitiendo distribuir el trabajo
 * de actualización del ecosistema entre múltiples núcleos.
 *
 * Mecánica general:
 *  - Se divide el conjunto de entidades (conejos y zorros) en bloques
 *    que son procesados por diferentes hilos.
 *  - Los hilos ejecutan funciones independientes para movimiento,
 *    reproducción, cacería y resolución de conflictos.
 *  - Se usan mutex para proteger secciones críticas como:
 *        * Inserción en vectores de entidades
 *        * Escritura en next_ecosystem
 *        * Operaciones sobre índices compartidos
 *
 * Reglas del ecosistema (idénticas a la versión secuencial):
 *  - Movimiento según prioridades (caza → espacio libre)
 *  - Procreación bajo condiciones de edad y movimiento
 *  - Muerte por inanición según GEN_FOOD_FOXES
 *  - Cacería: el zorro elimina conejos de células adyacentes o destino
 *
 * Arquitectura:
 *  - Sistema de doble buffer (current / next) para evitar condiciones
 *    de actualización simultánea.
 *  - Paralelismo explícito con pthread_create() y pthread_join().
 *  - Resolución de conflictos concurrentes mediante mutex y lógica
 *    determinista basada en antigüedad y hambre.
 *
 * Objetivo:
 *  - Acelerar significativamente la simulación manteniendo resultados
 *    consistentes respecto a la versión secuencial.
 */

#include <bits/stdc++.h>
#include <pthread.h>
#include <filesystem>
#include <chrono>

#include "./include/cell.h"
#include "./include/entity.h"
#include "./include/generation.h"

#define sz(x) (int) x.size()
using namespace std;
namespace fs = std::filesystem;


/** Numero de hilos */
const int N_THREADS = 8;

/** Direcciones cardinales de movimiento */
const vector<pair<int, int>> directions = {{-1, 0}, {0, 1}, {1, 0}, {0, -1}};

int N_ROW, N_COL;

/* Datos para los hilos*/
class data_t{
  public:
    int start, end;
    int current_gen;
    vector<Entity>* current_rabbits;
    vector<Entity>* next_rabbits;
    vector<Entity>* current_foxes;
    vector<Entity>* next_foxes;
    Mat* current_ecosystem;
    Mat* next_ecosystem;
    const Param* param;
    
    vector<Entity>* current_entity;
    vector<Entity>* next_entity;
    EntityType type;

    set<int>* no_include;

  data_t() : 
    start(0), 
    end(0), 
    current_gen(0), 
    current_rabbits(nullptr), 
    next_rabbits(nullptr),
    current_foxes(nullptr),
    next_foxes(nullptr), 
    current_entity(nullptr),
    current_ecosystem(nullptr), 
    next_ecosystem(nullptr), 
    param(nullptr) {}

  data_t(int _start, int _end, int _current_gen, vector<Entity>* _current_rabbits, Mat* _current_ecosystem, vector<Entity>* _next_rabbits, Mat* _next_ecosytem, const Param* _param) : 
    start(_start), 
    end(_end), 
    current_gen(_current_gen),
    current_rabbits(_current_rabbits),
    current_ecosystem(_current_ecosystem),
    next_rabbits(_next_rabbits),
    next_ecosystem(_next_ecosytem),
    param(_param) {}

  data_t(int _start, int _end, Mat* _next_ecosystem,  vector<Entity>* _current_entity, vector<Entity>* _next_entity, set<int>* _no_include, EntityType _type):
    start(_start),
    end(_end),
    next_ecosystem(_next_ecosystem),
    current_entity(_current_entity),
    next_entity(_next_entity),
    no_include(_no_include),
    type(_type) {}

  data_t(int _start, int _end, Mat* _current_ecosystem, vector<Entity>* _current_entity) :
    start(_start),
    end(_end),
    current_ecosystem(_current_ecosystem),
    current_entity(_current_entity) {}
  data_t(
    int _start, 
    int _end,
    int _current_gen, 
    Mat* _current_ecosystem,  
    Mat* _next_ecosystem, 
    vector<Entity>* _current_foxes,
    vector<Entity>* _next_foxes, 
    set<int>* _no_include, 
    const Param* _param ) :
    start(_start),
    end(_end),
    current_gen(_current_gen),
    current_ecosystem(_current_ecosystem),
    current_foxes(_current_foxes),
    next_foxes(_next_foxes),
    next_ecosystem(_next_ecosystem),
    no_include(_no_include), 
    param(_param) {}
};

/* Hilos y Datos*/
pthread_t threads[N_THREADS];
data_t thdata[N_THREADS];

/* Semaforos */
pthread_mutex_t rabbits_mod, foxes_mod, copy_ent;

/**
 * @brief Copia entidades desde el buffer actual al buffer siguiente de manera paralela.
 *
 * Cada hilo copia un rango de entidades hacia la matriz next_ecosystem,
 * siempre y cuando su índice no esté marcado en el conjunto no_include.
 * El acceso concurrente a next_entity y next_ecosystem se protege con un mutex.
 *
 * @param args Puntero a estructura data_t con rangos e información compartida
 * @return nullptr
 */
void* copy_entity_mult(void * args){
  data_t * data = (data_t*) args;
  Mat& next_ecosystem = (*data->next_ecosystem);
  vector<Entity>& current_entity = (*data->current_entity);
  vector<Entity>& next_entity = (*data->next_entity);
  set<int>& no_include = (*data->no_include);
  EntityType type = data->type;
  for(int i = data->start; i < data->end; ++i){
    Entity& act = current_entity[i];
    if(no_include.empty() || !no_include.count(i)){
      pthread_mutex_lock(&copy_ent);
      next_entity.push_back(act);
      next_ecosystem[act.get_x()][act.get_y()] = Cell(type, sz(next_entity) - 1);
      pthread_mutex_unlock(&copy_ent);
    }
  }
  return NULL;
}

/**
 * @brief Ejecuta la copia paralela de entidades hacia el ecosistema next.
 *
 * Divide el vector de entidades en bloques y lanza múltiples hilos para copiar
 * sus posiciones originales en next_ecosystem, excluyendo aquellos índices
 * presentes en el conjunto no_include.
 *
 * @param next_ecosystem Matriz destino
 * @param current_entity Entidades actuales
 * @param next_entity Vector destino donde se almacenan las copias
 * @param no_include Índices de entidades que no deben copiarse
 * @param type Tipo de entidad a escribir en las celdas
 */
void copy_entity(Mat& next_ecosystem, vector<Entity>& current_entity, vector<Entity>& next_entity, set<int>& no_include,  EntityType type){
  int n = sz(current_entity);
  int num_threads = min(N_THREADS, n);

  int len_chunk = n / num_threads;
  int rem = n % num_threads;
  int act = 0;
  for(int i = 0; i < num_threads; ++i){
    int mv = (i < rem) ? 1 : 0;
    int to = act + len_chunk + mv;
    thdata[i] = data_t(act, to, &next_ecosystem, &current_entity, &next_entity, &no_include, type);
    pthread_create(&threads[i], nullptr, copy_entity_mult, (void*) &thdata[i]);
    act = to;
  }

  for(int i = 0; i < num_threads; ++i){
    pthread_join(threads[i], nullptr);
  }
}

/**
 * @brief Limpia entidades dentro de la matriz actual en paralelo.
 *
 * Marca como vacía cada celda asociada a las entidades dentro del rango
 * procesado por el hilo (start → end).
 *
 * @param args Puntero a estructura data_t con datos del hilo
 * @return nullptr
 */
void* delete_entity_mult(void * args){
  data_t * data = (data_t*) args;
  Mat& current_ecosystem = (*data->current_ecosystem);
  vector<Entity>& current_entity = (*data->current_entity);
  for(int i = data->start; i < data->end; ++i){
    Entity& act = current_entity[i];
    current_ecosystem[act.get_x()][act.get_y()] = Cell(EntityType::EMPTY, -1);
  }
  return NULL;
}

/**
 * @brief Elimina todas las entidades del ecosistema actual mediante paralelización.
 *
 * Cada hilo marca como vacías las celdas ocupadas por un subconjunto
 * de entidades, y al final el vector current_entity se vacía.
 *
 * @param current_ecosystem Matriz actual del ecosistema
 * @param current_entity Vector de entidades a eliminar
 */
void delete_entity(Mat& current_ecosystem, vector<Entity>& current_entity){
  int n = sz(current_entity);
  int num_threads = min(N_THREADS, n);

  int len_chunk = n / num_threads;
  int rem = n % num_threads;
  int act = 0;
  for(int i = 0; i < num_threads; ++i){
    int mv = (i < rem) ? 1 : 0;
    int to = act + len_chunk + mv;
    thdata[i] = data_t(act, to, &current_ecosystem, &current_entity);
    pthread_create(&threads[i], nullptr, delete_entity_mult, (void*) &thdata[i]);
    act = to;
  }

  for(int i = 0; i < num_threads; ++i){
    pthread_join(threads[i], nullptr);
  }
  current_entity.clear();
}

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
 * @brief Aplica todas las reglas de movimiento, cacería y procreación para un zorro.
 *
 * Funcionalidad:
 *  - Calcula el movimiento del zorro mediante step_fox().
 *  - Maneja la muerte por hambre según GEN_FOOD_FOXES.
 *  - Marca conejos a eliminar si el zorro los caza.
 *  - Procrea si cumple GEN_PROC_FOXES y se movió.
 *  - Escribe en next_ecosystem con protección por mutex.
 *
 * @param act Zorro actual
 * @param current_gen Número de generación global
 * @param current_ecosystem Ecosistema actual
 * @param next_ecosystem Ecosistema siguiente
 * @param next_foxes Vector donde se guardan los zorros resultantes
 * @param index_rabbits_no_include Conjunto de índices de conejos a eliminar
 * @param param Parámetros globales de la simulación
 */
void move_single_fox(Entity& act, int current_gen, Mat& current_ecosystem,  Mat& next_ecosystem, vector<Entity>& next_foxes, set<int>& index_rabbits_no_include, const Param& param ){    
  pair<int, int> mv = step_fox(act, current_ecosystem, current_gen);
  bool moved = !(act.get_x() == mv.first && act.get_y() == mv.second);
  int new_proc = act.get_local_proc() + 1;
  bool abj_rabbit = current_ecosystem[mv.first][mv.second].get_type() == EntityType::RABBIT;
  
  if(act.get_local_food() + 1 >= param.GEN_FOOD_FOXES && !abj_rabbit)
    return;

  bool can_procreate = (act.get_local_proc() >= param.GEN_PROC_FOXES) && moved;
  if(can_procreate){
    new_proc = 0;
    Entity baby(
      act.get_x(),
      act.get_y(),
      current_gen + 1,
      0,
      0
    );
    
    pthread_mutex_lock(&foxes_mod);
    next_foxes.push_back(baby);
    next_ecosystem[act.get_x()][act.get_y()] = Cell(
      EntityType::FOX, sz(next_foxes) - 1
    );
    pthread_mutex_unlock(&foxes_mod);
  }

  Entity new_step(
    mv.first,
    mv.second, 
    current_gen + 1,
    new_proc,
    act.get_local_food() + 1
  );

  pthread_mutex_lock(&foxes_mod);
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
  pthread_mutex_unlock(&foxes_mod);
}

/**
 * @brief Hilo encargado de procesar un subconjunto de zorros.
 *
 * Cada hilo aplica move_single_fox() a los zorros cuyo índice
 * esté dentro del rango [start, end).
 *
 * @param args Puntero a estructura data_t con datos del hilo
 * @return nullptr
 */
void* move_mult_foxes(void* args){
  data_t* data = (data_t*) args;
  Mat& current_ecosystem = (*data->current_ecosystem);
  Mat& next_ecosystem = (*data->next_ecosystem);
  vector<Entity>& next_foxes = (*data->next_foxes);
  set<int>& no_include = (*data->no_include);
  const Param param = (*data->param);
  int current_gen = (data->current_gen);
  for(int i = data->start; i < data->end; ++i){
    Entity& act = (*data->current_foxes)[i];
    move_single_fox(act, current_gen, current_ecosystem, next_ecosystem, next_foxes, no_include, param);
  }
  return NULL;
}

/**
 * @brief Controlador principal del movimiento de todos los zorros.
 *
 * Funcionalidad:
 *  - Divide los zorros en bloques y lanza hilos move_mult_foxes().
 *  - Espera a que todos los hilos terminen.
 *  - Marca conejos cazados para no copiarlos al siguiente buffer.
 *  - Copia los conejos que sobreviven.
 *  - Limpia el buffer actual de zorros y conejos.
 *
 * @param current Generación actual
 * @param next Generación siguiente
 * @param current_gen Generación global
 */
void move_foxes(Generation* current, Generation* next, int current_gen){
  Mat& current_ecosystem = current->get_ecosystem();
  Mat& next_ecosystem    = next->get_ecosystem();
  
  const Param& param = current->get_param();
  
  vector<Entity>& current_foxes = current->get_foxes();
  vector<Entity>& next_foxes    = next->get_foxes();
  set<int> no_include;

  int n = sz(current_foxes);
  int num_threads = min(N_THREADS, n);

  int len_chunk = n / num_threads;
  int rem = n % num_threads;
  int act = 0;
  for(int i = 0; i < num_threads; ++i){
    int mv = (i < rem) ? 1 : 0;
    int to = act + len_chunk + mv;
    thdata[i] = data_t(act, to, current_gen, &current_ecosystem, &next_ecosystem, &current_foxes, &next_foxes, &no_include, &param);
    pthread_create(&threads[i], nullptr, move_mult_foxes, (void*) &thdata[i]);
    act = to;
  }
  
  for(int i = 0; i < num_threads; ++i){
    pthread_join(threads[i], nullptr);
  }

  vector<Entity>& current_rabbits = current->get_rabbits();
  vector<Entity>& next_rabbits = next->get_rabbits();

  copy_entity(next_ecosystem, current_rabbits, next_rabbits, no_include, EntityType::RABBIT);
  delete_entity(current_ecosystem, current_rabbits);
  delete_entity(current_ecosystem, current_foxes);
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
 * @brief Aplica reglas de movimiento y procreación para un conejo.
 *
 * Funcionalidad:
 *  - Calcula movimiento mediante step_rabbit().
 *  - Incrementa edad de procreación.
 *  - Si cumple condiciones, genera un nuevo conejo.
 *  - Maneja conflictos entre conejos con el criterio de antigüedad.
 *  - Acceso protegido con mutex.
 *
 * @param act Conejo actual
 * @param current_ecosystem Matriz del ecosistema actual
 * @param current_gen Número de generación global
 * @param param Parámetros globales
 * @param next_rabbits Vector destino de conejos
 * @param next_ecosystem Matriz destino
 */
void move_single_rabbit(Entity& act, Mat& current_ecosystem, int current_gen, const Param& param, vector<Entity>& next_rabbits, Mat& next_ecosystem){
  pair<int,int> mv = step_rabbit(act, current_ecosystem, current_gen);
  bool moved = !(mv.first == act.get_x() && mv.second == act.get_y());

  int new_proc = act.get_local_proc() + 1;

  bool can_procreate = (act.get_local_proc() >= param.GEN_PROC_RABBITS) && moved;
  if (can_procreate) {
    new_proc = 0;

    Entity baby(
        act.get_x(),
        act.get_y(),
        current_gen + 1,
        0, 0);
    pthread_mutex_lock(&rabbits_mod);
    next_rabbits.push_back(baby);
    next_ecosystem[act.get_x()][act.get_y()] = Cell(
        EntityType::RABBIT, sz(next_rabbits) - 1
    );
    pthread_mutex_unlock(&rabbits_mod);
  }

  Entity new_step(
      mv.first,
      mv.second,
      current_gen + 1,
      new_proc, 0);
  
  pthread_mutex_lock(&rabbits_mod);
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
  pthread_mutex_unlock(&rabbits_mod);
}

/**
 * @brief Hilo encargado de procesar un subconjunto de conejos.
 *
 * Ejecuta move_single_rabbit() sobre los conejos dentro del rango asignado.
 *
 * @param args Puntero a data_t con datos del hilo
 * @return nullptr
 */
void* move_mult_rabbits(void * args){
  data_t* data = (data_t*) args;
  Mat& current_ecosystem = (*data->current_ecosystem);
  const Param& param = (*data->param);
  vector<Entity>& next_rabbits = (*data->next_rabbits);
  Mat& next_ecosystem = (*data->next_ecosystem);

  int current_gen = (data->current_gen);
  for(int i = data->start; i < data->end; ++i){
    Entity& act = (*data->current_rabbits)[i];
    move_single_rabbit(act, current_ecosystem, current_gen, param, next_rabbits, next_ecosystem);
  }
  return NULL;
}

/**
 * @brief Controlador principal del movimiento de todos los conejos.
 *
 * Funcionalidad:
 *  - Paraleliza el movimiento usando move_mult_rabbits().
 *  - Copia zorros sin moverlos (todavía no se procesan).
 *  - Limpia el buffer de conejos y zorros.
 *
 * @param current Generación actual
 * @param next Generación siguiente
 * @param current_gen Generación global
 */
void move_rabbits(Generation* current, Generation* next, int current_gen){
  Mat& current_ecosystem = current->get_ecosystem();
  Mat& next_ecosystem    = next->get_ecosystem();

  vector<Entity>& current_rabbits = current->get_rabbits();
  vector<Entity>& next_rabbits    = next->get_rabbits();

  const Param& param = current->get_param();
  
  int n = sz(current_rabbits);
  int num_threads = min(N_THREADS, n);

  int len_chunk = n / num_threads;
  int rem = n % num_threads;
  int act = 0;
  for(int i = 0; i < num_threads; ++i){
    int mv = (i < rem) ? 1 : 0;
    int to = act + len_chunk + mv;
    thdata[i] = data_t(act, to, current_gen, &current_rabbits, &current_ecosystem, &next_rabbits, &next_ecosystem, &param);
    pthread_create(&threads[i], nullptr, move_mult_rabbits, (void*) &thdata[i]);
    act = to;
  }

  for(int i = 0; i < num_threads; ++i){
    pthread_join(threads[i], nullptr);
  }

  set<int> no_include;
  vector<Entity>& current_foxes = current->get_foxes();
  vector<Entity>& next_foxes    = next->get_foxes();
  copy_entity(next_ecosystem, current_foxes, next_foxes, no_include, EntityType::FOX);
  delete_entity(current_ecosystem, current_rabbits);
  delete_entity(current_ecosystem, current_foxes);
}

/**
 * @brief Ejecuta toda la simulación durante N_GEN generaciones.
 *
 * Proceso por generación:
 *  1. Mover conejos
 *  2. Intercambiar buffers
 *  3. Mover zorros
 *  4. Intercambiar buffers
 *
 * Mantiene el número de generación y usa doble buffer consistente.
 *
 * @param N_GEN Número total de generaciones a simular
 * @param start Generación inicial
 * @return Generación final después de la simulación
 */
Generation simulation(int N_GEN, Generation& start){
  Generation first(start);
  Generation second(N_ROW, N_COL, start.get_rocks(), start.get_param());

  Generation* current = &first;
  Generation* next    = &second;

  int current_gen = start.get_n_gen();
  current->set_n_gen(current_gen);
  //current->print_grid();
  for (int step = 0; step < N_GEN; ++step) {
    move_rabbits(current, next, current_gen);
    swap(current, next);
    move_foxes(current, next, current_gen);
    swap(current, next);
    ++current_gen;
    next->set_n_gen(current_gen);
    current->set_n_gen(current_gen);
    //current->print_grid();
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

  pthread_mutex_init(&rabbits_mod, nullptr);
  pthread_mutex_init(&foxes_mod, nullptr);
  pthread_mutex_init(&copy_ent, nullptr);
  
  Generation start(0, ecosystem, rocks, rabbits, foxes, param);
  Generation end_state = simulation(N_GEN, start);
  end_state.print();

  pthread_mutex_destroy(&rabbits_mod);
  pthread_mutex_destroy(&foxes_mod);
  pthread_mutex_destroy(&copy_ent);
  
  return 0;
}



