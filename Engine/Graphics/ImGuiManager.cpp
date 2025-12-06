#include "ImGuiManager.h"

#ifdef ENABLE_IMGUI
#include "Externals/ImGui/imgui_impl_dx12.h"
#include "Externals/ImGui/imgui_impl_win32.h"
#endif // ENABLE_IMGUI


#include "Core/Graphics.h"
#include "Core/SwapChain.h"
#include "Core/CommandContext.h"
#include <string> 
#include <assert.h>
void ImGuiManager::Initialize(HWND hWnd, DXGI_FORMAT rtvFormat) {
#ifdef ENABLE_IMGUI
    auto graphics = Graphics::GetInstance();
    descriptor_ = graphics->AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    auto& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    ImGui::StyleColorsDark();
    ImGui::StyleColorsClassic();

    // Win32 Init
    ImGui_ImplWin32_Init(hWnd);

    // DX12 Init (ここでフォントテクスチャがGPUに作られます)
    ImGui_ImplDX12_Init(
        graphics->GetDevice(),
        SwapChain::kNumBuffers,
        rtvFormat,
        graphics->GetDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV),
        descriptor_,
        descriptor_);
#else
    hWnd; rtvFormat;
#endif // ENABLE_IMGUI
}