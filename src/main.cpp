#include "include.h"

int APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
{
    const HANDLE hMutex = CreateMutexW(nullptr, FALSE, L"Global\\{dev.hunlongyu.screen}");
    if (GetLastError() == ERROR_ALREADY_EXISTS) {
        return 1; // 退出程序
    }

    CheckConfigFile(); // 读取配置文件
    MyRegisterClass(hInstance);

    // 执行应用程序初始化:
    if (!InitInstance(hInstance)) {
        return FALSE;
    }

    const HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_SCREEN));
    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0)) {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }
    if (hMutex) {
        CloseHandle(hMutex);
    }
    return static_cast<int>(msg.wParam);
}

ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;
    wcex.cbSize        = sizeof(WNDCLASSEX);
    wcex.style         = 0;
    wcex.lpfnWndProc   = WndProc;
    wcex.cbClsExtra    = 0;
    wcex.cbWndExtra    = 0;
    wcex.hInstance     = hInstance;
    wcex.hIcon         = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_SCREEN));
    wcex.hCursor       = nullptr;
    wcex.hbrBackground = nullptr;
    wcex.lpszMenuName  = MAKEINTRESOURCEW(IDC_SCREEN);
    wcex.lpszClassName = szWindowClass.c_str();
    wcex.hIconSm       = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SCREEN));
    return RegisterClassExW(&wcex);
}

void AddTrayIcon(HINSTANCE hInstance, HWND hWnd, UINT uCallbackMessage)
{
    // 创建弹出菜单
    hPopupMenu = CreatePopupMenu();

    // 锁屏
    if (isLock) {
        AppendMenu(hPopupMenu, MF_STRING | MF_CHECKED, IDM_LOCK, TEXT("锁屏"));
    }
    else {
        AppendMenu(hPopupMenu, MF_STRING, IDM_LOCK, TEXT("锁屏"));
    }

    // 快捷键
    if (isShortcut) {
        AppendMenu(hPopupMenu, MF_STRING | MF_CHECKED, IDM_SHORTCUT, TEXT("快捷键"));
    }
    else {
        AppendMenu(hPopupMenu, MF_STRING, IDM_SHORTCUT, TEXT("快捷键"));
    }

    // 开机自启
    if (isAutoStart) {
        AppendMenu(hPopupMenu, MF_STRING | MF_CHECKED, IDM_AUTO_START, TEXT("开机自启"));
    }
    else {
        AppendMenu(hPopupMenu, MF_STRING, IDM_AUTO_START, TEXT("开机自启"));
    }

    AppendMenu(hPopupMenu, MF_SEPARATOR, 0, nullptr);
    AppendMenu(hPopupMenu, MF_STRING, IDM_EXIT, TEXT("退出"));

    nid.cbSize             = sizeof(NOTIFYICONDATA);
    nid.hWnd               = hWnd;
    nid.uID                = TRUE;
    nid.uFlags             = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    nid.hIcon              = LoadIcon(hInstance, "IDI_SCREEN");
    const std::string tips = "息屏助手 " + version + "\n" + "快捷键 ALT + L";
    strcpy_s(nid.szTip, tips.c_str());
    nid.uCallbackMessage = uCallbackMessage;
    Shell_NotifyIcon(NIM_ADD, &nid);
}

