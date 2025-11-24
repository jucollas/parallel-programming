/**
 * @autor: JuCollas 
 * @version: 5.0
 * @date: 20 nov 2025
 * Project Parallel-Programming
 * 
 * @brief Simulador secuencial de un ecosistema compuesto por conejos, zorros y rocas.
 *
 * Descripción general:
 * --------------------
 * Esta versión implementa la lógica del ecosistema de forma secuencial
 * (un solo hilo de ejecución). El ecosistema está representado por una
 * matriz de R × C donde cada celda contiene una entidad o está vacía.
 *
 * Mecánica general del algoritmo:
 *  - Se recorre la matriz fila por fila, celda por celda.
 *  - En cada turno se procesan primero todos los conejos y luego todos los zorros.
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
 * Implementar una versión base secuencial que produzca los mismos resultados
 * lógicos que la versión paralela, sirviendo como referencia de corrección.
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

/** Para cambiar de string a EntityType */
const map<string, EntityType> str_to_type = {
  {"RABBIT", EntityType::RABBIT},
  {"FOX", EntityType::FOX},
  {"ROCK", EntityType::ROCK},
};

/* Dimensiones de la matriz */
int N_ROW, N_COL;

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

/**
 * @brief Elimina todas las entidades dinámicas del ecosistema de forma secuencial.
 *
 * @param ecosystem Matriz del ecosistema a limpiar.
 *
 * @details
 * Recorre todas las filas y aplica delete_single_row en cada una.
 * Se usa para limpiar el buffer current_ecosystem antes de reutilizarlo.
 */
void delete_entities(Mat& ecosystem){
  const int n = sz(ecosystem);
  for(int row = 0; row < n; ++row){
    delete_single_row(row, ecosystem);
  }
}

/**
 * @brief Inserta una entidad en el next_ecosystem resolviendo conflictos.
 *
 * @param type Tipo de la entidad (RABBIT o FOX).
 * @param entity Puntero a la entidad a insertar.
 * @param ecosystem Matriz destino (next_ecosystem).
 *
 * @details
 * Si la celda está vacía → inserción directa.
 * Si hay conflicto (misma especie):
 *  - Conejos y zorros: sobrevive el de mayor antigüedad (local_proc).
 *  - Si empatan en antigüedad: sobrevive el de menor hambre (local_food).
 * La entidad perdedora es liberada con delete.
 */
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

/**
 * @brief Copia una entidad al next_ecosystem sin aplicar reglas de turno.
 *
 * @param type Tipo de entidad a copiar.
 * @param entity Entidad duplicada a insertar.
 * @param ecosystem Matriz destino (next_ecosystem).
 *
 * @details
 * Se usa cuando el turno actual no corresponde al tipo de entidad procesado.
 * Solo copia si la celda está vacía; nunca reemplaza ocupantes.
 */
