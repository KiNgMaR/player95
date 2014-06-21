/**
 * player95 - (c) Ingmar Runge 2014
 *
 * License: MIT
 **/

#include "stdafx.h"
#include "Win32/PlayerMainWindow.h"
#include "Win32/Direct2DUtility.h"
#include "Subsonic/ApiClient.h"

using namespace Player95::Win32;

PlayerMainWindow::PlayerMainWindow(PlayerApplication *app)
	: Window(), m_app(app)
{
	m_windowClassName = L"Player95::PlayerMainWindow";
}

bool PlayerMainWindow::Create()
{
	if (!RegisterWindowClass(m_app->GetInstanceHandle()))
	{
		return false;
	}

	if (!CreateWindowOnScreen(m_app->GetInstanceHandle()))
	{
		return false;
	}

	SetWindowTitle(L"player95");

	return true;
}

HRESULT PlayerMainWindow::OnCreate()
{
	HRESULT hr = CreateDeviceIndependentResources();
	// :TODO: check for error

	return S_OK;
}

HRESULT PlayerMainWindow::OnEraseBackground()
{
	return S_OK; // prevent flickering
}

HRESULT PlayerMainWindow::OnSize(unsigned int width, unsigned int height)
{
	if (!width || !height)
	{
		return S_OK;
	}

	if (m_renderTarget)
	{
		m_renderTarget->Resize(D2D1::SizeU(width, height));
	}

	return S_OK;
}
