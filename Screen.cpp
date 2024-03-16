// Screen.cpp : 定义应用程序的入口点。

#include "Screen.h"

#include <cstdio>
#include <string>
#include <filesystem>
#include "framework.h"
#include <shlwapi.h>
#include "ini/inicpp.h"

#pragma comment(lib, "shlwapi.lib")

#define MAX_LOADSTRING   100
#define HOTKEY_ID        1010 // 全局快捷键 ID
#define WM_MYTRAYMESSAGE (WM_USER + 1)
#define IDM_AUTO_START   1000 // 开机自启
#define IDM_LOCK         1001 // 息屏锁屏
#define IDM_SHORTCUT     1002 // 快捷键
#define IDM_EXIT         1003 // 软件退出

// 全局变量:
HINSTANCE hInst;                     // 当前实例
WCHAR szTitle[MAX_LOADSTRING];       // 标题栏文本
WCHAR szWindowClass[MAX_LOADSTRING]; // 主窗口类名
HMENU hPopupMenu;                    // 托盘右键弹出菜单
NOTIFYICONDATA nid = {0};            // 托盘图标句柄

std::string version{"1.0.0"}; // 软件版本号
bool isShortcut{true};        // 快捷键是否生效
bool isLock{false};           // 息屏时，是否先锁屏
bool isAutoStart{true};       // 开机自启
std::string configPath{""};   // 配置文件的路径
ini::IniFile m_ini;           // INI 配置

// 此代码模块中包含的函数的前向声明:
ATOM MyRegisterClass(HINSTANCE hInstance);
BOOL InitInstance(HINSTANCE, int);
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

void CheckConfigFile();           // 检查配置文件是否存在, 并生效
void RestingScreenFn();           // 息屏函数
void ToggleAutoStart(bool);       // 切换开机自启功能
void ModifyConfigAutoStart(bool); // 修改开机自启配置项
void ModifyConfigLock(bool);      // 修改锁屏配置项
void ModifyConfigShortcut(bool);  // 修改快捷键配置项

std::wstring stringToWString(const std::string& s); // 字符串转

int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
    const HANDLE hMutex = CreateMutex(NULL, FALSE, L"Global\\{dev.hunlongyu.screen}");
    if (GetLastError() == ERROR_ALREADY_EXISTS) {
        return 1; // 退出程序
    }

    CheckConfigFile(); // 读取配置文件

    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_SCREEN, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // 执行应用程序初始化:
    if (!InitInstance(hInstance, nCmdShow)) {
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
    return msg.wParam;
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
    wcex.hCursor       = NULL;
    wcex.hbrBackground = NULL;
    wcex.lpszMenuName  = MAKEINTRESOURCEW(IDC_SCREEN);
    wcex.lpszClassName = szWindowClass;
    wcex.hIconSm       = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));
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

    AppendMenu(hPopupMenu, MF_STRING, IDM_EXIT, TEXT("退出"));

    nid.cbSize             = sizeof(NOTIFYICONDATA);
    nid.hWnd               = hWnd;
    nid.uID                = TRUE;
    nid.uFlags             = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    nid.hIcon              = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_SCREEN));
    const std::wstring ver = L"息屏助手" + stringToWString(version) + L"\n" + L"快捷键 ALT + L";
    wcsncpy_s(nid.szTip, ver.c_str(), sizeof(nid.szTip) / sizeof(WCHAR));
    nid.uCallbackMessage = uCallbackMessage;
    Shell_NotifyIcon(NIM_ADD, &nid);
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
    hInst           = hInstance;
    const HWND hWnd = CreateWindowExW(WS_EX_LAYERED, szWindowClass, szTitle, WS_POPUP, CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInst, nullptr);
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
                MessageBoxW(NULL, L"双击", L"提示", MB_OK | MB_ICONERROR);
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
                TrackPopupMenu(hPopupMenu, TPM_RIGHTBUTTON, pt.x, pt.y, 0, hWnd, NULL);
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
    if (GetModuleFileNameW(NULL, szPathToExe, MAX_PATH) == 0) {
        MessageBoxW(NULL, L"无法获取程序路径，开机自启功能异常", L"错误", MB_OK | MB_ICONERROR);
        return;
    }

    // 获取程序的名称
    const LPCWSTR szValueName = PathFindFileNameW(szPathToExe);

    // 打开注册表键
    if (RegOpenKeyExW(HKEY_CURRENT_USER, szPath, 0, KEY_ALL_ACCESS, &hKey) == ERROR_SUCCESS) {
        if (enable) {
            ModifyConfigAutoStart(true);
            // 向注册表写入值以开启自动启动
            if (RegSetValueExW(hKey, szValueName, 0, REG_SZ, (BYTE*)szPathToExe, wcslen(szPathToExe) * sizeof(wchar_t)) != ERROR_SUCCESS) {
                MessageBoxW(NULL, L"无法设置开机自启动，开机自启功能异常", L"错误", MB_OK | MB_ICONERROR);
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
                MessageBoxW(NULL, L"无法取消开机自启动，开机自启功能异常", L"错误", MB_OK | MB_ICONERROR);
            }
        }

        // 关闭注册表键
        RegCloseKey(hKey);
    }
    else {
        MessageBoxW(NULL, L"无法打开注册表键，开机自启功能异常", L"错误", MB_OK | MB_ICONERROR);
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
    const int size = MultiByteToWideChar(CP_UTF8, 0, &s[0], static_cast<int>(s.size()), NULL, 0);
    std::wstring result(size, 0);
    MultiByteToWideChar(CP_UTF8, 0, &s[0], static_cast<int>(s.size()), &result[0], size);
    return result;
}
