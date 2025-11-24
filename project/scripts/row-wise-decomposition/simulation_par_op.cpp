/**
 * @autor: JuCollas 
 * @version: 6.0
 * @date: 20 nov 2025
 * Project Parallel-Programming
 * 
 * @brief Simulador paralelo de un ecosistema compuesto por conejos, zorros y rocas.
 *
 * Descripción general:
 * --------------------
 * Esta versión paralela extiende la lógica de la implementación secuencial,
 * permitiendo ejecutar la simulación de forma concurrente mediante hilos POSIX (pthread).
 * El ecosistema está representado por una matriz de R × C donde cada celda contiene
 * una entidad o está vacía.
 *
 * Mecánica general del algoritmo:
 *  - Se divide la matriz en bloques de filas, asignados estáticamente a cada hilo.
 *  - Cada hilo procesa movimiento, reproducción, cacería y muerte por inanición
 *    de forma independiente dentro de su bloque.
 *  - Se garantiza la consistencia global mediante mutex que resuelven conflictos
 *    en las celdas destino (next_ecosystem).
 *  - Se emplea doble buffer (current / next) para evitar efectos de actualización
 *    simultánea y preservar la atomicidad por generación.
 *
 * Reglas del ecosistema:
 * ----------------------
 * - Conejos:
 *      * Se mueven a celdas vacías siguiendo una selección determinista.
 *      * Se reproducen cuando alcanzan GEN_PROC_RABBITS.
 * - Zorros:
 *      * Prioridad 1: moverse hacia un conejo adyacente.
 *      * Prioridad 2: moverse hacia una celda vacía.
 *      * Mueren si alcanzan GEN_FOOD_FOXES sin comer.
 *      * Se reproducen con GEN_PROC_FOXES.
 * - Rocas:
 *      * Son estáticas, nunca se mueven ni interactúan.
 *
 * Objetivo:
 * ---------
 * Acelerar de forma significativa la simulación manteniendo resultados
 * idénticos a la versión secuencial, resolviendo correctamente concurrencia
 * y conflictos entre hilos.
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

/** Para cambiar de string a EntityType */
const map<string, EntityType> str_to_type = {
  {"RABBIT", EntityType::RABBIT},
  {"FOX", EntityType::FOX},
  {"ROCK", EntityType::ROCK},
};

/* Dimesiones de la matrix*/
int N_ROW, N_COL;

/* Datos para los hilos*/
class data_t{
  public:
    int start;
    int end;
    int current_gen;
    int id_thread;
    EntityType turn;
    Mat* current_ecosystem;
    Mat* next_ecosystem;
    vector<vector<int>>* current_compress;
    vector<vector<int>>* next_compress;

    const Param* param;

  data_t() : 
    start(0), 
    end(0), 
    current_gen(0),
    id_thread(0),
    turn(EntityType::EMPTY),
    current_ecosystem(nullptr), 
    next_ecosystem(nullptr), 
    current_compress(nullptr),
    next_compress(nullptr),
    param(nullptr) {}

  /*move rabbits data */
  data_t(int _start, 
         int _end, 
         int _current_gen,
         int _id_thread,
         EntityType _turn, 
         Mat* _current_ecosystem, 
         Mat* _next_ecosytem,
         vector<vector<int>>* _current_compress,
         vector<vector<int>>* _next_compress,
         const Param* _param
    ) : 
    start(_start), 
    end(_end), 
    current_gen(_current_gen),
    id_thread(_id_thread),
    turn(_turn),
    current_ecosystem(_current_ecosystem),
    next_ecosystem(_next_ecosytem),
    current_compress(_current_compress),
    next_compress(_next_compress),
    param(_param) {}

    data_t(int _start, int _end, Mat* _ecosystem, vector<vector<int>>* _eco_compress):
    start(_start),
    end(_end),
    current_ecosystem(_ecosystem), 
    current_compress(_eco_compress) {}
};

/* Hilos, Semaforos y Datos*/
pthread_t threads[N_THREADS];
data_t thdata[N_THREADS];
pthread_mutex_t thmutex[N_THREADS * 2];

