#include "SceneManager.h"

#include "BaseScene.h"

SceneManager* SceneManager::GetInstance() {
    static SceneManager instance;
    return &instance;
}

void SceneManager::Update() {
    // フェードイン中
    if (sceneTransition_.IsPlaying()) {
        sceneTransition_.Update();
    }
    // フェードイン終了、次のシーンに移行しフェードアウト始動
    if (nextScene_ && !sceneTransition_.IsPlaying()) {
        if (sceneTransition_.GetMode() == SceneTransition::Mode::Wait) {
            sceneTransition_.Start(SceneTransition::Mode::Out);
        }
        if (currentScene_) { currentScene_->OnFinalize(); }
        currentScene_ = std::move(nextScene_);
        nextScene_ = nullptr;
        currentScene_->OnInitialize();
    }

    if (currentScene_) {
        currentScene_->OnUpdate();
    }
}

void SceneManager::Finalize() {
    if (currentScene_) { 
        currentScene_->OnFinalize();
        currentScene_ = nullptr;
    }
    if (nextScene_) { 
        nextScene_->OnFinalize(); 
        nextScene_ = nullptr;
    }
}

SceneManager::SceneManager() {
}

SceneManager::~SceneManager() {
}

