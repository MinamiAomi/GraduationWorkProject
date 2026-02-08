///
/// マスターオーディオ
///  

#pragma once


#include <xaudio2.h>
#include <wrl/client.h>
#include <list>

class AudioSource;
class AudioDevice {
public:
    static AudioDevice* GetInstance();

    void Initialize();
    void Finalize();

    void SetMasterVolume(float volume);
    float GetMasterVolume();

    IXAudio2* GetXAudio2() const { return xAudio2_.Get(); }

    IXAudio2SourceVoice* CreateAudioSource(AudioSource* audioSource, const WAVEFORMATEX* waveFormat);
    void RemoveAudioSource(AudioSource* audioSource);

private:
    AudioDevice() = default;
    ~AudioDevice() = default;
    AudioDevice(const AudioDevice&) = delete;
    AudioDevice& operator=(const AudioDevice&) = delete;

    Microsoft::WRL::ComPtr<IXAudio2> xAudio2_; 
    IXAudio2MasteringVoice* masterVoice_;
    std::list<AudioSource*> createdSourceList_;
};