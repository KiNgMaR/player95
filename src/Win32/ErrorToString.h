/**
* player95 - (c) Ingmar Runge 2014
*
* License: MIT
**/

#pragma once

#include <Windows.h>
#include <string>
#include <sstream>
#include <iomanip>

namespace Player95
{
	namespace Win32
	{
		namespace Helpers
		{
			const __inline std::wstring ErrorToString(int errorCode = 0, const wchar_t *sourceLib = nullptr)
			{
				if (!errorCode)
				{
					errorCode = ::GetLastError();
				}

				std::wstringstream ss;
				ss << L"Error 0x" << std::setfill(L'0') << std::setw(8) << std::hex << std::uppercase << errorCode
					<< L" (" << std::dec << std::setw(0) << errorCode << L")";

				LPWSTR msgBuf;

				DWORD flags = FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS;
				HMODULE sourceHandle = nullptr;

				if (sourceLib)
				{
					sourceHandle = ::GetModuleHandleW(sourceLib);
					flags |= FORMAT_MESSAGE_FROM_HMODULE;
				}

				if (::FormatMessageW(flags, sourceHandle, errorCode, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
					(LPWSTR)&msgBuf, 0, nullptr) != 0)
				{
					size_t length = wcslen(msgBuf);

					// remove trailing newlines:
					while (length > 0 && iswcntrl(msgBuf[length - 1]))
					{
						--length;
					}

					ss << L": " << std::wstring(msgBuf, length);

					::LocalFree(msgBuf);
				}

				return ss.str();
			}
		}
	}
}
