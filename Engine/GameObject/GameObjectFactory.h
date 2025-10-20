#pragma once
#include <string>

#include "GameObject.h"

class GameObjectManager;

class GameObjectFactory {
public:
    GameObjectFactory(GameObjectManager& owner) : owner(owner) {}
    virtual ~GameObjectFactory() {}
    virtual std::shared_ptr<GameObject> CreateGameObject(const std::string& id) const = 0;
    virtual void CreateGameObjectFromEditer() {}
    
    GameObjectManager& owner;
};

class DefaultGameObjectFactory :
    public GameObjectFactory {
public:
    using GameObjectFactory::GameObjectFactory;

    std::shared_ptr<GameObject> CreateGameObject(const std::string& id) const override;
    void CreateGameObjectFromEditer() override;
};
