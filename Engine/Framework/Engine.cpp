#include "Engine.h"

#include <memory>

#include "Game.h"
#include "Graphics/GameWindow.h"
#include "Graphics/RenderManager.h"
#include "Input/Input.h"
#include "Audio/AudioDevice.h"
#include "Scene/SceneManager.h"
#include "AssetManager.h"
#include "ThreadPool.h"
#ifdef ENABLE_IMGUI
#include "Editer/EditerManager.h"
#endif // ENABLE_IMGUI

namespace {
    Game* g_game = nullptr;
    std::unique_ptr<ThreadPool> g_threadPool = nullptr;
    GameWindow* g_gameWindow = nullptr;
    Graphics* g_graphics = nullptr;
    Input* g_input = nullptr;
    AudioDevice* g_audioDevice = nullptr;
    RenderManager* g_renderManager = nullptr;
    SceneManager* g_sceneManager = nullptr;
    AssetManager* g_assetManager = nullptr;
#ifdef ENABLE_IMGUI
    std::unique_ptr<Editer::EditerManager> g_editerManager = nullptr;
#endif // ENABLE_IMGUI

}

void Engine::Run(Game* game) {


    g_game = game;

    g_threadPool = std::make_unique<ThreadPool>();
    const wchar_t kWindowTitle[] = L"俺ノ灯走劇";
    g_gameWindow = GameWindow::GetInstance();
    g_gameWindow->Initialize(kWindowTitle, kWindowWidth, kWindowHeight);

    g_graphics = Graphics::GetInstance();
    g_graphics->Initialize();

    g_input = Input::GetInstance();
    g_input->Initialize(g_gameWindow->GetHWND());

    g_audioDevice = AudioDevice::GetInstance();
    g_audioDevice->Initialize();

    g_renderManager = RenderManager::GetInstance();
    g_renderManager->Initialize();

    g_sceneManager = SceneManager::GetInstance();
    g_assetManager = AssetManager::GetInstance();


#ifdef ENABLE_IMGUI
    g_editerManager = std::make_unique<Editer::EditerManager>();
    g_editerManager->Initialize();
#endif // ENABLE_IMGUI

    g_game->OnInitialize();

    while (g_gameWindow->ProcessMessage() && !g_sceneManager->IsTerminateSystem()) {
        g_input->Update();

        if ((g_input->IsKeyPressed(DIK_LALT) && g_input->IsKeyTrigger(DIK_RETURN))) {
            SetWindowMode(g_gameWindow->GetWindowMode() == WindowMode::Window ? WindowMode::Borderless : WindowMode::Window);
        }

#ifdef ENABLE_IMGUI
        g_editerManager->Render();
#endif // ENABLE_IMGUI

        g_sceneManager->Update();


        g_renderManager->Render();
    }

#ifdef ENABLE_IMGUI
    g_editerManager->Finalize();
    g_editerManager.reset();
#endif // ENABLE_IMGUI
    g_sceneManager->Finalize();
    g_game->OnFinalize();

    g_threadPool.reset();
    g_audioDevice->Finalize();
    g_renderManager->Finalize();
    g_graphics->Finalize();
    g_gameWindow->Shutdown();
}

void Engine::SetWindowMode(WindowMode mode) {
    g_gameWindow->SetWindowMode(mode);

    uint32_t newWidth = g_gameWindow->GetClientWidth();
    uint32_t newHeight = g_gameWindow->GetClientHeight();

    g_renderManager->GetSwapChain().SetFullscreen(mode == WindowMode::Fullscreen);
    g_renderManager->OnWindowResize(newWidth, newHeight);
}

Game* Engine::GetGame() {
    return g_game;
}

GameWindow* Engine::GetGameWindow() {
    return g_gameWindow;
}

Graphics* Engine::GetGraphics() {
    return g_graphics;
}

Input* Engine::GetInput() {
    return g_input;
}

AudioDevice* Engine::GetAudioDevice() {
    return g_audioDevice;
}

RenderManager* Engine::GetRenderManager() {
    return g_renderManager;
}

SceneManager* Engine::GetSceneManager() {
    return g_sceneManager;
}

AssetManager* Engine::GetAssetManager() {
    return g_assetManager;;
}

ThreadPool* Engine::GetThreadPool() {
    return g_threadPool.get();
}

#ifdef ENABLE_IMGUI
Editer::EditerManager* Engine::GetEditerManager() {
    return g_editerManager.get();
}
#endif // ENABLE_IMGUI
