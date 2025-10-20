#include "TestScene.h"

#include "Framework/Engine.h"
#include "Input/Input.h"
#include "Graphics/RenderManager.h"
#include "Framework/AssetManager.h"
#include "Graphics/Core/TextureLoader.h"
#include "LevelLoader.h"

void TestScene::OnInitialize() {

    sunLight_ = std::make_shared<DirectionalLight>();
    sunLight_->direction = -Vector3::unitY;
    RenderManager::GetInstance()->SetSunLight(sunLight_);


    const Vector3 kOffset = { 50.0f, 20.0f, 0.0f };
    const float kWidth = 3.0f;
    const Vector3 kSize = { kColumnCount * kWidth, kRowCount * kWidth, 0.0f };
    
    auto sphereModel = AssetManager::GetInstance()->modelMap.Get("sphere");
    for (uint32_t row = 0; row < kRowCount; ++row) {
        for (uint32_t column = 0; column < kColumnCount; ++column) {
            auto& sphere = spheres_[row][column];
            Vector3 position = Vector3{ column * kWidth, row * kWidth, 0.0f } - (kSize * 0.5f) + kOffset;
            sphere.material = std::make_shared<Material>();
            sphere.material->albedo = { 1.0f, 1.0f, 1.0f };
            sphere.material->metallic = (float)row / (kRowCount - 1);
            sphere.material->roughness = (float)column / (kColumnCount - 1);
            sphere.model.SetModel(sphereModel->Get());
            sphere.model.SetMaterial(sphere.material);
            sphere.model.SetWorldMatrix(Matrix4x4::MakeTranslation(position));
            sphere.model.SetBeReflected(false);
        }
    }

    LevelLoader::Load("Resources/scene.json", *Engine::GetGameObjectManager());

    std::shared_ptr<Texture> texture = Texture::Load("Resources/Sprite-0001.dds");

    sprite_.SetTexture(texture);
    sprite_.SetTexcoordRect({ 0.0f, 0.0f }, { 256.0f, 256.0f });
    sprite_.SetPosition({ 200.0f, 200.0f });
    sprite_.SetScale({ 200.0f, 200.0f });
}

void TestScene::OnUpdate() {

    Engine::GetGameObjectManager()->Update();

}

void TestScene::OnFinalize() {

}
