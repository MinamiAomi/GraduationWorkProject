#include "TextureAsset.h"

#include <cassert>

#include "Graphics/Texture.h"

void TextureAsset::RenderInInspectorView() {
#ifdef ENABLE_IMGUI
    Asset::RenderInInspectorView();
    if (!IsReady()) { return; }
    if (core_) {
        ImGui::Text("Width    : %d", core_->GetWidth());
        ImGui::Text("Height   : %d", core_->GetHeight());
    }
#endif // ENABLE_IMGUI
}

#ifdef ENABLE_IMGUI
ThumbnailData TextureAsset::GetThumbnail() {
    // ロードされていない
    if (state_ != State::Loaded) {
        return ThumbnailData();
    }
    
    assert(core_);

    ThumbnailData thumbnail{};
    thumbnail.size.x =  static_cast<float>(core_->GetWidth());
    thumbnail.size.y =  static_cast<float>(core_->GetHeight());
    thumbnail.image = reinterpret_cast<ImTextureID>(core_->GetResource()->GetSRV().GetGPU().ptr);
    return thumbnail;
}
#endif // ENABLE_IMGUI

void TextureAsset::InternalLoad() {
    assert(state_ == State::Loading);
    type_ = Type::Texture;
    core_ = Texture::Load(path_);
}