/**
 * @brief Elimina todas las entidades vivas (RABBIT o FOX) en una fila del ecosistema.
 *
 * @param id_row Índice de la fila a limpiar.
 * @param ecosystem Matriz completa del ecosistema.
 *
 * @details
 * Libera memoria de todas las entidades dinámicas en la fila indicada y
 * restablece cada celda a estado vacío (EMPTY). No afecta rocas.
 */
void delete_single_row(const int id_row, Mat& ecosystem, vector<vector<int>>& eco_compress){
  for(int i = 0; i < sz(eco_compress[id_row]); ++i){
    int id_col = eco_compress[id_row][i];
    Cell& cell = ecosystem[id_row][id_col];
    EntityType type = cell.get_type(); 
    if(type == EntityType::RABBIT || type == EntityType::FOX){
      delete(cell.get_entity());
      cell.set_entity(nullptr);
      cell.set_type(EntityType::EMPTY);
    }
  }
  eco_compress[id_row].clear();
}

/**
 * @brief Función ejecutada por un hilo para limpiar múltiples filas del ecosistema.
 *
 * @param args Puntero a estructura data_t con {start, end, current_ecosystem}.
 * @return Siempre retorna NULL.
 *
 * @details
 * Itera sobre el rango [start, end) y aplica delete_single_row en cada fila.
 */
void* delete_mult_row(void * args){
  data_t* data = (data_t*) args;
  Mat& ecosystem = (*data->current_ecosystem);
  vector<vector<int>>& eco_compress = (*data->current_compress); 
  for(int row = data->start; row < data->end; ++row){
    delete_single_row(row, ecosystem, eco_compress);
  }
  return NULL;
}

/**
 * @brief Elimina todas las entidades dinámicas del ecosistema usando múltiples hilos.
 *
 * @param ecosystem Matriz del ecosistema a limpiar.
 *
 * @details
 * Divide las filas entre N_THREADS hilos para acelerar la limpieza del buffer
 * current_ecosystem antes de reutilizarlo en la siguiente generación.
 */
void delete_entities(Mat& ecosystem, vector<vector<int>>& eco_compress){
  const int n = sz(ecosystem);
  const int num_threads = min(N_THREADS, n);
  const int len_chunk = n / num_threads;
  const int rem = n % num_threads;
  int act = 0;
  for(int i = 0; i < num_threads; ++i){
    int mv = (i < rem) ? 1 : 0;
    int to = act + len_chunk + mv;
    thdata[i] = data_t(act, to, &ecosystem,  &eco_compress);
    pthread_create(&threads[i], nullptr, delete_mult_row, (void*) &thdata[i]);
    act = to;
  }
  for(int i = 0; i < num_threads; ++i){
    pthread_join(threads[i], nullptr);
  }
}

/**
 * @brief Calcula el índice del mutex asociado a un hilo y su sub-mutex interno.
 *
 * @param x Identificador del hilo.
 * @param y Subíndice (0 o 1) que representa el mutex inferior o superior.
 * @return Índice lineal correspondiente dentro de thmutex[].
 */
int index_mutex(int x, int y){
  return 2 * x + y; 
}

/**
 * @brief Determina si la inserción en una fila requiere bloqueo mediante mutex.
 *
 * @param id_row Fila donde se insertará la entidad.
 * @param id_thread Identificador del hilo que ejecuta la operación.
 * @param lower_limit Límite inferior del bloque de filas del hilo.
 * @param upper_limit Límite superior del bloque asignado al hilo.
 * @return Índice del mutex a usar o -1 si no se requiere bloqueo.
 *
 * @details
 * Las inserciones en bordes entre bloques de hilos requieren sincronización
 * explícita para evitar condiciones de carrera en next_ecosystem.
 */
int must_blocked(int id_row, int id_thread, int lower_limit, int upper_limit){
  const int len_chuck = upper_limit - lower_limit;
  int ans = -1;
  if(len_chuck == 1){
     if(id_row == lower_limit - 1){
      ans = index_mutex(id_thread - 1, 1);
    }else if(id_row == lower_limit){
      ans = index_mutex(id_thread, 0);
    }else if(id_row == upper_limit){
      ans = index_mutex(id_thread + 1, 0);
    }
  }else if(id_row != 0 && id_row != N_ROW - 1){
    if(id_row == lower_limit - 1){
      ans = index_mutex(id_thread - 1, 1);
    }else if(id_row == lower_limit){
      ans = index_mutex(id_thread, 0);
    }else if(id_row == upper_limit - 1){
      ans = index_mutex(id_thread, 1);
    }else if(id_row == upper_limit){
      ans = index_mutex(id_thread + 1, 0);
    }
  }
  return ans;
}

