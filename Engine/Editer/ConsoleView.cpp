#include "ConsoleView.h"

#ifdef ENABLE_IMGUI
#include "Externals/ImGui/imgui.h"
#include "Externals/ImGui/imgui_stdlib.h"
#include "Externals/ImGui/imgui_internal.h"
#endif // ENABLE_IMGUI

namespace Editer {

    static const ImVec4 kTextColors[LogType::NumLogTypes] = {
        { 1.0f, 1.0f, 1.0f, 1.0f },
        { 1.0f, 1.0f, 0.0f, 1.0f },
        { 1.0f, 0.0f, 0.0f, 1.0f },
        { 0.0f, 1.0f, 0.0f, 1.0f },
    };

    static const char* kLogTypeString[LogType::NumLogTypes] = {
        "Normal",
        "Warning",
        "Error",
        "Information"
    };

    void ConsoleView::Render() {

#ifdef ENABLE_IMGUI
        if (!isDisplayed) { return; }

        ImGui::Begin("Console", &isDisplayed, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_MenuBar);

        if (ImGui::BeginMenuBar()) {
            if (ImGui::BeginMenu("Edit")) {
                for (int i = 0; i < LogType::NumLogTypes; ++i) {
                    ImGui::PushStyleColor(ImGuiCol_Text, kTextColors[i]);
                    if (ImGui::MenuItem(kLogTypeString[i], "", showLogTypes_[i])) {
                        showLogTypes_[i] ^= true;
                    }
                    ImGui::PopStyleColor();
                }
                ImGui::EndMenu();
            }
            if (ImGui::MenuItem("Clear")) {
                ClearLog();
            }
            ImGui::EndMenuBar();
        }

        ImGui::BeginChild("scrolling_region", { 0.0f, -ImGui::GetFrameHeightWithSpacing() }, false, ImGuiWindowFlags_HorizontalScrollbar);

        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, { 0.0f, 1.0f });
        for (const auto& log : logs_) {
            if (!showLogTypes_[log.first.type]) { continue; }
            ImGui::PushStyleColor(ImGuiCol_Text, kTextColors[log.first.type]);
            ImGui::TextUnformatted(log.second.c_str());
            ImGui::PopStyleColor();
        }
        ImGui::PopStyleVar();

        if (requestedScrollingToBottom_) {
            ImGui::SetScrollHereY(1.0f);
        }
        requestedScrollingToBottom_ = false;
        ImGui::EndChild();

        // 入力エリア
        ImGui::PushItemWidth(ImGui::GetCurrentWindow()->Rect().GetWidth() - ImGui::CalcTextSize("Input").x - 20);
        if (ImGui::InputText("Input", &inputText_, ImGuiInputTextFlags_EnterReturnsTrue) && !inputText_.empty()) {
            AddLog(LogType::Normal, inputText_);
            inputText_.clear();
        }
        ImGui::PopItemWidth();

        ImGui::End();
#endif // ENABLE_IMGUI
    }

    void ConsoleView::AddLog(LogType logType, const std::string& log) {
        logs_.emplace_back(std::make_pair(logType, log));
        if (logs_.size() > kMaxLogs) {
            logs_.erase(logs_.begin());
        }
        requestedScrollingToBottom_ = true;
    }

    void ConsoleView::AddLog(LogType logType, const char* fmt, ...) {
        va_list args;
        va_start(args, fmt);
        int len = std::vsnprintf(nullptr, 0, fmt, args) + 1;
        std::vector<char> buffer(len);
        std::vsnprintf(buffer.data(), len, fmt, args);
        AddLog(logType, std::string(buffer.data(), buffer.size() - 1));
        va_end(args);
    }

    void ConsoleView::ClearLog() {
        logs_.clear();
        requestedScrollingToBottom_ = true;
    }
}