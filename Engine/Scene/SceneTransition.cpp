#include "SceneTransition.h"

#include "Graphics/RenderManager.h"

void SceneTransition::Update() {
    if (isPlaying_) {
        switch (mode_) {
        case SceneTransition::Mode::Ready:
            time_ = 0.0f;
            break;
        case SceneTransition::Mode::In:
            time_ += 1.0f / length_;
            if (time_ >= 1.0f) {
                time_ = 1.0f;
                mode_ = SceneTransition::Mode::Wait;
                isPlaying_ = false;
            }
            break;
        case SceneTransition::Mode::Wait:
            time_ = 1.0f;
            break;
        case SceneTransition::Mode::Out:
            time_ -= 1.0f / length_;
            if (time_ <= 0.0f) {
                time_ = 0.0f;
                mode_ = SceneTransition::Mode::Ready;
                isPlaying_ = false;
            }
            break;
        default:
            break;
        }
        RenderManager::GetInstance()->GetTransition().SetTime(time_);
    }
}

void SceneTransition::Start(Mode mode) {
    isPlaying_ = true;
    mode_ = mode;
    switch (mode) {
    case SceneTransition::Mode::Ready:
    case SceneTransition::Mode::In:
        time_ = 0.0f;
        break;
    case SceneTransition::Mode::Wait:
    case SceneTransition::Mode::Out:
        time_ = 1.0f;
        break;
    }
}
