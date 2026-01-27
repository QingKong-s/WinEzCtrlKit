#pragma once
#include "ECK.h"

ECK_NAMESPACE_BEGIN
struct __declspec(uuid("FAF92698-0D74-4D14-90A5-4E66C10A9630"))
    ITimeLine : public IUnknown
{
    // 滴答时间线
    virtual void TlTick(int iMs) noexcept = 0;
    // 时间线是否有效
    virtual BOOL TlIsValid() noexcept = 0;
    // 取当前滴答间隔
    virtual int TlGetCurrentInterval() noexcept = 0;
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

    BOOL TlIsValid() noexcept override { return TRUE; }
    int TlGetCurrentInterval() noexcept override { return 0; }
};
ECK_NAMESPACE_END