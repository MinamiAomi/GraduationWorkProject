#include "ProjectView.h"

#include <algorithm>
#include <filesystem>
#include <ranges>
#include <Windows.h>

#ifdef ENABLE_IMGUI
#include "Externals/ImGui/imgui.h"
#include "Externals/ImGui/imgui_stdlib.h"
#include "Externals/ImGui/imgui_internal.h"
#endif // ENABLE_IMGUI

#include "FrameWork/Engine.h"
#include "Graphics/GameWindow.h"
#include "EditerManager.h"
#include "Debug/Debug.h"
#include "Framework/AssetManager.h"


namespace {

    enum class Filter {
        Texture,
        Model,
        Animation,
        Sound
    };

    std::filesystem::path OpenFileDialog(const std::wstring& title, Filter filter) {
        OPENFILENAME ofn{};
        wchar_t szFile[256]{};

        ZeroMemory(&ofn, sizeof(ofn));
        ofn.lStructSize = sizeof(ofn);
        ofn.hwndOwner = Engine::GetGameWindow()->GetHWND();
        ofn.lpstrFile = szFile;
        ofn.lpstrFile[0] = L'\0';
        ofn.nMaxFile = sizeof(szFile);
        switch (filter)
        {
        case Filter::Texture:
            ofn.lpstrFilter = L"Texture\0*.PNG;*.JPG;*.DDS;*.HDR";
            break;
        case Filter::Model:
            ofn.lpstrFilter = L"Model\0*.OBJ;*.GLTF;*.GLB";
            break;
        case Filter::Animation:
            ofn.lpstrFilter = L"Animation\0*.GLTF;*.GLB";
            break;
        case Filter::Sound:
            ofn.lpstrFilter = L"Sound\0*.MP3;*.WAV";
            break;
        }
        ofn.nFilterIndex = 1;
        ofn.lpstrFileTitle = NULL;
        ofn.nMaxFileTitle = 0;
        ofn.lpstrInitialDir = NULL;
        ofn.lpstrTitle = title.c_str();
        ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

        if (GetOpenFileName(&ofn) == TRUE) {
            std::filesystem::path path = ofn.lpstrFile;
            // 絶対パスから相対パスに変換
            if (path.is_absolute()) {
                path = std::filesystem::relative(path, std::filesystem::current_path());
            }
            return path;
        }
        return std::filesystem::path();
    }

    std::string TruncateText(const std::string& text, float maxWidth) {
#ifdef ENABLE_IMGUI
        ImVec2 textSize = ImGui::CalcTextSize(text.c_str());
        if (textSize.x <= maxWidth) {
            return text;
        }

        std::string truncatedText = text;
        while (truncatedText.size() > 0 && ImGui::CalcTextSize((truncatedText + "...").c_str()).x > maxWidth) {
            truncatedText.pop_back();
        }

        return truncatedText + "...";
#else
        maxWidth;
        return text;
#endif // ENABLE_IMGUI
    }
#ifdef ENABLE_IMGUI
    ImVec2 CalcFitSize(const ImVec2& target, const ImVec2& size) {
        float aspect = size.x / size.y;
        if (aspect > 1.0f) {
            return { target.x, target.y * (1.0f / aspect) };
        }
        return { target.x * aspect, target.y };
    }
#endif

}

namespace Editer {

    void ProjectView::Render() {

#ifdef ENABLE_IMGUI
        if (!isDisplayed) { return; }

        if (ImGui::Begin("Project", &isDisplayed, ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse)) {


            if (ImGui::IsWindowHovered(ImGuiHoveredFlags_ChildWindows) &&
                ImGui::IsMouseClicked(ImGuiMouseButton_Right)) {
                ImGui::OpenPopup("ProjectPopup");
            }

            if (ImGui::BeginMenuBar()) {
                if (ImGui::BeginMenu("Import")) {
                    if (ImGui::MenuItem("Texture")) {
                        auto path = OpenFileDialog(L"Texture import", Filter::Texture);
                        if (!path.empty()) {
                            auto asset = std::make_shared<TextureAsset>();
                            asset->Load(path);
                            Engine::GetAssetManager()->textureMap.Add(asset);
                        }
                    }
                    if (ImGui::MenuItem("Model")) {
                        auto path = OpenFileDialog(L"Model import", Filter::Model);
                        if (!path.empty()) {
                            auto asset = std::make_shared<ModelAsset>();
                            asset->Load(path);
                            Engine::GetAssetManager()->modelMap.Add(asset);
                        }
                    }
                    if (ImGui::MenuItem("Animation")) {
                        auto path = OpenFileDialog(L"Animation import", Filter::Animation);
                        if (!path.empty()) {
                            auto asset = std::make_shared<AnimationAsset>();
                            asset->Load(path);
                            Engine::GetAssetManager()->animationMap.Add(asset);
                        }
                    }
                    if (ImGui::MenuItem("Sound")) {
                        auto path = OpenFileDialog(L"Sound import", Filter::Sound);
                        if (!path.empty()) {
                            auto asset = std::make_shared<SoundAsset>();
                            asset->Load(path);
                            Engine::GetAssetManager()->soundMap.Add(asset);
                        }
                    }
                    ImGui::EndMenu();
                }

                float sliderWidth = 100.0f;
                float sliderHeight = 10.0f;

                ImGui::SetNextItemWidth(sliderWidth);
                ImGui::SetCursorPosX(ImGui::GetWindowSize().x - ImGui::GetStyle().WindowPadding.x - sliderWidth);
                ImGui::SetCursorPosY(ImGui::GetCursorPosY() + (ImGui::GetFrameHeight() - sliderHeight) / 2.0f);
                ImVec4 color = ImGui::GetStyle().Colors[ImGuiCol_Text];
                color.w = 0.0f;
                ImGui::PushStyleColor(ImGuiCol_Text, color);
                ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, (sliderHeight - ImGui::GetFontSize()) / 2.0f));
                ImGui::SliderFloat("##ItemScale", &itemScale_, 0.5f, 2.0f);
                ImGui::PopStyleVar();
                ImGui::PopStyleColor();

                ImGui::EndMenuBar();
            }

