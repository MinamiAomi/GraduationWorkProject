///
/// オーディオプレイヤー
/// 

#pragma once

#include <xaudio2.h>
#include <wrl/client.h>

#include <memory>

#include "Sound.h"

class AudioSource {
public:
    ~AudioSource();

    AudioSource& operator=(const std::shared_ptr<Sound>& sound);

    void Play(bool loop = false);
    void Stop();

    void SetSound(const std::shared_ptr<Sound>& sound) { sound_ = sound; }
    void SetVolume(float volume);
    void SetPitch(float pitch);

    bool IsPlaying() const;

private:
    void Create();
    void Destroy();

    std::shared_ptr<Sound> sound_;
    IXAudio2SourceVoice* sourceVoice_;
};