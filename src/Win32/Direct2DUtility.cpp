/**
 * player95 - (c) Ingmar Runge 2014
 *
 * License: MIT
 **/

#include "stdafx.h"
#include "Win32/Direct2DUtility.h"

using namespace Player95::Win32;

ComPtr<ID2D1Factory> Direct2DUtility::m_pD2DFactory;
ComPtr<IDWriteFactory> Direct2DUtility::m_pDWriteFactory;

HRESULT Direct2DUtility::GetD2DFactory(__out ID2D1Factory** factory)
{
	HRESULT hr = S_OK;
	
	if (!m_pD2DFactory)
	{
		hr = ::D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &m_pD2DFactory);
	}

	if (SUCCEEDED(hr))
	{
		hr = m_pD2DFactory.AssignToOutputPointer(factory);
	}

	return hr;
}

HRESULT Direct2DUtility::GetDWriteFactory(__out IDWriteFactory** factory)
{
	HRESULT hr = S_OK;

	if (!m_pDWriteFactory)
	{
		hr = ::DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(m_pDWriteFactory), reinterpret_cast<IUnknown**>(&m_pDWriteFactory));
	}

	if (SUCCEEDED(hr))
	{
		hr = m_pDWriteFactory.AssignToOutputPointer(factory);
	}

	return hr;
}

void Direct2DUtility::ComCleanUp()
{
	unsigned long d2d_factory_ref_count =
	m_pD2DFactory.Reset();
	m_pDWriteFactory.Reset(); // shared, so no point in checking the new refcount

	_ASSERT_EXPR(d2d_factory_ref_count == 0, L"D2DFactory leaking");
}
