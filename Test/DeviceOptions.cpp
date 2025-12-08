#include "DeviceOptions.h"

#include "Framework/AssetManager.h"
#include <string>

enum GUI {
    GUI_GameStart = 0,
    GUI_ConnectionSettings,
    GUI_Quit,
    GUI_OptionCursor_Triangle0,
    GUI_OptionCursor_Triangle1,
    GUI_OptionCursor_Triangle2,
    GUI_ConnectionSettingsTitle,
    GUI_FindLightDevice,
    GUI_FoundLightDevice,
    GUI_MouseKeyboard,
    GUI_Back,
    GUI_ConnectionSettingsCursor_Triangle0,
    GUI_ConnectionSettingsCursor_Triangle1,
    GUI_ConnectionSettingsCursor_Triangle2,
    GUI_Circle0,
    GUI_Circle1,
    GUI_Circle2,
    GUI_Circle3,
    GUI_Circle4,
    GUI_Circle5,

    GUI_Count
};

struct GUIContact {
    std::string texture;
    Vector2 position;
    Vector2 size;
    Color color;
};

const GUIContact guiContact[GUI_Count] = {
    { "DCUI_GameStart", { 640, 800 }, { 326, 50 } },
    { "DCUI_ConnectionSettings", { 640, 900 }, { 569, 63 } },
    { "DCUI_Quit", { 640, 1000 }, { 117, 53 } },
    { "DCUI_Triangle", {}, { 64, 64 } },
    { "DCUI_Triangle", {}, { 64, 64 } },
    { "DCUI_Triangle", {}, { 64, 64 } },
    { "DCUI_ConnectionSettings", { 640, 200 }, { 569, 63 } },
    { "DCUI_FindLightDevice", { 640, 800 }, { 488, 62 } },
    { "DCUI_FoundLightDevice", { 640, 800 }, { 545, 62 } },
    { "DCUI_MouseKeyboard", { 640, 900 }, { 537, 63 } },
    { "DCUI_Back", { 640, 1000 }, { 140, 48 } },
    { "DCUI_Triangle", {}, { 64, 64 } },
    { "DCUI_Triangle", {}, { 64, 64 } },
    { "DCUI_Triangle", {}, { 64, 64 } },
    { "DCUI_Circle", {}, { 64, 64 }, { 1.0f, 1.0f, 1.0f, 1.0f } },
    { "DCUI_Circle", {}, { 64, 64 }, { 1.0f, 1.0f, 1.0f, 0.75f }},
    { "DCUI_Circle", {}, { 64, 64 }, { 1.0f, 1.0f, 1.0f, 0.5f }},
    { "DCUI_Circle", {}, { 64, 64 }, { 1.0f, 1.0f, 1.0f, 0.25f }},
    { "DCUI_Circle", {}, { 64, 64 }, { 1.0f, 1.0f, 1.0f, 0.5f }},
    { "DCUI_Circle", {}, { 64, 64 }, { 1.0f, 1.0f, 1.0f, 0.75f }},
};

void DeviceOptions::Initialize() {
    sprites_.resize(GUI_Count);

    auto assetManager = AssetManager::GetInstance();

    for (uint32_t i = 0; i < GUI_Count; ++i) {
        auto& sprite = sprites_[i] = std::make_unique<Sprite>();
        sprite->SetTexture(assetManager->textureMap.Get(guiContact[i].texture)->Get());
        sprite->SetPosition(guiContact[i].position);
        sprite->SetSize(guiContact[i].size);
        sprite->SetColor(guiContact[i].color);
    }
}