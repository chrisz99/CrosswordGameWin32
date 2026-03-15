#include "Game.h"
#include <algorithm>

//Game Constructor
Game::Game(std::vector<WordEntry> wordList)
    : wordList(wordList), puzzleBoard(10, 30) {


  std::sort(this->wordList.begin(), this->wordList.end(),
            [](const auto &a, const auto &b) {
          return a.word.length() > b.word.length();
            });
  PlaceWords();
}

//Getter function for Puzzleboard Object
const PuzzleBoard &Game::getPuzzleBoard() const { return puzzleBoard; }

//Function to get tile reference from board by position
Tile* Game::GetTile(const Vector2& position)
{
    if (!IsPosInBounds(position))
        return nullptr;

    return &puzzleBoard.board[position.x][position.y];
}

const Tile* Game::GetTile(const Vector2& position) const
{
    if (!IsPosInBounds(position))
        return nullptr;

    return &puzzleBoard.board[position.x][position.y];
}

//Getter function for Wordlist
const std::vector<WordEntry>& Game::getWordList() const { return wordList; }

//Function to place words from wordlist on board
bool Game::PlaceWords() {

  if (wordList.empty())
    return false;

  //Initialize character counter for words in wordlist
  //Loop through wordlist -> increment counter
  int charCount = 0;

  for (const auto& word : wordList) {
      charCount += word.word.length();
  }

  //Wordlist is pre-sorted from game constructor
  //Initialize Max word length based off first index
  //Create puzzleboard based off char count and max word length
  const int maxWordLength = wordList[0].word.length();
  puzzleBoard = PuzzleBoard(maxWordLength + charCount / 8, 30);

  // Place the very word word in the center (Horizontally)
  for (int r = 0; r < wordList[0].word.length(); r++) {
    puzzleBoard.board[puzzleBoard.boardSize / 2][r + 3].horizontalWord =
        wordList[0].word;
    puzzleBoard.board[puzzleBoard.boardSize / 2][r + 3].horizontalCharIndex = r;
  }

  //Bool vector to track words placed
  std::vector<bool> wordPlaced(wordList.size(), false);
  wordPlaced[0] = true;


  // Keep sweeping through the list as long as we keep finding homes for words
  bool madeProgress = true;

  while (madeProgress) {
    madeProgress = false;

    //Loop starting at second word
    for (int i = 1; i < wordList.size(); i++) {
      if (wordPlaced[i])
        continue; // Already placed, skip

      bool isPlaced = false;

      for (int r = 0; r < puzzleBoard.boardSize && !isPlaced; r++) {
        for (int c = 0; c < puzzleBoard.boardSize && !isPlaced; c++) {

          //Create tile reference and char to hold current letter
          Tile &tile = puzzleBoard.board[r][c];

          char currentLetter = '\0';

          //Check if tile has letter / word -> set currentLetter if conditions are met
          if (tile.horizontalCharIndex >= 0 && !tile.horizontalWord.empty()) {
            currentLetter = tile.horizontalWord[tile.horizontalCharIndex];
          } else if (tile.verticalCharIndex >= 0 &&
                     !tile.verticalWord.empty()) {
            currentLetter = tile.verticalWord[tile.verticalCharIndex];
          }

          //If tile does not have letter check placements
          //Place word if conditions are met, prioritize vertical placement
          if (currentLetter != '\0') {
            for (int w = 0; w < wordList[i].word.length() && !isPlaced; w++) {
              if (wordList[i].word[w] == currentLetter) {
                if (CheckVerticalPlacement(wordList[i].word, Vec2(r, c), w)) {
                  PlaceWordVertical(wordList[i].word, Vec2(r, c), w);
                  isPlaced = true;
                } else if (CheckHorizontalPlacement(wordList[i].word, Vec2(r, c),
                                                    w)) {
                  PlaceWordHorizontal(wordList[i].word, Vec2(r, c), w);
                  isPlaced = true;
                }
              }
            }
          }
        }
      }

      //If word is placed -> modify vector -> set madeProgress to true
      if (isPlaced) {
        wordPlaced[i] = true;
        madeProgress =
            true; // We placed a word, so we should sweep again later!
      }
    }
  }

  return true;
}

//Function to check if word can be placed horizontally given word, position, and offset
bool Game::CheckHorizontalPlacement(std::string word, Vector2 tilePosition,
                                    int letterOffset) {

    //X position of where we start
  int startCol = tilePosition.y - letterOffset;

  // Bounds check
  if (startCol < 0 || startCol + word.length() > puzzleBoard.boardSize)
    return false;

  // Must intersect with an existing word
  if (!CheckTile(tilePosition))
    return false;

  int row = tilePosition.x;

  for (int r = 0; r < word.length(); r++) {
    int column = startCol + r;
    Tile &tile = puzzleBoard.board[row][column];

    // If tile already has a HORIZONTAL word, we can't place here
    if (!tile.horizontalWord.empty()) {
      return false;
    }

    // If tile has a VERTICAL word, letters must match
    if (!tile.verticalWord.empty()) {
      if (tile.verticalCharIndex >= 0 &&
          tile.verticalWord[tile.verticalCharIndex] != word[r])
        return false;
      // Letters match — valid intersection, skip neighbor check
      continue;
    }

    // Tile is entire empty — check UP and DOWN only (perpendicular to
    // horizontal)
    if (CheckTile(Vec2(row - 1, column)))
      return false;
    if (CheckTile(Vec2(row + 1, column)))
      return false;
  }

  // End caps — make sure nothing is directly touching the start/end of the word
  if (CheckTile(Vec2(row, startCol - 1)))
    return false;
  if (CheckTile(Vec2(row, startCol + word.length())))
    return false;

  return true;
}


