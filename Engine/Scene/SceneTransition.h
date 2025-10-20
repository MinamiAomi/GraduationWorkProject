#pragma once

#include <cstdint>

class SceneTransition {
public: 
    enum class Mode {
        Ready,
        In,
        Wait,
        Out,
    };
    /// <summary>
    /// 更新
    /// </summary>
    void Update();
    /// <summary>
    /// 遷移をスタート
    /// </summary>
    /// <param name="mode"></param>
    void Start(Mode mode);

    /// <summary>
    /// プレイ中か
    /// </summary>
    /// <returns></returns>
    bool IsPlaying() const { return isPlaying_; }
    
    // ゲッター

    Mode GetMode() const { return mode_; }

private:
    uint32_t length_ = 60;
    float time_;
    Mode mode_;
    bool isPlaying_;
};