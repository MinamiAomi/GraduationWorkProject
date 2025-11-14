#include "AnimationAsset.h"

#include <cassert>

#include "Graphics/Animation.h"

void AnimationAsset::InternalLoad() {
    assert(state_ == State::Loading);
    type_ = Type::Animation;
    core_ = Animation::Load(path_);
}