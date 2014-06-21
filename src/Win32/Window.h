/**
 * player95 - (c) Ingmar Runge 2014
 *
 * License: MIT
 **/

#pragma once

#include <Windows.h>

namespace Player95
{
	namespace Win32
	{
		class Window
		{
		public:
			virtual bool Create() = 0;

			HWND GetHwnd() const { return m_hWnd; }

			// We'll try to map/handle all possible messages in this class, therefore the following
			// method is not virtual. Have to check how all this works with CreateDialog* etc...
			HRESULT WindowProc(UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT* result);

			virtual HRESULT OnCreate() { return E_NOTIMPL; }
			virtual HRESULT OnEraseBackground() { return E_NOTIMPL; }
			virtual HRESULT OnRender() { return E_NOTIMPL; }
			virtual HRESULT OnSize(unsigned int width, unsigned int height) { return E_NOTIMPL; }
			virtual HRESULT OnGetMinMaxInfo(PMINMAXINFO info) { return E_NOTIMPL; }
			virtual HRESULT OnClose() { return E_NOTIMPL; }
			virtual HRESULT OnDestroy()
			{
				::PostQuitMessage(0);

				return S_OK;
			}

			void GetClientRect(RECT *pRect);
			void SetWindowTitle(const wchar_t title[]) { ::SetWindowText(m_hWnd, title); }
			void SetWindowTitle(const std::wstring& title) { SetWindowTitle(title.c_str()); }

		protected:
			Window();

			bool RegisterWindowClass(HINSTANCE hInstance);
			bool CreateWindowOnScreen(HINSTANCE hInstance);

			static LRESULT CALLBACK StaticWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

			const wchar_t *m_windowClassName;

		private:
			HWND m_hWnd;
		};
	}
}
