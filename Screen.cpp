// Screen.cpp : 定义应用程序的入口点。
//

#include "Screen.h"

#include <cstdio>
#include <fstream>
#include <iostream>
#include <string>

#include "framework.h"
#include "ini/INIReader.h"

#define MAX_LOADSTRING   100
#define HOTKEY_ID        1010 // 全局快捷键 ID
#define WM_MYTRAYMESSAGE (WM_USER + 1)
#define IDM_SHOW_HIDE    1001 // 窗口显示隐藏
#define IDM_EXIT         1002 // 软件退出

// 全局变量:
HINSTANCE hInst;                     // 当前实例
WCHAR szTitle[MAX_LOADSTRING];       // 标题栏文本
WCHAR szWindowClass[MAX_LOADSTRING]; // 主窗口类名
HMENU hPopupMenu;                    // 托盘右键弹出菜单
NOTIFYICONDATA nid = {0};            // 托盘图标句柄

constexpr int winSize  = 80; // 正方体窗口的尺寸
constexpr int winRound = 10; // 窗口的圆角尺寸
bool isStart{true};          // 启动立即熄屏
bool isWindow{false};        // 启动显示窗口
bool isShortcut{true};       // 快捷键是否生效
bool isLock{false};          // 息屏时，是否先锁屏

// 此代码模块中包含的函数的前向声明:
ATOM MyRegisterClass(HINSTANCE hInstance);
BOOL InitInstance(HINSTANCE, int);
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
void CheckConfigFile(); // 检查配置文件是否存在, 并生效
void RestingScreenFn(); // 息屏函数

int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
    const HANDLE hMutex = CreateMutex(NULL, FALSE, L"Global\\{dev.hunlongyu.screen}");
    if (GetLastError() == ERROR_ALREADY_EXISTS) {
        return 1; // 退出程序
    }

    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    CheckConfigFile(); // 首先尝试读取配置文件

    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_SCREEN, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // 执行应用程序初始化:
    if (!InitInstance(hInstance, nCmdShow)) {
        return FALSE;
    }

    if (isStart) {
        RestingScreenFn();
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
    wcex.style         = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc   = WndProc;
    wcex.cbClsExtra    = 0;
    wcex.cbWndExtra    = 0;
    wcex.hInstance     = hInstance;
    wcex.hIcon         = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_SCREEN));
    wcex.hCursor       = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName  = MAKEINTRESOURCEW(IDC_SCREEN);
    wcex.lpszClassName = szWindowClass;
    wcex.hIconSm       = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

