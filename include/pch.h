#ifndef PCH_H
#define PCH_H

#pragma once

#include <SDKDDKVer.h>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <shellapi.h>

#include <string>
#include <filesystem>
#include <shlwapi.h>
#pragma comment(lib, "shlwapi.lib")

#endif // PCH_H