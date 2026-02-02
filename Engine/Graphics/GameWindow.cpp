#include "GameWindow.h"

#ifdef ENABLE_IMGUI
#include "Externals/ImGui/imgui.h"
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
#endif // ENABLE_IMGUI

#include "Core/Helper.h"


namespace {
    // ウィンドウプロシージャ
    LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
#ifdef ENABLE_IMGUI
        if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wparam, lparam)) { return true; }
#endif // ENABLE_IMGUI

        // メッセージに対してゲーム固有の処理を行う
        switch (msg) {
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
        }
        return DefWindowProc(hwnd, msg, wparam, lparam);
    }
}

GameWindow* GameWindow::GetInstance() {
    static GameWindow instance;
    return &instance;
}

void GameWindow::Initialize(const wchar_t* title, uint32_t clientWidth, uint32_t clientHeight) {
    ASSERT_IF_FAILED(CoInitializeEx(0, COINIT_MULTITHREADED));

    windowStyle_ = WS_OVERLAPPEDWINDOW;

    // ウィンドウクラスを生成
    WNDCLASS wc{};
    wc.lpfnWndProc = WindowProc;	// ウィンドウプロシージャ
    wc.lpszClassName = L"LE3A_20_ミナミアオミ";	// ウィンドウクラス名
    wc.hInstance = GetModuleHandle(nullptr);	// インスタンスハンドル
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);	// カーソル
    RegisterClass(&wc);	// ウィンドウクラスを登録

    // ウィンドウサイズを表す構造体にクライアント領域を入れる
    RECT wrc{ 0,0,static_cast<LONG>(clientWidth),static_cast<LONG>(clientHeight) };
    // クライアント領域を元に実際のサイズにwrcを変更してもらう
    AdjustWindowRect(&wrc, windowStyle_, false);

    // ウィンドウの生成
    hWnd_ = CreateWindow(
        wc.lpszClassName,		// 利用するクラス名
        title,				// タイトルバーの文字
        windowStyle_,	// よく見るウィンドウスタイル
        CW_USEDEFAULT,			// 表示X座標（WindowsOSに任せる）
        CW_USEDEFAULT,			// 表示Y座標（WindowsOSに任せる）
        wrc.right - wrc.left,	// ウィンドウ横幅
        wrc.bottom - wrc.top,	// ウィンドウ縦幅
        nullptr,				// 親ウィンドウハンドル
        nullptr,				// メニューハンドル
        wc.hInstance,			// インスタンスハンドル
        nullptr);				// オプション
    clientWidth_ = clientWidth;
    clientHeight_ = clientHeight;

    // サイズ変更不可
    LONG style = GetWindowLong(hWnd_, GWL_STYLE);
    style &= ~WS_THICKFRAME;
    style |= WS_MINIMIZEBOX | WS_MAXIMIZEBOX;
    SetWindowLong(hWnd_, GWL_STYLE, style);

    SetWindowLongPtr(hWnd_, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));

    ShowWindow(hWnd_, SW_SHOW);
}

bool GameWindow::ProcessMessage() const {
    MSG msg{};

    while (msg.message != WM_QUIT) {
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        else {
            break;
        }
    }
    return msg.message != WM_QUIT;
}

void GameWindow::Shutdown() {
    CloseWindow(hWnd_);
    hWnd_ = nullptr;
    CoUninitialize();
}

void GameWindow::SetWindowMode(WindowMode windowMode) {
    if (windowMode_ == windowMode) {
        return;
    }

    if (windowMode == WindowMode::Fullscreen || windowMode == WindowMode::Borderless) {
        if (windowMode_ == WindowMode::Window) {
            GetWindowRect(hWnd_, &windowRect_);
            windowStyle_ = GetWindowLong(hWnd_, GWL_STYLE);
        }

        SetWindowLong(hWnd_, GWL_STYLE, WS_POPUP | WS_VISIBLE);

        HMONITOR monitor = MonitorFromWindow(hWnd_, MONITOR_DEFAULTTONEAREST);
        MONITORINFO info{};
        info.cbSize = sizeof(info);
        GetMonitorInfo(monitor, &info);

        int w = info.rcMonitor.right - info.rcMonitor.left;
        int h = info.rcMonitor.bottom - info.rcMonitor.top;

        SetWindowPos(hWnd_, HWND_TOP, info.rcMonitor.left, info.rcMonitor.top, w, h, SWP_FRAMECHANGED | SWP_NOACTIVATE);

        clientWidth_ = w;
        clientHeight_ = h;
    }
    else if (windowMode == WindowMode::Window) {
        SetWindowLong(hWnd_, GWL_STYLE, windowStyle_);

        SetWindowPos(hWnd_, HWND_NOTOPMOST,
            windowRect_.left, windowRect_.top,
            windowRect_.right - windowRect_.left,
            windowRect_.bottom - windowRect_.top,
            SWP_FRAMECHANGED | SWP_NOACTIVATE);

        RECT clientRect{};
        GetClientRect(hWnd_, &clientRect);
        clientWidth_ = static_cast<uint32_t>(clientRect.right - clientRect.left);
        clientHeight_ = static_cast<uint32_t>(clientRect.bottom - clientRect.top);
    }

    windowMode_ = windowMode;

}

void GameWindow::SetWindowSize(uint32_t clientWidth, uint32_t clientHeight) {
    clientWidth_ = clientWidth;
    clientHeight_ = clientHeight;

    if (windowMode_ != WindowMode::Window) {
        windowRect_.right = windowRect_.left = static_cast<LONG>(clientWidth);
        windowRect_.bottom = windowRect_.top = static_cast<LONG>(clientHeight);
        return;
    }

    RECT wrc = { 0, 0, static_cast<LONG>(clientWidth), static_cast<LONG>(clientHeight) };
    AdjustWindowRect(&wrc, windowStyle_, FALSE);

    SetWindowPos(hWnd_, nullptr, 0, 0,
        wrc.right - wrc.left, wrc.bottom - wrc.top,
        SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED);
}
