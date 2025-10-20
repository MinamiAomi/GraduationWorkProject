#include "ModelAsset.h"

#include <cassert>

#include "Graphics/Model.h"
#include "Graphics/ImGuiManager.h"
#include "Graphics/Core/ColorBuffer.h"
#include "Graphics/Core/CommandContext.h"


void ModelAsset::RenderInInspectorView() {
#ifdef ENABLE_IMGUI
    Asset::RenderInInspectorView();
    if (!IsReady()) { return; }
    if (core_) {
        ImGui::Text("Num Vertices  : %d", core_->GetNumVertices());
        ImGui::Text("Num Indices   : %d", core_->GetNumIndices());
        ImGui::Text("Num Meshes    : %d", core_->GetMeshes().size());
        ImGui::Text("Num Materials : %d", core_->GetMaterials().size());
    }
#endif // ENABLE_IMGUI
}

#ifdef ENABLE_IMGUI
ThumbnailData ModelAsset::GetThumbnail() {
    // ロードされていない
    if (state_ != State::Loaded) {
        return ThumbnailData();
    }

    assert(core_);

    ThumbnailData thumbnail{};
    return thumbnail;
}
#endif // ENABLE_IMGUI

void ModelAsset::InternalLoad() {
    assert(state_ == State::Loading);
    type_ = Type::Model;
    core_ = Model::Load(path_);

#ifdef ENABLE_IMGUI
    //// サムネイル画像を作成
    //ColorBuffer colorBuffer;
    //colorBuffer.Create(L"Thumbnail ColorBuffer", 256, 256, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB);
    //
    //
    //thumbnail_ = std::make_unique<TextureResource>();
    //thumbnail_->Create()

#endif // ENABLE_IMGUI
}