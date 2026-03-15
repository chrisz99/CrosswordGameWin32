#pragma once
#include "../Core/Game.h"
#include <unordered_map>
#include <windows.h>


//Class that handles UI for the Crossword Game
class GameWindow {

    //Enum IDS for Controls
public:
    enum class MenuBarIds : int {
        NewGame = 1000,
        SaveGame,
        LoadGame,
        About,
        AboutClose
    };

    enum class NewGameDialogIds : int {
        TitleLabel = 2000,
        TextBox,
        SubmitButton
    };


    //Static Windows UI Callback that acts as a Trampoline for Window Proc
  static LRESULT CALLBACK StaticWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam,
                                           LPARAM lParam);

  //Standard Windows UI Callback -> Handles bulk of all UI messages
  LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam,
                              LPARAM lParam);

  //WinMessageCallback Sub Functions
  
  LRESULT CALLBACK WPOnCommand(HWND hwnd, WPARAM wParam, LPARAM lParam);

  LRESULT CALLBACK WPOnCreate(HWND hwnd);

  LRESULT CALLBACK WPOnColorEdit(WPARAM wParam, LPARAM lParam);

  LRESULT CALLBACK WPOnColorStatic(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);


  //Windows UI Callback for New Game Window
 static LRESULT CALLBACK NewGameDialogProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

//NewGameDialog Sub Functions

 LRESULT CALLBACK NGDOnCommand(HWND hwnd,UINT uMsg, WPARAM wParam, LPARAM lParam);

 LRESULT CALLBACK NGDOnCreate(HWND hwnd);

 //Subclass that handles input and rendering for our tiles in the Crosswrod Game
 friend LRESULT CALLBACK TileSubclassProc(HWND hwnd, UINT uMsg, WPARAM wParam,
     LPARAM lParam);

 //Constructor for UI
  GameWindow(Game *gameInstance, HINSTANCE hInstance);

  //Builds our initial game window
  bool CreateGameWindow(HINSTANCE hInstance);

  //Builds Our UI Tile Controls and Populates ActiveTextFields
  bool CreateGameBoard(HWND hwnd);

  //Called Every Frame -> Checks our controls and game state and updates tile bg color
  void UpdateGameBoard();

  //Standard WIN32 Message Loop
  void GameLoop();

  //Resizes UI based on Window Width and Height
  void ResizeBoard(HWND hwnd);

  //Updates Clue Texts
  void UpdateClues();

  //Serializes Game State and Saves to txt file
  void SaveGame();

  //Loads txt file -> creates new    and gameboard
  void LoadGame(HWND hwnd);

  //Stores our Edit controls with ID in ActiveTextField map
  void AddActiveTextField(int id, HWND hwnd);

  //Retrieves those edit controls by ID
  HWND &GetActiveTextField(int id);

private:

    //Pointer to Game Instance and App Instance
  Game *gameInstance;
  HINSTANCE hInstance;

  //Container for our textfields
  std::unordered_map<int, HWND> activeTextFields;


  //Main Window HWND
  HWND mainHwnd;

  //Labels for our hints
  HWND hintsLabel;
  HWND horizontalWordsLabel;
  HWND verticalWordsLabel;

  //Container for our fonts
  HFONT hFont = nullptr;

  //Bool to determine last movement
  bool isLastMoveHorizontal = false;
};