BOOL InitInstance(HINSTANCE hInstance)
{
    hInst           = hInstance;
    const HWND hWnd = CreateWindowExW(WS_EX_LAYERED, szWindowClass.c_str(), szWindowClass.c_str(), WS_POPUP, CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInst, nullptr);
    if (!hWnd) {
        return FALSE;
    }
    if (isShortcut) {
        const auto flag = RegisterHotKey(hWnd, HOTKEY_ID, MOD_ALT, 0x4C);
        if (flag != 0) {
            printf("success");
        }
    }
    UpdateWindow(hWnd);
    ShowWindow(hWnd, SW_HIDE);
    constexpr UINT uCallbackMessage = WM_MYTRAYMESSAGE;
    AddTrayIcon(hInst, hWnd, uCallbackMessage);

    return TRUE;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message) {
        case WM_HOTKEY: {
            if (wParam == HOTKEY_ID) {
                RestingScreenFn();
            }
        }
        case WM_MYTRAYMESSAGE: {
            if (lParam == WM_LBUTTONDBLCLK) {
                MessageBoxW(nullptr, L"双击", L"提示", MB_OK | MB_ICONERROR);
                break;
            }
            else if (lParam == WM_RBUTTONUP) {
                POINT pt;
                GetCursorPos(&pt);
                SetForegroundWindow(hWnd);
                // 根据窗口的可见性修改菜单项的文本
                if (isLock) {
                    ModifyMenu(hPopupMenu, IDM_LOCK, MF_STRING | MF_CHECKED, IDM_LOCK, TEXT("锁屏"));
                }
                else {
                    ModifyMenu(hPopupMenu, IDM_LOCK, MF_STRING, IDM_LOCK, TEXT("锁屏"));
                }

                if (isShortcut) {
                    ModifyMenu(hPopupMenu, IDM_SHORTCUT, MF_STRING | MF_CHECKED, IDM_SHORTCUT, TEXT("快捷键"));
                }
                else {
                    ModifyMenu(hPopupMenu, IDM_SHORTCUT, MF_STRING, IDM_SHORTCUT, TEXT("快捷键"));
                }

                if (isAutoStart) {
                    ModifyMenu(hPopupMenu, IDM_AUTO_START, MF_STRING | MF_CHECKED, IDM_AUTO_START, TEXT("开机自启"));
                }
                else {
                    ModifyMenu(hPopupMenu, IDM_AUTO_START, MF_STRING, IDM_AUTO_START, TEXT("开机自启"));
                }
                TrackPopupMenu(hPopupMenu, TPM_RIGHTBUTTON, pt.x, pt.y, 0, hWnd, nullptr);
                PostMessage(hWnd, WM_NULL, 0, 0);
            }
            break;
        }
        case WM_COMMAND: {
            switch (LOWORD(wParam)) {
                case IDM_LOCK: {
                    const auto flag = !isLock;
                    ModifyConfigLock(flag);
                    break;
                }
                case IDM_AUTO_START: {
                    const auto flag = !isAutoStart;
                    ToggleAutoStart(flag);
                    break;
                }
                case IDM_SHORTCUT: {
                    const auto flag = !isShortcut;
                    ModifyConfigShortcut(flag);
                    break;
                }
                case IDM_EXIT: {
                    DestroyWindow(hWnd); // 退出程序
                    break;
                }
                default: {
                    break;
                }
            }
            break;
        }
        case WM_DESTROY: {
            Shell_NotifyIcon(NIM_DELETE, &nid);
            PostQuitMessage(0);
            break;
        }
        default: return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

void CheckConfigFile()
{
    char* appdata = nullptr;
    size_t len    = 0;
    if (_dupenv_s(&appdata, &len, "APPDATA") == 0 && appdata != nullptr) {
        std::filesystem::path appdata_path(appdata);
        appdata_path /= "screen/config.ini";
        configPath = exists(appdata_path) ? appdata_path.string() : "config.ini";
    }
    free(appdata);

    m_ini.setMultiLineValues(true);
    m_ini.load(configPath);
    version     = m_ini["Application"]["version"].as<std::string>();
    isShortcut  = m_ini["Application"]["shortcut"].as<bool>();
    isLock      = m_ini["Application"]["lock"].as<bool>();
    isAutoStart = m_ini["Application"]["autostart"].as<bool>();

    ToggleAutoStart(isAutoStart);
}

void RestingScreenFn()
{
    if (isLock) {
        LockWorkStation();
    }
    PostMessage(HWND_BROADCAST, WM_SYSCOMMAND, SC_MONITORPOWER, (LPARAM)2);
}

void ToggleAutoStart(bool enable)
{
    HKEY hKey;
    const LPCWSTR szPath = L"Software\\Microsoft\\Windows\\CurrentVersion\\Run";

    // 获取程序的完整路径
    WCHAR szPathToExe[MAX_PATH];
    if (GetModuleFileNameW(nullptr, szPathToExe, MAX_PATH) == 0) {
        MessageBoxW(nullptr, L"无法获取程序路径，开机自启功能异常", L"错误", MB_OK | MB_ICONERROR);
        return;
    }

    // 获取程序的名称
    const LPCWSTR szValueName = PathFindFileNameW(szPathToExe);

    // 打开注册表键
    if (RegOpenKeyExW(HKEY_CURRENT_USER, szPath, 0, KEY_ALL_ACCESS, &hKey) == ERROR_SUCCESS) {
        if (enable) {
            ModifyConfigAutoStart(true);
            // 向注册表写入值以开启自动启动
            if (RegSetValueExW(hKey, szValueName, 0, REG_SZ, (BYTE*)szPathToExe, (DWORD)(wcslen(szPathToExe) * sizeof(wchar_t))) != ERROR_SUCCESS) {
                MessageBoxW(nullptr, L"无法设置开机自启动，开机自启功能异常", L"错误", MB_OK | MB_ICONERROR);
            }
            else {
                CheckMenuItem(hPopupMenu, IDM_AUTO_START, MF_BYCOMMAND | MF_CHECKED);
            }
        }
        else {
            ModifyConfigAutoStart(false);
            // 从注册表中删除值以取消自动启动
            const LONG lRet = RegDeleteValueW(hKey, szValueName);
            if (lRet != ERROR_SUCCESS && lRet != ERROR_FILE_NOT_FOUND) {
                MessageBoxW(nullptr, L"无法取消开机自启动，开机自启功能异常", L"错误", MB_OK | MB_ICONERROR);
            }
        }

        // 关闭注册表键
        RegCloseKey(hKey);
    }
    else {
        MessageBoxW(nullptr, L"无法打开注册表键，开机自启功能异常", L"错误", MB_OK | MB_ICONERROR);
    }
}

void ModifyConfigAutoStart(bool flag)
{
    isAutoStart                       = flag;
    m_ini["Application"]["autostart"] = flag;
    m_ini.save(configPath);
}

void ModifyConfigLock(bool flag)
{
    isLock                       = flag;
    m_ini["Application"]["lock"] = flag;
    m_ini.save(configPath);
}

void ModifyConfigShortcut(bool flag)
{
    isShortcut                       = flag;
    m_ini["Application"]["shortcut"] = flag;
    m_ini.save(configPath);
}

std::wstring stringToWString(const std::string& s)
{
    const int size = MultiByteToWideChar(CP_UTF8, 0, &s[0], static_cast<int>(s.size()), nullptr, 0);
    std::wstring result(size, 0);
    MultiByteToWideChar(CP_UTF8, 0, &s[0], static_cast<int>(s.size()), &result[0], size);
    return result;
}
