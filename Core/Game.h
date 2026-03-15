#pragma once
#include "../Models/PuzzleBoard.h"
#include <vector>

class Game {
public:
  Game(std::vector<WordEntry> wordList);

  // Getter for board
  const PuzzleBoard &getPuzzleBoard() const;

  //Function to get tile reference from board by position
  const Tile* GetTile(const Vector2& position) const;
  Tile* GetTile(const Vector2& position);

  //Getter for WordList
  const std::vector<WordEntry>& getWordList() const;

  // Places Words on board
  bool PlaceWords();

  // Check if word can be placed horizontally
  bool CheckHorizontalPlacement(std::string word, Vector2 tilePosition,
                                int letterOffset);

  // Check if Word can be placed vertically
  bool CheckVerticalPlacement(std::string word, Vector2 tilePosition,
                              int letterOffset);

  // Check If Tile Contains Word -> Returns True if it does
  bool CheckTile(Vector2 tilePosition);

  // Place word vertically on board
  void PlaceWordVertical(std::string word, Vector2 tilePosition,
                         int letterOffset);

  //Place word horizontally on board
  void PlaceWordHorizontal(std::string word, Vector2 tilePosition,
                           int letterOffset);

  //Return tile positions (all) by word
  std::vector<Vector2> ReturnPositionsByWord(std::string word);

  //Get tile position (1) by id
  const Vector2& GetPositionById(int id) const;

  //Get ID of Tile by position
  int GetIdByPosition(const Vector2& pos) const;

  //Check if Vec2 position is in bound of board
  bool IsPosInBounds(const Vector2& pos) const;
  
  //Check if Tile has word vertically 
  bool IsTileOccupiedVertical(const Vector2& pos) const;

  //Check if Tile has word horizontally
  bool IsTileOccupiedHorizontal(const Vector2& pos) const;

  //Funct to restart game given new wordlist
  void RestartGame(const std::vector<WordEntry> words);

private:
    //Our Puzzleboard struct
  PuzzleBoard puzzleBoard;
  //Wordlist
  std::vector<WordEntry> wordList;
};
