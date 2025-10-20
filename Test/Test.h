#pragma once
#include "Framework/Game.h"

class Test :
    public Game {
public:

    /// <summary>
    /// 初期化
    /// </summary>
    void OnInitialize() override;
    /// <summary>
    /// 終了処理
    /// </summary>
    void OnFinalize() override;

private:
    /// <summary>
    /// リソースをロード
    /// </summary>
    void LoadResource();

};