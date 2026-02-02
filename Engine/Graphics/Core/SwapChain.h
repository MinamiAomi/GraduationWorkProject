#pragma once

#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl/client.h>

#include <cstdint>
#include <memory>

#include "DescriptorHandle.h"

class ColorBuffer;

class SwapChain {
public:
    static const uint32_t kNumBuffers = 3;

    void Create(HWND hWnd);
    void Present();
    void Resize(uint32_t width, uint32_t height);
    void SetFullscreen(bool mode);

    ColorBuffer& GetColorBuffer(uint32_t bufferIndex) { return *buffers_[bufferIndex]; }
    const ColorBuffer& GetColorBuffer(uint32_t bufferIndex) const { return *buffers_[bufferIndex]; }
    uint32_t GetCurrentBackBufferIndex() const { return swapChain_->GetCurrentBackBufferIndex(); }

private:
    Microsoft::WRL::ComPtr<IDXGISwapChain4> swapChain_;
    std::unique_ptr<ColorBuffer> buffers_[kNumBuffers];
    HWND hWnd_ = nullptr;
    int32_t refreshRate_ = 0;
};