///
/// エディターマネージャー
/// 

#pragma once

#include <memory>
#include <vector>

#include "Graphics/Core/DescriptorHandle.h"
#include "Graphics/Core/CommandContext.h"

#include "EditerInterface.h"
#include "EditerView.h"
#include "HierarchyView.h"
#include "InspectorView.h"
#include "SceneView.h"
#include "ProjectView.h"
#include "ConsoleView.h"

namespace Editer {

    class EditerManager {
    public:
        EditerManager();

        void Initialize();
        void Render();
        void RenderToColorBuffer(CommandContext& commandContext);
        void Finalize();

        ConsoleView& GetConsoleView() { return *consoleView_; }

        void SetSelectedObject(const std::shared_ptr<SelectableInEditer>& selectedObject) { selectedObject_ = selectedObject; }
        const std::shared_ptr<SelectableInEditer>& GetSelectedObject() const { return selectedObject_; }

    private:
        void RenderDockingSpace();
        void FileMenu();
        void SettingMenu();
        void WindowMenu();
        void HelpMenu();
        void LoadStyle();
        void SaveStyle();
        
        std::unique_ptr<HierarchyView> hierarchyView_;
        std::unique_ptr<InspectorView> inspectorView_;
        std::unique_ptr<SceneView> sceneView_;
        std::unique_ptr<ProjectView> projectView_;
        std::unique_ptr<ConsoleView> consoleView_;
        // 選択されたオブジェクト
        std::shared_ptr<SelectableInEditer> selectedObject_ = nullptr;
        DescriptorHandle descriptor_;
    };

}