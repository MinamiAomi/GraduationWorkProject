#include "SceneManager.h"

//ゲームと干渉しちゃうけどまあええか
#include "../Test/PersistentData.h"
#include "BaseScene.h"
#include "BaseGameSystem.h"

SceneManager* SceneManager::GetInstance() {
    static SceneManager instance;
    return &instance;
}

SceneManager::SceneManager() {
    persistentData_ = std::make_shared<PersistentData>();
}

SceneManager::~SceneManager() {
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

    if (gameSystem_ != nullptr) {
        gameSystem_->OnUpdate();
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

    if (gameSystem_ != nullptr) {
        gameSystem_->OnFinalize();
    }

    persistentData_ = nullptr;
}

bool SceneManager::IsTerminateSystem() const {
    if (gameSystem_ != nullptr) {
        return gameSystem_->IsTerminateSystem();
    }
    return false;
}

std::shared_ptr<PersistentData> SceneManager::GetPersistentData() const {
    return persistentData_;
}
