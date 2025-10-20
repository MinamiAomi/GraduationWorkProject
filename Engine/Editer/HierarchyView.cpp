#include "HierarchyView.h"

#ifdef ENABLE_IMGUI
#include "Externals/ImGui/imgui.h"
#include "Externals/ImGui/imgui_stdlib.h"
#include "Externals/ImGui/imgui_internal.h"
#endif // ENABLE_IMGUI

#include "GameObject/GameObjectManager.h"
#include "GameObject/GameObject.h"
#include "EditerManager.h"

namespace Editer {

    void HierarchyView::Render(GameObjectManager& gameObjectManager) {
        gameObjectManager;
#ifdef ENABLE_IMGUI
        if (!isDisplayed) { return; }
        ImGui::Begin("Hierarchy", &isDisplayed, ImGuiWindowFlags_MenuBar);

        if (ImGui::BeginMenuBar()) {
           //if (ImGui::Button("Create")) {
           //    ImGui::OpenPopup("create_game_object_popup");
           //}
            ImGui::PushItemWidth(100);
            ImGui::InputText("##Search", &searchString_);
            ImGui::PopItemWidth();
            ImGui::EndMenuBar();
        }

        // 背景のドラッグドロップターゲット
        if (ImGui::BeginDragDropTargetCustom(ImGui::GetCurrentWindow()->Rect(), ImGui::GetID("background"))) {
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("HierarchyView GameObject")) {
                IM_ASSERT(payload->DataSize == sizeof(std::shared_ptr<GameObject>));
                std::shared_ptr<GameObject>* dropObject = (std::shared_ptr<GameObject>*)payload->Data;
                (*dropObject)->SetParent(nullptr);
            }
            ImGui::EndDragDropTarget();
        }
        // ウィンドウ内でクリックされたらポップアップメニューを開く
        if (ImGui::IsWindowHovered(ImGuiHoveredFlags_AllowWhenBlockedByPopup) && 
            ImGui::IsMouseClicked(ImGuiMouseButton_Right)) {
            ImGui::OpenPopup("create_game_object_popup");
        }

        // ポップアップメニュー
        if (ImGui::BeginPopup("create_game_object_popup")) {
            if (ImGui::MenuItem("Create Empty")) {
                auto newGameObject = std::make_shared<GameObject>();
                newGameObject->SetName("NewObject");
                gameObjectManager.AddGameObject(newGameObject);
            }
            gameObjectManager.GetFactory().CreateGameObjectFromEditer();
            ImGui::EndPopup();
        }

        const auto& gameObjects = gameObjectManager.GetGameObjects();

        if (searchString_.empty()) {
            for (auto& gameObject : gameObjects) {
                // 親がいないオブジェクトから再帰的に
                if (!gameObject->HasParent()) {
                    RenderGameObject(gameObject);
                }
            }
        }
        else {
            for (auto& gameObject : gameObjects) {
                if (gameObject->GetName().find(searchString_) != std::string::npos) {
                    RenderSearchGameObject(gameObject);
                }
            }
        }

        ImGui::End();
#endif // ENABLE_IMGUI
    }

    void HierarchyView::RenderGameObject(const std::shared_ptr<GameObject>& gameObject) {
        ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick;
        if (gameObject == owner.GetSelectedObject()) { flags |= ImGuiTreeNodeFlags_Selected; }
        // 子なしは葉ノード
        if (gameObject->GetChildren().empty()) { flags |= ImGuiTreeNodeFlags_Leaf; }

        ImGui::PushID(gameObject.get());
        if (ImGui::TreeNodeEx(gameObject->GetName().c_str(), flags)) {
            // ドラッグドロップソース
            if (ImGui::BeginDragDropSource()) {
                ImGui::SetDragDropPayload("HierarchyView GameObject", &gameObject, sizeof(std::shared_ptr<GameObject>));
                ImGui::Text("Dragging %s", gameObject->GetName().c_str());
                ImGui::EndDragDropSource();
            }
            // ドラッグドロップターゲット
            if (ImGui::BeginDragDropTarget()) {
                if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("HierarchyView GameObject")) {
                    IM_ASSERT(payload->DataSize == sizeof(std::shared_ptr<GameObject>));
                    std::shared_ptr<GameObject>* dropObject = (std::shared_ptr<GameObject>*)payload->Data;

                    if (*dropObject != gameObject) {
                        (*dropObject)->SetParent(gameObject);
                    }
                }
                ImGui::EndDragDropTarget();
            }

            if (ImGui::IsItemClicked(ImGuiMouseButton_Left)) {
                owner.SetSelectedObject(gameObject);
            }
            for (auto& child : gameObject->GetChildren()) {
                if (auto sp = child.lock()) {
                    RenderGameObject(sp);
                }
            }
            ImGui::TreePop();
        }
        ImGui::PopID();
    }

    void HierarchyView::RenderSearchGameObject(const std::shared_ptr<GameObject>& gameObject) {
        ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_Leaf;
        if (gameObject == owner.GetSelectedObject()) { flags |= ImGuiTreeNodeFlags_Selected; }

        ImGui::PushID(gameObject.get());
        if (ImGui::TreeNodeEx(gameObject->GetName().c_str(), flags)) {
            // ドラッグドロップソース
            if (ImGui::BeginDragDropSource()) {
                ImGui::SetDragDropPayload("HierarchyView GameObject", &gameObject, sizeof(std::shared_ptr<GameObject>));
                ImGui::Text("Dragging %s", gameObject->GetName().c_str());
                ImGui::EndDragDropSource();
            }

            if (ImGui::IsItemClicked(ImGuiMouseButton_Left)) {
                owner.SetSelectedObject(gameObject);
            }
            ImGui::TreePop();
        }
        ImGui::PopID();
    }

}
