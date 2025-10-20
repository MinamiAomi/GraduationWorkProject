///
/// アセットマネージャー
/// 

#pragma once

#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <functional>

#include "Asset.h"
#include "TextureAsset.h"
#include "ModelAsset.h"
#include "MaterialAsset.h"
#include "AnimationAsset.h"
#include "SoundAsset.h"

class Model;
class TextureResource;
class Sound;
class Animation;

template<class T>
class AssetMap {
    static_assert(std::is_base_of<Asset, T>::value, "継承されていません。");
public:
    /// <summary>
    /// 追加
    /// </summary>
    /// <param name="asset"></param>
    void Add(const std::shared_ptr<T>& asset) {
        std::lock_guard<std::mutex> lock(mutex_);
        list_.emplace_back(asset);
    }

    /// <summary>
    /// 取得
    /// </summary>
    /// <param name="name"></param>
    /// <returns></returns>
    std::shared_ptr<T> Get(const std::string& name) const {
        std::shared_ptr<T> ptr;
        auto iter = std::find_if(list_.begin(), list_.end(),
            [name](auto asset) {return name == asset->GetName(); });
        if (iter != list_.end()) {
            ptr = *iter;
        }
        return ptr;
    }

    /// <summary>
    /// マップに対して関数を適用
    /// </summary>
    /// <param name="func"></param>
    void ForEach(std::function<void(const std::shared_ptr<T>&)> func) {
        std::lock_guard<std::mutex> lock(mutex_);
        for (auto& iter : list_) {
            func(iter);
        }
    }

    /// <summary>
    /// 削除
    /// </summary>
    /// <param name="asset"></param>
    void Remove(const std::shared_ptr<T>& asset) {
        std::lock_guard<std::mutex> lock(mutex_);
        list_.remove(asset);
    }

    /// <summary>
    /// クリア
    /// </summary>
    void Clear() {
        std::lock_guard<std::mutex> lock(mutex_);
        list_.clear();
    }

private:
    std::mutex mutex_;
    std::list<std::shared_ptr<T>> list_;

};

class AssetManager {
public:
    /// <summary>
    /// シングルトンインスタンスを取得
    /// </summary>
    /// <returns></returns>
    static AssetManager* GetInstance();

    /// <summary>
    /// クリア
    /// </summary>
    void Clear();

    AssetMap<TextureAsset> textureMap;
    AssetMap<ModelAsset> modelMap;
    AssetMap<MaterialAsset> materialMap;
    AssetMap<AnimationAsset> animationMap;
    AssetMap<SoundAsset> soundMap;

private:
    AssetManager() = default;
    ~AssetManager() = default;
    AssetManager(const AssetManager&) = delete;
    AssetManager& operator=(const AssetManager&) = delete;

};