void AddTrayIcon(HINSTANCE hInstance, HWND hWnd, UINT uCallbackMessage)
{
    // 创建弹出菜单
    hPopupMenu = CreatePopupMenu();
    // 添加菜单项
    AppendMenu(hPopupMenu, MF_STRING, IDM_SHOW_HIDE, TEXT("显示"));
    AppendMenu(hPopupMenu, MF_STRING, IDM_EXIT, TEXT("退出"));

    nid.cbSize = sizeof(NOTIFYICONDATA);
    nid.hWnd   = hWnd;
    nid.uID    = TRUE;
    nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    nid.hIcon  = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_SCREEN));
    wcsncpy_s(nid.szTip, L"息屏助手\nV1.0.0", sizeof(nid.szTip) / sizeof(WCHAR));
    nid.uCallbackMessage = uCallbackMessage;
    Shell_NotifyIcon(NIM_ADD, &nid);
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
    hInst           = hInstance;
    const HWND hWnd = CreateWindowExW(WS_EX_LAYERED, szWindowClass, szTitle, WS_POPUP, CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);
    if (!hWnd) {
        return FALSE;
    }
    SetWindowPos(hWnd, HWND_TOPMOST, 0, 0, winSize, winSize, SWP_NOMOVE | SWP_NOACTIVATE);
    SetLayeredWindowAttributes(hWnd, RGB(0, 0, 0), 40, LWA_COLORKEY | LWA_ALPHA);

    const HRGN hRgn = CreateRoundRectRgn(0, 0, winSize, winSize, winRound, winRound);
    SetWindowRgn(hWnd, hRgn, TRUE);
    DeleteObject(hRgn);

    if (isShortcut) {
        const auto flag = RegisterHotKey(hWnd, HOTKEY_ID, MOD_ALT, 0x4C);
        if (flag != 0) {
            printf("success");
        }
    }
    UpdateWindow(hWnd);
    constexpr UINT uCallbackMessage = WM_MYTRAYMESSAGE;
    AddTrayIcon(hInstance, hWnd, uCallbackMessage);

    HKEY hKey;
    if (RegOpenKeyEx(HKEY_CURRENT_USER, TEXT("Software\\Hunlongyu\\Resting"), 0, KEY_ALL_ACCESS, &hKey) == ERROR_SUCCESS) {
        WINDOWPLACEMENT wp;
        DWORD size = sizeof(WINDOWPLACEMENT);
        if (RegQueryValueEx(hKey, TEXT("WindowPlacement"), NULL, NULL, (BYTE*)&wp, &size) == ERROR_SUCCESS) {
            SetWindowPlacement(hWnd, &wp);
            if (isWindow) {
                ShowWindow(hWnd, SW_SHOW);
            }
            else {
                ShowWindow(hWnd, SW_HIDE);
            }
        }
        RegCloseKey(hKey);
    }
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
        case WM_MYTRAYMESSAGE:
            if (lParam == WM_LBUTTONUP) {
                ShowWindow(hWnd, SW_SHOW);
            }
            else if (lParam == WM_RBUTTONUP) {
                POINT pt;
                GetCursorPos(&pt);
                SetForegroundWindow(hWnd);
                // 根据窗口的可见性修改菜单项的文本
                if (IsWindowVisible(hWnd)) {
                    ModifyMenu(hPopupMenu, IDM_SHOW_HIDE, MF_BYCOMMAND | MF_STRING, IDM_SHOW_HIDE, TEXT("隐藏"));
                }
                else {
                    ModifyMenu(hPopupMenu, IDM_SHOW_HIDE, MF_BYCOMMAND | MF_STRING, IDM_SHOW_HIDE, TEXT("显示"));
                }
                TrackPopupMenu(hPopupMenu, TPM_RIGHTBUTTON, pt.x, pt.y, 0, hWnd, NULL);
                PostMessage(hWnd, WM_NULL, 0, 0);
            }
            break;
        case WM_COMMAND:
            switch (LOWORD(wParam)) {
                case IDM_SHOW_HIDE:
                    if (IsWindowVisible(hWnd)) {
                        ShowWindow(hWnd, SW_HIDE); // 隐藏窗口
                    }
                    else {
                        ShowWindow(hWnd, SW_SHOW); // 显示窗口
                    }
                    break;
                case IDM_EXIT:
                    DestroyWindow(hWnd); // 退出程序
                    break;
            }
            break;
        case WM_NCLBUTTONUP: {
            RestingScreenFn();
        } break;
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            EndPaint(hWnd, &ps);
        } break;

        case WM_NCHITTEST: return HTCAPTION;

        case WM_DESTROY:
            WINDOWPLACEMENT wp;
            wp.length = sizeof(WINDOWPLACEMENT);
            // 获取窗口布局信息
            GetWindowPlacement(hWnd, &wp);

            // 在Registry中保存布局信息
            HKEY hKey;
            if (RegCreateKeyEx(HKEY_CURRENT_USER, TEXT("Software\\Hunlongyu\\Resting"), 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey, NULL) == ERROR_SUCCESS) {
                RegSetValueEx(hKey, TEXT("WindowPlacement"), 0, REG_BINARY, (BYTE*)&wp, sizeof(WINDOWPLACEMENT));
                RegCloseKey(hKey);
            }
            Shell_NotifyIcon(NIM_DELETE, &nid);
            PostQuitMessage(0);
            break;
        default: return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

void CheckConfigFile()
{
    const std::string path = "config.ini";
    const INIReader reader(path);

    if (reader.ParseError() < 0) {
        return;
    }

    isStart    = reader.GetBoolean("Application", "start", false);
    isWindow   = reader.GetBoolean("Application", "window", false);
    isShortcut = reader.GetBoolean("Application", "shortcut", false);
    isLock     = reader.GetBoolean("Application", "lock", false);
}

void RestingScreenFn()
{
    if (isLock) {
        LockWorkStation();
    }
    PostMessage(HWND_BROADCAST, WM_SYSCOMMAND, SC_MONITORPOWER, (LPARAM)2);
}
