#pragma once
#include "Core/RootSignature.h"
#include "Core/PipelineState.h"
#include "Math/MathUtils.h"

class CommandContext;
class ColorBuffer;

class Transition {
public:
    /// <summary>
    /// 初期化
    /// </summary>
    void Initialize();
    /// <summary>
    /// ディスパッチ
    /// </summary>
    /// <param name="commandContext"></param>
    /// <param name="texture"></param>
    void Dispatch(CommandContext& commandContext, ColorBuffer& texture);

    // セッター

    void SetTime(float t) { time_ = t; }
    void SetFadeColor(const Vector3& color) { fadeColor_ = color; }

private:
    RootSignature rootSignature_;
    PipelineState pipelineState_;
    Vector3 fadeColor_;
    float time_ = 0.0f;
};