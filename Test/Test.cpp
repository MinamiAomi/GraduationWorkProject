#include "Test.h"

#include <memory>
#include <fstream>

#include "Externals/nlohmann/json.hpp"
#include "Framework/Engine.h"
#include "Framework/ThreadPool.h"
#include "Scene/SceneManager.h"
#include "Framework/AssetManager.h"
#include "Graphics/Model.h"
#include "Audio/Sound.h"
#include "Graphics/Animation.h"
#include "Graphics/Sprite.h"
#include "Graphics/Core/TextureLoader.h"
#include "Debug/Debug.h"

#include "TitleScene.h"
#include "GameScene.h"
#include "DemoGameObjectFactory.h"
#include "DemoComponentRegisterer.h"

namespace {
    const char kResourceAssociationFile[] = "Resources/Association.json";
}

void Test::OnInitialize() {
    auto gameObjectManager = Engine::GetGameObjectManager();
    gameObjectManager->SetFactory<DemoGameObjectFactory>();
    gameObjectManager->SetComponentRegisterer<DemoComponentRegisterer>();

    SceneManager* sceneManager = SceneManager::GetInstance();
    //シーン設定
    sceneManager->ChangeScene<TitleScene>(false);

    LoadResource();
}

void Test::OnFinalize() {
}

void Test::LoadResource() {
    std::ifstream file;
    file.open(kResourceAssociationFile);
    assert(file.is_open());

    nlohmann::json json;
    file >> json;
    file.close();


    AssetManager* assetManager = AssetManager::GetInstance();
    for (auto& texture : json["Texture"].items()) {
#ifdef _DEBUG
        auto duration = Debug::ElapsedTime([&]() {
#endif
            auto asset = std::make_shared<TextureAsset>();
            asset->Load("Resources/" + texture.value().get<std::string>());
            asset->SetName(texture.key());
            assetManager->textureMap.Add(asset);
#ifdef _DEBUG
            });
        std::stringstream str;
        str << "LoadTexture : " << texture.key() << " - " << duration << "\n";
        OutputDebugStringA(str.str().c_str());
#endif
    }
    for (auto& model : json["Model"].items()) {
#ifdef _DEBUG
        auto duration = Debug::ElapsedTime([&]() {
#endif
            auto asset = std::make_shared<ModelAsset>();
            asset->Load("Resources/" + model.value().get<std::string>());
            asset->SetName(model.key());
            assetManager->modelMap.Add(asset);
#ifdef _DEBUG
            });
        std::stringstream str;
        str << "LoadModel : " << model.key() << " - " << duration << "\n";
        OutputDebugStringA(str.str().c_str());
#endif
    }


    for (auto& sound : json["Sound"].items()) {
#ifdef _DEBUG
        auto duration = Debug::ElapsedTime([&]() {
#endif

            auto asset = std::make_shared<SoundAsset>();
            asset->Load("Resources/" + sound.value().get<std::string>());
            asset->SetName(sound.key());
            assetManager->soundMap.Add(asset);
#ifdef _DEBUG
            });
        std::stringstream str;
        str << "LoadSound : " << sound.key() << " - " << duration << "\n";
        OutputDebugStringA(str.str().c_str());
#endif
    }
    for (auto& animation : json["Animation"].items()) {
#ifdef _DEBUG
        auto duration = Debug::ElapsedTime([&]() {
#endif
            auto asset = std::make_shared<AnimationAsset>();
            asset->Load("Resources/" + animation.value().get<std::string>());
            asset->SetName(animation.key());
            assetManager->animationMap.Add(asset);
#ifdef _DEBUG
            });
        std::stringstream str;
        str << "LoadAnimation : " << animation.key() << " - " << duration << "\n";
        OutputDebugStringA(str.str().c_str());
#endif
    }

    Engine::GetThreadPool()->WaitForAll();
}
