///
/// ゲーム基底クラス
/// 

#pragma once

class Game {
public:
    virtual ~Game() {}

    /// <summary>
    /// 初期化
    /// </summary>
    virtual void OnInitialize() = 0;
    /// <summary>
    /// 終了処理
    /// </summary>
    virtual void OnFinalize() = 0;
};