            ImVec2 windowSize = ImGui::GetWindowSize();
            float childPosY = ImGui::GetFrameHeight() * 2.0f;

            // 子ウィンドウの上部の高さ
            ImGui::SetCursorPos({ 0.0f, childPosY });
            // 左ウィンドウ
            ImGui::BeginChild("Left", { leftWindowSize_, windowSize.y - childPosY }, ImGuiChildFlags_ResizeX);
            ImGui::SetCursorPos(ImGui::GetStyle().WindowPadding);
            leftWindowSize_ = ImGui::GetWindowWidth();
            RenderLeftWindow();
            ImGui::EndChild();


            ImGui::SameLine();
            ImGui::SetCursorPos({ leftWindowSize_, childPosY });
            ImGui::BeginChild("Right", { windowSize.x - leftWindowSize_, windowSize.y - childPosY }, ImGuiChildFlags_AlwaysUseWindowPadding);
            RenderRightWindow();
            ImGui::EndChild();

            ImGui::SameLine();
            ImGui::SetCursorPos({ leftWindowSize_ - 2.0f, childPosY });
            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
            ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_Border));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImGui::GetStyleColorVec4(ImGuiCol_Border));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImGui::GetStyleColorVec4(ImGuiCol_Border));
            ImGui::Button("##Border20240725", { 2.0f, windowSize.y - childPosY });
            ImGui::PopStyleColor(3);
            ImGui::PopStyleVar();


            if (ImGui::BeginPopup("ProjectPopup")) {
                ImGui::Text("popup");

                ImGui::EndPopup();
            }
        }
        ImGui::End();



