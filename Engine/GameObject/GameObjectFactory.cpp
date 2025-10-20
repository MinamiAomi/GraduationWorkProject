#include "GameObjectFactory.h"

std::shared_ptr<GameObject> DefaultGameObjectFactory::CreateGameObject(const std::string& id) const {
    id;
    return std::make_shared<GameObject>();
}

void DefaultGameObjectFactory::CreateGameObjectFromEditer() {}
