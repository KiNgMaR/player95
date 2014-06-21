/**
 * player95 - (c) Ingmar Runge 2014
 *
 * License: MIT
 **/

#include "stdafx.h"
#include "Win32/Window.h"

using namespace Player95::Win32;

Window::Window()
	: m_hWnd(0), m_windowClassName(nullptr)
{

}

HRESULT Window::WindowProc(UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT* result)
{
	HRESULT hr = E_NOTIMPL;

	// If SUCCEEDED(hr), *result automatically is 0 (which usually means "handled"),
	// but it can be overwritten where necessary.

	switch (uMsg)
	{
	case WM_CREATE:
		hr = OnCreate();
		break;
	case WM_ERASEBKGND:
		hr = OnEraseBackground();
		if (SUCCEEDED(hr))
		{
			// success = application takes care of erasing the background
			*result = 1;
		}
		break;
	case WM_PAINT:
	case WM_DISPLAYCHANGE: {
		PAINTSTRUCT ps;
		::BeginPaint(m_hWnd, &ps);
		hr = OnRender();
		::EndPaint(m_hWnd, &ps);
		break; }
	case WM_SIZE:
		hr = OnSize(LOWORD(lParam), HIWORD(lParam));
		break;
	case WM_GETMINMAXINFO:
		hr = OnGetMinMaxInfo(reinterpret_cast<PMINMAXINFO>(lParam));
		break;
	case WM_CLOSE:
		hr = OnClose();
		break;
	case WM_DESTROY:
		hr = OnDestroy();
		break;
	}

	return hr;
}

bool Window::RegisterWindowClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex = { 0 };

	if (::GetClassInfoEx(hInstance, m_windowClassName, &wcex) == 0)
	{
		wcex.cbSize        = sizeof(WNDCLASSEX);

		wcex.hInstance     = hInstance;
		wcex.style         = CS_HREDRAW | CS_VREDRAW;
		wcex.lpszClassName = m_windowClassName;
		wcex.lpfnWndProc   = &StaticWindowProc;
		wcex.cbWndExtra    = sizeof(LONG_PTR);
		wcex.hCursor       = ::LoadCursor(nullptr, IDC_ARROW);

		if (::RegisterClassEx(&wcex) == 0)
		{
			return false;
		}
	}

	return true;
}

bool Window::CreateWindowOnScreen(HINSTANCE hInstance)
{
	HWND hWnd = ::CreateWindowEx(
		0,
		m_windowClassName,
		nullptr, // no title for now
		(/* has parent ? WS_CHILD : */ WS_OVERLAPPEDWINDOW) | WS_VISIBLE,
		CW_USEDEFAULT, CW_USEDEFAULT,
		CW_USEDEFAULT, CW_USEDEFAULT,
		/* has parent ? parent hWnd : */ 0,
		nullptr,
		hInstance,
		this);

	return (hWnd != nullptr && m_hWnd != nullptr);
}

LRESULT CALLBACK Window::StaticWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	LRESULT result = 0;
	Window* window = nullptr;

	if (uMsg == WM_NCCREATE)
	{
		const LPCREATESTRUCT pcs = (LPCREATESTRUCT)lParam;
		window = reinterpret_cast<Window*>(pcs->lpCreateParams);
		window->m_hWnd = hWnd;

		::SetWindowLongPtr(hWnd, GWLP_USERDATA, PtrToUlong(window));
	}
	else
	{
		window = reinterpret_cast<Window*>(::GetWindowLongPtr(hWnd, GWLP_USERDATA));
	}

	if (!window || FAILED(window->WindowProc(uMsg, wParam, lParam, &result)))
	{
		result = ::DefWindowProc(hWnd, uMsg, wParam, lParam);
	}

	if (uMsg == WM_DESTROY && window)
	{
		// detach message handler.
		::SetWindowLongPtr(window->GetHwnd(), GWLP_USERDATA, 0);
	}

	return result;
}

void Window::GetClientRect(RECT *pRect)
{
	::GetClientRect(m_hWnd, pRect);
}
