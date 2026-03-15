#pragma once
#include "Vector.h"
#include <string>
#include <vector>
#include <windows.h>


//Individual Tile

struct Tile {
  // Holds ID for Window Edit Control
    int id = 0;

    // Horizontal word data
    std::string horizontalWord = "";
    int horizontalCharIndex = -1;

    // Vertical word data
    std::string verticalWord = "";
    int verticalCharIndex = -1;

  // If Tile is Empty
  bool isBlankTile = false;

  // Position of Tile
  Vector2 tilePos = {0.0f, 0.0f};
  COLORREF bgColor = RGB(255, 255, 255);
};

//Puzzleboard Struct
struct PuzzleBoard {

    //2D Array of Tiles
    std::vector<std::vector<Tile>> board;

    //Board / Box Size 
    int boardSize;
    int boxSize;

    //Constructor to build basic board
    PuzzleBoard(const int boardSize, const int boxSize)
        : boardSize(boardSize),
        boxSize(boxSize),
        board(boardSize, std::vector<Tile>(boardSize))
    {
        for (int r = 0; r < boardSize; r++) {
            for (int c = 0; c < boardSize; c++) {

                Tile& tile = board[r][c];

                tile.tilePos.x = static_cast<float>(boxSize * c);
                tile.tilePos.y = static_cast<float>(boxSize * r);
            }
        }
    }
};

//Single entry in wordlist
struct WordEntry {
    std::string word;
    std::string hint;
};