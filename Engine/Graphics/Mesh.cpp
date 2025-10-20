#include "Mesh.h"

#include <cassert>

#include "Core/Graphics.h"
#include "Core/CommandContext.h"
#include "Core/CommandQueue.h"

void Mesh::CreateBuffers(CommandContext& commandContext) {
    assert(!vertices.empty());

    vertexBuffer.Create(L"Mesh VertexBuffer", vertices.size(), sizeof(vertices[0]));
    indexBuffer.Create(L"Mesh IndexBuffer", indices.size(), sizeof(indices[0]));

    commandContext.CopyBuffer(vertexBuffer, vertexBuffer.GetBufferSize(), vertices.data());
    commandContext.CopyBuffer(indexBuffer, indexBuffer.GetBufferSize(), indices.data());
    commandContext.TransitionResource(vertexBuffer, D3D12_RESOURCE_STATE_GENERIC_READ);
    commandContext.TransitionResource(indexBuffer, D3D12_RESOURCE_STATE_GENERIC_READ);
    commandContext.FlushResourceBarriers();
}
