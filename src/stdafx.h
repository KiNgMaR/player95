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

// Windows/SDK headers:
#include <Windows.h>
#include <d2d1.h>
#include <d2d1helper.h>
#include <dwrite.h>
#include <wincodec.h>

// using Windows RT for ComPtr class:
#include <wrl\client.h>
// with some additions (fingers crossed):
template<typename T> class ComPtr : public Microsoft::WRL::ComPtr<T>
{
public:
	T** operator&()
	{
		return GetAddressOf();
	}

	operator T*() const
	{
		return Get();
	}

	operator bool() const
	{
		return (Get() != nullptr);
	}

	HRESULT AssignToOutputPointer(T** pp)
	{
		_ASSERT(pp);

		*pp = Get();

		if (*pp != nullptr)
		{
			(*pp)->AddRef();
		}

		return S_OK;
	}
};

// C++ and C library headers:
#include <string>
#include <sstream>
#include <memory>
#include <algorithm>
#include <functional>
#include <vector>
#include <map>

// other dependencies:
#include <bass.h>
