///
/// テクスチャのアセット
/// 

#pragma once
#include "Asset.h"

class Texture;

class TextureAsset :
    public Asset {
public:
    void RenderInInspectorView() override;


    std::shared_ptr<Texture> Get() const { return core_; }

#ifdef ENABLE_IMGUI
    // サムネイル画像を返す
    ThumbnailData GetThumbnail() override;
#endif // ENABLE_IMGUI
private:
    void InternalLoad() override;

    std::shared_ptr<Texture> core_;
};