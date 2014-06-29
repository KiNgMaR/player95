#pragma once

// using Windows RT for ComPtr class:
#include <wrl\client.h>
// with some additions (fingers crossed):
template<typename T> class ComPtr : public Microsoft::WRL::ComPtr<T>
{
public:
	T** operator&()
	{
		return GetAddressOf();
	}

	operator T*() const
	{
		return Get();
	}

	operator bool() const
	{
		return (Get() != nullptr);
	}

	HRESULT AssignToOutputPointer(T** pp)
	{
		_ASSERT(pp);

		*pp = Get();

		if (*pp != nullptr)
		{
			(*pp)->AddRef();
		}

		return S_OK;
	}
};
