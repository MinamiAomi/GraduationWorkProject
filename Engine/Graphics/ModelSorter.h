#pragma once

#include <vector>
#include <map>

#include "Model.h"
#include "Math/Camera.h"

class ModelSorter {
public:
    void Sort(const Camera& camera);

    const std::map<Model*, std::vector<ModelInstance*>>& GetModelInstanceMap() const { return modelInstanceMap_; }
    const std::vector<ModelInstance*>& GetDrawModels(DrawType drawType = kOpaque) const { return drawModels_.at(drawType); }

private:
    std::map<Model*, std::vector<ModelInstance*>> modelInstanceMap_;
    std::map<DrawType, std::vector<ModelInstance*>> drawModels_;
};