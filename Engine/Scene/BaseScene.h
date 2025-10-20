#pragma once

class BaseScene {
public:
    virtual ~BaseScene() {}

    /// <summary>
    /// 初期化
    /// </summary>
    virtual void OnInitialize() = 0 {}
    /// <summary>
    /// 更新
    /// </summary>
    virtual void OnUpdate() = 0 {}
    /// <summary>
    /// 終了処理
    /// </summary>
    virtual void OnFinalize() = 0 {}
};