#include "EditerManager.h"

#include <Windows.h>
#include <d3d12.h>
#include <fstream>
#include <filesystem>

#ifdef ENABLE_IMGUI
#include "Externals/ImGui/imgui.h"
#include "Externals/ImGui/imgui_stdlib.h"
#include "Externals/ImGui/imgui_impl_dx12.h"
#include "Externals/ImGui/imgui_impl_win32.h"
#endif // ENABLE_IMGUI
#include "Externals/nlohmann/json.hpp"

#include "Framework/Engine.h"
#include "Graphics/Core/Graphics.h"
#include "Graphics/GameWindow.h"
#include "Graphics/RenderManager.h"
#include "Scene/SceneIO.h"

namespace Editer {

    static const char kStyleFile[] = "imgui_style.json";

    nlohmann::json SerializeStyle(const ImGuiStyle& style) {
        nlohmann::json j;
        j["Alpha"] = style.Alpha;
        j["WindowPadding"] = { style.WindowPadding.x, style.WindowPadding.y };
        j["WindowRounding"] = style.WindowRounding;
        j["WindowBorderSize"] = style.WindowBorderSize;
        j["WindowMinSize"] = { style.WindowMinSize.x, style.WindowMinSize.y };
        j["WindowTitleAlign"] = { style.WindowTitleAlign.x, style.WindowTitleAlign.y };
        j["ChildRounding"] = style.ChildRounding;
        j["ChildBorderSize"] = style.ChildBorderSize;
        j["PopupRounding"] = style.PopupRounding;
        j["PopupBorderSize"] = style.PopupBorderSize;
        j["FramePadding"] = { style.FramePadding.x, style.FramePadding.y };
        j["FrameRounding"] = style.FrameRounding;
        j["FrameBorderSize"] = style.FrameBorderSize;
        j["ItemSpacing"] = { style.ItemSpacing.x, style.ItemSpacing.y };
        j["ItemInnerSpacing"] = { style.ItemInnerSpacing.x, style.ItemInnerSpacing.y };
        j["IndentSpacing"] = style.IndentSpacing;
        j["ScrollbarSize"] = style.ScrollbarSize;
        j["ScrollbarRounding"] = style.ScrollbarRounding;
        j["GrabMinSize"] = style.GrabMinSize;
        j["GrabRounding"] = style.GrabRounding;
        j["TabRounding"] = style.TabRounding;
        j["ButtonTextAlign"] = { style.ButtonTextAlign.x, style.ButtonTextAlign.y };
        j["SelectableTextAlign"] = { style.SelectableTextAlign.x, style.SelectableTextAlign.y };
        for (int i = 0; i < ImGuiCol_COUNT; ++i) {
            j["Colors"][i] = { style.Colors[i].x, style.Colors[i].y, style.Colors[i].z, style.Colors[i].w };
        }
        return j;
    }

    void DeserializeStyle(const nlohmann::json& j, ImGuiStyle& style) {
        style.Alpha = j["Alpha"];
        style.WindowPadding = ImVec2(j["WindowPadding"][0], j["WindowPadding"][1]);
        style.WindowRounding = j["WindowRounding"];
        style.WindowBorderSize = j["WindowBorderSize"];
        style.WindowMinSize = ImVec2(j["WindowMinSize"][0], j["WindowMinSize"][1]);
        style.WindowTitleAlign = ImVec2(j["WindowTitleAlign"][0], j["WindowTitleAlign"][1]);
        style.ChildRounding = j["ChildRounding"];
        style.ChildBorderSize = j["ChildBorderSize"];
        style.PopupRounding = j["PopupRounding"];
        style.PopupBorderSize = j["PopupBorderSize"];
        style.FramePadding = ImVec2(j["FramePadding"][0], j["FramePadding"][1]);
        style.FrameRounding = j["FrameRounding"];
        style.FrameBorderSize = j["FrameBorderSize"];
        style.ItemSpacing = ImVec2(j["ItemSpacing"][0], j["ItemSpacing"][1]);
        style.ItemInnerSpacing = ImVec2(j["ItemInnerSpacing"][0], j["ItemInnerSpacing"][1]);
        style.IndentSpacing = j["IndentSpacing"];
        style.ScrollbarSize = j["ScrollbarSize"];
        style.ScrollbarRounding = j["ScrollbarRounding"];
        style.GrabMinSize = j["GrabMinSize"];
        style.GrabRounding = j["GrabRounding"];
        style.TabRounding = j["TabRounding"];
        style.ButtonTextAlign = ImVec2(j["ButtonTextAlign"][0], j["ButtonTextAlign"][1]);
        style.SelectableTextAlign = ImVec2(j["SelectableTextAlign"][0], j["SelectableTextAlign"][1]);
        for (int i = 0; i < ImGuiCol_COUNT; ++i) {
            style.Colors[i] = ImVec4(j["Colors"][i][0], j["Colors"][i][1], j["Colors"][i][2], j["Colors"][i][3]);
        }
    }

