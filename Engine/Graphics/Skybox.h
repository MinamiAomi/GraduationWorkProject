#pragma once

#include "Core/RootSignature.h"
#include "Core/PipelineState.h"
#include "Core/TextureResource.h"
#include "Math/MathUtils.h"

class CommandContext;
class Camera;

class Skybox {
public:
    /// <summary>
    /// 初期化
    /// </summary>
    /// <param name="rtvFormat"></param>
    /// <param name="dsvFormat"></param>
    void Initialize(DXGI_FORMAT rtvFormat, DXGI_FORMAT dsvFormat);
    /// <summary>
    /// 描画
    /// </summary>
    /// <param name="commandContext"></param>
    /// <param name="camera"></param>
    void Render(CommandContext& commandContext, const Camera& camera);

    // セッター

    void SetWorldMatrix(const Matrix4x4& worldMatrix) { worldMatrix_ = worldMatrix; }
    void SetColor(const Vector4& color) { color_ = color; }
    void SetTexture(const std::shared_ptr<TextureResource>& texture) { texture_ = texture; }

private:
    RootSignature rootSignature_;
    PipelineState pipelineState_;
    Matrix4x4 worldMatrix_;
    Vector4 color_ = Vector4::one;
    std::shared_ptr<TextureResource> texture_;

};