void modify_ecosystem_copy(const EntityType type, Entity* entity, Mat& ecosystem){
  Cell& dest = ecosystem[entity->get_x()][entity->get_y()];
  if(dest.get_type() == EntityType::EMPTY){
    dest.set_entity(entity);
    dest.set_type(type);
  }else{
    delete entity;
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
 * @param current Ecosistema actual.
 * @param next Ecosistema siguiente.
 * @param param Parámetros globales.
 *
 * @details
 * - Aumenta antigüedad (local_proc).
 * - Si alcanzó GEN_PROC_RABBITS y se movió → reproduce.
 * - Inserta movimiento final mediante modify_ecosystem().
 */
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

/**
 * @brief Copia una entidad sin procesarla según reglas del turno actual.
 *
 * @param x Fila.
 * @param y Columna.
 * @param current Ecosistema actual.
 * @param next Ecosistema siguiente.
 *
 * @details
 * Se usa para trasladar entidades que no deben actuar en la fase actual
 * (p.ej., zorros durante el turno de conejos).
 */
void copy_entity(const int x, const int y,
                 const Mat& current, Mat& next){
  Entity* entity = new Entity(*current[x][y].get_entity());
  EntityType type = current[x][y].get_type();
  modify_ecosystem_copy(type, entity, next);
}

/**
 * @brief Determina qué acción realizar según el turno actual (RABBIT o FOX).
 *
 * @param x Fila.
 * @param y Columna.
 * @param G Generación actual.
 * @param turn Tipo de entidad que se procesa en esta fase.
 * @param current Ecosistema actual.
 * @param next Ecosistema siguiente.
 * @param param Parámetros globales.
 *
 * @details
 * - Si turn == RABBIT → procesa solo conejos; copia zorros.
 * - Si turn == FOX → procesa zorros; copia conejos.
 */
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

/**
 * @brief Procesa una fila completa aplicando movimiento según el turno actual.
 *
 * @param id_row Fila a procesar.
 * @param G Generación global.
 * @param turn Tipo de entidad a procesar.
 * @param current Ecosistema actual.
 * @param next Ecosistema siguiente.
 * @param param Parámetros globales.
 */
void move_single_row(const int id_row, const int G, const EntityType turn,
                     const Mat& current, Mat& next, const Param& param){
  for(int i = 0; i < sz(current[id_row]); ++i){
    move_sigle_entity(id_row, i, G, turn, current, next, param);
  }
}

/**
 * @brief Controlador del movimiento por turno (primero conejos, luego zorros).
 *
 * @param current Ecosistema actual (buffer de lectura).
 * @param next Ecosistema siguiente (buffer de escritura).
 * @param turn Tipo de entidad a procesar (RABBIT o FOX).
 *
 * @details
 * Recorre secuencialmente todas las filas y delega en move_single_row().
 * Al final limpia current_ecosystem para reuso mediante delete_entities().
 */
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
 */
Generation simulation(const int N_GEN, Generation& start){
  Generation first(start);
  Generation second(N_ROW, N_COL, start.get_ecosystem(), start.get_param());
  Generation* current = &first;
  Generation* next = &second;

  for (int step = 0; step < N_GEN; ++step) {
    //current->print_grid();
    move_entities(current, next, EntityType::RABBIT);
    swap(current, next);
    move_entities(current, next, EntityType::FOX);
    swap(current, next);
    next->increase_generation();
    current->increase_generation();
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
/*int main(){
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
}*/

// -----------------------------------------------------------------------------
// Procesa UN archivo de prueba
// -----------------------------------------------------------------------------
void process_file(const fs::path& input_path, const fs::path& output_path) {
    ifstream in(input_path);
    if (!in.is_open()) {
        cerr << "Error abriendo archivo: " << input_path << "\n";
        return;
    }

    ofstream out(output_path);
    if (!out.is_open()) {
        cerr << "Error creando salida: " << output_path << "\n";
        return;
    }

    // --- Redirigir cin y cout temporalmente ---
    streambuf* cin_backup = cin.rdbuf();
    streambuf* cout_backup = cout.rdbuf();
    cin.rdbuf(in.rdbuf());
    cout.rdbuf(out.rdbuf());

    // ======== MEDIR TIEMPOS ==========
    auto wall_start = chrono::high_resolution_clock::now();
    clock_t cpu_start = clock();

    // ========= LLAMAR TU SIMULACIÓN =========
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

    // Imprimir salida del ecosistema
    end_state.print();

    // ======== MEDIR TIEMPO FINAL ===========
    auto wall_end = chrono::high_resolution_clock::now();
    clock_t cpu_end = clock();

    double cpu_time = double(cpu_end - cpu_start) / CLOCKS_PER_SEC;
    double wall_time = chrono::duration<double>(wall_end - wall_start).count();

    // Restaurar cin y cout
    cin.rdbuf(cin_backup);
    cout.rdbuf(cout_backup);

    // Imprimir tiempos en consola
    cout << "[OK] " << input_path.filename()
         << "  CPU: " << cpu_time << "s"
         << "  Wall: " << wall_time << "s\n";

    // Agregar tiempos al archivo de salida
    out << "\n# CPU_TIME " << cpu_time << "\n";
    out << "# WALL_TIME " << wall_time << "\n";
}

int main() {
    fs::path input_dir = "./testing/input/";
    fs::path output_dir = ".output_row_seq/";

    if (!fs::exists(output_dir)) fs::create_directory(output_dir);

    cout << "Procesando carpeta: " << input_dir << "\n";

    for (const auto& entry : fs::directory_iterator(input_dir)) {
        if (entry.is_regular_file()) {
            fs::path file = entry.path();
            fs::path out_file = output_dir / (file.stem().string() + ".out");

            cout << "[RUN] " << file.filename() << "\n";
            process_file(file, out_file);
        }
    }

    cout << "\n=== Procesamiento terminado ===\n";
    return 0;
}