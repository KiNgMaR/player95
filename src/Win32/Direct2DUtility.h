/**
 * player95 - (c) Ingmar Runge 2014
 *
 * License: MIT
 **/

#pragma once

#include <Windows.h>
#include <d2d1.h>
#include <dwrite.h>

namespace Player95
{
	namespace Win32
	{
		class Direct2DUtility
		{
		public:
			static HRESULT GetD2DFactory(__out ID2D1Factory** factory);
			static HRESULT GetDWriteFactory(__out IDWriteFactory** factory);

			// used to Release() stuff before CoUnitialize is called:
			static void ComCleanUp();

		private:
			static ComPtr<ID2D1Factory> m_pD2DFactory;
			static ComPtr<IDWriteFactory> m_pDWriteFactory;
		};
	}
}
