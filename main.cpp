#include "UI/GameWindow.h"
#include "Core/Game.h"
#include <string>
#include <windows.h>



//Win32 Main Function
int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance,
                    _In_ PWSTR pCmdLine, _In_ int nCmdShow) {

    //Initialize Game and GameWindow
    //Pass Game and hInstance to GameWindow
    //If Game window successfully creates begin game loop
    Game game({ {"lost","Unable to find one's way"}, {"renegade","Someone who deserts or betrays"}, {"home","A subjective place, a place one feel's safe"} , {"constitution", "A persons physical state"} });
    GameWindow gameWindow(&game, hInstance);
    if (gameWindow.CreateGameWindow(hInstance)) {

        gameWindow.GameLoop();
    }
    


  return 0;
}
