/**
 * @file h1_puzzle_solver.cpp
 * @brief 4x4 Sliding Puzzle Solver using A* (best-first with f = g + h)
 * 
 * This program solves the 4x4 sliding puzzle by finding the minimum number
 * of moves required to reach the goal state from the initial state using A*.
 * 
 * Goal state: "ABCDEFGHIJKLMNO#"
 * where '#' represents the empty space.
 * 
 * A* uses a priority queue ordered by f(n) = g(n) + h(n),
 * where g(n) is the path cost (moves so far) and h(n) is the heuristic
 * estimate to the goal (here, a simple "misplaced tiles" count).
 * 
 * @author JuCollas
 * @version 1.7
 */

#include <iostream>
#include <queue>
#include <unordered_set>
#include <utility>
#include <string>

using namespace std;

const char EMPTY_VALUE = '#';

int nRow, nCol;
string GOAL;
vector<pair<int, int>> goalPos;

/**
 * @brief Possible movement directions
 * 
 * Up, Down, Left, Right.
 */
const int NUMBER_MOV = 4;
const int dRow[] = {-1, 1, 0, 0}; // UP, DOWN, LEFT, RIGHT
const int dCol[] = {0, 0, -1, 1};
const string MOVES[] = {"UP", "DOWN", "LEFT", "RIGHT"};


struct State {
    string board;    ///< flattened board (row-major)
    int blankPos;    ///< index of the empty cell '#'
    int cost_g;      ///< path cost from the start (g)
    int cost_h;      ///< heuristic cost to the goal (h)

    State(string b, int pos, int g, int h)
        : board(std::move(b)), blankPos(pos), cost_g(g), cost_h(h) {}
};

/**
 * @brief Comparator for priority_queue to prioritize the lowest total cost (f = g + h)
 *
 * For A*, we want the state with the smallest f on top. Since std::priority_queue
 * is a max-heap by default, this comparator returns true when lhs has a larger f
 * than rhs (so the smallest f has higher priority).
 */
struct CompareState {
    bool operator()(const State& lhs, const State& rhs) const {
        int f_lhs = lhs.cost_g + lhs.cost_h;
        int f_rhs = rhs.cost_g + rhs.cost_h;
        return f_lhs > f_rhs; 
    }
};

// =============================================================================
/** @name Helper functions */
// =============================================================================

/**
 * @brief Returns a new board after swapping two positions
 * 
 * Assumes @p position1 is the current location of the empty cell '#'.
 * Swaps the characters at @p position1 and @p position2 so the empty
 * cell moves to @p position2.
 *
 * @param currentBoard Current board string (flattened)
 * @param position1 Index of the empty cell '#'
 * @param position2 Index of the neighbor to swap with
 * @return A new board string with the two positions swapped
 */
string swapBoardTiles(const string& currentBoard, int position1, int position2){
    string newBoard = currentBoard;          
    std::swap(newBoard[position1], newBoard[position2]);
    return newBoard;
}

/**
 * @brief Finds the index of the empty space '#'
 * @param board Board string (flattened)
 * @return Index of '#', or -1 if not found
 */
int findEmptySpace(const string& board) {
    int ans = -1;
    int i = 0;
    while(i < (int) board.size() && ans == -1){
        if (board[i] == EMPTY_VALUE) ans = i;
        ++i;
    }
    return ans;
}

/**
 * @brief Converts a linear index to (row, col)
 * @param index Linear index
 * @return Pair (row, col)
 */
pair<int, int> index_to_cord(int index) {
    return {index / nCol, index % nCol};
}

/**
 * @brief Converts (row, col) to a linear index
 * @param cord Pair (row, col)
 * @return Linear index
 */
int cord_to_index(const pair<int, int>& cord) {
    return cord.first * nCol + cord.second;
}

/**
 * @brief Checks whether (row, col) is inside the board
 * @param cord Pair (row, col)
 * @return true if inside bounds; false otherwise
 */
bool is_valid(const pair<int, int>& cord) {
    return 0 <= cord.first && cord.first < nRow && 0 <= cord.second && cord.second < nCol;
}


int manhattan(const pair<int, int>& a, const pair<int, int>& b){
      return std::abs(a.first - b.first) + std::abs(a.second - b.second);
}

int char_to_index(char x){
      return x - 'A';
}

int sumManhattanDist(const string& board){
  int ans = 0;
  pair<int, int> u = {0, 0};
  int k = 0;
  for(int i = 0; i < nRow; ++i){
    u.second = 0;
    for(int j = 0; j < nCol; ++j){
      if(board[k] != EMPTY_VALUE)
        ans += manhattan(u, goalPos[char_to_index(board[k])]);
      u.second += 1;
      k++;
    }
    u.first += 1;
  }
  return ans;
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

vector<pair<int, int>> calculeGoalPos(int n, int m){
  vector<pair<int, int>> ans;
  for(int i = 0; i < (n * m) - 1; ++i){
    ans.push_back(index_to_cord(i));
  }
  return ans;
}

/**
 * @brief A* search to find the shortest path to the goal state
 * 
 * Uses a priority queue ordered by f = g + h. The first time the goal is popped,
 * the optimal number of moves has been found.
 * 
 * @param start Initial board configuration
 * @return Minimum number of moves to reach GOAL, or -1 if unreachable
 */
int aStarSearch(const string& start){
    priority_queue<State, vector<State>, CompareState> q;
    unordered_set<string> visited;

    int blankPos = findEmptySpace(start);

    q.emplace(start, blankPos, 0, sumManhattanDist(start));
    visited.emplace(start);

    int ans = -1;

    while (!q.empty() && ans == -1){
        State cur = q.top();
        q.pop();
        if (cur.board == GOAL){
            ans = cur.cost_g;
        } else {
            const pair<int, int> curPos = index_to_cord(cur.blankPos);
            for (int i = 0; i < NUMBER_MOV; i++){
                const pair<int, int> newCord = {curPos.first + dRow[i], curPos.second + dCol[i]};
                if (is_valid(newCord)){
                      const int newPos = cord_to_index(newCord);
                      string newBoard = swapBoardTiles(cur.board, cur.blankPos, newPos);
                      if (!visited.count(newBoard)){
                          const int h = sumManhattanDist(cur.board);
                          visited.emplace(newBoard);
                          q.emplace(std::move(newBoard), newPos, cur.cost_g + 1, h);
                      }
                }

            }
        }

    }
    return ans;
}


/**
 * @brief Main function - program entry point
 * 
 * Reads the initial board from standard input, runs A*, and prints
 * the minimum number of moves to reach the goal (or -1 if unreachable).
 */
int main(){
    cin >> nRow >> nCol;
    string start;
    cin >> start;
    GOAL = calculeGoal(nRow, nCol);
    goalPos = calculeGoalPos(nRow, nCol);
    int result = aStarSearch(start);
    cout << result << endl;
    return 0;
}
