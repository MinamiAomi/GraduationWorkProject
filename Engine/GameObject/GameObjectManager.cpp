#include "GameObjectManager.h"

#include "Graphics/ImGuiManager.h"

void GameObjectManager::Update() {
    // 未初期化のコンポーネントを初期化
    for (auto& gameObject : gameObjects_) {
        if (!gameObject->HasParent()) {
            gameObject->InitializeUninitializedComponents();
        }
    }
    // 更新
    for (auto& gameObject : gameObjects_) {
        if (!gameObject->HasParent()) {
            gameObject->Update();
        }
    }
}

void GameObjectManager::Clear() {
    gameObjects_.clear();
}

