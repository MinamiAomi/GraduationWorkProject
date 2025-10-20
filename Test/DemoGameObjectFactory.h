#pragma once
#include "GameObject/GameObjectFactory.h"

class DemoGameObjectFactory :
    public GameObjectFactory {
public:
    using GameObjectFactory::GameObjectFactory;

    /// <summary>
    /// オブジェクトを生成
    /// </summary>
    /// <param name="id">どのオブジェクトを作るかのID</param>
    /// <returns></returns>
    std::shared_ptr<GameObject> CreateGameObject(const std::string& id) const override;
    /// <summary>
    /// エディター
    /// </summary>
    void CreateGameObjectFromEditer() override;

};

