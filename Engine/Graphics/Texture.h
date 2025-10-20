#pragma once

#include <memory>
#include <filesystem>

#include "Core/TextureResource.h"

class Texture {
public:
    enum class Interpolation {
        Point,
        Linear,
    };

    enum class Extension {
        Repeat,
        Clamp,
        Mirror
    };

    static std::shared_ptr<Texture> Load(const std::filesystem::path& path);

    std::shared_ptr<TextureResource> GetResource() const { return resource_; }
    Interpolation GetInterpolation() const { return interpolation_; }
    Extension GetExtension() const { return extension_; }
    uint32_t GetWidth() const { return (uint32_t)resource_->GetDesc().Width; }
    uint32_t GetHeight() const { return (uint32_t)resource_->GetDesc().Height; }

    void SetInterpolation(Interpolation interpolation) { interpolation_ = interpolation; }
    void SetExtension(Extension extension) { extension_ = extension; }

    const DescriptorHandle& GetSampler() const;

private:
    Texture() = default;
    ~Texture() = default;


    std::shared_ptr<TextureResource> resource_;

    Interpolation interpolation_ = Interpolation::Linear;
    Extension extension_ = Extension::Repeat;
};