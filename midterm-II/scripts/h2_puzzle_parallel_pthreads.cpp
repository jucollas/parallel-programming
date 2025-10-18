/**
 * @file h1_puzzle_parallel_pthreads.cpp
 * @brief Parallel A* (best-first) solver for an N×M sliding puzzle using Pthreads
 * 
 * Implements a data decomposition strategy: divides the search space
 * by initial moves (UP, DOWN, LEFT, RIGHT), assigning each part to a thread.
 * Each thread executes an independent A* search until the goal is found or
 * all options are exhausted.
 * 
 * Synchronization between threads is achieved through a shared mutex and
 * a global termination flag, ensuring consistent access to the best solution.
 * 
 * @note Compile with: g++ -pthread h1_puzzle_parallel_pthreads.cpp -o puzzle
 * @author JuCollas
 * @version 2.0
 */

#include <iostream>
#include <queue>
#include <unordered_set>
#include <utility>
#include <string>
#include <pthread.h>
#include <atomic>
#include <vector>
#include <chrono>

using namespace std;

const char EMPTY_VALUE = '#';
string GOAL;
vector<pair<int,int>> goalPos;
int nRow, nCol;

const int NUMBER_MOV = 4;
const int NUM_THREADS = 8;
const int dRow[] = {-1, 1, 0, 0}; // UP, DOWN, LEFT, RIGHT
const int dCol[] = {0, 0, -1, 1};
const string MOVES[] = {"UP", "DOWN", "LEFT", "RIGHT"};

/** Structure representing a puzzle state */
struct State {
    string board;
    int blankPos;
    int cost_g;
    int cost_h;
    State(string b, int pos, int g, int h)
        : board(std::move(b)), blankPos(pos), cost_g(g), cost_h(h) {}
};

/** Comparator to prioritize lowest f = g + h */
struct CompareState {
    bool operator()(const State& lhs, const State& rhs) const noexcept {
        return (lhs.cost_g + lhs.cost_h) > (rhs.cost_g + rhs.cost_h);
    }
};

// =============================================================================
// Helper Functions
// =============================================================================

string swapBoardTiles(const string& currentBoard, int p1, int p2) {
    string newBoard = currentBoard;
    std::swap(newBoard[p1], newBoard[p2]);
    return newBoard;
}

int findEmptySpace(const string& board) {
    for (int i = 0; i < (int)board.size(); ++i)
        if (board[i] == EMPTY_VALUE) return i;
    return -1;
}

pair<int, int> index_to_cord(int index) {
    return {index / nCol, index % nCol};
}

int cord_to_index(const pair<int, int>& cord) {
    return cord.first * nCol + cord.second;
}

bool is_valid(const pair<int, int>& cord) {
    return 0 <= cord.first && cord.first < nRow && 0 <= cord.second && cord.second < nCol;

}

vector<pair<int,int>> calculeGoalPos(int n, int m){
    vector<pair<int,int>> ans;
    for(int i=0;i<n*m-1;i++) ans.push_back(index_to_cord(i));
    return ans;
}

struct HeuristicArgs {
    const string* board;
    int startRow;
    int endRow;
    int partialSum;
};

int manhattan(const pair<int,int>& a, const pair<int,int>& b){
    return abs(a.first - b.first) + abs(a.second - b.second);
}

int char_to_index(char x){ return x - 'A'; }

void* partialHeuristic(void* arg){
    HeuristicArgs* data = (HeuristicArgs*)arg;
    const string &board = *(data->board);
    int idx = data->startRow * nCol;
    data->partialSum = 0;
    for(int i = data->startRow; i < data->endRow; ++i){
        for(int j = 0; j < nCol; ++j){
            if(board[idx] != EMPTY_VALUE){
                data->partialSum += manhattan({i,j}, goalPos[char_to_index(board[idx])]);
            }
            idx++;
        }
    }
    return nullptr;
}

int sumManhattanDist_parallel(const string &board){
    pthread_t threads[NUM_THREADS];
    HeuristicArgs args[NUM_THREADS];

    int rowsPerThread = nRow / NUM_THREADS;
    int total = 0;

    for(int i=0;i<NUM_THREADS;i++){
        args[i].board = &board;
        args[i].startRow = i * rowsPerThread;
        args[i].endRow = (i == NUM_THREADS-1)? nRow : (i+1)*rowsPerThread;
        pthread_create(&threads[i], nullptr, partialHeuristic, &args[i]);
    }

    for(int i=0;i<NUM_THREADS;i++){
        pthread_join(threads[i], nullptr);
        total += args[i].partialSum;
    }
    return total;
}

string calculeGoal(int n, int m) {
    string ans;
    char c = 'A';
    for (int i = 0; i < (n * m) - 1; ++i) ans.push_back(c++);
    ans.push_back(EMPTY_VALUE);
    return ans;
}

// =============================================================================
// Parallel Search Structures
// =============================================================================

