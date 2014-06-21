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
		class Application
		{
		public:
			Application(HINSTANCE hInstance);

			int RunMessageLoop();

			HINSTANCE GetInstanceHandle() const { return m_hInstance; }
		private:
			HINSTANCE m_hInstance;
		};
	}
}
