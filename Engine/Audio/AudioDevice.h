///
/// マスターオーディオ
///  

#pragma once


#include <xaudio2.h>
#include <wrl/client.h>

class AudioDevice {
public:
    static AudioDevice* GetInstance();

    void Initialize();
    void Finalize();

    void SetMasterVolume(float volume);
    float GetMasterVolume();

    IXAudio2* GetXAudio2() const { return xAudio2_.Get(); }

private:
    AudioDevice() = default;
    ~AudioDevice() = default;
    AudioDevice(const AudioDevice&) = delete;
    AudioDevice& operator=(const AudioDevice&) = delete;

    Microsoft::WRL::ComPtr<IXAudio2> xAudio2_; 
    IXAudio2MasteringVoice* masterVoice_;
};