///
/// アセットの基底クラス
/// 

#pragma once

#include <memory>
#include <string>
#include <filesystem>

#include "Graphics/ImGuiManager.h"

#ifdef ENABLE_IMGUI
struct ThumbnailData {
    ImTextureID image = nullptr;
    ImVec2 size = { 64.0f, 64.0f };
};
#endif

class Asset {
public:
    enum class Type {
        None,
        Texture,
        Model,
        Material,
        Animation,
        Sound,

        NumTypes
    };

    enum class State {
        Unloaded,
        Loading,
        Loaded,

        NumStates,
    };

    virtual ~Asset() {}

    /// <summary>
    /// 読み込み
    /// </summary>
    /// <param name="path">ファイルのパス</param>
    /// <param name="name">アセットの名前</param>
    void Load(const std::filesystem::path& path, const std::string& name = "");


    void SetName(const std::string& name) {
        name_ = name;
    }

    // ゲッター

    const std::filesystem::path& GetPath() const { return path_; }
    const std::string& GetName() const { return name_; }
    Type GetType() const { return type_; }
    State GetState() const { return state_; }

    /// <summary>
    /// 使用可能か
    /// </summary>
    /// <returns></returns>
    bool IsReady() const { return state_ == State::Loaded; }

protected:
    virtual void InternalLoad() = 0;

    std::filesystem::path path_;
    std::string name_;
    Type type_ = Type::None;
    State state_ = State::Unloaded;

};