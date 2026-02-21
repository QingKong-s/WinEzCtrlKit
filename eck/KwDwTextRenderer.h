#pragma once
#include "KwDef.h"

ECK_NAMESPACE_BEGIN
KW2D_NAMESPACE_BEGIN
class CDwTextRenderer : public IDWriteTextRenderer
{
public:
    STDMETHOD(GetCurrentTransform)(void* pClientDrawingContext, DWRITE_MATRIX* pMatrix) override
    {
        *pMatrix =
        {
            1.f, 0.f,
            0.f, 1.f,
            0.f, 0.f
        };
        return S_OK;
    }

    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid, void** ppvObj) override
    {
        const static QITAB qit[]
        {
            QITABENT(CDwTextRenderer, IDWriteTextRenderer),
            QITABENT(CDwTextRenderer, IDWritePixelSnapping),
            {},
        };
        return QISearch(this, qit, iid, ppvObj);
    }

    ULONG STDMETHODCALLTYPE AddRef() override { return 1; }
    ULONG STDMETHODCALLTYPE Release() override { return 1; }
};
KW2D_NAMESPACE_END
ECK_NAMESPACE_END