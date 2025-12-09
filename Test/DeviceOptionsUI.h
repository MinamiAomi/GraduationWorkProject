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

    void SetupMainMenu();
    void UpdateMainMenu();
    void SetupConnectionSettings();
    void UpdateConnectionSettings();
    void SetupFindLightDevice();
    void UpdateFindLightDevice();

    std::vector<std::unique_ptr<Sprite>> sprites_;
    State state_ = State::MainMenu;
    int16_t optionCursor_ = 0;
    int16_t connectionSettingsCursor_ = 0;

};