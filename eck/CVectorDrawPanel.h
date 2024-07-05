/*
* WinEzCtrlKit Library
*
* CVectorDrawPanel.h ： 矢量画板
*
* Copyright(C) 2023 QingKong
*/
#pragma once
#include "CWnd.h"
#include "GraphicsHelper.h"

#include <d2d1_2.h>

ECK_NAMESPACE_BEGIN
class CVectorDrawPanel : public CWnd
{
public:
	enum class Type :UINT
	{
		// 名称			// vPt含义
		Zero = 0,		//
		Invalid = 0,	//
		Line,			// 起始、终止
		Rect,			// 左上、右上、右下、左下
		Ellipse,		// 左上、右上、右下、左下
		Polygon,		// 点集
		Arc,			// 
		Pie,			// 
		Chord,			// 
		RoundRect,		// 
		Text,			// 
		Image,			// 
		Curve,			// 
		PloyLine,		// 
		BezierCurve,	// 

		MaskFill = 0x80000000,

		FilledRect = Rect | MaskFill,
		FilledEllipse = Ellipse | MaskFill,
		FilledPolygon = Polygon | MaskFill,
		FilledArc = Arc | MaskFill,
		FilledPie = Pie | MaskFill,
		FilledChord = Chord | MaskFill,
		FilledRoundRect = RoundRect | MaskFill,
		FilledText = Text | MaskFill,
	};

	ECK_ENUM_BIT_FLAGS_FRIEND(Type);
private:
	enum
	{
		HTR_PART_BEGIN = INT_MIN,

		HTR_SIZE_LT,
		HTR_SIZE_T,
		HTR_SIZE_RT,
		HTR_SIZE_L,
		HTR_SIZE_R,
		HTR_SIZE_LB,
		HTR_SIZE_B,
		HTR_SIZE_RB,
		HTR_ROTATE,

		HTR_PART_END,
	};
	struct GEO
	{
		std::variant<std::vector<D2D1_POINT_2F>, CRefStrW> vPt{};
		ID2D1Brush* pBrFill{};
		ID2D1Brush* pBrStroke{};
		ID2D1StrokeStyle* pStStroke{};
		ID2D1PathGeometry1* pPath{};
		ID2D1GeometryRealization* pGrStroke{};
		ID2D1GeometryRealization* pGrFill{};
		D2D1::Matrix3x2F mat{};
		D2D1_POINT_2F pt{};
		Type eType{};
		float fAngle{};
		float cxStroke{};
	};
	std::vector<GEO> m_vGeo{};

	ID2D1HwndRenderTarget* m_pRT{};
	ID2D1DeviceContext1* m_pDC1{};

	D2D1_COLOR_F m_crBkg{ ColorrefToD2dColorF(Colorref::White) };
	ID2D1Brush* m_pBrStroke{};
	ID2D1Brush* m_pBrFill{};
	ID2D1StrokeStyle* m_pStStroke{};
	IDWriteTextFormat* m_pTf{};

	ID2D1SolidColorBrush* m_pBrDefStroke{};
	ID2D1SolidColorBrush* m_pBrDefFill{};
	IDWriteTextFormat* m_pTfDef{};

	ID2D1Brush* m_pBrMark{};

	int m_cxClient{};
	int m_cyClient{};

	D2D1_RECT_F m_rcMark{};
	int m_idxMark{ -1 };

	D2D1_POINT_2F m_ptStart{};
	D2D1_RECT_F m_rcSel{};

	float m_cxStroke{ 1.f };

	D2D1_MATRIX_3X2_F m_matWorld{};

	BITBOOL m_bMark : 1 = FALSE;
	BITBOOL m_bDragging : 1 = FALSE;
	BITBOOL m_bDraggingMove : 1 = FALSE;
	BITBOOL m_bDraggingResize : 1 = FALSE;
	BITBOOL m_bDraggingRotate : 1 = FALSE;
	BITBOOL m_bDraggingSel : 1 = FALSE;

