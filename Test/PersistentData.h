#pragma once

#include <memory>

#include "Graphics/LightManager.h"

struct Score {
    uint32_t num;
};

// -- - シーン間
// で永続化したいデータをまとめる構造体 ---
struct PersistentData {

    // ここに必要なデータを shared_ptr や
    // 通常のメンバとして追加していく
    std::shared_ptr<Score> score_;
    std::shared_ptr<DirectionalLight> sunLight_;

    // コンストラクタで各データを初期化する
    PersistentData() {
        score_ = std::make_shared<Score>();
        sunLight_ = std::make_shared<DirectionalLight>();
    }
};