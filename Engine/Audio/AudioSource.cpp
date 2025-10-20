#include "AudioSource.h"

#include <cassert>

#include "AudioDevice.h"

AudioSource::~AudioSource() {
    Destroy();
}

AudioSource& AudioSource::operator=(const std::shared_ptr<Sound>& sound) {
    if (sound_ != sound) {
        sound_ = sound;
        Create();
    }
    return *this;
}

void AudioSource::Play(bool loop) {
    HRESULT hr = S_FALSE;
    XAUDIO2_BUFFER buffer{};
    buffer.pAudioData = sound_->GetMediaData().data();
    buffer.AudioBytes = static_cast<UINT32>(sizeof(BYTE) * sound_->GetMediaData().size());
    buffer.Flags = XAUDIO2_END_OF_STREAM;
    if (loop) {
        buffer.LoopCount = XAUDIO2_LOOP_INFINITE;
    }
    hr = sourceVoice_->SubmitSourceBuffer(&buffer);
    assert(SUCCEEDED(hr));
    hr = sourceVoice_->Start();
    assert(SUCCEEDED(hr));
}

void AudioSource::Stop() {
    assert(sourceVoice_);
    HRESULT hr = S_FALSE;
    hr = sourceVoice_->Stop();
    assert(SUCCEEDED(hr));
}

void AudioSource::SetVolume(float volume) {
    assert(sourceVoice_);
    HRESULT hr = S_FALSE;
    hr = sourceVoice_->SetVolume(volume);
    assert(SUCCEEDED(hr));
}

void AudioSource::SetPitch(float pitch) {
    assert(sourceVoice_);
    HRESULT hr = S_FALSE;
    hr = sourceVoice_->SetFrequencyRatio(pitch);
    assert(SUCCEEDED(hr));
}

bool AudioSource::IsPlaying() const {
    if (!sound_ || !sourceVoice_) { return false; }
    XAUDIO2_VOICE_STATE state{};
    sourceVoice_->GetState(&state);
    return state.BuffersQueued > 0;
}

void AudioSource::Create() {
    assert(sound_);
    Destroy();
    HRESULT hr = S_FALSE;
    hr = AudioDevice::GetInstance()->GetXAudio2()->CreateSourceVoice(&sourceVoice_, sound_->GetWaveFormat());
    assert(SUCCEEDED(hr));

}

void AudioSource::Destroy() {
    if (sourceVoice_) {
        sourceVoice_->DestroyVoice();
        sourceVoice_ = nullptr;
    }
}
