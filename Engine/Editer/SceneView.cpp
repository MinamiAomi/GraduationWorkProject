#include "SceneView.h"

#include "Graphics/RenderManager.h"

#ifdef ENABLE_IMGUI
#include "Externals/ImGui/imgui.h"
#endif // ENABLE_IMGUI

#ifdef ENABLE_IMGUI
ImVec2 CalcAspectFitSize(const ImVec2& windowSize, const ImVec2& imageSize) {
    float windowAspect = windowSize.x / windowSize.y;
    float imageAspect = imageSize.x / imageSize.y;

    ImVec2 newSize;
    if (windowAspect > imageAspect) {
        // ウィンドウがよりワイドの場合、高さに合わせて幅を計算
        newSize.y = windowSize.y;
        newSize.x = imageSize.x * (windowSize.y / imageSize.y);
    }
    else {
        // ウィンドウがよりタイトの場合、幅に合わせて高さを計算
        newSize.x = windowSize.x;
        newSize.y = imageSize.y * (windowSize.x / imageSize.x);
    }
    return newSize;
}
#endif // ENABLE_IMGUI


namespace Editer {

    void SceneView::Render(CommandContext& commandContext) {
        commandContext;
#ifdef ENABLE_IMGUI
        if (!isDisplayed) { return; }
        ImGui::Begin("Scene", &isDisplayed, ImGuiWindowFlags_NoScrollbar);
        ImTextureID imageID;
        ImVec2 size;
        if (useMainImage_) {
            auto& image = RenderManager::GetInstance()->GetFinalImageBuffer();
            commandContext.TransitionResource(image, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
            commandContext.FlushResourceBarriers();
            imageID = reinterpret_cast<ImTextureID>(image.GetSRV().GetGPU().ptr);
            size = { (float)image.GetWidth(), (float)image.GetHeight() };
        }
        else {

            auto& image = RenderManager::GetInstance()->GetLightingRenderingPass().GetResult();
            commandContext.TransitionResource(image, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
            commandContext.FlushResourceBarriers();
            imageID = reinterpret_cast<ImTextureID>(image.GetSRV().GetGPU().ptr);
            size = { (float)image.GetWidth(), (float)image.GetHeight() };
        }
        ImVec2 windowSize = { ImGui::GetWindowWidth(), ImGui::GetWindowHeight() - ImGui::CalcTextSize("Scene").y - ImGui::GetStyle().FramePadding.y * 2.0f};
        ImVec2 imageSize = CalcAspectFitSize(windowSize, size);
        ImVec2 imageOffset = { (windowSize.x - imageSize.x) * 0.5f,  ImGui::CalcTextSize("Scene").y + ImGui::GetStyle().FramePadding.y * 2.0f + (windowSize.y - imageSize.y) * 0.5f };
        ImGui::SetCursorPos(imageOffset);
        ImGui::Image(imageID, imageSize);
        ImGui::End();
#endif // ENABLE_IMGUI


    }

}