    std::filesystem::path OpenFileDialog(const std::wstring& title) {
        OPENFILENAME ofn{};
        wchar_t szFile[256]{};

        ZeroMemory(&ofn, sizeof(ofn));
        ofn.lStructSize = sizeof(ofn);
        ofn.hwndOwner = Engine::GetGameWindow()->GetHWND();
        ofn.lpstrFile = szFile;
        ofn.lpstrFile[0] = L'\0';
        ofn.nMaxFile = sizeof(szFile);
        ofn.lpstrFilter = L"Json\0*.JSON";
        ofn.nFilterIndex = 1;
        ofn.lpstrFileTitle = NULL;
        ofn.nMaxFileTitle = 0;
        ofn.lpstrInitialDir = NULL;
        ofn.lpstrTitle = title.c_str();
        ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

        if (GetOpenFileName(&ofn) == TRUE) {
            std::filesystem::path path = ofn.lpstrFile;
            return path;
        }
        return std::filesystem::path();
    }

    std::filesystem::path SaveFileDialog(const std::wstring& title) {
        OPENFILENAME ofn{};
        wchar_t szFile[256]{};

        ZeroMemory(&ofn, sizeof(ofn));
        ofn.lStructSize = sizeof(ofn);
        ofn.hwndOwner = Engine::GetGameWindow()->GetHWND();
        ofn.lpstrFile = szFile;
        ofn.lpstrFile[0] = L'\0';
        ofn.nMaxFile = sizeof(szFile);
        ofn.lpstrFilter = L"Json\0*.JSON";
        ofn.nFilterIndex = 1;
        ofn.lpstrFileTitle = NULL;
        ofn.nMaxFileTitle = 0;
        ofn.lpstrInitialDir = NULL;
        ofn.lpstrTitle = title.c_str();
        ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

        if (GetSaveFileName(&ofn) == TRUE) {
            std::filesystem::path path = ofn.lpstrFile;
            // 絶対パスから相対パスに変換
            if (path.is_absolute()) {
                path = std::filesystem::relative(path, std::filesystem::current_path());
            }
            return path;
        }
        return std::filesystem::path();
    }

    EditerManager::EditerManager() :
        hierarchyView_(std::make_unique<HierarchyView>(*this)),
        inspectorView_(std::make_unique<InspectorView>(*this)),
        sceneView_(std::make_unique<SceneView>(*this)),
        projectView_(std::make_unique<ProjectView>(*this)),
        consoleView_(std::make_unique<ConsoleView>(*this)) {
    }

