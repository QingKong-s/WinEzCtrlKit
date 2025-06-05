﻿#pragma once
#include "ECK.h"

ECK_NAMESPACE_BEGIN
struct __declspec(uuid("FAF92698-0D74-4D14-90A5-4E66C10A9630"))
	ITimeLine : public IUnknown
{
	// 滴答时间线
	virtual void Tick(int iMs) = 0;
	// 时间线是否有效
	virtual BOOL IsValid() = 0;
	// 取当前滴答间隔
	virtual int GetCurrTickInterval() = 0;
};

struct CFixedTimeLine : public ITimeLine
{
	ULONG STDMETHODCALLTYPE AddRef() { return 1; }
	ULONG STDMETHODCALLTYPE Release() { return 1; }
	HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppvObject)
	{
		if (riid == __uuidof(ITimeLine) || riid == IID_IUnknown)
		{
			*ppvObject = this;
			return S_OK;
		}
		else
			return E_NOINTERFACE;
	}

	BOOL IsValid() { return TRUE; }
	int GetCurrTickInterval() { return 0; }
};
ECK_NAMESPACE_END