#include "DemoGameObjectFactory.h"

#include "Framework/Engine.h"
#include "GameObject/GameObjectManager.h"
#include "MeshComponent.h"

std::shared_ptr<GameObject> DemoGameObjectFactory::CreateGameObject(const std::string& id) const {
    auto gameObjectManager = Engine::GetGameObjectManager();
    if (id == "Default") {
        auto gameObject = std::make_shared<GameObject>();
        gameObject->SetName("Empty");
        gameObject->SetIsActive(true);
        gameObject->AddComponent<MeshComponent>();
        gameObjectManager->AddGameObject(gameObject);
        return gameObject;
    }

    return std::shared_ptr<GameObject>();
}

void DemoGameObjectFactory::CreateGameObjectFromEditer() {
}
