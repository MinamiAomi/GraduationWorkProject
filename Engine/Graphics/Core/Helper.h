#pragma once

#include "Externals/DirectXTex/Include/d3dx12.h"

#include <winnt.h>

namespace Helper {

    extern const D3D12_RASTERIZER_DESC RasterizerDefault;
    extern const D3D12_RASTERIZER_DESC RasterizerNoCull;
    extern const D3D12_RASTERIZER_DESC RasterizerWireframe;

    extern const D3D12_BLEND_DESC BlendNoColorWrite;
    extern const D3D12_BLEND_DESC BlendDisable;     // ブレンド無効
    extern const D3D12_BLEND_DESC BlendAlpha;       // アルファブレンド
    extern const D3D12_BLEND_DESC BlendMultiply;    // 乗算合成
    extern const D3D12_BLEND_DESC BlendAdditive;    // 加算合成
    extern const D3D12_BLEND_DESC BlendSubtract;    // 加算合成

    extern const D3D12_DEPTH_STENCIL_DESC DepthStateDisabled;
    extern const D3D12_DEPTH_STENCIL_DESC DepthStateReadWrite;
    extern const D3D12_DEPTH_STENCIL_DESC DepthStateReadOnly;


    template<typename T>
    T AlignUp(T value, size_t alignment) {
        return T((size_t(value) + (alignment - 1)) & ~(alignment - 1));
    }

    struct DWParam {
        DWParam(FLOAT f) : v{ .f = f } {}
        DWParam(UINT u) : v{ .u = u } {}
        DWParam(INT i) : v{ .i = i } {}

        void operator=(FLOAT f) { v.f = f; }
        void operator=(UINT u) { v.u = u; }
        void operator=(INT i) { v.i = i; }

        union Value {
            FLOAT f;
            UINT u;
            INT i;
        } v;
    };

    DXGI_FORMAT GetBaseFormat(DXGI_FORMAT format);
    DXGI_FORMAT GetUAVFormat(DXGI_FORMAT format);
    DXGI_FORMAT GetDSVFormat(DXGI_FORMAT format);
    DXGI_FORMAT GetDepthFormat(DXGI_FORMAT format);
    DXGI_FORMAT GetStencilFormat(DXGI_FORMAT format);
    DXGI_FORMAT GetSRGBFormat(DXGI_FORMAT format);
    size_t GetBytePerPixel(DXGI_FORMAT format);

    void AssertIfFailed(HRESULT hr, const wchar_t* str);

    std::wstring GetCommandListTypeStr(D3D12_COMMAND_LIST_TYPE type);
    std::wstring GetDescriptorHeapTypeStr(D3D12_DESCRIPTOR_HEAP_TYPE type);
}

#ifndef HELPER_DEFINES
#define HELPER_DEFINES

#define STRINGIFY_(x) #x
#define STRINGIFY(x) STRINGIFY_(x)
#define WIDE_(x) L##x
#define WIDE(x) WIDE_(x)

#define ASSERT_IF_FAILED(hr) Helper::AssertIfFailed(hr, WIDE(__FILE__) "(" STRINGIFY(__LINE__) ")\n" STRINGIFY(hr))

#ifdef _DEBUG
#define D3D12_OBJECT_SET_NAME(object, name) object->SetName(name)
#else
#define D3D12_OBJECT_SET_NAME(object, name) ((void)object),((void)name)
#endif // _DEBUG


#endif // HELPER_DEFINES