#pragma once
#include "GameObject/Component.h"

#include "Framework/ModelAsset.h"
#include "Graphics/Model.h"

class MeshComponent :
    public Component {
    COMPONENT_IMPL(MeshComponent);
public:
    /// <summary>
    /// 初期化
    /// </summary>
    void Initialize() override;
    /// <summary>
    /// 更新
    /// </summary>
    void Update() override;
    /// <summary>
    /// エディターで使用される
    /// </summary>
    void Edit() override;
    /// <summary>
    /// jsonにエクスポート
    /// </summary>
    /// <param name="json"></param>
    void Export(nlohmann::json& json) const override;
    /// <summary>
    /// jsonにインポート
    /// </summary>
    /// <param name="json"></param>
    void Import(const nlohmann::json& json) override;

    // セッター

    void SetModelName(const std::string& name) { modelName_ = name; }

private:
    void ApplyModel();

    ModelInstance model_;
    std::shared_ptr<Material> customMaterial_;
    std::shared_ptr<ModelAsset> asset_;
    std::string modelName_;
};