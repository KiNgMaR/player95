/**
 * player95 - (c) Ingmar Runge 2014
 *
 * License: MIT
 **/

#include "stdafx.h"
#include "Win32/PlayerMainWindow.h"
#include "Win32/Direct2DUtility.h"

using namespace Player95::Win32;

HRESULT PlayerMainWindow::OnRender()
{
	HRESULT hr = CreateDeviceResources();

	if (SUCCEEDED(hr) && !(m_renderTarget->CheckWindowState() & D2D1_WINDOW_STATE_OCCLUDED))
	{
		m_renderTarget->BeginDraw();

		D2D1_MATRIX_3X2_F identity = D2D1::Matrix3x2F::Identity();
		m_renderTarget->SetTransform(identity);

		D2D1_SIZE_F size = m_renderTarget->GetSize();

		// Paint background
		ComPtr<ID2D1SolidColorBrush> backgroundBrush;
		hr = m_renderTarget->CreateSolidColorBrush(
			D2D1::ColorF(D2D1::ColorF::White),
			&backgroundBrush
			);

		if (SUCCEEDED(hr))
		{
			m_renderTarget->FillRectangle(D2D1::RectF(0, 0, size.width, size.height), backgroundBrush);
		}

		hr = m_renderTarget->EndDraw();

		if (hr == D2DERR_RECREATE_TARGET)
		{
			DiscardDeviceResources();
		}
	}

	return S_OK; // we did something alright
}

HRESULT PlayerMainWindow::CreateDeviceIndependentResources()
{
	HRESULT hr = Direct2DUtility::GetD2DFactory(&m_d2dFactory);

	if (SUCCEEDED(hr))
	{
		hr = Direct2DUtility::GetDWriteFactory(&m_dWriteFactory);
	}

	return hr;
}

HRESULT PlayerMainWindow::CreateDeviceResources()
{
	HRESULT hr = S_OK;

	if (m_renderTarget)
	{
		return hr;
	}

	RECT rect;
	GetClientRect(&rect);

	hr = m_d2dFactory->CreateHwndRenderTarget(D2D1::RenderTargetProperties(),
		D2D1::HwndRenderTargetProperties(GetHwnd(), D2D1::SizeU(rect.right, rect.bottom)),
		&m_renderTarget);

	return hr;
}

void PlayerMainWindow::DiscardDeviceResources()
{
	m_renderTarget.Reset();
}
