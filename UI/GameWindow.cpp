#include "GameWindow.h"
#include <sstream>
#include <fstream>

//Static Windows UI Callback that acts as a Trampoline for Window Proc
LRESULT CALLBACK GameWindow::StaticWindowProc(HWND hwnd, UINT uMsg,
                                             WPARAM wParam, LPARAM lParam) {
    
    //Initialize GameWindow Ptr -> Cast from LParam from Window Initialization
    GameWindow *pThis = nullptr;

  if (uMsg == WM_NCCREATE) {
    CREATESTRUCT *pCreate = reinterpret_cast<CREATESTRUCT *>(lParam);
    pThis = reinterpret_cast<GameWindow *>(pCreate->lpCreateParams);
    SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pThis));

  } else {
    pThis =
        reinterpret_cast<GameWindow *>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
  }

  //If we sucessfully capture our GameWindow ptr , trampoline to the actual WindowProc
  if (pThis) {
    return pThis->WindowProc(hwnd, uMsg, wParam, lParam);
  }

  //Return Defaults
  return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

//Static Win32 Callback for New Game Dialog
LRESULT CALLBACK GameWindow::NewGameDialogProc(HWND hwnd, UINT uMsg,
                                               WPARAM wParam, LPARAM lParam) {

    //Initialize GameWindow Ptr -> Cast from LParam from Window Initialization
  GameWindow *pThis = nullptr;
  if (uMsg == WM_NCCREATE) {


    CREATESTRUCT *pCreate = reinterpret_cast<CREATESTRUCT *>(lParam);
    pThis = reinterpret_cast<GameWindow *>(pCreate->lpCreateParams);

    SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pThis));
  } else {
    pThis =
        reinterpret_cast<GameWindow *>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
  }

  //New Game Dialog Message Switch
  switch (uMsg) {
  case WM_CREATE: {
      pThis->NGDOnCreate(hwnd);
    
    break;
  }

    //New Game Dialog Message Handling
  case WM_COMMAND: {
      pThis->NGDOnCommand(hwnd,uMsg,wParam,lParam);
      break;
    
  }
  //Case on exit
  case WM_CLOSE: {
      //Enable interaction with main window -> Grab Parent Window
      HWND hMainBoard = GetWindow(hwnd, GW_OWNER);
      EnableWindow(hMainBoard, TRUE);

    // Kill New Game Dialog Window
    DestroyWindow(hwnd);
    break;
  }
  }
  return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

LRESULT GameWindow::NGDOnCommand(HWND hwnd,UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    int wmId = LOWORD(wParam);

    switch (wmId) {

        //Submit Button Action
        //Grabs and parses text in text field with error checking
        //If everything parses correctly -> set up new game
    case (int)NewGameDialogIds::SubmitButton: {

        //Grabbing text from the Edit Field in New Game Dialog
        HWND textField = GetDlgItem(hwnd, (int)NewGameDialogIds::TextBox);
        int length = GetWindowTextLength(textField);
        std::wstring text(length + 1, L'\0');
        GetWindowText(textField, &text[0], length + 1);
        text.resize(length);

        //Initialize stream reader to parse text line by line
        std::wistringstream stream(text);
        std::wstring line;

        //Initialize new word list object
        std::vector<WordEntry> wordList;

        //Loop text line by line
        while (std::getline(stream, line))
        {
            //If Line is Empty -> Continue
            if (line.empty())
                continue;

            // Strip carriage return from Windows line endings (\r\n)
            if (!line.empty() && line.back() == L'\r')
                line.pop_back();

            //If Line is still Empty -> Continue
            if (line.empty())
                continue;

            //Get index of comma in line
            size_t commaPos = line.find(L',');

            // No comma found - malformed line, skip it
            if (commaPos == std::wstring::npos)
                continue;

            //Parse everything before the comma and after
            std::wstring first = line.substr(0, commaPos);
            std::wstring second = line.substr(commaPos + 1);

            // Trim whitespace from both sides of each value
          // so "lost , Unable to find" doesn't break things
            auto trim = [](std::wstring& s) {
                size_t start = s.find_first_not_of(L" \t");
                size_t end = s.find_last_not_of(L" \t");
                s = (start == std::wstring::npos) ? L"" : s.substr(start, end - start + 1);
                };
            trim(first);
            trim(second);

            // Skip if either side is empty after trimming
            if (first.empty() || second.empty())
                continue;

            // Skip if word contains non-alpha characters
            bool validWord = true;

            for (wchar_t ch : first) {
                if (!iswalpha(ch)) {
                    validWord = false;
                    break;
                }
            }

            //Parse wstrings into std::strings 
            std::string firstPair = std::string(first.begin(), first.end());
            std::string secondPair = std::string(second.begin(), second.end());

            //Check for duplicates -> Flag Error if duplicate is present
            for (const auto& word : wordList) {
                if (word.word == firstPair)
                    validWord = false;
            }

            //Error Message
            if (!validWord)
            {
                MessageBox(hwnd, L"Please enter valid words (letters only, one per line, word,clue format).", L"Input Error", MB_OK | MB_ICONWARNING);
                return DefWindowProc(hwnd, uMsg, wParam, lParam); // stay on dialog
            }

            WordEntry entry;
            entry.word = firstPair;
            entry.hint = secondPair;

            //Push Word/Hint into Wordlist if all conditions are met
            wordList.push_back(entry);


        }

        // After the loop, check we got something usable
        if (wordList.empty())
        {
            MessageBox(hwnd, L"No valid words were entered.", L"Input Error", MB_OK | MB_ICONWARNING);
            break;
        }

        //If wordlist is not empty -> Set up a new game with it
        gameInstance->RestartGame(wordList);

        HWND hMainBoard = GetWindow(hwnd, GW_OWNER);
        CreateGameBoard(hMainBoard);

        UpdateClues();
        ResizeBoard(hMainBoard);
        EnableWindow(hMainBoard, TRUE);

        // 2. Kill ONLY this popup window
        DestroyWindow(hwnd);
        break;

    }
    }
    return 0;
}

