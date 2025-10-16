/**
 * @file board_available.cpp
 * @brief 4x4 Sliding Puzzle Available Moves Finder
 * @author JuCollas
 * @version 1.0
 * 
* This program analyzes a 4x4 sliding puzzle board configuration and determines
 * all valid moves that can be made from the current state.
 * 
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
    : content(_content), emp(-1), n(n), m(m)
  {
    int i = 0; 
    while (i < (int) content.size() && emp == -1){
      if(content[i] == EMPTY_VALUE)
        emp = i;
      ++i;
    }
  }

  /**
    * @brief Finds and displays all available moves for the current board state
    *
    */
  vector<string> listAvailable() const {
    pair<int, int> cord = index_to_cord(emp);
    vector<string> ans;
    if(0 < cord.first) ans.push_back("UP");
    if(cord.first < n - 1) ans.push_back("DOWN");
    if(0 < cord.second) ans.push_back("LEFT");
    if(cord.second < m - 1) ans.push_back("RIGHT");
    return ans;
  }
};

/**
 * @brief Main function - program entry point
 * 
 * Reads the board configuration from standard input and displays all
 * available moves based on the empty space position.
 */
int main(){
      string line;
      cin >> line;
      Board board(line, DEFAULT_SIZE, DEFAULT_SIZE);
      vector<string> ans = board.listAvailable();
      for(int i = 0; i < (int) ans.size(); ++i){
        if(i != 0) cout << '\n';
        cout << ans[i];
      }
      return 0;
}