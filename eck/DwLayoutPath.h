#pragma once
#include "CGeometrySinkOffsetTranformer.h"

ECK_NAMESPACE_BEGIN
#if !ECK_OPT_NO_DX
// 将此类实例传入IDWriteTextLayout::Draw以提取字形轮廓
// 1. 上下文参数必须为IUnknown指针，该接口必须可被QI为
//    ID2D1SimplifiedGeometrySink和IGeometrySinkTransformer
// 2. 调用方负责在调用Draw前对IGeometrySinkTransformer使用GstEnableOffset(TRUE)
class CDwFetchPathRenderer final : public IDWriteTextRenderer
{
private:
    float m_fDpi{};
    BOOL m_bPixelSnappingDisabled{};
    ComPtr<ID2D1SimplifiedGeometrySink> m_pSink{};
    ComPtr<IGeometrySinkTransformer> m_pTransformer{};

    void DrawLine(float x, float y, float cx, float cy)
    {
        m_pTransformer->GstEnableOffset(FALSE);
        D2D1_POINT_2F pt[3];
        pt[0].x = x;
        pt[0].y = y;
        m_pSink->BeginFigure(pt[0], D2D1_FIGURE_BEGIN_FILLED);
        pt[0].x += cx;
        pt[1].x = pt[0].x;
        pt[1].y = pt[0].y + cy;
        pt[2].x = x;
        pt[2].y = pt[1].y;
        m_pSink->AddLines(pt, 3);
        m_pSink->EndFigure(D2D1_FIGURE_END_CLOSED);
        m_pTransformer->GstEnableOffset(TRUE);
    }

    HRESULT PrepareSink(void* pClientDrawingContext)
    {
        if (!m_pSink.Get())
        {
            const auto pUnknown = (IUnknown*)pClientDrawingContext;
            HRESULT hr;
            hr = pUnknown->QueryInterface(m_pSink.AddrOfClear());
            if (FAILED(hr))
                return E_NOINTERFACE;
            hr = pUnknown->QueryInterface(m_pTransformer.AddrOfClear());
            if (FAILED(hr))
                return E_NOINTERFACE;
        }
        return S_OK;
    }
public:
    constexpr CDwFetchPathRenderer(float fDpi = 96.f, BOOL bPixelSnappingDisabled = FALSE) noexcept
        : m_fDpi{ fDpi }, m_bPixelSnappingDisabled{ bPixelSnappingDisabled }
    {}

    STDMETHOD(DrawGlyphRun)(void* pClientDrawingContext,
        FLOAT xOrgBaseline, FLOAT yOrgBaseline,
        DWRITE_MEASURING_MODE MeasuringMode, DWRITE_GLYPH_RUN const* pGlyphRun,
        DWRITE_GLYPH_RUN_DESCRIPTION const* pGlyphRunDesc, IUnknown* pClientDrawingEffect) override
    {
        const auto hr = PrepareSink(pClientDrawingContext);
        if (FAILED(hr))
            return hr;
        m_pTransformer->GstSetOffset(xOrgBaseline, yOrgBaseline);
        return pGlyphRun->fontFace->GetGlyphRunOutline(pGlyphRun->fontEmSize,
            pGlyphRun->glyphIndices, pGlyphRun->glyphAdvances,
            pGlyphRun->glyphOffsets, pGlyphRun->glyphCount,
            pGlyphRun->isSideways, pGlyphRun->bidiLevel & 1, m_pSink.Get());
    }

    STDMETHOD(DrawUnderline)(void* pClientDrawingContext,
        FLOAT xOrgBaseline, FLOAT yOrgBaseline,
        DWRITE_UNDERLINE const* pUnderline, IUnknown* pClientDrawingEffect) override
    {
        const auto hr = PrepareSink(pClientDrawingContext);
        if (FAILED(hr))
            return hr;
        DrawLine(
            xOrgBaseline,
            yOrgBaseline + pUnderline->offset,
            pUnderline->width,
            pUnderline->thickness);
        return S_OK;
    }

    STDMETHOD(DrawStrikethrough)(void* pClientDrawingContext,
        FLOAT xOrgBaseline, FLOAT yOrgBaseline,
        DWRITE_STRIKETHROUGH const* pStrikeThrough, IUnknown* pClientDrawingEffect) override
    {
        const auto hr = PrepareSink(pClientDrawingContext);
        if (FAILED(hr))
            return hr;
        DrawLine(
            xOrgBaseline,
            yOrgBaseline + pStrikeThrough->offset,
            pStrikeThrough->width,
            pStrikeThrough->thickness);
        return S_OK;
    }

