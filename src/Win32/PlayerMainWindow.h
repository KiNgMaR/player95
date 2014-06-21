/**
 * player95 - (c) Ingmar Runge 2014
 *
 * License: MIT
 **/

#pragma once

#include "Win32/Window.h"
#include "Win32/PlayerApplication.h"

namespace Player95
{
	namespace Win32
	{
		class PlayerMainWindow : public Window
		{
		public:
			PlayerMainWindow(PlayerApplication *app);

			bool Create() override;
			HRESULT OnCreate() override;
			HRESULT OnEraseBackground() override;
			HRESULT OnRender() override;
			HRESULT OnSize(unsigned int width, unsigned int height) override;

		protected:
			PlayerApplication *m_app;

			ComPtr<ID2D1Factory> m_d2dFactory;
			ComPtr<IDWriteFactory> m_dWriteFactory;

			ComPtr<ID2D1HwndRenderTarget> m_renderTarget;

			HRESULT CreateDeviceIndependentResources();
			HRESULT CreateDeviceResources();
			void DiscardDeviceResources();
			//void PrepareBackgroundBrush(D2D1_SIZE_F size);
		};
	}
}
