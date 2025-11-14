#include "ModelAsset.h"

#include <cassert>

#include "Graphics/Model.h"
#include "Graphics/ImGuiManager.h"
#include "Graphics/Core/ColorBuffer.h"
#include "Graphics/Core/CommandContext.h"

void ModelAsset::InternalLoad() {
    assert(state_ == State::Loading);
    type_ = Type::Model;
    core_ = Model::Load(path_);
}