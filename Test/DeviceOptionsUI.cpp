#include "DeviceOptionsUI.h"

#include <string>
#include "Framework/AssetManager.h"
#include "Input/Input.h"
#include "Scene/SceneManager.h"
#include "GameSystem.h"
#include "StageSelectScene.h"
#include "LightDeviceInput.h"


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
    { "DCUI_Triangle", { 531, 1000 }, { 64, 64 } },
    { "DCUI_ConnectionSettings", { 640, 200 }, { 569, 63 } },
    { "DCUI_FindLightDevice", { 640, 800 }, { 488, 62 } },
    { "DCUI_MouseKeyboard", { 640, 900 }, { 537, 63 } },
    { "DCUI_Back", { 640, 1000 }, { 140, 48 } },
    { "DCUI_Triangle", { 346, 800 }, { 64, 64 } },
    { "DCUI_Triangle", { 321, 900 }, { 64, 64 } },
    { "DCUI_Triangle", { 520, 1000 }, { 64, 64 } },
    { "DCUI_FoundLightDevice", { 640, 800 }, { 545, 62 } },
    { "DCUI_NotFoundLightDevice", { 640, 800 }, { 664, 62 } },
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
        sprites_[i] = std::make_unique<Sprite>();
        auto sprite = sprites_[i].get();
        sprite->SetTexture(assetManager->textureMap.Get(guiContact[i].texture)->Get());
        sprite->SetIsActive(false);
        sprite->SetPosition({ guiContact[i].position.x, 720.0f - guiContact[i].position.y * (720.0f / 1080.0f) });
        sprite->SetScale(guiContact[i].size);
        sprite->SetUVRect({ {} , Vector2::one }, Sprite::UVMode::UV);
        sprite->SetColor(guiContact[i].color);
    }
    needsStateInitialization_ = true;
    state_ = State::MainMenu;
}

void DeviceOptionsUI::Update() {

    // 状態初期化
    if (needsStateInitialization_) {
        switch (state_)
        {
        case State::MainMenu:
            SetupMainMenu();
            break;
        case State::ConnectionSettings:
            SetupConnectionSettings();
            break;
        case State::FindLightDevice:
            SetupFindLightDevice();
            break;
        default:
            break;
        }
        needsStateInitialization_ = false;
    }

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

    sprites_[GUI_GameStart]->SetIsActive(true);
    sprites_[GUI_ConnectionSettings]->SetIsActive(true);
    sprites_[GUI_Quit]->SetIsActive(true);
    optionCursor_ = 0;
    selectionTimer_ = kSelectionDelay;
}

