#include "LevelLoader.h"

#include <fstream>

#include "File/JsonConverter.h"
#include "Externals/nlohmann/json.hpp"
#include "MeshComponent.h"
#include "CameraComponent.h"
#include "Collision/Collider.h"
#include "Collision/CollisionManager.h"

namespace LevelLoader {

    void Load(const std::filesystem::path& path, GameObjectManager& gameObjectManager) {
        std::ifstream file(path);
        assert(file.is_open());

        nlohmann::json json;
        file >> json;

        file.close();

        assert(json.is_object());
        assert(json.contains("name"));
        assert(json.at("name").is_string() && json.at("name").get<std::string>() == "scene");

        for (auto& object : json.at("objects")) {
            std::shared_ptr<GameObject> gameObject = std::make_shared<GameObject>();

            if (object.contains("name")) {
                gameObject->SetName(object.at("name"));
            }
            if (object.contains("transform")) {
                object.at("transform").get_to(gameObject->transform);
            }
            if (object.contains("camera")) {
                auto component = gameObject->AddComponent<CameraComponent>();
                component;
            }
            if (object.contains("model_name")) {
                auto component = gameObject->AddComponent<MeshComponent>();
                component->SetModelName(object.at("model_name"));
            }
            if(object.contains("collider")) {
                auto collider = object.at("collider");
                if (collider.at("type") == "BOX") {
                    auto component = gameObject->AddComponent<BoxCollider>();
                    Vector3 center, size;
                    collider.at("center").get_to(center);
                    collider.at("size").get_to(size);
                    component->SetCenter(center);
                    component->SetSize(size);
                }
            }
            gameObjectManager.AddGameObject(gameObject);
        }
    }
}