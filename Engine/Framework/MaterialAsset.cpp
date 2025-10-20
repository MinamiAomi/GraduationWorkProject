#include "MaterialAsset.h"

#include <cassert>

#include "Graphics/Material.h"

#ifdef ENABLE_IMGUI
ThumbnailData MaterialAsset::GetThumbnail() {
    // ロードされていない
    if (state_ != State::Loaded) {
        return ThumbnailData();
    }

    assert(core_);

    ThumbnailData thumbnail{};
    return thumbnail;
}
#endif // ENABLE_IMGUI

void MaterialAsset::InternalLoad() {
    assert(state_ == State::Loading);
    type_ = Type::Material;
    core_ = std::shared_ptr<Material>();
}