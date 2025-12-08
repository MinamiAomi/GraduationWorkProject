#pragma once

#include "Graphics/Sprite.h"

class DeviceOptions {
public:
  
    void Initialize();
    void Update();

private:
    std::vector<std::unique_ptr<Sprite>> sprites_;
    uint16_t optionCursor = 0;
    uint16_t connectionSettingsCursor = 0;

};