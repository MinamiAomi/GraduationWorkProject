#include "SoundAsset.h"

#include <cassert>

#include "Audio/Sound.h"

void SoundAsset::InternalLoad() {
    assert(state_ == State::Loading);
    type_ = Type::Sound;
    core_ = Sound::Load(path_);
}