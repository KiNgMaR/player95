/**
 * player95 - (c) Ingmar Runge 2014
 *
 * License: MIT
 **/

#include "stdafx.h"
#include "Win32/Application.h"

using namespace Player95::Win32;

Application::Application(HINSTANCE hInstance)
	: m_hInstance(hInstance)
{

}

int Application::RunMessageLoop()
{
	MSG msg;
	BOOL ret;

	while ((ret = ::GetMessage(&msg, nullptr, 0, 0)) != 0)
	{
		if (ret == -1)
		{
			return -1;
		}

		::TranslateMessage(&msg);
		::DispatchMessage(&msg);
	}

	return static_cast<int>(msg.wParam);
}