struct ThreadArgs {
    string startBoard;
    int initialMove;
    int nodesExp;
};

pthread_mutex_t mutex_best;
atomic<bool> goalFound(false);
int bestSolution = -1;

/**
 * @brief Thread worker: executes A* for a given initial move
 */
void* thread_aStar(void* arg) {
    ThreadArgs* args = (ThreadArgs*)arg;
    string start = args->startBoard;
    int blankPos = findEmptySpace(start);

    auto [r, c] = index_to_cord(blankPos);
    pair<int, int> newCord = {r + dRow[args->initialMove], c + dCol[args->initialMove]};
    if (!is_valid(newCord)) pthread_exit(nullptr);

    int newPos = cord_to_index(newCord);
    string newBoard = swapBoardTiles(start, blankPos, newPos);

    priority_queue<State, vector<State>, CompareState> q;
    unordered_set<string> visited;

    q.emplace(newBoard, newPos, 1, sumManhattanDist_parallel(newBoard));
    visited.insert(newBoard);

    while (!q.empty() && !goalFound.load()) {
        State cur = q.top();
        q.pop();
        args->nodesExp += 1;
        if (cur.board == GOAL) {
            pthread_mutex_lock(&mutex_best);
            if (!goalFound || cur.cost_g < bestSolution || bestSolution == -1) {
                bestSolution = cur.cost_g;
                goalFound = true;
            }
            pthread_mutex_unlock(&mutex_best);
            break;
        }

        auto [r2, c2] = index_to_cord(cur.blankPos);
        for (int i = 0; i < NUMBER_MOV; ++i) {
            pair<int, int> newCord2 = {r2 + dRow[i], c2 + dCol[i]};
            if (is_valid(newCord2)) {
                int newPos2 = cord_to_index(newCord2);
                string nextBoard = swapBoardTiles(cur.board, cur.blankPos, newPos2);
                if (!visited.count(nextBoard)) {
                    visited.insert(nextBoard);
                    int h = sumManhattanDist_parallel(nextBoard);
                    q.emplace(std::move(nextBoard), newPos2, cur.cost_g + 1, h);
                }
            }
        }
    }

    pthread_exit(nullptr);
}

// =============================================================================
// Main Function
// =============================================================================

/*int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    cin >> nRow >> nCol;
    string start;
    cin >> start;

    GOAL = calculeGoal(nRow, nCol);
    pthread_mutex_init(&mutex_best, nullptr);

    vector<pthread_t> threads(NUMBER_MOV);
    vector<ThreadArgs> args(NUMBER_MOV);

    // Launch one thread per initial direction
    for (int i = 0; i < NUMBER_MOV; ++i) {
        args[i].startBoard = start;
        args[i].initialMove = i;
        pthread_create(&threads[i], nullptr, thread_aStar, &args[i]);
    }

    for (pthread_t& t : threads) pthread_join(t, nullptr);

    pthread_mutex_destroy(&mutex_best);

    cout << bestSolution << endl;
    return 0;
}*/

/**
 * @brief Main experiment: ejecución paralela del A* con heurística h1 o h2.
 * @note Ejecutar con: ./puzzle_h1.out < ../test/puzzles.txt
 */
int main() {
    nRow = 5;
    nCol = 5;
    string start;
    GOAL = calculeGoal(nRow, nCol);
    goalPos = calculeGoalPos(nRow,nCol);


    pthread_mutex_init(&mutex_best, nullptr);

    int nCase = 1;
    while (cin >> start) {
        goalFound.store(false);
        bestSolution = -1; 

        vector<pthread_t> threads(NUMBER_MOV);
        vector<ThreadArgs> args(NUMBER_MOV);

        auto start_time = chrono::high_resolution_clock::now();

        for (int i = 0; i < NUMBER_MOV; ++i) {
            args[i].startBoard = start;
            args[i].initialMove = i;
            args[i].nodesExp = 0;
            if (pthread_create(&threads[i], nullptr, thread_aStar, &args[i]) != 0) {
                cerr << "Error al crear el hilo " << i << endl;
                exit(EXIT_FAILURE);
            }
        }

        for (pthread_t& t : threads)
            pthread_join(t, nullptr);

        auto end_time = chrono::high_resolution_clock::now();
        chrono::duration<double> elapsed = end_time - start_time;

        cout << "Caso " << nCase++ << ":\n";
        cout << "Nodos expandidos por hilo:\n";
        for (int i = 0; i < NUMBER_MOV; ++i)
            cout << "  Hilo " << i << ": " << args[i].nodesExp << '\n';

        cout << "Mejor resultado (coste): " << bestSolution << '\n';
        cout << "Tiempo de ejecución: " << elapsed.count() << " segundos\n";
        cout << "---------------------------------------------\n";
    }

    pthread_mutex_destroy(&mutex_best);
    return 0;
}
