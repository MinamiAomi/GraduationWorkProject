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
    std::shared_ptr<Model> Get() const { return core_; }

private:
    void InternalLoad() override;

    std::shared_ptr<Model> core_;
};