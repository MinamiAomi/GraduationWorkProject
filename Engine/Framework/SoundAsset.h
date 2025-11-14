/// 
/// サウンドのアセット
/// 

#pragma once
#include "Asset.h"

class Sound;

class SoundAsset :
    public Asset {
public:
    std::shared_ptr<Sound> Get() const { return core_; }

private:
    void InternalLoad() override;

    std::shared_ptr<Sound> core_;
};