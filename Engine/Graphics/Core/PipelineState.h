#pragma once

#include <d3d12.h>
#include <wrl/client.h>

#include <string>

class PipelineState {
public:
    ~PipelineState() { Destroy(); }
    void Create(const std::wstring& name, const D3D12_GRAPHICS_PIPELINE_STATE_DESC& desc);
    void Create(const std::wstring& name, const D3D12_COMPUTE_PIPELINE_STATE_DESC& desc);

    operator ID3D12PipelineState* () const { return pipelineState_.Get(); }
    operator bool() const { return pipelineState_; }

    const Microsoft::WRL::ComPtr<ID3D12PipelineState>& Get() const { return pipelineState_; }

private:
    void Destroy();

    Microsoft::WRL::ComPtr<ID3D12PipelineState> pipelineState_;
    
#ifdef _DEBUG
    std::wstring name_;
#endif // _DEBUG
};