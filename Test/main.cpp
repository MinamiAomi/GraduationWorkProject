#include <Windows.h>

#include "Framework/Engine.h"
#include "Test.h"

int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int) {

    Game* game = new Test;
    Engine::Run(game);
    delete game;

    return 0;
}

// 