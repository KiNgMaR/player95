/**
 * player95 - (c) Ingmar Runge 2014
 *
 * License: MIT
 **/

#pragma once

// define Windows 7 as oldest supported platform:
#include <WinSDKVer.h>
#define _WIN32_WINNT 0x0601
#include <SDKDDKVer.h>

#define NOMINMAX

// Windows/SDK headers:
#include <Windows.h>
#include <d2d1.h>
#include <d2d1helper.h>
#include <dwrite.h>
#include <wincodec.h>
#include "Winhttp.h"

#include "Win32/ComPtr.h"
#include "Win32/ErrorToString.h"

// C++ and C library headers:
#include <string>
#include <sstream>
#include <iomanip>
#include <memory>
#include <algorithm>
#include <functional>
#include <vector>
#include <map>
#include <codecvt>
#include <locale>
#include <mutex>

// other dependencies:
#include <bass.h>
