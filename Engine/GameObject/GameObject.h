#pragma once
#include "Editer/EditerInterface.h"

#include <typeindex>
#include <typeinfo>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "Component.h"
#include "Math/Transform.h"

class GameObject :
    public std::enable_shared_from_this<GameObject>, public Editer::SelectableInEditer {
    friend class GameObjectManager;
public:
    virtual ~GameObject() {}

    /// <summary>
    /// インスペクタービューに描画
    /// </summary>
    void RenderInInspectorView() override;

    /// <summary>
    /// 未初期化のコンポーネントを初期化
    /// </summary>
    void InitializeUninitializedComponents();
    /// <summary>
    /// 更新
    /// </summary>
    void Update();

    /// <summary>
    /// 親をセット
    /// </summary>
    /// <param name="gameObject"></param>
    void SetParent(const std::shared_ptr<GameObject>& gameObject);
    /// <summary>
    /// 親がいるか
    /// </summary>
    /// <returns></returns>
    bool HasParent() const { return !parent_.expired(); }
    
    /// <summary>
    /// 親子関係を取得
    /// </summary>
    /// <returns></returns>
    const std::weak_ptr<GameObject>& GetParent() const { return parent_; }
    /// <summary>
    /// 親子関係を取得
    /// </summary>
    /// <returns></returns>
    const std::vector<std::weak_ptr<GameObject>>& GetChildren() const { return children_; }

    /// <summary>
    /// コンポーネントを追加
    /// </summary>
    /// <typeparam name="T"></typeparam>
    /// <returns></returns>
    template<class T>
    std::shared_ptr<T> AddComponent() {
        static_assert(std::is_base_of<Component, T>::value, "Componentが継承されていません。");
        auto iter = componentList_.find(typeid(T));
        if (iter != componentList_.end()) {
            return std::static_pointer_cast<T>(iter->second);
        }
        std::shared_ptr<T> component = std::make_shared<T>();
        component->gameObject_ = shared_from_this();
        uninitializedComponents_.emplace_back(component);
        componentList_.emplace(typeid(T), component);
        return component;
    }

    /// <summary>
    /// コンポーネントを取得
    /// </summary>
    /// <typeparam name="T"></typeparam>
    /// <returns></returns>
    template<class T>
    std::shared_ptr<T> GetComponent() const {
        static_assert(std::is_base_of<Component, T>::value, "Componentが継承されていません。");
        auto iter = componentList_.find(typeid(T));
        if (iter != componentList_.end()) {
            return std::static_pointer_cast<T>(iter->second);
        }
        return std::shared_ptr<T>();
    }

    /// <summary>
    /// コンポーネントを削除
    /// </summary>
    /// <typeparam name="T"></typeparam>
    template<class T>
    void RemoveComponent() {
        static_assert(std::is_base_of<Component, T>::value, "Componentが継承されていません。");
        auto iter = componentList_.find(typeid(T));
        if (iter != componentList_.end()) {
            // 初期化されてないコンポーネントも削除
            uninitializedComponents_.erase(
                std::remove_if(uninitializedComponents_.begin(), uninitializedComponents_.end(), [&](const std::shared_ptr<Component>& component) {
                    return component == iter->second;
                    }),
                uninitializedComponents_.end());
            componentList_.erase(iter);
        }

    }

    /// <summary>
    /// 名前をセット
    /// </summary>
    /// <param name="name"></param>
    void SetName(const std::string& name) { name_ = name; }
    /// <summary>
    /// 名前を取得
    /// </summary>
    /// <returns></returns>
    const std::string& GetName() const { return name_; }

    /// <summary>
    /// アクティブフラグをセット
    /// </summary>
    /// <param name="isActive"></param>
    void SetIsActive(bool isActive) { isActive_ = isActive; }
    /// <summary>
    /// アクティブフラグを取得
    /// </summary>
    /// <returns></returns>
    bool IsActive() const { return isActive_; }

    Transform transform;

private:
    void AddChild(const std::shared_ptr<GameObject>& gameObject);
    void RemoveChild(const std::shared_ptr<GameObject>& gameObject);

    // 親
    std::weak_ptr<GameObject> parent_;
    // 子
    std::vector<std::weak_ptr<GameObject>> children_;
    // 未初期化のコンポーネント
    // 最初の更新で初期化される
    std::vector<std::shared_ptr<Component>> uninitializedComponents_;
    // コンポーネントリスト
    std::map<std::type_index, std::shared_ptr<Component>> componentList_;
    // 名前
    std::string name_;
    bool isActive_;
};