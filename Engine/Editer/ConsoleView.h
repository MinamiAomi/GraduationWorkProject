///
/// コンソールビュー
/// 

#pragma once

#include "EditerView.h"

#include <string>
#include <vector>

namespace Editer {

    struct LogType {
        enum Enum {
            Normal,
            Warning,
            Error,
            Information,

            NumLogTypes
        };
        LogType(Enum t) : type(t) {}
        Enum type;
    };

    class ConsoleView :
        public View {
    public:
        using View::View;

        static const size_t kMaxLogs = 1024;

        void Render();
        void AddLog(LogType type, const std::string& log);
        void AddLog(LogType type, const char* fmt, ...);
        void ClearLog();

    private:

        std::vector<std::pair<LogType, std::string>> logs_;
        std::string inputText_;
        bool showLogTypes_[LogType::NumLogTypes] = { true, true, true, true };
        bool requestedScrollingToBottom_;

    };

}