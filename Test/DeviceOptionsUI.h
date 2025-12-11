#pragma once

#include "Graphics/Sprite.h"

class DeviceOptionsUI {
public:
  
    void Initialize();
    void Update();

private:
    enum class State {
        MainMenu,
        ConnectionSettings,
        FindLightDevice,
    };

    const int32_t kSelectionDelay = 10;
    const int32_t kAnimationCircle = 30;

    void SetupMainMenu();
    void UpdateMainMenu();
    void SetupConnectionSettings();
    void UpdateConnectionSettings();
    void SetupFindLightDevice();
    void UpdateFindLightDevice();

    std::vector<std::unique_ptr<Sprite>> sprites_;
    State state_ = State::MainMenu;
    int32_t animationTimer_ = 0;
    int32_t selectionTimer_ = 0;
    int16_t optionCursor_ = 0;
    bool needsStateInitialization_ = false;
};