/**
 * @brief Inserta una entidad en el next_ecosystem resolviendo conflictos concurrentes.
 *
 * @param lower_limit Límite inferior del bloque del hilo.
 * @param upper_limit Límite superior del bloque del hilo.
 * @param id_thread ID del hilo que inserta.
 * @param type Tipo de la entidad (RABBIT o FOX).
 * @param entity Puntero a la entidad a insertar.
 * @param ecosystem Matriz destino (next_ecosystem).
 *
 * @details
 * Si la celda está vacía → inserción directa.
 * Si hay conflicto:
 *  - Conejos: sobrevive el de mayor antigüedad (local_proc).
 *  - Zorros: primero mayor antigüedad, y si empatan, menor hambre (local_food).
 * Maneja sincronización mediante mutex según must_blocked().
 */
void modify_ecosystem(const int lower_limit, const int upper_limit, const int id_thread, const EntityType type, Entity* entity, Mat& ecosystem, vector<vector<int>>& eco_compress){
  const int id_row = entity->get_x();
  const int ind_mutex = must_blocked(id_row, id_thread, lower_limit, upper_limit);
  const bool block = ind_mutex != -1;

  Entity* better = entity;
  const int proc_better = better->get_local_proc();
  const int food_better = better->get_local_food();

  const int x = entity->get_x();
  const int y = entity->get_y();

  if(block){
    pthread_mutex_lock(&thmutex[ind_mutex]);
  }

  Cell& dest = ecosystem[x][y];

  if(dest.get_type() == EntityType::EMPTY){
    eco_compress[x].push_back(y);
  }

  if(dest.get_type() == type){
    Entity* occupant = dest.get_entity();
    const int proc_occup = occupant->get_local_proc();
    const int food_occup = occupant->get_local_food();
    if(proc_occup > proc_better){
      delete(better);
      better = occupant;
    }else if(proc_occup == proc_better && food_occup < food_better){
      delete(better);
      better = occupant;
    }else{
      delete(occupant);
    }
  }
  dest.set_entity(better);
  dest.set_type(type);

  if(block){
    pthread_mutex_unlock(&thmutex[ind_mutex]);
  }
}

/**
 * @brief Copia una entidad al next_ecosystem sin aplicar reglas de conflicto.
 *
 * @param lower_limit Límite inferior del bloque del hilo.
 * @param upper_limit Límite superior del bloque del hilo.
 * @param id_thread Identificador del hilo.
 * @param type Tipo de entidad a copiar.
 * @param entity Entidad duplicada a insertar.
 * @param ecosystem Matriz destino (next_ecosystem).
 *
 * @details
 * Se usa cuando el turno actual no corresponde al tipo de entidad procesado.
 * Solo copia si la celda está vacía; nunca reemplaza ocupantes.
 */
void modify_ecosystem_copy(const int lower_limit, const int upper_limit, const int id_thread, const EntityType type, Entity* entity, Mat& ecosystem, vector<vector<int>>& next_compress){
  const int id_row = entity->get_x();
  const int ind_mutex = must_blocked(id_row, id_thread, lower_limit, upper_limit);
  const bool block = ind_mutex != -1;
  const int x = entity->get_x();
  const int y = entity->get_y(); 

  if(block){
    pthread_mutex_lock(&thmutex[ind_mutex]);
  }

  Cell& dest = ecosystem[x][y];
  if(dest.get_type() == EntityType::EMPTY){
    dest.set_entity(entity);
    dest.set_type(type);
    next_compress[x].push_back(y);
  }

  if(block){
    pthread_mutex_unlock(&thmutex[ind_mutex]);
  }
}

/**
 * @brief Verifica si una posición está dentro de los límites de la matriz.
 *
 * @param x Fila a validar.
 * @param y Columna a validar.
 * @return true si (x,y) pertenece al ecosistema, false en caso contrario.
 */
