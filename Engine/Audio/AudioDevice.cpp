#include "AudioDevice.h"

#include <cassert>
#include <mfapi.h>
#include <mfidl.h>
#include <mfreadwrite.h>

#pragma comment(lib,"xaudio2.lib")
#pragma comment(lib, "Mf.lib")
#pragma comment(lib, "mfplat.lib")
#pragma comment(lib, "Mfreadwrite.lib")
#pragma comment(lib, "mfuuid.lib")

template<class T>
using ComPtr = Microsoft::WRL::ComPtr<T>;

AudioDevice* AudioDevice::GetInstance() {
    static AudioDevice instance;
    return &instance;
}

void AudioDevice::Initialize() {
    MFStartup(MF_VERSION, MFSTARTUP_NOSOCKET);

    HRESULT hr;
    // XAudioエンジンのインスタンスを作成
    hr = XAudio2Create(xAudio2_.GetAddressOf(), 0, XAUDIO2_DEFAULT_PROCESSOR);
    assert(SUCCEEDED(hr));
#ifdef _DEBUG
    XAUDIO2_DEBUG_CONFIGURATION xAudio2DebugConfig{};
    xAudio2DebugConfig.TraceMask = XAUDIO2_LOG_ERRORS | XAUDIO2_LOG_WARNINGS;
    xAudio2DebugConfig.BreakMask = XAUDIO2_LOG_ERRORS;
    xAudio2_->SetDebugConfiguration(&xAudio2DebugConfig, 0);
#endif // _DEBUG

    // マスターボイスを作成
    hr = xAudio2_->CreateMasteringVoice(&masterVoice_);
    assert(SUCCEEDED(hr));
}

void AudioDevice::Finalize() {
    masterVoice_->DestroyVoice();
    MFShutdown();
}

void AudioDevice::SetMasterVolume(float volume) {
    masterVoice_->SetVolume(volume);
}

float AudioDevice::GetMasterVolume() {
    float volume = 0.0f;
    masterVoice_->GetVolume(&volume);
    return volume;
}
