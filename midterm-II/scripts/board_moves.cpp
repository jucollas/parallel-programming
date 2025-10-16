/**
 * @file board_moves.cpp
 * @brief 4x4 Sliding Puzzle Move Simulator
 * @author JuCollas
 * @version 1.0
 */
#include <iostream>
#include <vector>
#include <map>
#include <string>
#include <cstddef>

using namespace std;

const char EMPTY_VALUE = '#';
const int DEFAULT_SIZE = 4;

const map<string, pair<int, int>> actions = {
    std::make_pair("UP",    std::make_pair(-1, 0)),
    std::make_pair("DOWN",  std::make_pair( 1, 0)),
    std::make_pair("LEFT",  std::make_pair( 0,-1)),
    std::make_pair("RIGHT", std::make_pair( 0, 1))
};

/**
 * @brief Simple board for an m x n sliding puzzle
 *
 * Holds the board as a flat string and the index of the empty cell.
 */
class Board{
private:
  string content;   ///< flattened board, row-major
  int emp;          ///< current index of EMPTY_VALUE in content
  int n;            ///< number of rows
  int m;            ///< number of cols

  /**
   * @brief Converts a linear index to (row, col)
   * @param index Linear index in [0, n*m)
   * @return (row, col) coordinates
   */
  pair<int, int> index_to_cord(int index) const {
    return {index / m, index % m};
  }

  /**
   * @brief Converts (row, col) to a linear index
   * @param cord (row, col) coordinates
   * @return Linear index
   */
  int cord_to_index(const pair<int, int>& cord) const {
    return cord.first * m + cord.second;
  }

  /**
   * @brief Checks whether (row, col) lies inside the board
   * @param cord (row, col) coordinates
   * @return true if inside bounds, false otherwise
   */
  bool is_valid(const pair<int, int>& cord) const {
    return 0 <= cord.first && cord.first < n &&
           0 <= cord.second && cord.second < m;
  }

public:
  /**
   * @brief Builds a board from a flattened content string
   * @param _content Flattened content of size n*m
   * @param n Number of rows
   * @param m Number of columns
   *
   * If the content size does not match n*m, the board will try to use
   * the given content up to n*m cells and warn to std::cerr.
   */
  Board(const string& _content, int n, int m)
    : content(_content), emp(-1), n(n), m(m) {
    int i = 0;
    while (i < (int) content.size() && emp == -1) {
      if (content[i] == EMPTY_VALUE)
        emp = i;
      ++i;
    }
  }

  /**
   * @brief Prints the board with spaces between cells
   *
   * Format:
   * a b c d
   * e f g h
   * ...
   */
  void print_board() const {
    int k = 0;
    for (int i = 0; i < n; i++){
      if (i != 0) cout << '\n';
      cout << content[k++];
      for (int j = 1; j < m; j++){
        cout << ' ' << content[k++];
      }
    }
  }

  /**
   * @brief Executes a move by sliding the empty cell
   *
   * Looks up the direction in the global actions map and attempts to
   * swap the empty cell with its neighbor in that direction. The move
   * is ignored if the action is unknown or if the target cell is out of bounds.
   *
   * @param move One of {"UP","DOWN","LEFT","RIGHT"}
   */
  void doMove(const string& move){
    const pair<int, int>& act = actions.at(move);

    pair<int, int> cord = index_to_cord(emp);
    pair<int, int> cord_move = { cord.first + act.first, cord.second + act.second };

    if (is_valid(cord_move)){
      int index_move = cord_to_index(cord_move);
      std::swap(content[emp], content[index_move]);
      emp = index_move;
    }
  }
};

/**
 * @brief Main function - program entry point
 *
 * Reads input, initializes the board, and executes the requested move.
 * Input format:
 * - First token: flattened board string (length DEFAULT_SIZE*DEFAULT_SIZE)
 * - Second token: move action (UP/DOWN/LEFT/RIGHT)
 */
int main(){
  string line;
  string move;
  cin >> line >> move;
  Board board(line, DEFAULT_SIZE, DEFAULT_SIZE);
  board.doMove(move);
  board.print_board();
  return 0;
}
