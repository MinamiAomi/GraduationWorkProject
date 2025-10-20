#include "SceneIO.h"

#include <cassert>
#include <fstream>
#include <queue>

#include "Framework/Engine.h"
#include "GameObject/GameObjectManager.h"
#include "Framework/AssetManager.h"
#include "Externals/nlohmann/json.hpp"
#include "File/JsonConverter.h"

namespace {

    // シーンを構築する
    void BuildScene(const nlohmann::json& objects) {
        assert(objects.is_array());
        auto gameObjectManager = Engine::GetGameObjectManager();
        // 親から順に読み込んでいく
        std::queue<std::pair<const nlohmann::json*, std::shared_ptr<GameObject>>> que;
        // 親をキューに追加
        for (auto& object : objects) {
            que.push(std::make_pair(&object, std::shared_ptr<GameObject>()));
        }

        while (!que.empty()) {
            auto& object = *que.front().first;
            auto parent = que.front().second;
            que.pop();
            auto gameObject = std::make_shared<GameObject>();
            if (parent) {
                gameObject->SetParent(parent);
            }
            if (object.contains("name")) {
                gameObject->SetName(object.at("name").get<std::string>());
            }
            if (object.contains("isActive")) {
                gameObject->SetIsActive(object.at("isActive").get<bool>());
            }
            if (object.contains("transform")) {
                object.at("transform").get_to(gameObject->transform);
            }
            if (object.contains("components")) {
                for (auto& componentName : object.at("components")) {
                    auto component = gameObjectManager->GetComponentRegisterer().Register(*gameObject, componentName);
                    if (object.contains(componentName)) {
                        component->Import(object.at(componentName));
                    }
                }
            }
            if (object.contains("children")) {
                for (auto& child : object.at("children")) {
                    que.push(std::make_pair(&child, gameObject));
                }
            }
            gameObjectManager->AddGameObject(gameObject);
        }
    }

    // アセットを読み込む
    void LoadAssets(const nlohmann::json& assets) {
        assert(assets.is_array());
        auto assetManager = Engine::GetAssetManager();

        for (auto& asset : assets) {
            if (asset.contains("type")) {
                assert(asset.contains("path") && asset.contains("name"));
                Asset::Type type = static_cast<Asset::Type>(asset.at("type"));
                switch (type)
                {
                case Asset::Type::Texture: {
                    auto texture = std::make_shared<TextureAsset>();
                    texture->Load(asset.at("path"), asset.at("name"));
                    assetManager->textureMap.Add(texture);
                    break;
                }
                case Asset::Type::Model: {
                    auto model = std::make_shared<ModelAsset>();
                    model->Load(asset.at("path"), asset.at("name"));
                    assetManager->modelMap.Add(model);
                    break;
                }
                case Asset::Type::Material: {
                    auto material = std::make_shared<MaterialAsset>();
                    material->Load(asset.at("path"), asset.at("name"));
                    assetManager->materialMap.Add(material);
                    break;
                }
                case Asset::Type::Animation: {
                    auto animation = std::make_shared<AnimationAsset>();
                    animation->Load(asset.at("path"), asset.at("name"));
                    assetManager->animationMap.Add(animation);
                    break;
                }
                case Asset::Type::Sound: {
                    auto sound = std::make_shared<SoundAsset>();
                    sound->Load(asset.at("path"), asset.at("name"));
                    assetManager->soundMap.Add(sound);
                    break;
                }
                default:
                    // ここには来ないよ
                    assert(true);
                    break;
                }
            }
        }
    }
}

namespace SceneIO {

    void Load(const std::filesystem::path& path) {

        // 既存シーンをクリア
        Engine::GetGameObjectManager()->Clear();
        Engine::GetAssetManager()->Clear();

        // ファイルを読み込む
        std::ifstream file(path);
        assert(file.is_open());

        nlohmann::json json;
        file >> json;

        file.close();

        assert(json.is_object());
        assert(json.contains("name"));
        assert(json.at("name").is_string());

        auto sceneName = json.at("name").get<std::string>();

        if (json.contains("objects")) {
            BuildScene(json.at("objects"));
        }

        if (json.contains("assets")) {
            LoadAssets(json.at("assets"));
        }
    }

    void Save(const std::filesystem::path& path) {
        path;
    }

}