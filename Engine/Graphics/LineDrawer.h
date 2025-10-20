#pragma once

#include <vector>

#include "Core/RootSignature.h"
#include "Core/PipelineState.h"
#include "Core/UploadBuffer.h"

#include "Math/MathUtils.h"
#include "Math/Camera.h"

class CommandContext;

class LineDrawer {
public:
    struct Vertex {
        Vector3 position;
        Vector4 color;
    };

    void Initialize(DXGI_FORMAT rtvFormat);
    void AddLine(const Vector3& start, const Vector3& end, const Vector4& color = Vector4::one);
    void Render(CommandContext& commandContext, const Camera& camera);

private:
    RootSignature rootSignature_;
    PipelineState pipelineState_;
    std::vector<Vertex> vertices_;
};