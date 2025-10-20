#pragma once
#include "PixelBuffer.h"

#include "DescriptorHandle.h"

class ColorBuffer : public PixelBuffer {
public:
    struct RTV {
        enum Enum {
            Linear,
            SRGB,

            NumRTVs
        };
    };

    void CreateFromSwapChain(const std::wstring& name, ID3D12Resource* resource);
    void Create(const std::wstring& name, uint32_t width, uint32_t height, DXGI_FORMAT format);
    void CreateArray(const std::wstring& name, uint32_t width, uint32_t height, uint32_t arraySize, DXGI_FORMAT format);

    void SetClearColor(const float* clearColor);
    const float* GetClearColor() const { return clearColor_; }
    
    DXGI_FORMAT GetRTVFormat(RTV::Enum rtv = RTV::Linear) const { return rtvFormats_[rtv]; }

    const DescriptorHandle& GetRTV(RTV::Enum rtv = RTV::Linear) const { return rtvHandles_[rtv]; }
    const DescriptorHandle& GetSRV() const { return srvHandle_; }
    const DescriptorHandle& GetUAV() const { return uavHandle_; }
    bool IsSRGB() const { return rtvFormats_[RTV::Linear] != rtvFormats_[RTV::SRGB]; }

private:
    void CreateViews();

    float clearColor_[4]{ 0.0f,0.0f,0.0f,0.0f };
    DescriptorHandle srvHandle_;
    DescriptorHandle uavHandle_;
    DescriptorHandle rtvHandles_[RTV::NumRTVs];
    DXGI_FORMAT rtvFormats_[RTV::NumRTVs];
};
