#include "Asset.h"

#include <cassert>
#include <thread>

#include "Engine.h"
#include "ThreadPool.h"
#include "Graphics/ImGuiManager.h"

void Asset::Load(const std::filesystem::path& path, const std::string& name) {
    assert(!path.empty());

    path_ = path;
    // 名前が指定されていない場合はパスの拡張子を除いた名前を使用
    SetName(name.empty() ? path.stem().string() : name);

    // 非同期読み込み
    Engine::GetThreadPool()->PushTask([this]() {
        state_ = State::Loading;
        InternalLoad();
        // GPUが終わってない場合、この状態遷移はエラーの原因
        // 要修正
        state_ = State::Loaded;
        });
}
