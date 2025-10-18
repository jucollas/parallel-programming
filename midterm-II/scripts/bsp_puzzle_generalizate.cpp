/**
 * @file bsp_puzzle_generalizate.cpp
 * @brief 4x4 Sliding Puzzle Solver using BFS (Breadth-First Search)
 * 
 * This program solves the 4x4 sliding puzzle by finding the minimum number
 * of moves required to reach the goal state from the initial state.
 * 
 * Goal state: "ABCDEFGHIJKLMNO#"
 * Where '#' represents the empty space.
 * 
 * @author JuCollas
 * @version 1.7
 */

#include <iostream>
#include <queue>
#include <unordered_set>
#include <utility>
#include <string>
#include <chrono>

using namespace std;

const char EMPTY_VALUE = '#';

string GOAL;
int nRow, nCol;
int nodesExp;

/**
 * @brief Possible movement directions
 * 
 * Up, Down, Left, Right
 */
const int NUMBER_MOV = 4;
const int dRow[] = {-1, 1, 0, 0}; // UP, DOWN, LEFT, RIGHT
const int dCol[] = {0, 0, -1, 1};
const string MOVES[] = {"UP", "DOWN", "LEFT", "RIGHT"};

struct State{
    string board;   ///< flattened board (row-major)
    int blankPos;   ///< index of the empty cell '#'
    int cost;       ///< steps taken from the start
    // NOTE: keep as simple POD-like struct for BFS queue
    State(string b, int pos, int c) : board(std::move(b)), blankPos(pos), cost(c) {}
};

// =============================================================================
// HELPER FUNCTIONS
// =============================================================================

/**
 * @brief Swaps two tiles on the board and returns the new board state
 * 
 * The function assumes @p position1 is the current location of the empty
 * cell '#'. It swaps the characters at @p position1 and @p position2 so the
 * empty cell moves to @p position2.
 *
 * @param currentBoard Current board string (flattened)
 * @param position1 Index of the empty cell '#'
 * @param position2 Index of the neighbor to swap with
 * @return A new board string with the two positions swapped
 */
string swapBoardTiles(const string& currentBoard, int position1, int position2){
    string newBoard = currentBoard;           // one local copy (unavoidable for new state)
    std::swap(newBoard[position1], newBoard[position2]); // correct and cheap swap
    return newBoard;
}

/**
 * @brief Finds the index of the empty space '#'
 * @param board Board string (flattened)
 * @return Index of '#', or -1 if not found
 */
int findEmptySpace(const string& board) {
    for (int i = 0; i < static_cast<int>(board.size()); ++i){
        if (board[i] == EMPTY_VALUE) return i;
    }
    return -1;
}

/**
 * @brief Maps linear index -> (row, col)
 * @param index Linear index
 * @return Pair (row, col)
 */
pair<int, int> index_to_cord(int index) {
    return {index / nCol, index % nCol};
}

/**
 * @brief Maps (row, col) -> linear index
 * @param cord Pair (row, col)
 * @return Linear index
 */
int cord_to_index(const pair<int, int>& cord) {
    return cord.first * nCol + cord.second;
}

/**
 * @brief Checks grid bounds for a coordinate
 * @param cord Pair (row, col)
 * @return true if (row, col) is inside the board
 */
bool is_valid(const pair<int, int>& cord) {
    return 0 <= cord.first && cord.first < nRow && 0 <= cord.second && cord.second < nCol;
}

/**
 * @brief Calulate state goal for size n x m
 * @param n number of rows
 * @param m number of colmuns
 * @return String representend goal state 
 */
string calculeGoal(int n, int m){
  string ans;
  char curret = 'A';
  for(int i = 0; i < (n * m) - 1; ++i){
    ans.push_back(curret);
    curret += 1;
  }
  ans.push_back(EMPTY_VALUE);
  return ans;
}

/**
 * @brief Breadth-First Search to find shortest path to goal state
 * 
 * Explores all possible states level by level, guaranteeing the shortest path
 * will be found first due to BFS properties.
 * 
 * @param start Initial board configuration
 * @return Minimum number of moves to reach GOAL, or -1 if unreachable
 */
int bfs(const string& start){
    queue<State> q;
    unordered_set<string> visited;

    int blankPos = findEmptySpace(start);

    q.emplace(start, blankPos, 0);
    visited.emplace(start);

    int ans = -1;

    while (!q.empty() && ans == -1){
        State cur = q.front();
        q.pop();
        nodesExp += 1;
        if (cur.board == GOAL){
            ans = cur.cost;
        } else {
            const pair<int, int> curPos = index_to_cord(cur.blankPos);
            for (int i = 0; i < NUMBER_MOV; i++){
                const pair<int, int> newCord = {curPos.first + dRow[i], curPos.second + dCol[i]};
                if (is_valid(newCord)){
                      const int newPos = cord_to_index(newCord);
                      string newBoard = swapBoardTiles(cur.board, cur.blankPos, newPos);
                      if (!visited.count(newBoard)){
                          visited.emplace(newBoard);
                          q.emplace(std::move(newBoard), newPos, cur.cost + 1);
                      }
                }

            }
        }

    }
    return ans;
}

// =============================================================================
// MAIN FUNCTION
// =============================================================================

int main(){
    cin >> nRow >> nCol;
    string start;
    cin >> start;
    GOAL = calculeGoal(nRow, nCol);
    nodesExp = 0;

    auto start_time = chrono::high_resolution_clock::now();
    int result = bfs(start);
    auto end_time = chrono::high_resolution_clock::now();

    chrono::duration<double> elapsed = end_time - start_time;
    cout << "Node Exp: " << nodesExp << endl;
    cout << "Resultado: " << result << endl;
    cout << "Tiempo de ejecución: " << elapsed.count() << " segundos" << endl;
    return 0;
}

/**
 * @brief Main experimet
 * @note Excute with: puzzel_bfs.out < ../test/puzzles.txt
 */

/*int main(){
    nRow = 4;
    nCol = 4;
    string start;
    GOAL = calculeGoal(nRow, nCol);
    int nCase = 1;
    while (cin >> start){
        nodesExp = 0;
        auto start_time = chrono::high_resolution_clock::now();
        int result = bfs(start);
        auto end_time = chrono::high_resolution_clock::now();
        
        chrono::duration<double> elapsed = end_time - start_time;
        cout << "Caso " << nCase++ << ":\n";
        cout << "Node Exp: " << nodesExp << endl;
        cout << "Resultado: " << result << endl;
        cout << "Tiempo de ejecución: " << elapsed.count() << " segundos" << endl;
    }
    return 0;
}*/