    void EditerManager::Initialize() {
        auto graphics = Engine::GetGraphics();
        auto window = Engine::GetGameWindow();
        auto renderManager = Engine::GetRenderManager();

        descriptor_ = graphics->AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        auto& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
        //ImGui::StyleColorsDark();
        //ImGui::StyleColorsClassic();
        // スタイルを読む
        LoadStyle();

        ImGui_ImplWin32_Init(window->GetHWND());
        ImGui_ImplDX12_Init(
            graphics->GetDevice(),
            SwapChain::kNumBuffers,
            renderManager->GetSwapChain().GetColorBuffer(0).GetRTVFormat(ColorBuffer::RTV::SRGB),
            graphics->GetDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV),
            descriptor_,
            descriptor_);
    }

    void EditerManager::Render() {
        ImGui_ImplWin32_NewFrame();
        ImGui_ImplDX12_NewFrame();
        ImGui::NewFrame();

        //RenderDockingSpace();

        //auto gameObjectManager = Engine::GetGameObjectManager();
        //hierarchyView_->Render(*gameObjectManager);
        //inspectorView_->Render(selectedObject_);
        //consoleView_->Render();
    }

    void EditerManager::RenderToColorBuffer(CommandContext& commandContext) {
        //projectView_->Render();
        //sceneView_->Render(commandContext);
        ImGui::Render();
        ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), commandContext);
    }

    void EditerManager::Finalize() {
        ImGui_ImplDX12_Shutdown();
        ImGui_ImplWin32_Shutdown();
        ImGui::DestroyContext();
    }

    void EditerManager::RenderDockingSpace() {
        static bool opt_fullscreen = true;
        static bool opt_padding = false;
        static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

        // ウィンドウフラグの設定
        ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
        if (opt_fullscreen) {
            ImGuiViewport* viewport = ImGui::GetMainViewport();
            ImGui::SetNextWindowPos(viewport->Pos);
            ImGui::SetNextWindowSize(viewport->Size);
            ImGui::SetNextWindowViewport(viewport->ID);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
            window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
            window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
        }

        if (!opt_padding)
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

        // メインドッキングスペースの開始
        ImGui::Begin("DockSpace", nullptr, window_flags);

        if (!opt_padding)
            ImGui::PopStyleVar();

        if (opt_fullscreen)
            ImGui::PopStyleVar(2);

        // ドッキングスペースの設定
        ImGuiIO& io = ImGui::GetIO();
        if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable) {
            ImGuiID dockspace_id = ImGui::GetID("MyDockspace");
            ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
        }

        // メニューバーの例
        if (ImGui::BeginMenuBar()) {
            FileMenu();
            SettingMenu();
            WindowMenu();
            HelpMenu();
            ImGui::EndMenuBar();
        }

        ImGui::End();
    }

    void EditerManager::FileMenu() {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("Load Scene")) {
                auto path = OpenFileDialog(L"Load scene");
                if (!path.empty()) {
                    SceneIO::Load(path);
                }
            }
            if (ImGui::MenuItem("Save Scene")) {
                auto path = SaveFileDialog(L"Save scene");
                if (!path.empty()) {
                    SceneIO::Save(path);
                }
            }
            ImGui::EndMenu();
        }
    }

    void EditerManager::SettingMenu() {
        if (ImGui::BeginMenu("Setting")) {
            if (ImGui::BeginMenu("Style")) {
                if (ImGui::Button("Save All")) {
                    SaveStyle();
                }
                ImGui::SameLine();
                if (ImGui::Button("Load All")) {
                    LoadStyle();
                }
                ImGui::ShowStyleEditor();
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("PostEffect")) {
                auto& postEffect = RenderManager::GetInstance()->GetPostEffect();

                if (ImGui::BeginMenu("Grayscale")) {
                    auto& grayscale = postEffect.GetGrayscale();
                    ImGui::Checkbox("IsActive", &grayscale.isActive);
                    ImGui::ColorEdit3("Color", &grayscale.color.x);
                    ImGui::EndMenu();
                }
                if (ImGui::BeginMenu("Vignette")) {
                    auto& vignette = postEffect.GetVignette();
                    ImGui::Checkbox("IsActive", &vignette.isActive);
                    ImGui::DragFloat("Intensity", &vignette.intensity, 0.001f);
                    ImGui::DragFloat("Power", &vignette.power, 0.001f);
                    ImGui::EndMenu();
                }

                if (ImGui::BeginMenu("HSVFilter")) {
                    auto& hsvFilter = postEffect.GetHSVFilter();
                    ImGui::DragFloat3("Bias", &hsvFilter.bias.x, 0.001f);
                    ImGui::DragFloat3("Factor", &hsvFilter.factor.x, 0.001f);
                    ImGui::EndMenu();
                }

                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Sky")) {
                RenderManager::GetInstance()->GetSky().DrawImGui();
                ImGui::EndMenu();
            }
            bool pathtracing = !sceneView_->SetUseMainImage();
            ImGui::Checkbox("Pathtracing", &pathtracing);
            sceneView_->SetUseMainImage(!pathtracing);
            ImGui::EndMenu();
        }
    }

    void EditerManager::WindowMenu() {
        if (ImGui::BeginMenu("Window")) {
            if (ImGui::MenuItem("Hierarchy", NULL, hierarchyView_->isDisplayed)) {
                hierarchyView_->isDisplayed ^= true;
            }
            if (ImGui::MenuItem("Inspector", NULL, inspectorView_->isDisplayed)) {
                inspectorView_->isDisplayed ^= true;
            }
            if (ImGui::MenuItem("Scene", NULL, sceneView_->isDisplayed)) {
                sceneView_->isDisplayed ^= true;
            }
            if (ImGui::MenuItem("Project", NULL, projectView_->isDisplayed)) {
                projectView_->isDisplayed ^= true;
            }
            if (ImGui::MenuItem("Console", NULL, consoleView_->isDisplayed)) {
                consoleView_->isDisplayed ^= true;
            }

            ImGui::EndMenu();
        }
    }

    void EditerManager::HelpMenu() {
        if (ImGui::BeginMenu("Help")) {
            const char helpMessage[] = {
                "Camera Options\n"
                "Move forward and backward : Mouse wheel\n" // 前後移動
                "Move up : Space key\n"
                "Move down : LShift key\n"
                "Move camera : W A S D key\n"
                "Focus point transition : Middle mouse button drag\n" // 注視点移動
                "View point transition : Right mouse button drag" // 視点移動
            };
            ImGui::Text(helpMessage);
            ImGui::EndMenu();
        }
    }

    void EditerManager::LoadStyle() {
        std::ifstream file(kStyleFile);
        //　スタイルファイルがない場合クラシックにする
        if (!file.is_open()) {
            ImGui::StyleColorsClassic();
            return;
        }

        nlohmann::json j;
        file >> j;
        DeserializeStyle(j, ImGui::GetStyle());
        file.close();
    }

    void EditerManager::SaveStyle() {
        std::ofstream file(kStyleFile);
        if (file.is_open()) {
            auto j = SerializeStyle(ImGui::GetStyle());
            file << j.dump(4);
            file.close();
        }
    }

}