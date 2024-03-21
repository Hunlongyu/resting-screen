#include "pch.h"
#include "ini/inicpp.h"

#define IDS_APP_TITLE    103
#define IDI_SCREEN       107
#define IDC_SCREEN       109
#define MAX_LOADSTRING   100
#define HOTKEY_ID        1010 // 全局快捷键 ID
#define WM_MYTRAYMESSAGE (WM_USER + 1)
#define IDM_AUTO_START   1000 // 开机自启
#define IDM_LOCK         1001 // 息屏锁屏
#define IDM_SHORTCUT     1002 // 快捷键
#define IDM_EXIT         1003 // 软件退出

HINSTANCE hInst;                     // 当前实例
std::wstring szWindowClass{L"Screen"}; // 主窗口类名
HMENU hPopupMenu;                    // 托盘右键弹出菜单
NOTIFYICONDATA nid = {0};            // 托盘图标句柄

std::string version{"1.0.0"}; // 软件版本号
bool isShortcut{true};        // 快捷键是否生效
bool isLock{false};           // 息屏时，是否先锁屏
bool isAutoStart{true};       // 开机自启
std::string configPath{""};   // 配置文件的路径
ini::IniFile m_ini;           // INI 配置

ATOM MyRegisterClass(HINSTANCE hInstance);
BOOL InitInstance(HINSTANCE hInstance);
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

void CheckConfigFile();           // 检查配置文件是否存在, 并生效
void RestingScreenFn();           // 息屏函数
void ToggleAutoStart(bool);       // 切换开机自启功能
void ModifyConfigAutoStart(bool); // 修改开机自启配置项
void ModifyConfigLock(bool);      // 修改锁屏配置项
void ModifyConfigShortcut(bool);  // 修改快捷键配置项

std::wstring stringToWString(const std::string& s); // 字符串转