LRESULT GameWindow::NGDOnCreate(HWND hwnd)
{
    //Get Size of New Game Dialog Window
    RECT rect;
    GetClientRect(hwnd, &rect);

    int halfWidth = rect.right / 2;
    int halfHeight = rect.bottom / 2;

    //Header Label For New Game Dialog
    CreateWindowEx(0, L"STATIC",
        L"Enter Text Below to Start a New Game.",
        WS_CHILD | WS_VISIBLE, halfWidth / 2, 10, halfWidth, 50, hwnd,
        (HMENU)NewGameDialogIds::TitleLabel, NULL, NULL);

    //Initialize wstring to hold wordlist ( words / hints )
    std::wstring wordList;

    //Iterate through wordlist -> append words and hints ( word , hint ) 
    for (const auto& word : gameInstance->getWordList()) {
        wordList += std::wstring(word.word.begin(), word.word.end()) + L"," + std::wstring(word.hint.begin(), word.hint.end()) + L"\r\n";
    }

    //Text Field for New Game Dialog
    CreateWindowEx(WS_EX_CLIENTEDGE, L"EDIT", wordList.c_str(), WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL | ES_MULTILINE, 20, 60, halfWidth * 2 - 40, halfHeight * 1.5, hwnd, (HMENU)NewGameDialogIds::TextBox, NULL, NULL);

    //Button for New Game Dialog
    CreateWindowEx(0, L"BUTTON", L"Create Game", WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON, halfWidth / 2, halfHeight * 1.5 + 70, halfWidth, halfHeight / 6, hwnd, (HMENU)NewGameDialogIds::SubmitButton, NULL, NULL);

    return 0;
}

