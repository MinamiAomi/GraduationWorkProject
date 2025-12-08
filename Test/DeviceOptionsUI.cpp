#include "DeviceOptionsUI.h"

#include <string>
#include "Framework/AssetManager.h"
#include "Input/Input.h"

enum GUI {
    GUI_GameStart = 0,
    GUI_ConnectionSettings,
    GUI_Quit,
    GUI_OptionCursor_Triangle0,
    GUI_OptionCursor_Triangle1,
    GUI_OptionCursor_Triangle2,
    GUI_ConnectionSettingsTitle,
    GUI_FindLightDevice,
    GUI_MouseKeyboard,
    GUI_Back,
    GUI_ConnectionSettingsCursor_Triangle0,
    GUI_ConnectionSettingsCursor_Triangle1,
    GUI_ConnectionSettingsCursor_Triangle2,
    GUI_FoundLightDevice,
    GUI_NotFoundLightDevice,
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
    { "DCUI_Triangle", { 427, 800 }, { 64, 64 } },
    { "DCUI_Triangle", { 305, 900 }, { 64, 64 } },
    { "DCUI_Triangle", { 531, 100 }, { 64, 64 } },
    { "DCUI_ConnectionSettings", { 640, 200 }, { 569, 63 } },
    { "DCUI_FindLightDevice", { 640, 800 }, { 488, 62 } },
    { "DCUI_MouseKeyboard", { 640, 900 }, { 537, 63 } },
    { "DCUI_Back", { 640, 1000 }, { 140, 48 } },
    { "DCUI_Triangle", { 346, 800 }, { 64, 64 } },
    { "DCUI_Triangle", { 321, 900 }, { 64, 64 } },
    { "DCUI_Triangle", { 520, 1000 }, { 64, 64 } },
    { "DCUI_FoundLightDevice", { 540, 800 }, { 545, 62 } },
    { "DCUI_NotFoundLightDevice", { 540, 800 }, { 545, 62 } },
    { "DCUI_Circle", {}, { 64, 64 }, { 1.0f, 1.0f, 1.0f, 1.0f } },
    { "DCUI_Circle", {}, { 64, 64 }, { 1.0f, 1.0f, 1.0f, 0.75f }},
    { "DCUI_Circle", {}, { 64, 64 }, { 1.0f, 1.0f, 1.0f, 0.5f }},
    { "DCUI_Circle", {}, { 64, 64 }, { 1.0f, 1.0f, 1.0f, 0.25f }},
    { "DCUI_Circle", {}, { 64, 64 }, { 1.0f, 1.0f, 1.0f, 0.5f }},
    { "DCUI_Circle", {}, { 64, 64 }, { 1.0f, 1.0f, 1.0f, 0.75f }},
};

void DeviceOptionsUI::Initialize() {
    sprites_.resize(GUI_Count);

    auto assetManager = AssetManager::GetInstance();

    for (uint32_t i = 0; i < GUI_Count; ++i) {
        auto& sprite = sprites_[i] = std::make_unique<Sprite>();
        sprite->SetTexture(assetManager->textureMap.Get(guiContact[i].texture)->Get());
        sprite->SetPosition(guiContact[i].position);
        sprite->SetScale(guiContact[i].size);
        sprite->SetColor(guiContact[i].color);
    }
}

void DeviceOptionsUI::Update() {


    switch (state_)
    {
    case State::MainMenu:
        UpdateMainMenu();
        break;
    case State::ConnectionSettings:
        UpdateConnectionSettings();
        break;
    case State::FindLightDevice:
        UpdateFindLightDevice();
        break;
    default:
        break;
    }
}

void DeviceOptionsUI::SetupMainMenu() {
    for (auto& sprite : sprites_) {
        sprite->SetIsActive(false);
    }

    state_ = State::MainMenu;
    sprites_[GUI_GameStart]->SetIsActive(true);
    sprites_[GUI_ConnectionSettings]->SetIsActive(true);
    sprites_[GUI_Quit]->SetIsActive(true);
    optionCursor_ = 0;

}

void DeviceOptionsUI::UpdateMainMenu() {
    Input* input = Input::GetInstance();

    sprites_[GUI_OptionCursor_Triangle0]->SetIsActive(false);
    sprites_[GUI_OptionCursor_Triangle1]->SetIsActive(false);
    sprites_[GUI_OptionCursor_Triangle2]->SetIsActive(false);

    if (input->IsKeyTrigger(DIK_DOWN)) {
        optionCursor_ = (optionCursor_ + 1) % 3;
    }
    if (input->IsKeyTrigger(DIK_UP)) {
        optionCursor_ = (optionCursor_ - 1 + 3) % 3;
    }

    switch (optionCursor_)
    {
    case 0: {
        sprites_[GUI_OptionCursor_Triangle0]->SetIsActive(true);
        if (input->IsKeyTrigger(DIK_SPACE)) {

        }
        break;
    }
    case 1: {
        sprites_[GUI_OptionCursor_Triangle1]->SetIsActive(true);
        if (input->IsKeyTrigger(DIK_SPACE)) {

        }
        break;
    }
    case 2: {
        sprites_[GUI_OptionCursor_Triangle2]->SetIsActive(true);
        if (input->IsKeyTrigger(DIK_SPACE)) {

        }
        break;
    }
    default:
        break;
    }
}

void DeviceOptionsUI::SetupConnectionSettings() {
    for (auto& sprite : sprites_) {
        sprite->SetIsActive(false);
    }

    state_ = State::ConnectionSettings;
    sprites_[GUI_ConnectionSettingsTitle]->SetIsActive(true);
    sprites_[GUI_FindLightDevice]->SetIsActive(true);
    sprites_[GUI_MouseKeyboard]->SetIsActive(true);
    sprites_[GUI_Back]->SetIsActive(true);
    sprites_[GUI_ConnectionSettingsCursor_Triangle0]->SetIsActive(true);
    connectionSettingsCursor_ = 0;
}

void DeviceOptionsUI::UpdateConnectionSettings() {

}

void DeviceOptionsUI::SetupFindLightDevice() {
    for (auto& sprite : sprites_) {
        sprite->SetIsActive(false);
    }

    state_ = State::FindLightDevice;
    sprites_[GUI_FoundLightDevice]->SetIsActive(true);
    sprites_[GUI_NotFoundLightDevice]->SetIsActive(false);
}

void DeviceOptionsUI::UpdateFindLightDevice() {

}