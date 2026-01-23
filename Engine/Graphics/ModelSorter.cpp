#include "ModelSorter.h"

void ModelSorter::Sort(const Camera& camera) {
    modelInstanceMap_.clear();
    for (auto& list : drawModels_) {
        list.clear();
    }
    (void)camera;
    auto& instanceList = ModelInstance::GetInstanceList();
    for (auto& instance : instanceList) {
        if (!instance || !instance->IsActive()) continue;

        auto model = instance->GetModel().get();
        if (!model) continue;

        //わける
        drawModels_[kOpaque].emplace_back(instance);
    }

    for (auto& list : drawModels_) {
        if (list.empty()) continue;

        std::sort(list.begin(), list.end(), [](ModelInstance* a, ModelInstance* b) {
            return a->GetModel().get() < b->GetModel().get();
        });
    }
}
