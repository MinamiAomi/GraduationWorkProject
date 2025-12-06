#include "ImGuiManager.h"

#ifdef ENABLE_IMGUI
#include "Externals/ImGui/imgui_impl_dx12.h"
#include "Externals/ImGui/imgui_impl_win32.h"
#endif // ENABLE_IMGUI


#include "Core/Graphics.h"
#include "Core/SwapChain.h"
#include "Core/CommandContext.h"

ImGuiManager* ImGuiManager::GetInstance() {
    static ImGuiManager instance;
    return &instance;
}

void ImGuiManager::Initialize(HWND hWnd, DXGI_FORMAT rtvFormat) {
#ifdef ENABLE_IMGUI
    auto graphics = Graphics::GetInstance();
    descriptor_ = graphics->AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    auto& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    // --- フォント読み込み ---
    // パス区切り文字を \\ に修正
    ImFont* font = io.Fonts->AddFontFromFileTTF(
        "c:/Windows/Fonts/meiryo.ttc",
        18.0f,
        nullptr,
        io.Fonts->GetGlyphRangesJapanese()
    );

    // 読み込みチェック（失敗したらここで止まります）
    // 止まった場合、パスが間違っているかファイルがありません
    assert(font != nullptr);

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

void ImGuiManager::NewFrame() {
#ifdef ENABLE_IMGUI
    ImGui_ImplWin32_NewFrame();
    ImGui_ImplDX12_NewFrame();
    ImGui::NewFrame();
#endif // ENABLE_IMGUI
}

void ImGuiManager::Render(CommandContext& commandContext) {
#ifdef ENABLE_IMGUI
    ImGui::Render();
    ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), commandContext);
#else
    commandContext;
#endif // ENABLE_IMGUI
}

void ImGuiManager::Shutdown() {
#ifdef ENABLE_IMGUI
    ImGui_ImplDX12_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
#endif // ENABLE_IMGUI

}
