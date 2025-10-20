#include "StateObject.h"

#include "../Core/Graphics.h"
#include "../Core/Helper.h"

void StateObject::Create(const std::wstring& name, const D3D12_STATE_OBJECT_DESC& desc) {
    Destroy();
    ASSERT_IF_FAILED(Graphics::GetInstance()->GetDXRDevoce()->CreateStateObject(&desc, IID_PPV_ARGS(stateObject_.ReleaseAndGetAddressOf())));
    D3D12_OBJECT_SET_NAME(stateObject_, name.c_str());
#ifdef _DEBUG
    name_ = name;
#endif // _DEBUG
}

void StateObject::Destroy() {
    if (stateObject_) {
        Graphics::GetInstance()->GetReleasedObjectTracker().AddObject(stateObject_);
        stateObject_ = nullptr;
    }
}
