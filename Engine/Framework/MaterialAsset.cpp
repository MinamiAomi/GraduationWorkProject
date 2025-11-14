#include "MaterialAsset.h"

#include <cassert>

#include "Graphics/Material.h"

void MaterialAsset::InternalLoad() {
    assert(state_ == State::Loading);
    type_ = Type::Material;
    core_ = std::shared_ptr<Material>();
}