#include "SoundAsset.h"

#include <cassert>

#include "Audio/Sound.h"

#ifdef ENABLE_IMGUI
ThumbnailData SoundAsset::GetThumbnail() {
    // ロードされていない
    if (state_ != State::Loaded) {
        return ThumbnailData();
    }

    assert(core_);

    ThumbnailData thumbnail{};
    return thumbnail;
}
#endif // ENABLE_IMGUI

void SoundAsset::InternalLoad() {
    assert(state_ == State::Loading);
    type_ = Type::Sound;
    core_ = Sound::Load(path_);
}