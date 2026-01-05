#pragma once
#include "CUnknown.h"
#include "ComPtr.h"

ECK_NAMESPACE_BEGIN
#if !ECK_OPT_NO_DX
class CGeometrySinkForwarder final :
	public CUnknown<CGeometrySinkForwarder, ID2D1SimplifiedGeometrySink>
{
private:
	ComPtr<ID2D1SimplifiedGeometrySink> m_pSink{};
	float m_oxCurr{}, m_oyCurr{};
public:
	CGeometrySinkForwarder(ID2D1SimplifiedGeometrySink* pSink) : m_pSink{ pSink } {}

	STDMETHOD_(void, SetFillMode)(D2D1_FILL_MODE fillMode) { m_pSink->SetFillMode(fillMode); }

	STDMETHOD_(void, SetSegmentFlags)(D2D1_PATH_SEGMENT vertexFlags) { m_pSink->SetSegmentFlags(vertexFlags); }

	STDMETHOD_(void, BeginFigure)(D2D1_POINT_2F startPoint, D2D1_FIGURE_BEGIN figureBegin)
	{
		startPoint.x += m_oxCurr;
		startPoint.y += m_oyCurr;
		m_pSink->BeginFigure(startPoint, figureBegin);
	}

	STDMETHOD_(void, AddLines)(CONST D2D1_POINT_2F* points, UINT32 pointsCount)
	{
		if (m_oxCurr != 0.f || m_oyCurr != 0.f)
		{
			auto p = (D2D1_POINT_2F*)_malloca(pointsCount * sizeof(D2D1_POINT_2F));
			EckAssert(p);
			memcpy(p, points, pointsCount * sizeof(D2D1_POINT_2F));
			EckCounter(pointsCount, i)
			{
				p[i].x += m_oxCurr;
				p[i].y += m_oyCurr;
			}
			m_pSink->AddLines(p, pointsCount);
			_freea(p);
		}
		else
			m_pSink->AddLines(points, pointsCount);
	}

	STDMETHOD_(void, AddBeziers)(CONST D2D1_BEZIER_SEGMENT* beziers, UINT32 beziersCount)
	{
		if (m_oxCurr != 0.f || m_oyCurr != 0.f)
		{
			auto p = (D2D1_BEZIER_SEGMENT*)_malloca(beziersCount * sizeof(D2D1_BEZIER_SEGMENT));
			EckAssert(p);
			memcpy(p, beziers, beziersCount * sizeof(D2D1_BEZIER_SEGMENT));
			EckCounter(beziersCount, i)
			{
				p[i].point1.x += m_oxCurr;
				p[i].point1.y += m_oyCurr;
				p[i].point2.x += m_oxCurr;
				p[i].point2.y += m_oyCurr;
				p[i].point3.x += m_oxCurr;
				p[i].point3.y += m_oyCurr;
			}
			m_pSink->AddBeziers(p, beziersCount);
			_freea(p);
		}
		else
			m_pSink->AddBeziers(beziers, beziersCount);
	}

	STDMETHOD_(void, EndFigure)(D2D1_FIGURE_END figureEnd) { m_pSink->EndFigure(figureEnd); }

	STDMETHOD(Close)() { return m_pSink->Close(); }

	void SetOffset(float ox, float oy)
	{
		m_oxCurr = ox;
		m_oyCurr = oy;
	}
};

// 调用IDWriteTextLayout::Draw时，上下文参数必须为CGeometrySinkForwarder指针
class CDwFetchPathRender final : public IDWriteTextRenderer
{
private:
	float m_fDpi{};
	BOOL m_bPixelSnappingDisabled{};

