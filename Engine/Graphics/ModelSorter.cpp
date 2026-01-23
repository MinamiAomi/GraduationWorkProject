#include "ModelSorter.h"

void ModelSorter::Sort(const Camera& camera) {
    modelInstanceMap_.clear();
    drawModels_.clear();
    camera;
    auto& instanceList = ModelInstance::GetInstanceList();
    size_t numDrawModels = 0;
    for (auto& instance : instanceList) {
        auto model = instance->GetModel().get();
        if (!model) continue;

        //‚í‚¯‚é
        drawModels_[kOpaque].emplace_back(instance);
    }

    for (auto& list : drawModels_) {
        if (list.empty()) continue;

        std::sort(list.begin(), list.end(), [](ModelInstance* a, ModelInstance* b) {
            return a->GetModel().get() < b->GetModel().get();
        });
    }
}