    STDMETHOD(DrawInlineObject)(void* pClientDrawingContext,
        FLOAT xOrg, FLOAT yOrg, IDWriteInlineObject* pInlineObject,
        BOOL bSideways, BOOL bRightToLeft, IUnknown* pClientDrawingEffect) override
    {
        return pInlineObject->Draw(pClientDrawingContext, this,
            xOrg, yOrg, bSideways, bRightToLeft, pClientDrawingEffect);
    }

    STDMETHOD(IsPixelSnappingDisabled)(void* pClientDrawingContext, BOOL* pbDisabled) override
    {
        *pbDisabled = m_bPixelSnappingDisabled;
        return S_OK;
    }

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

    STDMETHOD(GetPixelsPerDip)(void* pClientDrawingContext, FLOAT* pfPixelsPerDip) override
    {
        *pfPixelsPerDip = m_fDpi / 96.f;
        return S_OK;
    }

    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid, void** ppvObj) override
    {
        const static QITAB qit[]
        {
            QITABENT(CDwFetchPathRenderer, IDWriteTextRenderer),
            QITABENT(CDwFetchPathRenderer, IDWritePixelSnapping),
            {},
        };
        return QISearch(this, qit, iid, ppvObj);
    }

    ULONG STDMETHODCALLTYPE AddRef() override { return 1; }
    ULONG STDMETHODCALLTYPE Release() override { return 1; }
};


/// <summary>
/// DW文本布局到路径几何形
/// </summary>
/// <param name="cLayout">布局个数</param>
/// <param name="pLayout">DW文本布局数组</param>
/// <param name="cyPadding">间隔数组，至少指向cLayout个元素，但最后一个元素被忽略</param>
/// <param name="x">X坐标数组</param>
/// <param name="yStart">起始Y</param>
/// <param name="pPathGeometry">结果路径几何形</param>
/// <param name="fDpi">DPI</param>
/// <param name="bPixelSnappingDisabled">是否禁用像素贴靠</param>
/// <param name="pD2DFactory">D2D工厂</param>
/// <returns>HRESULT</returns>
inline HRESULT GetTextLayoutPathGeometry(
    size_t cLayout,
    _In_reads_(cLayout) IDWriteTextLayout* const* pLayout,
    _In_reads_(cLayout) const float* cyPadding,
    _In_reads_(cLayout) const float* x,
    float yStart,
    _Out_ ID2D1PathGeometry1*& pPathGeometry,
    float fDpi = 96.f,
    BOOL bPixelSnappingDisabled = FALSE,
    _In_opt_ ID2D1Factory1* pD2DFactory = nullptr) noexcept
{
    if (!pD2DFactory)
        pD2DFactory = g_pD2DFactory;
    ComPtr<ID2D1PathGeometry1> pPath;
    ComPtr<ID2D1GeometrySink> pSink;
    pD2DFactory->CreatePathGeometry(&pPath);
    pPath->Open(&pSink);

    CDwFetchPathRenderer Renderer{ fDpi,bPixelSnappingDisabled };
    CGeometrySinkOffsetTranformer Forwarder{};
    Forwarder.GstSetSink(pSink.Get());
    Forwarder.GstEnableOffset(TRUE);

    DWRITE_TEXT_METRICS tm;
    HRESULT hr = S_OK;
    EckCounter(cLayout, i)
    {
        if (pLayout[i])
        {
            hr = pLayout[i]->Draw(&Forwarder, &Renderer, x[i], yStart);
            if (FAILED(hr))
                break;
            pLayout[i]->GetMetrics(&tm);
            yStart += (tm.height + cyPadding[i]);
        }
        else
            yStart += cyPadding[i];
    }
    if (SUCCEEDED(hr))
        hr = Forwarder.Close();
    pPathGeometry = (SUCCEEDED(hr) ? pPath.Detach() : nullptr);
    return hr;
}

/// <summary>
/// DW文本布局到路径几何形
/// </summary>
/// <param name="pLayout">DW文本布局</param>
/// <param name="x">X</param>
/// <param name="y">Y</param>
/// <param name="pPathGeometry">结果路径几何形</param>
/// <param name="fDpi">DPI</param>
/// <param name="bPixelSnappingDisabled">是否禁用像素贴靠</param>
/// <param name="pD2DFactory">D2D工厂</param>
/// <returns>HRESULT</returns>
inline HRESULT GetTextLayoutPathGeometry(
    _In_ IDWriteTextLayout* pLayout,
    float x, float y,
    _Out_ ID2D1PathGeometry1*& pPathGeometry,
    float fDpi = 96.f,
    BOOL bPixelSnappingDisabled = FALSE,
    _In_opt_ ID2D1Factory1* pD2dFactory = nullptr) noexcept
{
    constexpr float cyPadding{};
    return GetTextLayoutPathGeometry(1, &pLayout, &cyPadding, &x,
        y, pPathGeometry, fDpi, bPixelSnappingDisabled, pD2dFactory);
}
#endif // !ECK_OPT_NO_DX
ECK_NAMESPACE_END