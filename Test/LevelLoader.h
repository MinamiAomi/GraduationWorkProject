#pragma once

#include <filesystem>

#include "GameObject/GameObjectManager.h"


namespace LevelLoader {
    /// <summary>
    /// ファイルからGameObjectManagerにロード
    /// </summary>
    /// <param name="path">ファイルパス</param>
    /// <param name="gameObjectManager">マネージャー</param>
    void Load(const std::filesystem::path& path, GameObjectManager& gameObjectManager);
}
