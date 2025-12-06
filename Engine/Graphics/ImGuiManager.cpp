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

    // フォント読み込み
    ImFont* font = io.Fonts->AddFontFromFileTTF(
        "Resources/Fonts/meiryo.ttc",
        18.0f,
        nullptr,
        io.Fonts->GetGlyphRangesJapanese()
    );

    // ★このチェックを追加してください★
    if (font == nullptr) {
        // 読み込み失敗！パスが間違っているか、ファイルがロックされています。
        // 試しに別のフォント (msgothic.ttc) を読み込んでみてください。
        font = io.Fonts->AddFontFromFileTTF("c:/Windows/Fonts/msgothic.ttc", 18.0f, nullptr, io.Fonts->GetGlyphRangesJapanese());
    }

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