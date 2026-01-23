#include "ModelSorter.h"

void ModelSorter::Sort(const Camera& camera) {
    modelInstanceMap_.clear();
    drawModels_.clear();
    camera;
    auto& instanceList = ModelInstance::GetInstanceList();
    for (auto& instance : instanceList) {
        auto model = instance->GetModel().get();
        if (!model) continue;

        //わける
        drawModels_[kOpaque].emplace_back(instance);
    }

    for (auto& list : drawModels_) {
        if (list.second.empty()) continue;

        std::sort(list.second.begin(), list.second.end(), [](ModelInstance* a, ModelInstance* b) {
            return a->GetModel().get() < b->GetModel().get();
        });
    }
}