#endif // ENABLE_IMGUI
    }

    void ProjectView::RenderLeftWindow() {
#ifdef ENABLE_IMGUI
        ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_DefaultOpen;

        if (ImGui::TreeNodeEx("Assets", flags)) {
            auto LeafFlags = [flags, this](AssetType showAssetType) {
                auto leafFlags = flags | ImGuiTreeNodeFlags_Leaf;
                if ((showAssetTypes_ & showAssetType)) {
                    leafFlags |= ImGuiTreeNodeFlags_Selected;
                }
                return leafFlags;
                };

            if (ImGui::TreeNodeEx("Textures", LeafFlags(Texture))) {
                if (ImGui::IsItemClicked()) {
                    showAssetTypes_ = static_cast<AssetType>(showAssetTypes_ ^ Texture);
                }
                ImGui::TreePop();
            }
            if (ImGui::TreeNodeEx("Models", LeafFlags(Model))) {
                if (ImGui::IsItemClicked()) {
                    showAssetTypes_ = static_cast<AssetType>(showAssetTypes_ ^ Model);
                }
                ImGui::TreePop();
            }
            if (ImGui::TreeNodeEx("Materials", LeafFlags(Material))) {
                if (ImGui::IsItemClicked()) {
                    showAssetTypes_ = static_cast<AssetType>(showAssetTypes_ ^ Material);
                }
                ImGui::TreePop();
            }
            if (ImGui::TreeNodeEx("Animations", LeafFlags(Animation))) {
                if (ImGui::IsItemClicked()) {
                    showAssetTypes_ = static_cast<AssetType>(showAssetTypes_ ^ Animation);
                }
                ImGui::TreePop();
            }
            if (ImGui::TreeNodeEx("Sounds", LeafFlags(Sound))) {
                if (ImGui::IsItemClicked()) {
                    showAssetTypes_ = static_cast<AssetType>(showAssetTypes_ ^ Sound);
                }
                ImGui::TreePop();
            }

            ImGui::TreePop();
        }

#endif ENABLE_IMGUI

    }



    void ProjectView::RenderRightWindow() {
#ifdef ENABLE_IMGUI
        auto assetManager = Engine::GetAssetManager();


        const auto& io = ImGui::GetIO();
        if (ImGui::IsWindowHovered() && ImGui::IsKeyPressed(ImGuiKey_LeftCtrl)) {
            if (io.MouseWheel != 0) {
                itemScale_ += io.MouseWheel * 0.2f;
                itemScale_ = std::clamp(itemScale_, 0.5f, 2.0f);
            }
        }

        ImVec2 windowSize = ImGui::GetWindowSize();

        // 仮のパラメータ
        const float itemSize = 64.0f * itemScale_;  // テクスチャのサイズ

        // 一行に収まる数 
        float itemSpace = windowSize.x - ImGui::GetStyle().WindowPadding.x * 2.0f;
        // スクロールバーがある場合そのサイズも考慮
        if (ImGui::GetScrollMaxY() > 0.0f) {
            itemSpace -= ImGui::GetStyle().ScrollbarSize;
        }
        int32_t numItemsInLine = int32_t((itemSpace + ImGui::GetStyle().ItemSpacing.x) / (itemSize + ImGui::GetStyle().ItemSpacing.x));
        numItemsInLine = std::max(numItemsInLine, 1);

        uint32_t i = 0;
        auto DrawItem = [&](const std::shared_ptr<Asset>& asset) {

            ImGui::BeginGroup();
            // イメージ描画
            ImVec2 groupLocalCursourBase = ImGui::GetCursorPos();

            std::string name = asset->GetName();
            auto thumbnail = asset->GetThumbnail();
            thumbnail.size = CalcFitSize({ itemSize, itemSize }, thumbnail.size);
            // 画像が真ん中に来るよう合わせる
            ImGui::SetCursorPos({ groupLocalCursourBase.x + (itemSize - thumbnail.size.x) * 0.5f, groupLocalCursourBase.y + (itemSize - thumbnail.size.y) * 0.5f });
            if (thumbnail.image) {
                ImGui::Image(thumbnail.image, thumbnail.size);
            }
            else {
                if (asset->IsReady()) {
                    ImGui::Button("##", thumbnail.size);
                }
                else {
                    auto& style = ImGui::GetStyle();
                    auto button = style.Colors[ImGuiCol_Button];
                    auto buttonActive = style.Colors[ImGuiCol_ButtonActive];
                    auto buttonHovered = style.Colors[ImGuiCol_HeaderHovered];
                    button.x = buttonActive.x = buttonHovered.x = 1.0f;
                    ImGui::PushStyleColor(ImGuiCol_Button, button);
                    ImGui::PushStyleColor(ImGuiCol_ButtonActive, buttonActive);
                    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, buttonHovered);
                    ImGui::Button("##", thumbnail.size);
                    ImGui::PopStyleColor(3);
                }
            }

            // テキストの描画
            std::string text = name;
            text = TruncateText(text, itemSize);
            ImVec2 textSize = ImGui::CalcTextSize(text.c_str());
            float textOffsetX = (itemSize - textSize.x) * 0.5f;  // 中央揃えのためのオフセット計算
            groupLocalCursourBase = ImGui::GetCursorPos();
            ImGui::SetCursorPos({ groupLocalCursourBase.x + textOffsetX, groupLocalCursourBase.y + (itemSize - thumbnail.size.y) * 0.5f });
            //ImGui::SetCursorPosX(ImGui::GetCursorPosX() + textOffsetX);
            ImGui::Text("%s", text.c_str());

            ImGui::EndGroup();

            // グループが選択された
            if (ImGui::IsItemClicked()) {
                owner.SetSelectedObject(asset);
            }

            if (((i + 1) % numItemsInLine) != 0) {
                ImGui::SameLine();
            }
            i++;
            };

        if (showAssetTypes_ & Texture) {
            assetManager->textureMap.ForEach(DrawItem);
        }
        if (showAssetTypes_ & Model) {
            assetManager->modelMap.ForEach(DrawItem);
        }
        if (showAssetTypes_ & Material) {
            assetManager->materialMap.ForEach(DrawItem);
        }
        if (showAssetTypes_ & Animation) {
            assetManager->animationMap.ForEach(DrawItem);
        }
        if (showAssetTypes_ & Sound) {
            assetManager->soundMap.ForEach(DrawItem);
        }

#endif ENABLE_IMGUI
    }

}