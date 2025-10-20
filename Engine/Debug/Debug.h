///
/// デバッグに有用な関数たち
/// 

#pragma once

#include <chrono>
#include <functional>
#include <string>

namespace Debug {
    /// <summary>
    /// 関数の時間を計ります
    /// </summary>
    /// <typeparam name="duration">戻り値の時間の単位</typeparam>
    /// <param name="func">関数ポインタ</param>
    /// <returns>経過時間</returns>
    template<typename duration = std::chrono::milliseconds>
    duration ElapsedTime(std::function<void()> func) {
        std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
        func();
        return std::chrono::duration_cast<duration>(std::chrono::steady_clock::now() - start);
    }

    /// <summary>
    /// ログ
    /// </summary>
    /// <param name="fmt"></param>
    /// <param name=""></param>
    void Log(const char* fmt, ...);

    /// <summary>
    /// ログ
    /// </summary>
    /// <param name="str"></param>
    void Log(const std::string& str);

    /// <summary>
    /// ログ
    /// </summary>
    /// <param name="str"></param>
    void Log(const std::wstring& str);

    /// <summary>
    /// メッセージボックスを出す
    /// </summary>
    /// <param name="text">テキスト</param>
    /// <param name="caption">キャプション</param>
    void MsgBox(const std::string& text, const std::string& caption = "Debug");

    /// <summary>
    /// メッセージボックスを出す
    /// </summary>
    /// <param name="text">テキスト</param>
    /// <param name="caption">キャプション</param>
    void MsgBox(const std::wstring& text, const std::wstring& caption = L"Debug");
}