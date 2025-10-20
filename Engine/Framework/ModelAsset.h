/// 
/// モデルのアセット
/// 

#pragma once
#include "Asset.h"

#include "Graphics/Core/TextureResource.h"

class Model;

class ModelAsset :
    public Asset {
public:
    void RenderInInspectorView() override;

    std::shared_ptr<Model> Get() const { return core_; }

#ifdef ENABLE_IMGUI
    // サムネイル画像を返す
    ThumbnailData GetThumbnail() override;
#endif // ENABLE_IMGUI
private:
    void InternalLoad() override;

    std::shared_ptr<Model> core_;

#ifdef ENABLE_IMGUI
    std::unique_ptr<TextureResource> thumbnail_;
#endif // ENABLE_IMGUI
};