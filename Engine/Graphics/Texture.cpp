#include "Texture.h"

#include "Core/TextureLoader.h"
#include "Core/SamplerManager.h"

std::shared_ptr<Texture> Texture::Load(const std::filesystem::path& path) {
    struct Helper : Texture {
        Helper() : Texture() {}
    };
    std::shared_ptr<Texture> texture = std::make_shared<Helper>();
    texture->resource_ = TextureLoader::Load(path, true);

    return texture;
}

const DescriptorHandle& Texture::GetSampler() const {

    switch (interpolation_)
    {
    case Texture::Interpolation::Point:
        switch (extension_)
        {
        case Texture::Extension::Repeat:
            return SamplerManager::LinearWrap;
        case Texture::Extension::Clamp:
            return SamplerManager::LinearClamp;
        case Texture::Extension::Mirror:
            return SamplerManager::LinearMirror;
        }
    case Texture::Interpolation::Linear:
        switch (extension_)
        {
        case Texture::Extension::Repeat:
            return SamplerManager::PointWrap;
        case Texture::Extension::Clamp:
            return SamplerManager::PointClamp;
        case Texture::Extension::Mirror:
            return SamplerManager::PointMirror;
        }
    }
    return SamplerManager::LinearWrap;
}