	void DrawLine(CGeometrySinkForwarder* pSink,
		float x, float y, float cx, float cy)
	{
		pSink->SetOffset(0.f, 0.f);
		D2D1_POINT_2F pt[3];
		pt[0].x = x;
		pt[0].y = y;
		pSink->BeginFigure(pt[0], D2D1_FIGURE_BEGIN_FILLED);
		pt[0].x += cx;
		pt[1].x = pt[0].x;
		pt[1].y = pt[0].y + cy;
		pt[2].x = x;
		pt[2].y = pt[1].y;
		pSink->AddLines(pt, 3);
		pSink->EndFigure(D2D1_FIGURE_END_CLOSED);
	}
public:
	CDwFetchPathRender(float fDpi, BOOL bPixelSnappingDisabled)
		: m_fDpi{ fDpi }, m_bPixelSnappingDisabled{ bPixelSnappingDisabled } {
	}

	STDMETHOD(DrawGlyphRun)(void* pClientDrawingContext,
		FLOAT xOrgBaseline, FLOAT yOrgBaseline,
		DWRITE_MEASURING_MODE MeasuringMode, DWRITE_GLYPH_RUN const* pGlyphRun,
		DWRITE_GLYPH_RUN_DESCRIPTION const* pGlyphRunDesc, IUnknown* pClientDrawingEffect) override
	{
		const auto pSink = (CGeometrySinkForwarder*)pClientDrawingContext;
		pSink->SetOffset(xOrgBaseline, yOrgBaseline);
		return pGlyphRun->fontFace->GetGlyphRunOutline(pGlyphRun->fontEmSize,
			pGlyphRun->glyphIndices, pGlyphRun->glyphAdvances,
			pGlyphRun->glyphOffsets, pGlyphRun->glyphCount,
			pGlyphRun->isSideways, pGlyphRun->bidiLevel, pSink);
	}

	STDMETHOD(DrawUnderline)(void* pClientDrawingContext,
		FLOAT xOrgBaseline, FLOAT yOrgBaseline,
		DWRITE_UNDERLINE const* pUnderline, IUnknown* pClientDrawingEffect) override
	{
		const auto pSink = (CGeometrySinkForwarder*)pClientDrawingContext;
		DrawLine(pSink, xOrgBaseline, yOrgBaseline + pUnderline->offset,
			pUnderline->width, pUnderline->thickness);
		return S_OK;
	}

	STDMETHOD(DrawStrikethrough)(void* pClientDrawingContext,
		FLOAT xOrgBaseline, FLOAT yOrgBaseline,
		DWRITE_STRIKETHROUGH const* strikethrough, IUnknown* pClientDrawingEffect) override
	{
		const auto pSink = (CGeometrySinkForwarder*)pClientDrawingContext;
		DrawLine(pSink, xOrgBaseline, yOrgBaseline + strikethrough->offset,
			strikethrough->width, strikethrough->thickness);
		return S_OK;
	}

	STDMETHOD(DrawInlineObject)(void* pClientDrawingContext,
		FLOAT xOrg, FLOAT yOrg, IDWriteInlineObject* pInlineObject,
		BOOL bSideways, BOOL bRightToLeft, IUnknown* pClientDrawingEffect) override
	{
		return E_NOTIMPL;
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
			QITABENT(CDwFetchPathRender, IDWriteTextRenderer),
			QITABENT(CDwFetchPathRender, IDWritePixelSnapping),
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
	_In_opt_ ID2D1Factory1* pD2DFactory = nullptr)
{
	if (!pD2DFactory)
		pD2DFactory = g_pD2DFactory;
	ComPtr<ID2D1PathGeometry1> pPath;
	ComPtr<ID2D1GeometrySink> pSink;
	pD2DFactory->CreatePathGeometry(&pPath);
	pPath->Open(&pSink);

	CDwFetchPathRender Renderer{ fDpi,bPixelSnappingDisabled };
	CGeometrySinkForwarder Forwarder{ pSink.Get() };

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
	_In_opt_ ID2D1Factory1* pD2dFactory = nullptr)
{
	constexpr float cyPadding{};
	return GetTextLayoutPathGeometry(1, &pLayout, &cyPadding, &x,
		y, pPathGeometry, fDpi, bPixelSnappingDisabled, pD2dFactory);
}
#endif // !ECK_OPT_NO_DX
ECK_NAMESPACE_END