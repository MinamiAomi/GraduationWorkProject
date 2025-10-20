#pragma once

#include <vector>
#include <map>

#include "Model.h"
#include "Math/Camera.h"

class ModelSorter {
public:
    void Sort(const Camera& camera);

    const std::map<Model*, std::vector<ModelInstance*>>& GetModelInstanceMap() const { return modelInstanceMap_; }
    const std::vector<ModelInstance*>& GetDrawModels() const { return drawModels_; }

private:
    std::map<Model*, std::vector<ModelInstance*>> modelInstanceMap_;
    std::vector<ModelInstance*> drawModels_;
};