#pragma once
#include "../Core/GPUResource.h"

#include <vector>

#include "../Mesh.h"

class StructuredBuffer;
class CommandContext;

class BLAS : 
    public GPUResource {
public:
    void Create(const std::wstring& name, CommandContext& commandContext, const std::vector<Mesh>& meshes);
    void Create(const std::wstring& name, CommandContext& commandContext, const std::vector<D3D12_RAYTRACING_GEOMETRY_DESC>& geometryDescs, bool allowUpdate = false);
    void Update(CommandContext& commandContext, const std::vector<D3D12_RAYTRACING_GEOMETRY_DESC>& geometryDescs);

private:
    GPUResource scratchResource_;
    uint32_t numDescs_ = 0;
};