#include "PipelineState.h"

#include "Graphics.h"
#include "Helper.h"

void PipelineState::Create(const std::wstring& name, const D3D12_GRAPHICS_PIPELINE_STATE_DESC& desc) {
    Destroy();

    ASSERT_IF_FAILED(Graphics::GetInstance()->GetDevice()->CreateGraphicsPipelineState(&desc, IID_PPV_ARGS(pipelineState_.ReleaseAndGetAddressOf())));
    D3D12_OBJECT_SET_NAME(pipelineState_, name.c_str());

#ifdef _DEBUG
    name_ = name;
#endif // _DEBUG
}

void PipelineState::Create(const std::wstring& name, const D3D12_COMPUTE_PIPELINE_STATE_DESC& desc) {
    Destroy();

    ASSERT_IF_FAILED(Graphics::GetInstance()->GetDevice()->CreateComputePipelineState(&desc, IID_PPV_ARGS(pipelineState_.ReleaseAndGetAddressOf())));
    D3D12_OBJECT_SET_NAME(pipelineState_, name.c_str());

#ifdef _DEBUG
    name_ = name;
#endif // _DEBUG
}

void PipelineState::Destroy() {
    if (pipelineState_) {
        Graphics::GetInstance()->GetReleasedObjectTracker().AddObject(pipelineState_);
        pipelineState_ = nullptr;
    }
}
