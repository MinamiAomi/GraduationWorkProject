#pragma once

#include <filesystem>

namespace SceneIO {
    /// <summary>
    ///  シーンをロード
    /// </summary>
    /// <param name="path"></param>
    void Load(const std::filesystem::path& path);
    
    /// <summary>
    /// シーンをセーブ
    /// </summary>
    /// <param name="path"></param>
    void Save(const std::filesystem::path& path);
};