bool isValid(const int x, const int y){
    return 0 <= x && x < N_ROW && 0 <= y && y < N_COL;
}

/**
 * @brief Calcula el movimiento del zorro para la siguiente generación.
 *
 * @param x Fila actual del zorro.
 * @param y Columna actual del zorro.
 * @param ecosystem Estado actual del ecosistema.
 * @param G Número de la generación actual.
 * @return Par (fila, columna) del destino elegido.
 *
 * @details
 * Prioridades:
 *  1. Mover hacia un conejo adyacente.
 *  2. Mover hacia una celda vacía.
 *  Si no existe ninguna opción válida → permanece en su celda.
 * La selección entre múltiples destinos es determinista: (G + x + y) % P.
 */
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

/**
 * @brief Ejecuta todas las reglas asociadas al movimiento de un zorro.
 *
 * @param x Fila actual.
 * @param y Columna actual.
 * @param G Generación global.
 * @param start Límite inferior del bloque del hilo.
 * @param end Límite superior del bloque del hilo.
 * @param id_thread Identificador del hilo.
 * @param current Ecosistema actual.
 * @param next Ecosistema siguiente (buffer de escritura).
 * @param param Parámetros globales (GEN_PROC_FOXES, GEN_FOOD_FOXES, ...).
 *
 * @details
 * Procesos aplicados:
 *  - Movimiento mediante step_fox().
 *  - Muerte por inanición.
 *  - Cacería de conejo si cae en su celda.
 *  - Reproducción si cumple GEN_PROC_FOXES y se movió.
 * La escritura final en next_ecosystem se realiza mediante modify_ecosystem().
 */
void move_single_fox(const int x, const int y, const int G, const int start, const int end, const int id_thread, const Mat& current, Mat& next, vector<vector<int>>& next_compress, const Param& param ){    
  const Entity& act = *current[x][y].get_entity();
  const pair<int, int> step = step_fox(x, y, current, G);
 
  const bool moved = !(act.get_x() == step.first && act.get_y() == step.second);
  const bool abj_rabbit = current[step.first][step.second].get_type() == EntityType::RABBIT;

  int update_proc = act.get_local_proc() + 1;
  int update_food = act.get_local_food() + 1;
  
  if(act.get_local_food() + 1 >= param.GEN_FOOD_FOXES && !abj_rabbit)
    return;

  const bool can_procreate = (act.get_local_proc() >= param.GEN_PROC_FOXES) && moved;
  if(can_procreate){
    update_proc = 0;
    Entity* baby = new Entity(act.get_x(), act.get_y());
    modify_ecosystem(start, end, id_thread, EntityType::FOX, baby, next, next_compress); 
  }

  if(abj_rabbit) update_food = 0;

  Entity* act_mov = new Entity(step.first, step.second, update_proc, update_food);

  modify_ecosystem(start, end, id_thread, EntityType::FOX, act_mov, next, next_compress); 
}

/**
 * @brief Determina el movimiento del conejo para la siguiente generación.
 *
 * @param x Fila actual.
 * @param y Columna actual.
 * @param ecosystem Estado del ecosistema.
 * @param G Generación actual.
 * @return Par (fila, columna) del destino.
 *
 * @details
 * Solo puede moverse a celdas vacías.
 * Si no hay disponibles: permanece donde está.
 * Selección determinista mediante (G + x + y) % P.
 */
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

/**
 * @brief Aplica movimiento y reproducción a un conejo específico.
 *
 * @param x Fila del conejo.
 * @param y Columna.
 * @param G Generación global.
 * @param start Límite inferior del bloque del hilo.
 * @param end Límite superior del bloque del hilo.
 * @param id_thread ID del hilo procesando.
 * @param current Ecosistema actual.
 * @param next Ecosistema siguiente.
 * @param param Parámetros globales.
 *
 * @details
 * - Aumenta antigüedad (local_proc).
 * - Si alcanzó GEN_PROC_RABBITS y se movió → reproduce.
 * - Inserta movimiento final mediante modify_ecosystem().
 */
