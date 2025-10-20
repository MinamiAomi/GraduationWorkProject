#pragma once

#include <map>
#include <memory>

#include "Core/RootSignature.h"
#include "Core/PipelineState.h"

#include "Model.h"
#include "Skeleton.h"
#include "SkinCluster.h"

class CommandContext;

class SkinningManager {
public:
    enum RootParameter {
        kMatrixPalette,
        kInputVertices,
        kVertexInfluence,
        kOutputVertices,
        kSkinningInformation,

        kNumRootParameters
    };

    void Initialize();
    void Add(Skeleton* skeleton, const std::shared_ptr<Model>& model);
    void Remove(Skeleton* skeleton);
    void Update(CommandContext& commandContext);

    const SkinCluster* GetSkinCluster(Skeleton* key) const { 
        auto it = skinClusters_.find(key);
        if (it != skinClusters_.end()) {
            return it->second.get(); 
        }
        return nullptr;
    }

private:
    RootSignature rootSignature_;
    PipelineState pipelineState_;

    std::map<Skeleton*, std::unique_ptr<SkinCluster>> skinClusters_;
};