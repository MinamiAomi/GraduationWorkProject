#include "SwapChain.h"

#include <cassert>

#include "Graphics.h"
#include "Helper.h"
#include "ColorBuffer.h"

using namespace Microsoft::WRL;

void SwapChain::Create(HWND hWnd) {
    ComPtr<IDXGIFactory7> factory;
    ASSERT_IF_FAILED(CreateDXGIFactory(IID_PPV_ARGS(factory.GetAddressOf())));

    RECT clientRect{};
    if (!GetClientRect(hWnd, &clientRect)) {
        assert(false);
    }

    HDC hdc = GetDC(hWnd);
    refreshRate_ = GetDeviceCaps(hdc, VREFRESH);
    ReleaseDC(hWnd, hdc);

    LONG clientWidth = clientRect.right - clientRect.left;
    LONG clientHeight = clientRect.bottom - clientRect.top;

    DXGI_SWAP_CHAIN_DESC1 desc{};
    desc.Width = UINT(clientWidth); // 画面幅
    desc.Height = UINT(clientHeight); // 画面高
    desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; // 色の形式
    desc.SampleDesc.Count = 1; // マルチサンプル市内
    desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT; // 描画ターゲットとして利用する
    desc.BufferCount = kNumBuffers;
    desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD; // モニタに移したら、中身を破棄

    ASSERT_IF_FAILED(factory->CreateSwapChainForHwnd(
        Graphics::GetInstance()->GetCommandManager().GetCommandQueue(),
        hWnd,
        &desc,
        nullptr,
        nullptr,
        reinterpret_cast<IDXGISwapChain1**>(swapChain_.ReleaseAndGetAddressOf())));

    //swapChain_->SetMaximumFrameLatency(1);

    for (uint32_t i = 0; i < kNumBuffers; ++i) {
        ComPtr<ID3D12Resource> resource;
        ASSERT_IF_FAILED(swapChain_->GetBuffer(i, IID_PPV_ARGS(resource.GetAddressOf())));
        buffers_[i] = std::make_unique<ColorBuffer>();
        buffers_[i]->CreateFromSwapChain(L"SwapChainBuffer" + std::to_wstring(i), resource.Detach());
    }

    hWnd_ = hWnd;
}

void SwapChain::Present() {
    static constexpr int32_t kThreasholdRefreshRate = 58;
    int vsync = refreshRate_ < kThreasholdRefreshRate ? 0 : 1;
    HRESULT hr = swapChain_->Present(vsync, 0);
    hr;
    Graphics::GetInstance()->CheckDRED(hr);
}

void SwapChain::Resize(uint32_t width, uint32_t height) {
    if (swapChain_ == nullptr) return;

    for (uint32_t i = 0; i < kNumBuffers; ++i) {
        buffers_[i]->Release();
        buffers_[i].reset();
    }

    DXGI_SWAP_CHAIN_DESC1 desc{};
    swapChain_->GetDesc1(&desc);

    HRESULT hr = swapChain_->ResizeBuffers(
        kNumBuffers, width, height, desc.Format, desc.Flags);
    ASSERT_IF_FAILED(hr);

    for (uint32_t i = 0; i < kNumBuffers; ++i) {
        ComPtr<ID3D12Resource> resource;
        ASSERT_IF_FAILED(swapChain_->GetBuffer(i, IID_PPV_ARGS(resource.GetAddressOf())));
        buffers_[i] = std::make_unique<ColorBuffer>();
        buffers_[i]->CreateFromSwapChain(L"SwapChainBuffer" + std::to_wstring(i), resource.Detach());
    }
}

void SwapChain::SetFullscreen(bool mode) {
    swapChain_->SetFullscreenState(mode ? TRUE : FALSE, nullptr);
}
