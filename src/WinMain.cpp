/**
 * player95 - (c) Ingmar Runge 2014
 *
 * License: MIT
 **/

#include "stdafx.h"
#include "Win32/PlayerApplication.h"
#include "Win32/PlayerMainWindow.h"
#include "Win32/Direct2DUtility.h"

using namespace Player95::Win32;

int CALLBACK wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
{
	int exitCode = -1;

	::SetDllDirectory(L"");
	::HeapSetInformation(nullptr, HeapEnableTerminationOnCorruption, nullptr, 0);

	if (SUCCEEDED(::CoInitialize(nullptr)))
	{
		if (!::WinHttpCheckPlatform())
		{
			// shouldn't really happen because all platforms with D2D also have WinHttp, but whatever...
			::MessageBox(HWND_DESKTOP, L"Unsupported operating system - WinHttp is not available!", L"Error", MB_ICONERROR);
		}
		else
		{
			PlayerApplication app(hInstance);
			PlayerMainWindow mainWindow(&app);

			if (mainWindow.Create())
			{
				exitCode = app.RunMessageLoop();
			}

			// mainWindow + app must be destroyed before ComCleanUp is invoked.
		}

		Direct2DUtility::ComCleanUp();
	}

	::CoUninitialize();

	return exitCode;
}