void DeviceOptionsUI::UpdateMainMenu() {
    Input* input = Input::GetInstance();

    sprites_[GUI_OptionCursor_Triangle0]->SetIsActive(false);
    sprites_[GUI_OptionCursor_Triangle1]->SetIsActive(false);
    sprites_[GUI_OptionCursor_Triangle2]->SetIsActive(false);

    if (selectionTimer_ <= 0) {
        if (input->IsKeyPressed(DIK_DOWN)) {
            optionCursor_ = (optionCursor_ + 1) % 3;
            selectionTimer_ = kSelectionDelay;
        }
        if (input->IsKeyPressed(DIK_UP)) {
            optionCursor_ = (optionCursor_ - 1 + 3) % 3;
            selectionTimer_ = kSelectionDelay;
        }
    }
    else {
        selectionTimer_--;
    }

    switch (optionCursor_)
    {
    case 0: {
        sprites_[GUI_OptionCursor_Triangle0]->SetIsActive(true);
        if (input->IsKeyTrigger(DIK_SPACE)) {
            // ゲームスタート
            SceneManager::GetInstance()->ChangeScene<StageSelectScene>(true);
        }
        break;
    }
    case 1: {
        sprites_[GUI_OptionCursor_Triangle1]->SetIsActive(true);
        if (input->IsKeyTrigger(DIK_SPACE)) {
            // セッティングに移行
            state_ = State::ConnectionSettings;
            needsStateInitialization_ = true;
        }
        break;
    }
    case 2: {
        sprites_[GUI_OptionCursor_Triangle2]->SetIsActive(true);
        if (input->IsKeyTrigger(DIK_SPACE)) {
            // 終了
            GameSystem::GetInstance()->Quit();
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

    sprites_[GUI_ConnectionSettingsTitle]->SetIsActive(true);
    sprites_[GUI_FindLightDevice]->SetIsActive(true);
    sprites_[GUI_MouseKeyboard]->SetIsActive(true);
    sprites_[GUI_Back]->SetIsActive(true);
    sprites_[GUI_ConnectionSettingsCursor_Triangle0]->SetIsActive(true);
    optionCursor_ = 0;
    selectionTimer_ = kSelectionDelay;
}

void DeviceOptionsUI::UpdateConnectionSettings() {
    Input* input = Input::GetInstance();

    sprites_[GUI_ConnectionSettingsCursor_Triangle0]->SetIsActive(false);
    sprites_[GUI_ConnectionSettingsCursor_Triangle1]->SetIsActive(false);
    sprites_[GUI_ConnectionSettingsCursor_Triangle2]->SetIsActive(false);

    if (selectionTimer_ <= 0) {
        if (input->IsKeyPressed(DIK_DOWN)) {
            optionCursor_ = (optionCursor_ + 1) % 3;
            selectionTimer_ = kSelectionDelay;
        }
        if (input->IsKeyPressed(DIK_UP)) {
            optionCursor_ = (optionCursor_ - 1 + 3) % 3;
            selectionTimer_ = kSelectionDelay;
        }
    }
    else {
        selectionTimer_--;
    }

    switch (optionCursor_)
    {
    case 0: {
        sprites_[GUI_ConnectionSettingsCursor_Triangle0]->SetIsActive(true);
        if (input->IsKeyTrigger(DIK_SPACE)) {
            // FindLightDeviceへ移行
            state_ = State::FindLightDevice;
            needsStateInitialization_ = true;
        }
        break;
    }
    case 1: {
        sprites_[GUI_ConnectionSettingsCursor_Triangle1]->SetIsActive(true);
        if (input->IsKeyTrigger(DIK_SPACE)) {
            // 特に処理がないので戻る
            state_ = State::MainMenu;
            needsStateInitialization_ = true;
            GameSystem::GetInstance()->SetPlayDevice(GameSystem::PlayDevice::KeyboardMouse);
        }
        break;
    }
    case 2: {
        sprites_[GUI_ConnectionSettingsCursor_Triangle2]->SetIsActive(true);
        if (input->IsKeyTrigger(DIK_SPACE)) {
            // 戻る
            state_ = State::MainMenu;
            needsStateInitialization_ = true;
        }
        break;
    }
    default:
        break;
    }
}

void DeviceOptionsUI::SetupFindLightDevice() {
    for (auto& sprite : sprites_) {
        sprite->SetIsActive(false);
    }

    sprites_[GUI_FindLightDevice]->SetIsActive(true);
    sprites_[GUI_FoundLightDevice]->SetIsActive(false);
    sprites_[GUI_NotFoundLightDevice]->SetIsActive(false);
    optionCursor_ = 0;
    selectionTimer_ = kWaitTime;

    LightDeviceInput::GetInstance()->Initialize();
}

void DeviceOptionsUI::UpdateFindLightDevice() {
    auto lightDeviceInput = LightDeviceInput::GetInstance();

    switch (lightDeviceInput->GetConnectionState())
    {
    case LightDeviceInput::ConnectionState::Disconnected: {
        // 接続失敗

        for (int i = GUI_Circle0; i <= GUI_Circle5; ++i) {
            sprites_[i]->SetIsActive(false);
        }

        sprites_[GUI_FindLightDevice]->SetIsActive(false);
        sprites_[GUI_NotFoundLightDevice]->SetIsActive(true);

        if (selectionTimer_ <= 0) {
            state_ = State::ConnectionSettings;
            needsStateInitialization_ = true;
        }
        else {
            selectionTimer_--;
        }

        break;
    }
    case LightDeviceInput::ConnectionState::Connecting: {
        // 接続中
        animationTimer_ = (animationTimer_ + 1) % kAnimationCircle;

        float angle = Math::TwoPi / 6.0f;
        int32_t animationShift = animationTimer_ / (kAnimationCircle / 6);
        for (int i = GUI_Circle0; i <= GUI_Circle5; ++i) {
            sprites_[i]->SetIsActive(true);
            int32_t offset = (i - GUI_Circle0 + animationShift) % 6;
            sprites_[i]->SetPosition({ 640.0f + 100.0f * std::cos(angle * offset),
                                 360.0f + 100.0f * std::sin(angle * offset) });
        }
        break;
    }
    case LightDeviceInput::ConnectionState::Connected: {
        // 接続成功

        for (int i = GUI_Circle0; i <= GUI_Circle5; ++i) {
            sprites_[i]->SetIsActive(false);
        }

        sprites_[GUI_FindLightDevice]->SetIsActive(false);
        sprites_[GUI_FoundLightDevice]->SetIsActive(true);

        if (selectionTimer_ <= 0) {
            state_ = State::ConnectionSettings;
            needsStateInitialization_ = true;
            GameSystem::GetInstance()->SetPlayDevice(GameSystem::PlayDevice::LightDevice);
        }
        else {
            selectionTimer_--;
        }

        break;
    }
    }


}