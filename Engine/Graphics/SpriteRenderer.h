#pragma once

#include "Math/MathUtils.h"

#include "Core/RootSignature.h"
#include "Core/PipelineState.h"

class CommandContext;
class ColorBuffer;

class SpriteRenderer {
public:
    /// <summary>
    /// 初期化
    /// </summary>
    /// <param name="colorBuffer"></param>
    void Initialize(const ColorBuffer& colorBuffer);
    /// <summary>
    /// 描画
    /// </summary>
    /// <param name="commandContext"></param>
    /// <param name="left"></param>
    /// <param name="top"></param>
    /// <param name="right"></param>
    /// <param name="bottom"></param>
    void Render(CommandContext& commandContext, float left, float top, float right, float bottom);

private:
    RootSignature rootSignature_;
    PipelineState pipelineState_;
};