#pragma once

#include <string>
#include <memory>
#include <list>

#include "GameObject.h"
#include "GameObjectFactory.h"
#include "ComponentRegisterer.h"

class GameObjectManager {
public:
    /// <summary>
    /// 更新
    /// </summary>
    void Update();
    /// <summary>
    /// クリア
    /// </summary>
    void Clear();

    /// <summary>
    /// ゲームオブジェクトを追加
    /// </summary>
    /// <param name="gameObject"></param>
    void AddGameObject(const std::shared_ptr<GameObject>& gameObject) { gameObjects_.emplace_back(gameObject); }
    /// <summary>
    /// ゲームオブジェクトを取得
    /// </summary>
    /// <returns></returns>
    const std::list<std::shared_ptr<GameObject>>& GetGameObjects() const { return gameObjects_; }

    /// <summary>
    /// オブジェクトファクトリーをセット
    /// </summary>
    /// <typeparam name="T"></typeparam>
    template<class T>
    void SetFactory() {
        static_assert(std::is_base_of<GameObjectFactory, T>::value, "継承されていません。");
        factory_ = std::make_unique<T>(*this);
    }

    /// <summary>
    /// コンポーネントレジスタをセット
    /// </summary>
    /// <typeparam name="T"></typeparam>
    template<class T>
    void SetComponentRegisterer() {
        static_assert(std::is_base_of<ComponentRegisterer, T>::value, "継承されていません。");
        componentRegisterer_ = std::make_unique<T>();
    }

    // ゲッター

    const GameObjectFactory& GetFactory() const { return *factory_; }
    GameObjectFactory& GetFactory() { return *factory_; }

    const ComponentRegisterer& GetComponentRegisterer() const { return *componentRegisterer_; }
    ComponentRegisterer& GetComponentRegisterer() { return *componentRegisterer_; }

private:
    std::list<std::shared_ptr<GameObject>> gameObjects_;
    std::unique_ptr<GameObjectFactory> factory_;
    std::unique_ptr<ComponentRegisterer> componentRegisterer_;
};