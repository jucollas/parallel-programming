/**
 * @file board_printer.cpp
 * @brief 4x4 Board Initialization and Printing Utility
 * 
 * This program reads a 16-character string representing a 4x4 board configuration
 * and converts it into a 2D grid format for display. The input string is interpreted
 * as row-major order (left to right, top to bottom).
 * 
 * Input format: 16 consecutive characters (no spaces)
 * Output format: 4x4 grid with space-separated columns and newline-separated rows
 * 
 * Example:
 *   Input:  "ABCDEFGHIJKLMNO#"
 *   Output: A B C D
 *           E F G H
 *           I J K L
 *           M N O #
 * 
 * @author JuCollas
 * @version 1.0
 */

#include <iostream>
#include <string>

using namespace std;
const int DEFAULT_SIZE = 4;

/**
 * @brief Prints a 4x4 board with proper formatting
 */

void print_board(string& board, int n, int m){
      int k = 0;
      for (int i = 0; i < n; i++){
            if (i != 0) cout << '\n';
            cout << board[k++];
            for (int j = 1; j < m; j++){
                  cout << ' ' << board[k++];
            }
      }
}

/**
 * Reads a 16-character input string, converts it to a 4x4 board representation,
 * Compilation: 
 *      g++ -std=c++11 -o board_printer board_printer.cpp
 * 
 * # Linux/Mac/WSL
 * echo "ABCDEFGHIJKLMNO#" | ./board_printer
 * 
 * # Windows Command Prompt
 * echo ABCDEFGHIJKLMNO# | board_printer.exe
 * 
 * # Windows PowerShell
 * "ABCDEFGHIJKLMNO#" | .\board_printer.exe
 * 
 */
int main(){
      string board;
      cin >> board;
      print_board(board, DEFAULT_SIZE, DEFAULT_SIZE);
      return 0;
}