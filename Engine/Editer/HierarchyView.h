///
/// ヒエラルキービュー
/// 

#pragma once
#include "EditerView.h"

#include <memory>
#include <string>

class GameObject;
class GameObjectManager;

namespace Editer {

    class HierarchyView :
        public View {
    public:
        using View::View;

        void Render(GameObjectManager& gameObjectManager);

    private:
        void RenderGameObject(const std::shared_ptr<GameObject>& gameObject);
        void RenderSearchGameObject(const std::shared_ptr<GameObject>& gameObject);

        std::string searchString_;
    };

}