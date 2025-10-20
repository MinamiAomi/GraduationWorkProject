#pragma once

#include "DescriptorHandle.h"

namespace SamplerManager {

    extern DescriptorHandle AnisotropicWrap;

    extern DescriptorHandle LinearWrap;
    extern DescriptorHandle LinearClamp;
    extern DescriptorHandle LinearMirror;

    extern DescriptorHandle PointWrap;
    extern DescriptorHandle PointClamp;
    extern DescriptorHandle PointBorder;
    extern DescriptorHandle PointMirror;

    void Initialize();

}