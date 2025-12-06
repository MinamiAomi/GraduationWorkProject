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

namespace Editer {

    void EditerManager::Initialize() {
#ifdef ENABLE_IMGUI
        auto graphics = Engine::GetGraphics();
        auto window = Engine::GetGameWindow();
        auto renderManager = Engine::GetRenderManager();

        descriptor_ = graphics->AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        auto& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

        ImFont* font = io.Fonts->AddFontFromFileTTF(
            "c:/Windows/Fonts/meiryo.ttc",
            18.0f,
            nullptr,
            io.Fonts->GetGlyphRangesJapanese()
        );

        assert(font != nullptr);

        //ImGui::StyleColorsDark();
        //ImGui::StyleColorsClassic();
        // スタイルを読む

        ImGui_ImplWin32_Init(window->GetHWND());
        ImGui_ImplDX12_Init(
            graphics->GetDevice(),
            SwapChain::kNumBuffers,
            renderManager->GetSwapChain().GetColorBuffer(0).GetRTVFormat(ColorBuffer::RTV::SRGB),
            graphics->GetDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV),
            descriptor_,
            descriptor_);
#endif // ENABLE_IMGUI
    }

    void EditerManager::Render() {
#ifdef ENABLE_IMGUI
        ImGui_ImplWin32_NewFrame();
        ImGui_ImplDX12_NewFrame();
        ImGui::NewFrame();
#endif // ENABLE_IMGUI
    }

    void EditerManager::RenderToColorBuffer(CommandContext& commandContext) {
#ifdef ENABLE_IMGUI
        ImGui::Render();
        ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), commandContext);
#endif // ENABLE_IMGUI
    }

    void EditerManager::Finalize() {
#ifdef ENABLE_IMGUI
        ImGui_ImplDX12_Shutdown();
        ImGui_ImplWin32_Shutdown();
        ImGui::DestroyContext();
#endif // ENABLE_IMGUI
    }

}