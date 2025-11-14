///
/// アニメーションのアセット
/// 

#pragma once
#include "Asset.h"

class Animation;

class AnimationAsset :
    public Asset {
public:
    std::shared_ptr<Animation> Get() const { return core_; }

private:
    void InternalLoad() override;

    std::shared_ptr<Animation> core_;
};