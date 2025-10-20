#include "TextureLoader.h"

#include <mutex>
#include <map>

namespace TextureLoader {

    std::mutex mutex_;
    std::map<std::filesystem::path, std::shared_ptr<TextureResource>> g_map;

    std::shared_ptr<TextureResource> Load(const std::filesystem::path& path, bool useSRGB) {
        std::lock_guard lock(mutex_);

        auto iter = g_map.find(path);
        if (iter != g_map.end()) {
            return iter->second;
        }

        std::shared_ptr<TextureResource> texture = std::make_shared<TextureResource>();
        texture->Create(path.wstring(), useSRGB);

        g_map.insert(std::make_pair(path, texture));
        return texture;
    }

    void Release(const std::filesystem::path& path) {
        std::lock_guard lock(mutex_);
        g_map.erase(path);
    }

    void Release(const std::shared_ptr<TextureResource>& texture) {
        std::lock_guard lock(mutex_);
        std::erase_if(g_map, [&](const auto& iter) { return iter.second == texture; });
    }

    void ReleaseAll() {
        std::lock_guard lock(mutex_);
        g_map.clear();
    }

}