	static HRESULT MakePathFromLines(const D2D1_POINT_2F* pPt, size_t cPt,
		ID2D1PathGeometry1*& pPath, BOOL bFill, BOOL bOpen = TRUE)
	{
		EckAssert(cPt >= 2);
		HRESULT hr;
		ID2D1GeometrySink* pSink;
		g_pD2dFactory->CreatePathGeometry(&pPath);
		pPath->Open(&pSink);
		pSink->BeginFigure(pPt[0], bFill ? D2D1_FIGURE_BEGIN_FILLED : D2D1_FIGURE_BEGIN_HOLLOW);
		EckCounter(cPt - 1, i)
			pSink->AddLine(pPt[i + 1]);
		pSink->EndFigure(bOpen ? D2D1_FIGURE_END_OPEN : D2D1_FIGURE_END_CLOSED);
		if (FAILED(hr = pSink->Close()))
		{
			pSink->Release();
			pPath->Release();
			pPath = NULL;
			return hr;
		}
		pSink->Release();
		return S_OK;
	}

	static HRESULT MakePathFromLines(const std::vector<D2D1_POINT_2F>& vPt, ID2D1PathGeometry1*& pPath,
		BOOL bFill, BOOL bOpen = TRUE)
	{
		return MakePathFromLines(vPt.data(), vPt.size(), pPath, bFill, bOpen);
	}

	void RedrawGeometry(const D2D1_RECT_F* prcPaint)
	{
		m_pDC1->BeginDraw();
		m_pDC1->Clear(m_crBkg);
		for (auto& e : m_vGeo)
		{
			const auto mat = m_matWorld * e.mat;
			m_pDC1->SetTransform(mat);
			if ((e.eType & Type::MaskFill) != Type::Zero)
			{
				if (!e.pGrFill)
				{
					m_pDC1->CreateFilledGeometryRealization(e.pPath,
						D2D1::ComputeFlatteningTolerance(mat), &e.pGrFill);
					EckAssert(e.pGrFill);
				}
				m_pDC1->DrawGeometryRealization(e.pGrFill, e.pBrFill);
			}

			if (e.pBrStroke)
			{
				if (!e.pGrStroke)
				{
					m_pDC1->CreateStrokedGeometryRealization(e.pPath,
						D2D1::ComputeFlatteningTolerance(mat),
						e.cxStroke, e.pStStroke, &e.pGrStroke);
					EckAssert(e.pGrStroke);
				}
				m_pDC1->DrawGeometryRealization(e.pGrStroke, e.pBrStroke);
			}
		}
		m_pDC1->SetTransform(D2D1::Matrix3x2F::Identity());
		if (m_bMark)
			m_pDC1->DrawRectangle(m_rcMark, m_pBrMark, 2.5f);
		m_pDC1->EndDraw();
	}
public:
	ECK_CWND_CREATE;
	HWND Create(PCWSTR pszText, DWORD dwStyle, DWORD dwExStyle,
		int x, int y, int cx, int cy, HWND hParent, HMENU hMenu, PCVOID pData = NULL) override
	{
		return IntCreate(dwExStyle, WCN_VECDRAWPANEL, pszText, dwStyle,
			x, y, cx, cy, hParent, hMenu, eck::g_hInstance, this);
	}