void move_single_rabbit(int x, int y, int G, const int start, const int end, const int id_thread, const Mat& current, Mat& next, vector<vector<int>>& next_compress, const Param& param){
  const Entity& act = *current[x][y].get_entity();
  const pair<int,int> step = step_rabbit(x, y, current, G);
  const bool moved = !(step.first == act.get_x() && step.second == act.get_y());

  int update_proc = act.get_local_proc() + 1;

  const bool can_procreate = (act.get_local_proc() >= param.GEN_PROC_RABBITS) && moved;
  if (can_procreate) {
    update_proc = 0;

    Entity* baby = new Entity(act.get_x(), act.get_y());
    modify_ecosystem(start, end, id_thread, EntityType::RABBIT, baby, next, next_compress); 
  }

  Entity* act_mov = new Entity(step.first, step.second, update_proc);
  modify_ecosystem(start, end, id_thread, EntityType::RABBIT, act_mov, next, next_compress); 
}

/**
 * @brief Copia una entidad sin procesarla según reglas del turno actual.
 *
 * @param x Fila.
 * @param y Columna.
 * @param lower Límite inferior del bloque del hilo.
 * @param upper Límite superior del bloque.
 * @param id_thr ID del hilo.
 * @param current Ecosistema actual.
 * @param next Ecosistema siguiente.
 *
 * @details
 * Se usa para trasladar entidades que no deben actuar en la fase actual
 * (p.ej., zorros durante el turno de conejos).
 */
void copy_entity(const int x, const int y, const int lower, const int upper, const int id_thr, const Mat& current, Mat& next, vector<vector<int>>& next_compress){
  Entity* entity = new Entity(*current[x][y].get_entity());
  EntityType type = current[x][y].get_type();
  modify_ecosystem_copy(lower, upper, id_thr, type, entity, next, next_compress);
}

/**
 * @brief Determina qué acción realizar según el turno actual (RABBIT o FOX).
 *
 * @param x Fila.
 * @param y Columna.
 * @param G Generación actual.
 * @param start Límite inferior de filas del hilo.
 * @param end Límite superior del bloque.
 * @param id_thread Identificador del hilo.
 * @param turn Tipo de entidad que se procesa en esta fase.
 * @param current Ecosistema actual.
 * @param next Ecosistema siguiente.
 * @param param Parámetros globales.
 *
 * @details
 * - Si turn == RABBIT → procesa solo conejos; copia zorros.
 * - Si turn == FOX → procesa zorros; copia conejos.
 */
void move_sigle_entity(const int x, const int y, const int G, const int start, const int end, const int id_thread, const EntityType turn, const Mat& current, Mat& next, vector<vector<int>>& next_compress, const Param& param){
  EntityType type = current[x][y].get_type();

  if(type == EntityType::ROCK || type == EntityType::EMPTY) return;
  
  if(turn == EntityType::RABBIT){
    if(type == EntityType::RABBIT){
      move_single_rabbit(x, y, G, start, end, id_thread, current, next, next_compress, param);
    }else if(type == EntityType::FOX){
      copy_entity(x, y, start, end, id_thread, current, next, next_compress);
    }
  }else if(turn == EntityType::FOX){
    if(type == EntityType::FOX){
      move_single_fox(x, y, G, start, end, id_thread, current, next, next_compress, param);
    }else if(type == EntityType::RABBIT){
      copy_entity(x, y, start, end, id_thread, current, next, next_compress);
    }
  }
}

/**
 * @brief Procesa una fila completa aplicando movimiento según el turno actual.
 *
 * @param id_row Fila a procesar.
 * @param G Generación global.
 * @param start Inicio del bloque asignado.
 * @param end Fin del bloque asignado.
 * @param id_thread Identificador del hilo.
 * @param turn Tipo de entidad a procesar.
 * @param current Ecosistema actual.
 * @param next Ecosistema siguiente.
 * @param param Parámetros globales.
 */
void move_single_row(const int id_row, const int G, const int start, const int end, const int id_thread, const EntityType turn, const Mat& current, Mat& next, const vector<vector<int>>& current_compress, vector<vector<int>>& next_compress,  const Param& param){
  for(int i = 0; i < sz(current_compress[id_row]); ++i){
    int id_col = current_compress[id_row][i];
    move_sigle_entity(id_row, id_col, G, start, end, id_thread, turn, current, next, next_compress, param);
  }
}

