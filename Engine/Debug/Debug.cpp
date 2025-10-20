#include "Debug.h"

#include <Windows.h>

#include "Framework/Engine.h"
#ifdef ENABLE_IMGUI
#include "Editer/EditerManager.h"
#endif // ENABLE_IMGUI

namespace {
    std::string ConvertString(const std::wstring& str) {
        if (str.empty()) {
            return std::string();
        }
        int sizeNeeded = WideCharToMultiByte(CP_UTF8, 0, str.data(), static_cast<int>(str.size()), NULL, 0, NULL, NULL);
        if (sizeNeeded == 0) {
            return std::string();
        }
        std::string result(sizeNeeded, 0);
        WideCharToMultiByte(CP_UTF8, 0, str.data(), static_cast<int>(str.size()), result.data(), sizeNeeded, NULL, NULL);
        return result;
    }
}


namespace Debug {
    void Log(const char* fmt, ...) {
        va_list args;
        va_start(args, fmt);
        int len = std::vsnprintf(nullptr, 0, fmt, args) + 1;
        std::vector<char> buffer(len);
        std::vsnprintf(buffer.data(), len, fmt, args);
        Log(std::string(buffer.data(), buffer.size() - 1));
        va_end(args);
    }
    void Log(const std::string& str) {
        OutputDebugStringA(str.c_str());
#ifdef ENABLE_IMGUI
        Engine::GetEditerManager()->GetConsoleView().AddLog(Editer::LogType::Normal, str);
#endif // ENABLE_IMGUI

    }
    void Log(const std::wstring& str) {
        OutputDebugStringW(str.c_str());
#ifdef ENABLE_IMGUI
        Engine::GetEditerManager()->GetConsoleView().AddLog(Editer::LogType::Normal, ConvertString(str));
#endif // ENABLE_IMGUI
    }
    void MsgBox(const std::string& text, const std::string& caption) {
        MessageBoxA(nullptr, text.c_str(), caption.c_str(), S_OK);
    }
    void MsgBox(const std::wstring& text, const std::wstring& caption) {
        MessageBoxW(nullptr, text.c_str(), caption.c_str(), S_OK);
    }
}