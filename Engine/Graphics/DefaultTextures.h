#pragma once

#include "Core/TextureResource.h"

namespace DefaultTexture {

    extern TextureResource White;
    extern TextureResource Black;
    extern TextureResource Normal;
    extern TextureResource BlackCubeMap;

    void Initialize();
    void Finalize();

}