	LRESULT OnMsg(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) override
	{
		switch (uMsg)
		{
		case WM_LBUTTONDOWN:
		{
			const auto idx = HitTestI(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
			if (idx >= 0)
			{
				const auto& e = m_vGeo[idx];
				D2D1_RECT_F rcBound;
				const auto hr = e.pPath->GetBounds(m_matWorld * e.mat, &rcBound);
				if (rcBound.left > rcBound.right || rcBound.top > rcBound.bottom || FAILED(hr))
					e.pPath->GetWidenedBounds(e.cxStroke, NULL, m_matWorld * e.mat, &rcBound);
				if (m_bMark)
				{
					auto rc{ m_rcMark };
					m_rcMark = rcBound;
					UnionRect(rc, rc, m_rcMark);
					rc.left -= 3;
					rc.top -= 3;
					rc.right += 3;
					rc.bottom += 3;
					Redraw(rc);
				}
				else
				{
					m_bMark = TRUE;
					auto rc{ rcBound };
					m_rcMark = rcBound;
					rc.left -= 3;
					rc.top -= 3;
					rc.right += 3;
					rc.bottom += 3;
					Redraw(rc);
				}
			}
			else
			{
				if (m_bMark)
				{
					m_bMark = FALSE;
					m_rcMark.left -= 3;
					m_rcMark.top -= 3;
					m_rcMark.right += 3;
					m_rcMark.bottom += 3;
					Redraw(m_rcMark);
				}
			}
			if (IsMouseMovedBeforeDragging(hWnd, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)))
			{
				if (!IsValid())
					return 0;
				m_bDragging = TRUE;

			}
		}
		return 0;
		case WM_PAINT:
		{
			PAINTSTRUCT ps;
			BeginPaint(hWnd, &ps);
			Redraw(ps.rcPaint);
			EndPaint(hWnd, &ps);
		}
		return 0;
		case WM_SIZE:
		{
			ECK_GET_SIZE_LPARAM(m_cxClient, m_cyClient, lParam);
			m_pRT->Resize({ (UINT)m_cxClient, (UINT)m_cyClient });
			Redraw();
		}
		return 0;
		case WM_CREATE:
		{
			RECT rc;
			GetClientRect(hWnd, &rc);
			auto PropRt = D2D1::RenderTargetProperties();
			PropRt.pixelFormat = D2D1_PIXEL_FORMAT(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_IGNORE);
			PropRt.dpiX = 96.f;
			PropRt.dpiY = 96.f;
			g_pD2dFactory->CreateHwndRenderTarget(PropRt,
				D2D1::HwndRenderTargetProperties(hWnd, { (UINT)rc.right, (UINT)rc.bottom }), &m_pRT);
			m_pRT->QueryInterface(&m_pDC1);
			Redraw();
			m_pDC1->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Black), &m_pBrDefStroke);
			m_pDC1->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::White), &m_pBrDefFill);
			SetCurrStrokeBrush(m_pBrDefStroke);
			SetCurrFillBrush(m_pBrDefFill);

			ID2D1SolidColorBrush* pBr;
			m_pDC1->CreateSolidColorBrush(ColorrefToD2dColorF(Colorref::CyanBlue), &pBr);
			m_pBrMark = pBr;
			m_matWorld = D2D1::Matrix3x2F::Identity();

			g_pDwFactory->CreateTextFormat(L"微软雅黑", NULL, DWRITE_FONT_WEIGHT_NORMAL,
				DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, 70.f, L"zh-cn", &m_pTfDef);
			SetCurrTextFormat(m_pTfDef);
		}
		break;
		}
		return CWnd::OnMsg(hWnd, uMsg, wParam, lParam);
	}

	EckInline constexpr auto GetPDC() const { return m_pDC1; }

	void SetCurrStrokeBrush(ID2D1Brush* pBr)
	{
		std::swap(m_pBrStroke, pBr);
		if (m_pBrStroke)
			m_pBrStroke->AddRef();
		if (pBr)
			pBr->Release();
	}

	void SetCurrFillBrush(ID2D1Brush* pBr)
	{
		std::swap(m_pBrFill, pBr);
		if (m_pBrFill)
			m_pBrFill->AddRef();
		if (pBr)
			pBr->Release();
	}

	void SetCurrStrokeStyle(ID2D1StrokeStyle* pSt)
	{
		std::swap(m_pStStroke, pSt);
		if (m_pStStroke)
			m_pStStroke->AddRef();
		if (pSt)
			pSt->Release();
	}

	void SetCurrTextFormat(IDWriteTextFormat* pTf)
	{
		std::swap(m_pTf, pTf);
		if (m_pTf)
			m_pTf->AddRef();
		if (pTf)
			pTf->Release();
	}

	int HitTest(float x, float y) const
	{
		if (m_vGeo.empty())
			return -1;
		if (m_bMark)
		{

		}
		BOOL b;
		for (int i = (int)m_vGeo.size() - 1; i >= 0; --i)
		{
			const auto& e = m_vGeo[i];
			if ((e.eType & Type::MaskFill) != Type::Zero)
			{
				e.pPath->FillContainsPoint({ x, y }, m_matWorld * e.mat, &b);
				if (b)
					return (int)i;
			}
			if (e.pBrStroke)
			{
				e.pPath->StrokeContainsPoint({ x, y },
					e.cxStroke * 4, e.pStStroke, m_matWorld * e.mat, &b);
				if (b)
					return (int)i;
			}
		}
		return -1;
	}

	EckInline int HitTestI(int x, int y)
	{
		return HitTest((float)x, (float)y);
	}

	HRESULT AddLine(float x1, float y1, float x2, float y2, int pos = -1)
	{
		EckAssert(m_pBrStroke);
		const D2D1_POINT_2F ptOffset = { std::min(x1, x2), std::min(y1, y2) };

		std::vector<D2D1_POINT_2F> vPt(2u);
		vPt[0] = D2D1_POINT_2F{ x1,y1 } - ptOffset;
		vPt[1] = D2D1_POINT_2F{ x2,y2 } - ptOffset;

		ID2D1PathGeometry1* pPath;
		if (HRESULT hr; FAILED(hr = MakePathFromLines(vPt, pPath, FALSE)))
			return hr;

		auto& e = ((pos >= 0) ?
			(*m_vGeo.emplace(m_vGeo.begin() + pos, std::move(vPt))) :
			m_vGeo.emplace_back(std::move(vPt)));
		e.pt = ptOffset;
		e.eType = Type::Line;
		e.pBrStroke = m_pBrStroke;
		e.pBrStroke->AddRef();
		e.pPath = pPath;
		e.cxStroke = m_cxStroke;
		e.mat = D2D1::Matrix3x2F::Translation(ptOffset.x, ptOffset.y);
		return S_OK;
	}

	HRESULT AddRect(float x, float y, float w, float h, BOOL bFill = FALSE, int pos = -1)
	{
		EckAssert(w >= 0 && h >= 0);
		const D2D1_POINT_2F ptOffset = { x,y };

		std::vector<D2D1_POINT_2F> vPt(4u);
		vPt[1] = { w,0.f };
		vPt[2] = { w,h };
		vPt[3] = { 0.f,h };

		ID2D1PathGeometry1* pPath;
		if (HRESULT hr; FAILED(hr = MakePathFromLines(vPt, pPath, bFill, FALSE)))
			return hr;

		auto& e = ((pos >= 0) ?
			(*m_vGeo.emplace(m_vGeo.begin() + pos, std::move(vPt))) :
			m_vGeo.emplace_back(std::move(vPt)));
		e.pt = ptOffset;
		e.pBrStroke = m_pBrStroke;
		if (bFill)
		{
			e.eType = Type::FilledRect;
			e.pBrFill = m_pBrFill;
			e.pBrFill->AddRef();
			if (e.pBrStroke)
				e.pBrStroke->AddRef();
		}
		else
		{
			EckAssert(m_pBrStroke);
			e.eType = Type::Rect;
			e.pBrStroke->AddRef();
		}
		e.pPath = pPath;
		e.cxStroke = m_cxStroke;
		e.mat = D2D1::Matrix3x2F::Translation(ptOffset.x, ptOffset.y);
		return S_OK;
	}

	HRESULT AddEllipse(float x, float y, float w, float h, BOOL bFill = FALSE, int pos = -1)
	{
		const D2D1_POINT_2F ptOffset = { x,y };

		std::vector<D2D1_POINT_2F> vPt(4u);
		vPt[1] = { w,0.f };
		vPt[2] = { w,h };
		vPt[3] = { 0.f,h };

		HRESULT hr;
		ID2D1PathGeometry1* pPath;
		ComPtr<ID2D1GeometrySink> pSink;
		g_pD2dFactory->CreatePathGeometry(&pPath);
		pPath->Open(&pSink);
		pSink->BeginFigure({ w / 2.f,0.f }, bFill ? D2D1_FIGURE_BEGIN_FILLED : D2D1_FIGURE_BEGIN_HOLLOW);
		D2D1_ARC_SEGMENT Arc
		{
			.point = { w / 2.f,h },
			.size = { w / 2.f,h / 2.f },
			.rotationAngle = 0.f,
			.sweepDirection = D2D1_SWEEP_DIRECTION_CLOCKWISE,
			.arcSize = D2D1_ARC_SIZE_LARGE
		};
		pSink->AddArc(Arc);
		Arc.point.y = 0.f;
		pSink->AddArc(Arc);
		pSink->EndFigure(D2D1_FIGURE_END_CLOSED);
		if (FAILED(hr = pSink->Close()))
		{
			pPath->Release();
			return hr;
		}

		auto& e = ((pos >= 0) ?
			(*m_vGeo.emplace(m_vGeo.begin() + pos, std::move(vPt))) :
			m_vGeo.emplace_back(std::move(vPt)));
		e.pt = ptOffset;
		e.pBrStroke = m_pBrStroke;
		if (bFill)
		{
			e.eType = Type::FilledEllipse;
			e.pBrFill = m_pBrFill;
			e.pBrFill->AddRef();
			if (e.pBrStroke)
				e.pBrStroke->AddRef();
		}
		else
		{
			e.eType = Type::Ellipse;
			e.pBrStroke->AddRef();
		}
		e.pPath = pPath;
		e.cxStroke = m_cxStroke;
		e.mat = D2D1::Matrix3x2F::Translation(ptOffset.x, ptOffset.y);
		return S_OK;
	}

	HRESULT AddPolygon(const D2D1_POINT_2F* pPt, size_t cPt, BOOL bFill = FALSE, int pos = -1)
	{
		EckAssert(m_pBrStroke && pPt && cPt >= 3);

		D2D1_POINT_2F ptOffset{ FLT_MAX,FLT_MAX };
		std::vector<D2D1_POINT_2F> vPt(pPt, pPt + cPt);
		EckCounter(cPt, i)
		{
			ptOffset.x = std::min(ptOffset.x, pPt[i].x);
			ptOffset.y = std::min(ptOffset.y, pPt[i].y);
		}
		EckCounter(cPt, i)
			vPt[i] -= ptOffset;

		ID2D1PathGeometry1* pPath;
		if (HRESULT hr; FAILED(hr = MakePathFromLines(vPt, pPath, bFill, FALSE)))
			return hr;

		auto& e = ((pos >= 0) ?
			(*m_vGeo.emplace(m_vGeo.begin() + pos, std::move(vPt))) :
			m_vGeo.emplace_back(std::move(vPt)));
		e.pt = ptOffset;
		e.pBrStroke = m_pBrStroke;
		if (bFill)
		{
			e.eType = Type::FilledPolygon;
			e.pBrFill = m_pBrFill;
			e.pBrFill->AddRef();
			if (e.pBrStroke)
				e.pBrStroke->AddRef();
		}
		else
		{
			e.eType = Type::Polygon;
			e.pBrStroke->AddRef();
		}
		e.pPath = pPath;
		e.cxStroke = m_cxStroke;
		e.mat = D2D1::Matrix3x2F::Translation(ptOffset.x, ptOffset.y);
		return S_OK;
	}

	HRESULT AddText(PCWSTR pszText, int cchText, float x, float y, BOOL bFill = FALSE, int pos = -1)
	{
		EckAssert(m_pBrStroke && pszText);
		HRESULT hr;
		ComPtr<IDWriteTextLayout> pLayout;
		if (cchText < 0)
			cchText = (int)wcslen(pszText);
		if (FAILED(hr = g_pDwFactory->CreateTextLayout(pszText, (UINT32)cchText,
			m_pTf, m_cxClient, m_cyClient, &pLayout)))
			return hr;
		ID2D1PathGeometry1* pPath;
		if (FAILED(hr = GetTextLayoutPathGeometry(pLayout.Get(), m_pDC1, 0.f, 0.f, pPath)))
			return hr;

		auto& e = ((pos >= 0) ?
			(*m_vGeo.emplace(m_vGeo.begin() + pos, CRefStrW(pszText, cchText))) :
			m_vGeo.emplace_back(CRefStrW(pszText, cchText)));
		e.pt = { x,y };
		e.pBrStroke = m_pBrStroke;
		if (bFill)
		{
			e.eType = Type::FilledText;
			e.pBrFill = m_pBrFill;
			e.pBrFill->AddRef();
			if (e.pBrStroke)
				e.pBrStroke->AddRef();
		}
		else
		{
			e.eType = Type::Text;
			e.pBrStroke->AddRef();
		}
		e.pPath = pPath;
		e.cxStroke = m_cxStroke;
		e.mat = D2D1::Matrix3x2F::Translation(x, y);
		return S_OK;
	}

	void Redraw()
	{
		RedrawGeometry(NULL);
	}

	void Redraw(const D2D1_RECT_F& rcPaint)
	{
		RedrawGeometry(&rcPaint);
	}

	void Redraw(const RECT& rcPaint)
	{
		const auto rc = MakeD2DRcF(rcPaint);
		RedrawGeometry(&rc);
	}

	auto GetDefaultStrokeBrush() const { return m_pBrDefStroke; }

	auto GetDefaultFillBrush() const { return m_pBrDefFill; }
};

ECK_ENUM_BIT_FLAGS(CVectorDrawPanel::Type);
ECK_NAMESPACE_END