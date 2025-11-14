#include "TextureAsset.h"

#include <cassert>

#include "Graphics/Texture.h"

void TextureAsset::InternalLoad() {
    assert(state_ == State::Loading);
    type_ = Type::Texture;
    core_ = Texture::Load(path_);
}