/**
 * @brief Función ejecutada por cada hilo para procesar varias filas del ecosistema.
 *
 * @param args Puntero a data_t con toda la información del hilo.
 * @return Siempre NULL.
 *
 * @details
 * Llama a move_single_row para cada fila del rango [start, end).
 */
void* move_mult_row(void * args){
  data_t* data = (data_t*) args;

  const Mat& current = (*data->current_ecosystem);
  Mat& next = (*data->next_ecosystem);
  
  const vector<vector<int>>& current_compress = (*data->current_compress);
  vector<vector<int>>& next_compress = (*data->next_compress);
  
  const int G = (data->current_gen);
  const EntityType turn = (data->turn);
  const Param& param = (*data->param);
  
  const int start = data->start;
  const int end = data->end;
  const int id_thread = data->id_thread;
  for(int row = start; row < end; ++row){
    move_single_row(row, G, start, end, id_thread, turn, current, next, current_compress, next_compress, param);
  }
  return NULL;
}

/**
 * @brief Controlador del movimiento por turno (primero conejos, luego zorros).
 *
 * @param current Ecosistema actual (buffer de lectura).
 * @param next Ecosistema siguiente (buffer de escritura).
 * @param turn Tipo de entidad a procesar (RABBIT o FOX).
 *
 * @details
 * Divide las filas entre hilos,
 * crea pthreads ejecutando move_mult_row,
 * y sincroniza con pthread_join().
 * Finalmente limpia current_ecosystem para reuso.
 */
void move_entities(Generation* current, Generation* next, const EntityType turn){
  Mat& current_ecosystem = current->get_ecosystem();
  Mat& next_ecosystem    = next->get_ecosystem();

  vector<vector<int>>& current_compress = current->get_eco_compress();
  vector<vector<int>>& next_compress = next->get_eco_compress();

  const Param& param = current->get_param();
  const int current_gen = current->get_n_gen();
  const int n = sz(current_ecosystem);
  const int num_threads = min(N_THREADS, n);
  const int len_chunk = n / num_threads;
  const int rem = n % num_threads;
  
  int act = 0;
  for(int i = 0; i < num_threads; ++i){
    int mv = (i < rem) ? 1 : 0;
    int to = act + len_chunk + mv;
    thdata[i] = data_t(act, to, current_gen, i, turn, &current_ecosystem, &next_ecosystem, &current_compress, &next_compress, &param);
    pthread_create(&threads[i], nullptr, move_mult_row, (void*) &thdata[i]);
    act = to;
  }
  
  for(int i = 0; i < num_threads; ++i){
    pthread_join(threads[i], nullptr);
  }

  delete_entities(current_ecosystem, current_compress);
}

/**
 * @brief Ejecuta la simulación completa durante N_GEN generaciones.
 *
 * @param N_GEN Número total de generaciones a simular.
 * @param start Estado inicial del ecosistema.
 * @return Ecosistema final después de N_GEN iteraciones.
 *
 * @details
 * Fases por generación:
 *  1. Mover conejos.
 *  2. swap(current, next).
 *  3. Mover zorros.
 *  4. swap(current, next).
 *  5. Aumentar número de generación.
 *
 * Inicializa y destruye todos los mutex necesarios.
 */
Generation simulation(const int N_GEN, Generation& start){
  Generation first(start);
  Generation second(N_ROW, N_COL, start.get_ecosystem(), start.get_param());
  Generation* current = &first;
  Generation* next = &second;

  for(int i = 0; i < N_THREADS * 2; ++i){
    pthread_mutex_init(&thmutex[i], nullptr);
  }

  for (int step = 0; step < N_GEN; ++step) {
    //current->print_grid();
    move_entities(current, next, EntityType::RABBIT);
    swap(current, next);
    move_entities(current, next, EntityType::FOX);
    swap(current, next);
    next->increase_generation();
    current->increase_generation();
  }

  for(int i = 0; i < N_THREADS * 2; ++i){
    pthread_mutex_destroy(&thmutex[i]);
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
  Generation end_state = simulation(N_GEN, start);
  end_state.print();

  return 0;
}

