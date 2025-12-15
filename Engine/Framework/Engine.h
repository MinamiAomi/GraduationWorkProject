///
/// エンジン
/// 

#pragma once

#include <memory>

#include "Input/Input.h"


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
    static const uint32_t kWindowWidth = 1280;
    static const uint32_t kWindowHeight = 720;

    /// <summary>
    /// ゲームをスタート
    /// </summary>
    /// <param name="game"></param>
    static void Run(Game* game);

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