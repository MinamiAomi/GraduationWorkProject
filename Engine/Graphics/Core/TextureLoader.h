#pragma once

#include <memory>
#include <filesystem>

#include "TextureResource.h"

namespace TextureLoader {
    std::shared_ptr<TextureResource> Load(const std::filesystem::path& path, bool useSRGB = false);

    void Release(const std::filesystem::path& path);
    void Release(const std::shared_ptr<TextureResource>& texture);
    void ReleaseAll();
}