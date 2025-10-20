/// 
/// サウンドのアセット
/// 

#pragma once
#include "Asset.h"

class Sound;

class SoundAsset :
    public Asset {
public:
    void RenderInInspectorView() override {}

    std::shared_ptr<Sound> Get() const { return core_; }

#ifdef ENABLE_IMGUI
    // サムネイル画像を返す
    ThumbnailData GetThumbnail() override;
#endif // ENABLE_IMGUI
private:
    void InternalLoad() override;

    std::shared_ptr<Sound> core_;
};