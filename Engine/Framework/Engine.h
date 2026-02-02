///
/// エンジン
/// 

#pragma once

#include <memory>

#include "Input/Input.h"
#include "Graphics/GameWindow.h"

class Game;
class GameWindow;
class Graphics;
class Input;
class AudioDevice;
class RenderManager;
class SceneManager;
class AssetManager;
class ThreadPool;

#ifdef ENABLE_IMGUI
namespace Editer {
    class EditerManager;
}
#endif // ENABLE_IMGUI

class Engine {
public:
    static const uint32_t kWindowWidth = 1920;
    static const uint32_t kWindowHeight = 1080;

    /// <summary>
    /// ゲームをスタート
    /// </summary>
    /// <param name="game"></param>
    static void Run(Game* game);

    static void SetWindowMode(WindowMode mode);

    // ゲッター

    static Game* GetGame();
    static GameWindow* GetGameWindow();
    static Graphics* GetGraphics();
    static Input* GetInput();
    static AudioDevice* GetAudioDevice();
    static RenderManager* GetRenderManager();
    static SceneManager* GetSceneManager();
    static AssetManager* GetAssetManager();
    static ThreadPool* GetThreadPool();
#ifdef ENABLE_IMGUI
    static Editer::EditerManager* GetEditerManager();
#endif ENABLE_IMGUI
};