//Function to check if word can be placed vertically given word, position, and offset
bool Game::CheckVerticalPlacement(std::string word, Vector2 tilePosition,
                                  int letterOffset) {
  int startRow = tilePosition.x - letterOffset;

  // Bounds check
  if (startRow < 0 || startRow + word.length() > puzzleBoard.boardSize)
    return false;

  if (!CheckTile(tilePosition))
    return false;

  int column = tilePosition.y;

  for (int c = 0; c < word.length(); c++) {
    int row = startRow + c;
    Tile &tile = puzzleBoard.board[row][column];

    // If tile already has a VERTICAL word, we can't place here
    if (!tile.verticalWord.empty()) {
      return false;
    }

    // If tile has a HORIZONTAL word, letters must match
    if (!tile.horizontalWord.empty()) {
      if (tile.horizontalCharIndex >= 0 &&
          tile.horizontalWord[tile.horizontalCharIndex] != word[c])
        return false;
      // Letters match — valid intersection, skip neighbor check
      continue;
    }

    // Tile is entire empty — check LEFT and RIGHT only (perpendicular to
    // vertical)
    if (CheckTile(Vec2(row, column - 1)))
      return false;
    if (CheckTile(Vec2(row, column + 1)))
      return false;
  }

  // End caps — make sure nothing is directly touching above/below the word
  if (CheckTile(Vec2(startRow - 1, column)))
    return false;
  if (CheckTile(Vec2(startRow + word.length(), column)))
    return false;

  return true;
}

// Returns True if Tile Contains ANY Word
bool Game::CheckTile(Vector2 tilePosition) {
  // Bounds check
  if (tilePosition.x < 0 || tilePosition.x >= puzzleBoard.boardSize ||
      tilePosition.y < 0 || tilePosition.y >= puzzleBoard.boardSize)
    return false;

  Tile &tile = puzzleBoard.board[tilePosition.x][tilePosition.y];

  // Return True if Tile contains Word
  if ((!tile.horizontalWord.empty() && tile.horizontalCharIndex >= 0) ||
      (!tile.verticalWord.empty() && tile.verticalCharIndex >= 0))
    return true;

  return false;
}

//Function to place word vertically given word, position, and offset
void Game::PlaceWordVertical(std::string word, Vector2 tilePosition,
                             int letterOffset) {
  for (int c = 0; c < word.length(); c++) {
    int row = (tilePosition.x - letterOffset) + c;
    int column = tilePosition.y;
    Tile &tile = puzzleBoard.board[row][column];
    if (tile.verticalWord.empty()) {
      tile.verticalWord = word;
      tile.verticalCharIndex = c;
    }
  }
}

//Function to place word horizontally given word, position, and offset
void Game::PlaceWordHorizontal(std::string word, Vector2 tilePosition,
                               int letterOffset) {
  for (int r = 0; r < word.length(); r++) {
    int row = tilePosition.x;
    int column = (tilePosition.y - letterOffset) + r;
    Tile &tile = puzzleBoard.board[row][column];
    if (tile.horizontalWord.empty()) {
      tile.horizontalWord = word;
      tile.horizontalCharIndex = r;
    }
  }
}


//Function to return all tile positions by word
std::vector<Vector2> Game::ReturnPositionsByWord(std::string word) {
  std::vector<Vector2> positionList;

  for (int r = 0; r < puzzleBoard.boardSize; r++) {
    for (int c = 0; c < puzzleBoard.boardSize; c++) {
      Tile &tile = puzzleBoard.board[r][c];
      if (tile.horizontalWord == word || tile.verticalWord == word)
        positionList.push_back(Vec2(r, c));
    }
  }
  return positionList;
}

//Function to return tile position by id
const Vector2& Game::GetPositionById(int id) const
{
    for (int r = 0; r < puzzleBoard.boardSize; r++) {
        for (int c = 0; c < puzzleBoard.boardSize; c++) {
            const Tile& tile = puzzleBoard.board[r][c];
            if (tile.id == id)
                return Vec2(r, c);
        }
    }

    return Vec2(-1, -1);
}

//Function to return ID by position
int Game::GetIdByPosition(const Vector2& pos) const
{
    if (!IsPosInBounds(pos))
        return 0;



    return puzzleBoard.board[pos.x][pos.y].id;
}

//Function to determine if position is within bounds of board
bool Game::IsPosInBounds(const Vector2& pos) const
{
    //Check if Pos is valid in board and is a non negative value
    if (pos.x >= puzzleBoard.boardSize || pos.y >= puzzleBoard.boardSize || pos.x < 0 && pos.y < 0)
        return false;



    return true;
}

//Function to check if tile has word vertically
bool Game::IsTileOccupiedVertical(const Vector2& pos) const
{
    if (!IsPosInBounds(pos))
        return false;


    //Check if Pos has associated word and char index
    const Tile& tile = puzzleBoard.board[pos.x][pos.y];

    if (!tile.verticalWord.empty() && tile.verticalCharIndex >= 0)
        return true;
    else
        return false;
}

//Function to check if tile has word horizontally
bool Game::IsTileOccupiedHorizontal(const Vector2& pos) const
{
    if (!IsPosInBounds(pos))
        return false;


    //Check if Pos has associated word and char index
    const Tile& tile = puzzleBoard.board[pos.x][pos.y];

    if (!tile.horizontalWord.empty() && tile.horizontalCharIndex >= 0)
        return true;
    else
        return false;
}

//Function to restart game
void Game::RestartGame(const std::vector<WordEntry> words)
{
    if (words.empty())
        return;

    wordList = words;

    std::sort(this->wordList.begin(), this->wordList.end(),
        [](const auto& a, const auto& b) {
            return a.word.length() > b.word.length();
        });
    PlaceWords();
}
