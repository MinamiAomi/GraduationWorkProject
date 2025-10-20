#include "AnimationAsset.h"

#include <cassert>

#include "Graphics/Animation.h"

#ifdef ENABLE_IMGUI
ThumbnailData AnimationAsset::GetThumbnail() {
    // ロードされていない
    if (state_ != State::Loaded) {
        return ThumbnailData();
    }

    assert(core_);

    ThumbnailData thumbnail{};
    return thumbnail;
}
#endif // ENABLE_IMGUI

void AnimationAsset::InternalLoad() {
    assert(state_ == State::Loading);
    type_ = Type::Animation;
    core_ = Animation::Load(path_);
}