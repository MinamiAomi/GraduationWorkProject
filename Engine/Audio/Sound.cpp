#include "Sound.h"

#include <cassert>
#include <mfapi.h>
#include <mfidl.h>
#include <mfreadwrite.h>
#include <wrl.h>

template<class T>
using ComPtr = Microsoft::WRL::ComPtr<T>;

std::shared_ptr<Sound> Sound::Load(const std::filesystem::path& path) {
    // privateコンストラクタをmake_sharedで呼ぶためのヘルパー
    struct Helper : Sound {
        Helper() : Sound() {}
    };
    std::shared_ptr<Sound> sound = std::make_shared<Helper>();

    HRESULT hr = S_FALSE;

    ComPtr<IMFSourceReader> mfSourceReader;
    hr = MFCreateSourceReaderFromURL(path.wstring().c_str(), NULL, mfSourceReader.GetAddressOf());
    assert(SUCCEEDED(hr));

    ComPtr<IMFMediaType> mfMediaType;
    hr = MFCreateMediaType(mfMediaType.GetAddressOf());
    assert(SUCCEEDED(hr));
    hr = mfMediaType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Audio);
    assert(SUCCEEDED(hr));
    hr = mfMediaType->SetGUID(MF_MT_SUBTYPE, MFAudioFormat_PCM);
    assert(SUCCEEDED(hr));
    hr = mfSourceReader->SetCurrentMediaType((DWORD)MF_SOURCE_READER_FIRST_AUDIO_STREAM, nullptr, mfMediaType.Get());
    assert(SUCCEEDED(hr));

    mfMediaType = nullptr;
    hr = mfSourceReader->GetCurrentMediaType((DWORD)MF_SOURCE_READER_FIRST_AUDIO_STREAM, mfMediaType.GetAddressOf());
    assert(SUCCEEDED(hr));

    hr = MFCreateWaveFormatExFromMFMediaType(mfMediaType.Get(), &sound->waveFormat_, nullptr);
    assert(SUCCEEDED(hr));

    while (true) {
        ComPtr<IMFSample> mfSample;
        DWORD dwStreamFlags = 0;
        hr = mfSourceReader->ReadSample((DWORD)MF_SOURCE_READER_FIRST_AUDIO_STREAM, 0, nullptr, &dwStreamFlags, nullptr, mfSample.GetAddressOf());
        assert(SUCCEEDED(hr));

        if (dwStreamFlags & MF_SOURCE_READERF_ENDOFSTREAM) { break; }

        ComPtr<IMFMediaBuffer> mfMediaBuffer;
        hr = mfSample->ConvertToContiguousBuffer(mfMediaBuffer.GetAddressOf());
        assert(SUCCEEDED(hr));

        BYTE* buffer = nullptr;
        DWORD cdCurrentLength = 0;
        hr = mfMediaBuffer->Lock(&buffer, nullptr, &cdCurrentLength);
        assert(SUCCEEDED(hr));

        sound->mediaData_.resize(sound->mediaData_.size() + cdCurrentLength);
        memcpy(sound->mediaData_.data() + sound->mediaData_.size() - cdCurrentLength, buffer, cdCurrentLength);

        hr = mfMediaBuffer->Unlock();
        assert(SUCCEEDED(hr));
    }

    return sound;
}

Sound::Sound() :
    waveFormat_(nullptr) {
}

Sound::~Sound() {
    if (waveFormat_) {
        CoTaskMemFree(waveFormat_);
        waveFormat_ = nullptr;
    }
}