//Window Message Callback for Main Window
LRESULT CALLBACK GameWindow::WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam,
                                        LPARAM lParam) {
   
    //Main Message Switch
  switch (uMsg) {
  case WM_CREATE: {
      return WPOnCreate(hwnd);
      

  }
  case WM_COMMAND: {
     return WPOnCommand(hwnd,wParam, lParam);
    
  }

  //Draw Case
  case WM_CTLCOLOREDIT: {
      return WPOnColorEdit(wParam, lParam);
  }

//Draw Case ( Static Controls )
  case WM_CTLCOLORSTATIC: {
      return WPOnColorStatic(hwnd, uMsg, wParam, lParam);
  }
  
//Window Resize Case -> Invalidate UI / Resize Board
  case WM_SIZE: {
    InvalidateRect(hwnd, NULL,
                   TRUE); // Scrub the background clean of text ghosts
    ResizeBoard(hwnd);
    return 0;
  }
//Close Case
  case WM_CLOSE:
  {
      HWND parent = GetWindow(hwnd, GW_OWNER);
      if (parent) {
          EnableWindow(parent, TRUE);
          ResizeBoard(parent);
      }
      DestroyWindow(hwnd);
      return 0;
  }
  //Destroy Case -> Checks if we are closing Main Window -> Closes
  case WM_DESTROY: {
      if (hwnd == mainHwnd)
    PostQuitMessage(0);
    return 0;
  }
  }
  return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

LRESULT GameWindow::WPOnCommand(HWND hwnd, WPARAM wParam, LPARAM lParam)
{
    // Grab ID of Control that sends command
    int wmId = LOWORD(wParam);

    // Grab Event Type of Command
    int wmEvent = HIWORD(wParam);

    // Grab Handle of the Control
    HWND hControl = (HWND)lParam;

    //Main Window Message Switch ( ID ) 

    switch (wmId) {

        //If user clicks on New Game option

    case (int)MenuBarIds::NewGame: {

        // Register Window Class for Popup with NewGameDialogProc Callback
        WNDCLASS inputWc = {};
        inputWc.lpfnWndProc = NewGameDialogProc;
        inputWc.lpszClassName = L"NewGameDialog";
        inputWc.hbrBackground = CreateSolidBrush(RGB(240, 240, 240));

        RegisterClass(&inputWc);

        //Create New Game Dialog Window
        HWND inputWindow =
            CreateWindowEx(WS_EX_DLGMODALFRAME, L"NewGameDialog", L"Enter Words",
                WS_VISIBLE | WS_SYSMENU | WS_CAPTION, CW_USEDEFAULT,
                CW_USEDEFAULT, 400, 600, hwnd, NULL, NULL, this);

        //If New Game Dialog is created, disable interaction with main window

        if (inputWindow) {
            // Disable the MAIN window (hwnd), not the popup (inputWindow)!
            EnableWindow(hwnd, FALSE);
        }

        break;
    }
                                 //If User Hits Save Game Option
    case (int)MenuBarIds::SaveGame: {
        SaveGame();
        break;
    }
                                  //If User hits Load Game Option
    case (int)MenuBarIds::LoadGame: {
        LoadGame(hwnd);
        break;
    }
                                  //If User Closes the About Dialog
    case (int)MenuBarIds::AboutClose: {
        HWND parent = GetWindow(hwnd, GW_OWNER);
        if (parent) {
            EnableWindow(parent, TRUE);
            ResizeBoard(parent);
        }
        DestroyWindow(hwnd);
        return 0;

    }
                                    //If User Clicks on the About Dialog
    case (int)MenuBarIds::About: {
        //Create and Register About Class using StaticWindowProc Message Callback
        WNDCLASS aboutWc = {};
        aboutWc.lpfnWndProc = StaticWindowProc;
        aboutWc.lpszClassName = L"AboutGameDialog";
        aboutWc.hbrBackground = CreateSolidBrush(RGB(240, 240, 240));

        RegisterClass(&aboutWc);

        //Create About Window
        HWND aboutWindow = CreateWindowEx(WS_EX_DLGMODALFRAME, L"AboutGameDialog", L"About Game",
            WS_VISIBLE | WS_SYSMENU | WS_CAPTION, CW_USEDEFAULT, CW_USEDEFAULT, 400, 400, hwnd, NULL, NULL, this);

        //If its sucessfully created, disable interaction with main window
        if (aboutWindow) {
            EnableWindow(hwnd, FALSE);
            ResizeBoard(hwnd);
        }
    }
    }

    // Check if text field is edited
    if (wmEvent == EN_CHANGE) {

        UpdateGameBoard();


        // Check if its our textbox id
        Vec2 tilePosition = gameInstance->GetPositionById(wmId);

        //Initializes next positions for up and right
        Vec2 nextHorizontalPosition = tilePosition;
        nextHorizontalPosition.y += 1;
        Vec2 nextVerticalPosition = tilePosition;
        nextVerticalPosition.x += 1;

        //Creates the checks for vertical and horizontal movement
        bool canGoVertical = gameInstance->IsPosInBounds(tilePosition) &&
            gameInstance->IsTileOccupiedVertical(tilePosition);
        bool canGoHorizontal =
            gameInstance->IsPosInBounds(tilePosition) &&
            gameInstance->IsTileOccupiedHorizontal(tilePosition);

        //Initialize int for next tile ID
        int nextTileId = 0;

        //Set Next Tile ID based off of where user went previously and where user can go

        if (canGoHorizontal && isLastMoveHorizontal) {
            nextTileId = gameInstance->GetIdByPosition(nextHorizontalPosition);
        }
        else if (canGoVertical && !isLastMoveHorizontal)
            nextTileId = gameInstance->GetIdByPosition(nextVerticalPosition);
        else if (canGoHorizontal) {
            nextTileId = gameInstance->GetIdByPosition(nextHorizontalPosition);
            isLastMoveHorizontal = true;
        }
        else if (canGoVertical) {
            nextTileId = gameInstance->GetIdByPosition(nextVerticalPosition);
            isLastMoveHorizontal = false;
        }

        //Gets handle for control of next textbox
        HWND nextBox = GetActiveTextField(nextTileId);

        //Sets Textbox focus to next textbox and selects all the text
        if (nextBox) {
            SetFocus(nextBox);
            SendMessage(nextBox, EM_SETSEL, 0, -1);
        }

        //Checks if tile was edited, and if tile has no text ( If users backspaces on a tile ) 
        if (GetWindowTextLength(hControl) == 0) {
            //Gets position on board based off ID

            Vec2 tilePosition = gameInstance->GetPositionById(wmId);

            //Moves that position horizontally backwards
            tilePosition.y -= 1;

            //Checks if it is a valid position on board -> Grabs ID of that Textbox and Sets Focus
            if (gameInstance->IsPosInBounds(tilePosition) &&
                gameInstance->IsTileOccupiedHorizontal(tilePosition)) {
                int newId = gameInstance->GetIdByPosition(tilePosition);
                if (newId > 0) {
                    HWND textBox = GetActiveTextField(newId);
                    SetFocus(textBox);
                }
            }
        }
    }

    return 0;   // Grab ID of Control that sends command
   
}

LRESULT GameWindow::WPOnCreate(HWND hwnd)
{
    //Grab window class name, if it matches our about window -> build UI for about window
    wchar_t className[64];
    GetClassName(hwnd, className, 64);

    if (wcscmp(className, L"AboutGameDialog") == 0)
    {
        CreateWindowEx(
            0,
            L"STATIC",
            L"Crossword Game\n\nCreated using C++ Win32 API\n\nCreated by www.github.com/chrisz99",
            WS_CHILD | WS_VISIBLE | SS_CENTER,
            50,
            50,
            300,
            200,
            hwnd,
            NULL,
            NULL,
            NULL);

        CreateWindowEx(
            0,
            L"BUTTON",
            L"OK",
            WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
            150,
            250,
            100,
            40,
            hwnd,
            (HMENU)MenuBarIds::AboutClose,
            NULL,
            NULL);

        return 0;
    }


    // Build Initial UI ( Populate UI Map / Create TextFields in Window )
    CreateGameBoard(hwnd);

    // Takes Window Size / Resizes and Centers Board
    ResizeBoard(hwnd);

    return 0;
}

LRESULT GameWindow::WPOnColorEdit(WPARAM wParam, LPARAM lParam)
{
    HDC hdc = (HDC)wParam;
    int id = GetDlgCtrlID((HWND)lParam);

    //Iterate through gameboard to find Tile by ID
    //Once found draw that tiles background color by RGB value in Tile
    for (int r = 0; r < gameInstance->getPuzzleBoard().boardSize; r++) {
        for (int c = 0; c < gameInstance->getPuzzleBoard().boardSize; c++) {
            Tile tile = gameInstance->getPuzzleBoard().board[r][c];
            if (tile.id == id) {
                SetBkColor(hdc, tile.bgColor);
                SetTextColor(hdc, RGB(0, 0, 0));

                return (LRESULT)CreateSolidBrush(tile.bgColor);
            }
        }
    }
    return (LRESULT)CreateSolidBrush(RGB(255, 255, 255));
}

LRESULT GameWindow::WPOnColorStatic(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    HDC hdcStatic = (HDC)wParam;
    HWND hwndControl = (HWND)lParam;

    // Check if the control asking for color is one of our hint labels -> Set Text / BG Color
    if (hwndControl == hintsLabel || hwndControl == horizontalWordsLabel ||
        hwndControl == verticalWordsLabel) {
        SetTextColor(hdcStatic, RGB(0, 0, 0));
        SetBkMode(hdcStatic, OPAQUE);
        SetBkColor(hdcStatic, RGB(227, 236, 250));
        return (LRESULT)CreateSolidBrush(RGB(227, 236, 250));
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

//Constructor for GameWindow class -> Sets hInstance and Game Instance
GameWindow::GameWindow(Game *gameInstance, HINSTANCE hInstance) : gameInstance(gameInstance), hInstance(hInstance) {}

// Pointer to the original EDIT control window procedure.
// When we subclass a control, we replace its window procedure with our own.
// We store the original so we can forward messages that we do not handle.
WNDPROC OriginalEditProc;

//Window Procedure for our Tile / Tile Controls

LRESULT CALLBACK TileSubclassProc(HWND hwnd, UINT uMsg, WPARAM wParam,
                                  LPARAM lParam) {

    //Check if tile is focused / not focused / and click -> Forward to regular Windows Process ( Original Edit Proc )
  if (uMsg == WM_SETFOCUS || uMsg == WM_KILLFOCUS || uMsg == WM_LBUTTONDOWN ||
      uMsg == WM_CHAR) {
    LRESULT result =
        CallWindowProc(OriginalEditProc, hwnd, uMsg, wParam, lParam);

    InvalidateRect(hwnd, NULL, FALSE);
    UpdateWindow(hwnd);

    return result;
  }
  //Paint Case -> Drawn
  if (uMsg == WM_PAINT) {
    HWND parentHwnd = GetParent(hwnd);

    GameWindow *pThis = reinterpret_cast<GameWindow *>(
        GetWindowLongPtr(parentHwnd, GWLP_USERDATA));

    // Call Regular Window Proc Function to Draw Our Text Control
    LRESULT result =
        CallWindowProc(OriginalEditProc, hwnd, uMsg, wParam, lParam);

    // Get Device Context from Textbox -> Get Window Device Context()
    HDC hdc = GetWindowDC(hwnd);

    //Get ID of control
    int id = GetDlgCtrlID(hwnd);

    //Get Position on board by ID
    Vec2 tilePos = pThis->gameInstance->GetPositionById(id);

    //Create Tile reference
    Tile tile =
        pThis->gameInstance->getPuzzleBoard().board[tilePos.x][tilePos.y];

    //Initialize Number Label String
    std::wstring numberLabel = L"";

    //Grab Wordlist
    const auto &wordList = pThis->gameInstance->getWordList();

    //Iterate through wordlist, if Tile contains word and is first char index -> Append number to label by index
    if (tile.horizontalCharIndex == 0)
      for (int i = 0; i < wordList.size(); i++) {
        if (tile.horizontalWord == wordList[i].word) {
          numberLabel += std::to_wstring(i + 1);
          break;
        }
      }
    else if (tile.verticalCharIndex == 0)
      for (int i = 0; i < wordList.size(); i++) {
        if (tile.verticalWord == wordList[i].word) {
          numberLabel += std::to_wstring(i + 1);
          break;
        }
      }

    // Make box bg transparent, set text color
    SetBkMode(hdc, TRANSPARENT);
    SetTextColor(hdc, RGB(100, 100, 100));

    // Rectangle to Draw it
    RECT rect;
    GetClientRect(hwnd, &rect);
    rect.left += 2;
    rect.top += 2;

    // Draw Label Text and Release Device Context
    DrawText(hdc, numberLabel.c_str(), -1, &rect,
             DT_TOP | DT_LEFT | DT_SINGLELINE);
    ReleaseDC(hwnd, hdc);

    return result;
  }

  //Detect if user presses keydown on tile
  if (uMsg == WM_KEYDOWN) {

    //Get Parent window, grab GameWindow class pointer
    HWND parentHwnd = GetParent(hwnd);
    GameWindow *pThis = reinterpret_cast<GameWindow *>(
        GetWindowLongPtr(parentHwnd, GWLP_USERDATA));

    // Safety check: If we somehow failed to get the GameWindow, abort and run
    // default textbox logic.
    if (!pThis)
      return CallWindowProc(OriginalEditProc, hwnd, uMsg, wParam, lParam);

    //Get ID of Control
    int id = GetDlgCtrlID(hwnd);

    //Get position by ID
    Vec2 currentTilePos = pThis->gameInstance->GetPositionById(id);

    // Initialize new position
    int boardSize = pThis->gameInstance->getPuzzleBoard().boardSize;
    int nextRow = currentTilePos.x;
    int nextCol = currentTilePos.y;

    // Based on which arrow key was pressed, we calculate the coordinate of the
    // tile we WANT to jump to.
    switch (wParam) {
    case VK_UP:
      nextRow -= 1;
      break; // Move up one row
    case VK_DOWN:
      nextRow += 1;
      break; // Move down one row
    case VK_LEFT:
      nextCol -= 1;
      break; // Move left one column
    case VK_RIGHT:
      nextCol += 1;
      break; // Move right one column
    case VK_BACK:
      nextCol -= 1; // Backspace Condition
      break;
    default:
        //If key press is anything but movement, forward to original Window Proc
      return CallWindowProc(OriginalEditProc, hwnd, uMsg, wParam, lParam);
    }

    //Saftey Check -> Is new position inside board
    if (nextRow >= 0 && nextRow < boardSize && nextCol >= 0 &&
        nextCol < boardSize) {

      // Create Tile Reference
      Tile nextTile =
          pThis->gameInstance->getPuzzleBoard().board[nextRow][nextCol];
        
      // Grab Control by id
      HWND nextTextbox = pThis->GetActiveTextField(nextTile.id);
        
      //Set Focus / Selection if control exists
      if (nextTextbox != NULL) {
        SetFocus(nextTextbox);
        SendMessage(nextTextbox, EM_SETSEL, 0, -1);
      }
    }


    return 0;
  }

  return CallWindowProc(OriginalEditProc, hwnd, uMsg, wParam, lParam);
}

//Function to create main game window
bool GameWindow::CreateGameWindow(HINSTANCE hInstance) {

  // Configure Window
  WNDCLASS wc = {};
  wc.lpfnWndProc = StaticWindowProc;
  wc.hInstance = hInstance;
  wc.lpszClassName = L"CrosswordClass";
  wc.hCursor = LoadCursor(NULL, IDC_ARROW);
  wc.hbrBackground = CreateSolidBrush(RGB(227, 236, 250));

  // Register Window // Error Checking
  if (!RegisterClass(&wc)) {
    return 0;
  }

  // Create main menu bar
  HMENU hMenuBar = CreateMenu();

  // Create First Dropdown Menu
  HMENU hGameDropdown = CreatePopupMenu();

  // Append New Game Option and Seperator
  AppendMenu(hGameDropdown, MF_STRING, (int)MenuBarIds::NewGame, L"New Game");
  AppendMenu(hGameDropdown, MF_SEPARATOR, 0, NULL);
  AppendMenu(hGameDropdown, MF_STRING, (int)MenuBarIds::LoadGame, L"Load Game");
  AppendMenu(hGameDropdown, MF_SEPARATOR, 0, NULL);
  AppendMenu(hGameDropdown, MF_STRING, (int)MenuBarIds::SaveGame, L"Save Game");

  // Append Dropdown to main menu bar
  AppendMenu(hMenuBar, MF_POPUP, (UINT_PTR)hGameDropdown, L"Game");

  AppendMenu(hMenuBar, MF_STRING, (int)MenuBarIds::About, L"About");

  // Create Window and Append Menu Bar
   mainHwnd = CreateWindowEx(0, L"CrosswordClass", L"CrosswordGame",
                             WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
                             1280, 720, NULL, hMenuBar, hInstance, this);

  // Check if window is created
  if (mainHwnd == NULL)
    return 0;

  // Create Windows For Labels

  hintsLabel = CreateWindowEx(
      0, L"STATIC", L"Game Loading...", WS_CHILD | WS_VISIBLE | SS_LEFT, 50,
      600, 1000, 100, mainHwnd, NULL,
      (HINSTANCE)GetWindowLongPtr(mainHwnd, GWLP_HINSTANCE), NULL);

  horizontalWordsLabel = CreateWindowEx(
      0, L"STATIC", L"", WS_CHILD | WS_VISIBLE | SS_LEFT, 50, 500, 400, 200,
      mainHwnd, NULL, (HINSTANCE)GetWindowLongPtr(mainHwnd, GWLP_HINSTANCE), NULL);
  verticalWordsLabel = CreateWindowEx(
      0, L"STATIC", L"", WS_CHILD | WS_VISIBLE | SS_LEFT, 500, 500, 400, 200,
      mainHwnd, NULL, (HINSTANCE)GetWindowLongPtr(mainHwnd, GWLP_HINSTANCE), NULL);

  // Create Hints Labels
  std::wstring hintsText = L"Crossword Hints: ";
  std::wstring horizontalWordsText = L"Across:\n";
  std::wstring verticalWordsText = L"Down:\n";

  const auto wordList = gameInstance->getWordList();

  const PuzzleBoard &board = gameInstance->getPuzzleBoard();

  // Populate and Set Labels

  for (int r = 0; r < board.boardSize; r++) {
    for (int c = 0; c < board.boardSize; c++) {
      const Tile &tile = board.board[r][c];
      if (!tile.horizontalWord.empty() && tile.horizontalCharIndex == 0)
        for (int i = 0; i < wordList.size(); i++) {
          if (wordList[i].word == tile.horizontalWord) {
            horizontalWordsText += std::to_wstring(i + 1) + L". ";
            horizontalWordsText += std::wstring(wordList[i].hint.begin(),
                                                wordList[i].hint.end()) +
                                   L"\n";
            break;
          }
        }
      else if (!tile.verticalWord.empty() && tile.verticalCharIndex == 0)
        for (int i = 0; i < wordList.size(); i++) {
          if (wordList[i].word == tile.verticalWord) {
            verticalWordsText += std::to_wstring(i + 1) + L". ";
            verticalWordsText += std::wstring(wordList[i].hint.begin(),
                                              wordList[i].hint.end()) +
                                 L"\n";
            break;
          }
        }
    }
  }

  SetWindowText(horizontalWordsLabel, horizontalWordsText.c_str());
  SetWindowText(verticalWordsLabel, verticalWordsText.c_str());
  SetWindowText(hintsLabel, hintsText.c_str());

  // Render Window On Screen
  ShowWindow(mainHwnd, SW_SHOW);

  return true;
}

//Function to create game board -> Populate activetextfields
bool GameWindow::CreateGameBoard(HWND hwnd) {

    //Clear Active Text Fields

  int id = 100;

  for (auto it = activeTextFields.begin(); it != activeTextFields.end(); ++it) {
      if (it->second) DestroyWindow(it->second);
  }
  activeTextFields.clear();

  //Iterate through puzzleboard, create tiles based off size / word presence -> populate active text fields
  for (int r = 0; r < gameInstance->getPuzzleBoard().boardSize; r++) {
    for (int c = 0; c < gameInstance->getPuzzleBoard().boardSize; c++) {

        Tile* tile = gameInstance->GetTile(Vec2(r, c));
        tile->id = id;

      // ONLY create the textbox if this tile is actually part of ANY word
      if ((!tile->horizontalWord.empty() && tile->horizontalCharIndex >= 0) ||
          (!tile->verticalWord.empty() && tile->verticalCharIndex >= 0)) {

        HWND textBox = CreateWindowEx(
            WS_EX_CLIENTEDGE, L"EDIT", L"",
            WS_CHILD | WS_VISIBLE | WS_BORDER | ES_CENTER, tile->tilePos.x,
            tile->tilePos.y, gameInstance->getPuzzleBoard().boxSize,
            gameInstance->getPuzzleBoard().boxSize, hwnd, (HMENU)(UINT_PTR)id,
            (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), NULL);

        OriginalEditProc = (WNDPROC)SetWindowLongPtr(
            textBox, GWLP_WNDPROC, (LONG_PTR)TileSubclassProc);
        SendMessage(textBox, EM_SETLIMITTEXT, 1, 0);
        AddActiveTextField(id, textBox);
      }

      id += 1; // Always increment the ID so the grid coordinates align
               // correctly if you ever need them
    }
  }

  return true;
}

//Function to update tile color based off user input -> correct words -> tiles turn green
void GameWindow::UpdateGameBoard() {

    //Iterate through wordlist
    for (const auto& word : gameInstance->getWordList()) {

        //Get tile positions by word
        std::vector<Vector2> tilePositions = gameInstance->ReturnPositionsByWord(word.word);

        //If no tiles exists, skip word
        if (tilePositions.empty())
            continue;

        //Init valid check bool
        bool isWordComplete = true;

        //Iterate through tile positions
        for (Vec2& pos : tilePositions) {
            //Create tile reference -> grab textbox control by id
            Tile tile = gameInstance->getPuzzleBoard().board[pos.x][pos.y];
            HWND textBox = GetActiveTextField(tile.id);

            //Error checking
            if (!textBox) { isWordComplete = false; break; }

            //Initialize char for exp character -> Set from Tile 
            char expectedCharacter = '\0';
            if (tile.horizontalWord == word.word && tile.horizontalCharIndex >= 0)
                expectedCharacter = tile.horizontalWord[tile.horizontalCharIndex];
            else if (tile.verticalWord == word.word && tile.verticalCharIndex >= 0)
                expectedCharacter = tile.verticalWord[tile.verticalCharIndex];

            //If there is an expected character -> grab text from text field
            //Check if text matches expected character
            if (expectedCharacter != '\0') {
                std::wstring expectedLetter(1, expectedCharacter);
                int length = GetWindowTextLength(textBox);
                std::wstring currentText(length + 1, L'\0');
                GetWindowText(textBox, &currentText[0], length + 1);
                currentText.resize(length);
                if (currentText != expectedLetter) { isWordComplete = false; break; }
            }
        }

        //Set tile color based of valid check bool -> green if complete
        COLORREF newColor = isWordComplete ? RGB(0, 255, 0) : RGB(255, 255, 255);

        //Iterate through all tiles, set BG color accordingly
        for (Vec2& pos : tilePositions) {
            Tile* tile = gameInstance->GetTile(Vec2(pos.x, pos.y));
            // Only invalidate if the color actually needs to change
            if (tile->bgColor != newColor) {
                tile->bgColor = newColor;
                HWND textBox = GetActiveTextField(tile->id);
                if (textBox) InvalidateRect(textBox, NULL, TRUE);
            }
        }
    }
}

//Function to resize board / font -> based off window width / height
void GameWindow::ResizeBoard(HWND hwnd) {
  RECT rect;
  GetClientRect(hwnd, &rect);

  int clientWidth = rect.right;
  int clientHeight = rect.bottom;

  int boardSize = gameInstance->getPuzzleBoard().boardSize;

  // ---------- Compute board size ----------
  int availableSpace = min(clientWidth, clientHeight) * 0.6;
  int newBoxSize = availableSpace / boardSize * 1.25;

  int gridWidth = newBoxSize * boardSize;
  int gridHeight = gridWidth;

  // ---------- Center board ----------
  int offsetX = (clientWidth - gridWidth) / 2;
  int offsetY = -(clientHeight - gridHeight) / 4 / 4;

  int gridLeft = offsetX;
  int gridRight = offsetX + gridWidth;
  int gridCenter = offsetX + gridWidth / 2;
  int gridBottom = offsetY + gridHeight;

  // ---------- Tile font ----------
  if (hFont)
    DeleteObject(hFont);

  hFont =
      CreateFont((int)(newBoxSize * 0.8), 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
                 DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                 CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Segoe UI");

  // ---------- Resize tiles ----------
  for (int r = 0; r < boardSize; r++) {
    for (int c = 0; c < boardSize; c++) {
      Tile tile = gameInstance->getPuzzleBoard().board[r][c];
      HWND textBox = GetActiveTextField(tile.id);

      MoveWindow(textBox, gridLeft + (c * newBoxSize),
                 offsetY + (r * newBoxSize), newBoxSize, newBoxSize, TRUE);

      SendMessage(textBox, WM_SETFONT, (WPARAM)hFont, TRUE);
    }
  }

  // ---------- Layout constants ----------
  const int padding = 20;

  int hintsHeight = clientHeight / 18;
  int hintsWidth = gridWidth;

  int listTop = gridBottom + hintsHeight + padding;
  int columnWidth = gridWidth / 2;

  // ---------- Hint label (centered under board) ----------
  MoveWindow(hintsLabel, gridLeft, gridBottom, hintsWidth, hintsHeight, TRUE);

  // ---------- Across words ----------
  MoveWindow(horizontalWordsLabel, gridLeft, listTop, columnWidth,
             clientHeight - listTop - padding, TRUE);

  // ---------- Down words ----------
  MoveWindow(verticalWordsLabel, gridCenter, listTop, columnWidth,
             clientHeight - listTop - padding, TRUE);

  // ---------- Font for labels ----------
  HFONT labelFont =
      CreateFont((int)(newBoxSize * 1), 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
                 DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                 CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Segoe UI");

  SendMessage(hintsLabel, WM_SETFONT, (WPARAM)labelFont, TRUE);
  labelFont = CreateFont((int)(newBoxSize * 0.65), 0, 0, 0, FW_BOLD, FALSE,
                         FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
                         CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
                         DEFAULT_PITCH | FF_SWISS, L"Segoe UI");

  SendMessage(horizontalWordsLabel, WM_SETFONT, (WPARAM)labelFont, TRUE);
  SendMessage(verticalWordsLabel, WM_SETFONT, (WPARAM)labelFont, TRUE);
}

//Function to update clues labels
//Iterate through board finding tiles with word presence and beginning index
//Append to wstring and set label text controls
void GameWindow::UpdateClues()
{
    std::wstring horizontalWordsText = L"Across:\n";
    std::wstring verticalWordsText = L"Down:\n";

    const auto& wordList = gameInstance->getWordList();
    const PuzzleBoard& board = gameInstance->getPuzzleBoard();

    for (int r = 0; r < board.boardSize; r++) {
        for (int c = 0; c < board.boardSize; c++) {
            const Tile& tile = board.board[r][c];
            if (!tile.horizontalWord.empty() && tile.horizontalCharIndex == 0)
                for (int i = 0; i < wordList.size(); i++) {
                    if (wordList[i].word == tile.horizontalWord) {
                        horizontalWordsText += std::to_wstring(i + 1) + L". ";
                        horizontalWordsText += std::wstring(wordList[i].hint.begin(), wordList[i].hint.end()) + L"\n";
                        break;
                    }
                }
            else if (!tile.verticalWord.empty() && tile.verticalCharIndex == 0)
                for (int i = 0; i < wordList.size(); i++) {
                    if (wordList[i].word == tile.verticalWord) {
                        verticalWordsText += std::to_wstring(i + 1) + L". ";
                        verticalWordsText += std::wstring(wordList[i].hint.begin(), wordList[i].hint.end()) + L"\n";
                        break;
                    }
                }
        }
    }

    SetWindowText(horizontalWordsLabel, horizontalWordsText.c_str());
    InvalidateRect(horizontalWordsLabel, NULL, TRUE);
    SetWindowText(verticalWordsLabel, verticalWordsText.c_str());
    InvalidateRect(verticalWordsLabel, NULL, TRUE);

}

//Function to serialize game state and save to text file
void GameWindow::SaveGame() {
    OPENFILENAME ofn = {};
    wchar_t filePath[MAX_PATH] = {};

    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = NULL;
    ofn.lpstrFilter = L"Text Files (*.txt)\0*.txt\0All Files\0*.*\0";
    ofn.lpstrFile = filePath;
    ofn.nMaxFile = MAX_PATH;
    ofn.lpstrDefExt = L"txt";
    ofn.Flags = OFN_OVERWRITEPROMPT;

    if (!GetSaveFileName(&ofn))
        return; // user cancelled

    std::wofstream file(filePath);
    if (!file.is_open())
        return;

    // Save word list
    for (const auto& word : gameInstance->getWordList()) {
        file << std::wstring(word.word.begin(), word.word.end())
            << L","
            << std::wstring(word.hint.begin(), word.hint.end())
            << L"\n";
    }

    // Separator between word list and board state
    file << L"---\n";

    // Save current board input state
    for (int r = 0; r < gameInstance->getPuzzleBoard().boardSize; r++) {
        for (int c = 0; c < gameInstance->getPuzzleBoard().boardSize; c++) {
            Tile tile = gameInstance->getPuzzleBoard().board[r][c];
            HWND textBox = GetActiveTextField(tile.id);
            if (textBox) {
                int length = GetWindowTextLength(textBox);
                std::wstring text(length + 1, L'\0');
                GetWindowText(textBox, &text[0], length + 1);
                text.resize(length);
                file << r << L"," << c << L"," << (text.empty() ? L" " : text) << L"\n";
            }
        }
    }

    file.close();
}

//Function to load game from txt file and restore game state
void GameWindow::LoadGame(HWND hwnd) {
    OPENFILENAME ofn = {};
    wchar_t filePath[MAX_PATH] = {};

    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hwnd;
    ofn.lpstrFilter = L"Text Files (*.txt)\0*.txt\0All Files\0*.*\0";
    ofn.lpstrFile = filePath;
    ofn.nMaxFile = MAX_PATH;
    ofn.Flags = OFN_FILEMUSTEXIST;

    if (!GetOpenFileName(&ofn))
        return; // user cancelled

    std::wifstream file(filePath);
    if (!file.is_open())
        return;

    std::vector<WordEntry> wordList;
    std::vector<std::tuple<int, int, wchar_t>> boardState;

    std::wstring line;
    bool readingBoard = false;

    while (std::getline(file, line)) {
        if (line == L"---") { readingBoard = true; continue; }

        if (!readingBoard) {
            // Parse word list (word,clue)
            size_t comma = line.find(L',');
            if (comma != std::wstring::npos) {
                std::wstring first = line.substr(0, comma);
                std::wstring second = line.substr(comma + 1);
                wordList.push_back({
                    std::string(first.begin(), first.end()),
                    std::string(second.begin(), second.end())
                    });
            }
        }
        else {
            // Parse board state (row,col,letter)
            size_t first = line.find(L',');
            size_t second = line.find(L',', first + 1);
            if (first != std::wstring::npos && second != std::wstring::npos) {
                int r = std::stoi(line.substr(0, first));
                int c = std::stoi(line.substr(first + 1, second - first - 1));
                wchar_t ch = line[second + 1];
                if (ch != L' ')
                    boardState.push_back({ r, c, ch });
            }
        }
    }
    file.close();

    // Rebuild the game
    gameInstance->RestartGame(wordList);
    CreateGameBoard(hwnd);
    UpdateClues();

    for (int i = 0; i < boardState.size(); i++) {
        int r = std::get<0>(boardState[i]);
        int c = std::get<1>(boardState[i]);
        wchar_t ch = std::get<2>(boardState[i]);

        Tile tile = gameInstance->getPuzzleBoard().board[r][c];
        HWND textBox = GetActiveTextField(tile.id);
        if (textBox)
            SetWindowText(textBox, std::wstring(1, ch).c_str());
    }

    ResizeBoard(hwnd);
}

//Game loop function -> Standard Win32 Message loop
void GameWindow::GameLoop() {
  // Window running loop
  MSG msg = {};

  while (GetMessage(&msg, NULL, 0, 0)) {
    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }
}


//Setter function for active text field map
void GameWindow::AddActiveTextField(int id, HWND hwnd) {
  activeTextFields[id] = hwnd;
}

//Getter function for active text field map -> by id
HWND &GameWindow::GetActiveTextField(int id) { return activeTextFields[id]; }
