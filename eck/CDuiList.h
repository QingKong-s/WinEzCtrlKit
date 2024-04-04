/*
* WinEzCtrlKit Library
*
* CDuiList.h ： DUI列表
*
* Copyright(C) 2024 QingKong
*/
#pragma once
#include "DuiBase.h"
#include "CInertialScrollView.h"
#include "CD2dImageList.h"

#include <d2d1_2.h>

ECK_NAMESPACE_BEGIN
ECK_DUI_NAMESPACE_BEGIN
enum
{
	LEE_GETDISPINFO = EE_PRIVATE_BEGIN,
};

enum
{
	LEIM_TEXT = (1u << 0),
	LEIM_IMAGE = (1u << 1),
};

struct LEEDISPINFO
{
	UINT uFlags = 0;

	int idx = 0;

	int cchText = 0;
	PCWSTR pszText = NULL;

	ID2D1Bitmap* pImg = NULL;

	int idxImg = -1;
};

struct LEHITTEST
{
	POINT pt;
	BITBOOL bSBThumb : 1;
};

enum
{
	LEIF_SELECTED = (1u << 0),
};

class CList :public CElem
{
private:
	struct ITEM
	{
		IDWriteTextLayout* pLayout = NULL;
		UINT uFlags = 0;
		float cxText = 0.f;
	};

	std::vector<ITEM> m_vItem{};// 项目
	int m_idxTop = 0;			// 第一个可见项
	int m_idxHot = -1;			// 热点项
	int m_idxSel = -1;			// 选中的项的索引，仅用于单选
	int m_idxInsertMark = -1;	// 插入标记应当显示在哪一项之前
	int m_idxFocus = -1;		// 焦点项
	int m_idxMark = -1;			// 标记项

	int m_cyItem = 0;			// 项目高度
	int m_cyPadding = 0;		// 项目间距
	int m_oyTopItem = 0;		// 小于等于零的值，指示第一可见项的遮挡高度

	int m_cxyImage = 0;			// 图像大小

	int m_cyTopExtra = 0;
	int m_cyBottomExtra = 0;

	ID2D1DeviceContext1* m_pDC1 = NULL;// for geometry realization

	ID2D1SolidColorBrush* m_pBrush = NULL;
	IDWriteTextFormat* m_pTf = NULL;
	ID2D1GeometryRealization* m_pGrInsertMark = NULL;

	CInertialScrollView* m_psv = NULL;
	CD2dImageList* m_pImgList = NULL;

	BITBOOL m_bHoverThumb : 1 = FALSE;
	BITBOOL m_bThumbDrag : 1 = FALSE;
	BITBOOL m_bSingleSel : 1 = FALSE;

	CEasingCurve* m_pecThumb = NULL;

	ECK_DS_BEGIN(DPIS)
		ECK_DS_ENTRY_F(cxSBThumb, 8.f)
		ECK_DS_ENTRY_F(cxSBThumbSmall, 4.f)
		ECK_DS_ENTRY_F(minSBThumbSize, 20.f)
		ECK_DS_ENTRY_F(cyInsertMark, 2.f)
		;
	ECK_DS_END_VAR(m_DsF);

	void DrawItem(int idx, const D2D1_RECT_F& rcPaint)
	{
		auto& e = m_vItem[idx];
		LEEDISPINFO es{ LEIM_TEXT | LEIM_IMAGE,idx };
		GenElemNotify(LEE_GETDISPINFO, 0, (LPARAM)&es);
		if (!e.pLayout && es.pszText)
		{
			EckAssert(es.cchText > 0);
			g_pDwFactory->CreateTextLayout(es.pszText, es.cchText, m_pTf,
				GetWidthF() - m_pWnd->GetDs().CommMargin * 2, (float)m_cyItem, &e.pLayout);
			if (e.pLayout)
			{
				DWRITE_TEXT_METRICS tm;
				e.pLayout->GetMetrics(&tm);
				e.cxText = tm.width;
			}
			else
				e.cxText = 0.f;
		}

		const float fRad = m_pWnd->GetDs().CommRrcRadius;
		D2D1_ROUNDED_RECT rrc{ { 0.f,(float)GetItemY(idx),GetViewWidthF() },fRad,fRad};
		rrc.rect.bottom = rrc.rect.top + m_cyItem;

		UINT32 rgb = 0;
		BOOL bDoNotFill = FALSE;
		if ((e.uFlags & LEIF_SELECTED) || (m_bSingleSel && m_idxSel == idx))
			if (m_idxHot == idx)
				rgb = 0xa8cbcb;
			else
				rgb = 0xe3eee8;
		else if (m_idxHot == idx)
			rgb = 0xe0e2da;
		else
			bDoNotFill = TRUE;
		if (!bDoNotFill)
		{
			m_pBrush->SetColor(D2D1::ColorF(rgb));
			m_pDC->FillRoundedRectangle(rrc, m_pBrush);
		}

		const float xImage = m_pWnd->GetDs().CommMargin;
		const float yImage = (float)((m_cyItem - m_cxyImage) / 2);

		auto rcOld = rrc.rect;

		rrc.rect.left += xImage;
		rrc.rect.right = rrc.rect.left + m_cxyImage;
		rrc.rect.top += yImage;
		rrc.rect.bottom = rrc.rect.top + m_cxyImage;
		if (!(rrc.rect.right <= rcPaint.left || rrc.rect.left >= rcPaint.right))
			if (es.pImg)
				m_pDC->DrawBitmap(es.pImg, rrc.rect, 1.f, D2D1_INTERPOLATION_MODE_LINEAR);
			else if (m_pImgList && es.idxImg >= 0)
				m_pImgList->Draw(es.idxImg, rrc.rect);

		const float xText = rrc.rect.right + xImage;
		if (e.pLayout && !(xText + e.cxText <= rcPaint.left || xText >= rcPaint.right))
		{
			m_pBrush->SetColor(D2D1::ColorF(0x000000));
			m_pDC->DrawTextLayout(D2D1::Point2F(xText, rcOld.top), e.pLayout, m_pBrush);
		}
	}

	void ReCalcScroll()
	{
		m_psv->SetMinThumbSize((int)m_DsF.minSBThumbSize);
		m_psv->SetViewSize(GetHeight() - m_cyTopExtra - m_cyBottomExtra);
		m_psv->SetRange(-m_cyTopExtra, (int)m_vItem.size() * (m_cyItem + m_cyPadding) + m_cyBottomExtra);
		m_psv->SetPage(GetHeight());
	}

	void ReCalcTopItem()
	{
		m_idxTop = m_psv->GetPos() / (m_cyItem + m_cyPadding);
		m_oyTopItem = m_idxTop * (m_cyItem + m_cyPadding) - m_psv->GetPos();
	}

	BOOL GetSBThumbRectInClient(RECT& rc)
	{
		const int cyThumb = m_psv->GetThumbSize();
		const int yThumb = m_psv->GetThumbPos(cyThumb);
		if (cyThumb < 0 || yThumb < 0)
			return FALSE;
		rc.left = (long)(GetRectInClient().right - m_DsF.cxSBThumb);
		rc.top = GetRectInClient().top + m_cyTopExtra + yThumb;
		rc.right = (long)(rc.left + m_DsF.cxSBThumb);
		rc.bottom = rc.top + cyThumb;
		return TRUE;
	}

	BOOL GetSBThumbRectInClient(D2D1_RECT_F& rc)
	{
		const int cyThumb = m_psv->GetThumbSize();
		const int yThumb = m_psv->GetThumbPos(cyThumb);
		if (cyThumb < 0 || yThumb < 0)
			return FALSE;
		rc.left = GetRectInClientF().right - m_DsF.cxSBThumb;
		rc.top = GetRectInClientF().top + m_cyTopExtra + yThumb;
		rc.right = rc.left + m_DsF.cxSBThumb;
		rc.bottom = rc.top + cyThumb;
		return TRUE;
	}

	// 元素坐标
	BOOL GetSBThumbRect(RECT& rc) const
	{
		const int cyThumb = m_psv->GetThumbSize();
		const int yThumb = m_psv->GetThumbPos(cyThumb);
		if (cyThumb < 0 || yThumb < 0)
			return FALSE;
		rc.left = GetViewWidth() - (long)m_DsF.cxSBThumb;
		rc.top = m_cyTopExtra + yThumb;
		rc.right = rc.left + (long)m_DsF.cxSBThumb;
		rc.bottom = rc.top + cyThumb;
		return TRUE;
	}

	// 元素坐标
	BOOL GetSBThumbRect(D2D1_RECT_F& rc) const
	{
		const int cyThumb = m_psv->GetThumbSize();
		const int yThumb = m_psv->GetThumbPos(cyThumb);
		if (cyThumb < 0 || yThumb < 0)
			return FALSE;
		rc.left = GetViewWidthF() - m_DsF.cxSBThumb;
		rc.top = (float)(m_cyTopExtra + yThumb);
		rc.right = rc.left + m_DsF.cxSBThumb;
		rc.bottom = rc.top + cyThumb;
		return TRUE;
	}

	// 客户坐标
	void PaintScrollBar()
	{
		D2D1_ROUNDED_RECT rrc;
		rrc.radiusX = (m_bHoverThumb ? m_DsF.cxSBThumb : m_DsF.cxSBThumbSmall) / 2.f;
		rrc.radiusY = m_DsF.cxSBThumb / 2.f;
		if (!GetSBThumbRect(rrc.rect))
			return;

		if (m_pecThumb->IsActive())
			rrc.rect.right  = rrc.rect.left + m_DsF.cxSBThumbSmall+(m_DsF.cxSBThumb - m_DsF.cxSBThumbSmall) * m_pecThumb->GetCurrValue();
		else
			if (!m_bHoverThumb)
				rrc.rect.right = rrc.rect.left + m_DsF.cxSBThumbSmall;
		m_pBrush->SetColor(D2D1::ColorF(m_bHoverThumb ? 0xaeb0aa : 0xc8c9c4));
		m_pDC->FillRoundedRectangle(rrc, m_pBrush);
	}

	void RedrawThumb()
	{
		RECT rc;
		GetSBThumbRectInClient(rc);
		InvalidateRect(&rc);
	}

	void UpdateInsertMarkGeometry()
	{
		ID2D1PathGeometry* pPath;
		g_pD2dFactory->CreatePathGeometry(&pPath);
		EckAssert(pPath);

		ID2D1GeometrySink* pSink;
		pPath->Open(&pSink);
		pSink->BeginFigure(D2D1::Point2F(0, 0), D2D1_FIGURE_BEGIN_FILLED);

		const float cy = m_DsF.cyInsertMark;
		const float cy2 = cy * 2;
		const float cyTotal = cy * 5;
		const float cxElem = GetWidthF();
		const D2D1_POINT_2F pt[]
		{
			//{0,			0},			// 左上
			{cy2,			cy2},		// 左上2
			{cxElem - cy2,	cy2},		// 右上2
			{cxElem,		0},			// 右上
			{cxElem,		cyTotal},	// 右下
			{cxElem - cy2,	cy2 + cy},	// 右下2
			{cy2,			cy2 + cy},	// 左下2
			{0,				cyTotal},	// 左下
			//{0,			0},			// 左上
		};

		pSink->AddLines(pt, ARRAYSIZE(pt));
		pSink->EndFigure(D2D1_FIGURE_END_CLOSED);

		pSink->Close();
		pSink->Release();

		if (m_pGrInsertMark)
			m_pGrInsertMark->Release();
		m_pDC1->CreateFilledGeometryRealization(
			pPath,
			D2D1::ComputeFlatteningTolerance(D2D1::Matrix3x2F::Identity()),
			&m_pGrInsertMark);

		m_pDC1->Release();
	}
public:
	void SetItemCount(int c)
	{
		m_vItem.resize(c);
		ReCalcScroll();
		ReCalcTopItem();
		InvalidateRect();
	}

	LRESULT OnEvent(UINT uMsg, WPARAM wParam, LPARAM lParam) override
	{
		ECK_DUILOCK;
		switch (uMsg)
		{
		case WM_PAINT:
		{
			ELEMPAINTSTRU ps;
			BeginPaint(ps, wParam, lParam);
			m_pBrush->SetColor(D2D1::ColorF(0xffffff));
			m_pDC->FillRectangle(ps.rcfClipInElem, m_pBrush);
			if (!m_vItem.empty())
			{
				const int idxBegin = std::max(ItemFromY((int)ps.rcfClipInElem.top), 0);
				const int idxEnd = std::min(ItemFromY((int)ps.rcfClipInElem.bottom), (int)m_vItem.size() - 1);
				for (int i = idxBegin; i <= idxEnd; ++i)
					DrawItem(i, ps.rcfClipInElem);
				PaintScrollBar();

				if (m_idxInsertMark >= 0)
				{
					D2D1_RECT_F rcIm;
					GetInsertMarkRect(rcIm);
					if (rcIm.bottom > 0.f && rcIm.top < GetHeightF())
					{
						m_pDC1->SetTransform(D2D1::Matrix3x2F::Translation(
							GetRectInClientF().left, GetRectInClientF().top + rcIm.top));
						m_pBrush->SetColor(D2D1::ColorF(0x000000));
						m_pDC1->DrawGeometryRealization(m_pGrInsertMark, m_pBrush);
						m_pDC1->SetTransform(D2D1::Matrix3x2F::Identity());
					}
				}
			}
			EndPaint(ps);
		}
		return 0;

		case WM_MOUSEMOVE:
		{
			LEHITTEST ht{ ECK_GET_PT_LPARAM(lParam) };
			ClientToElem(ht.pt);

			if (m_bThumbDrag)
			{
				m_psv->OnMouseMove(ht.pt.y - m_cyTopExtra);
				ReCalcTopItem();
				InvalidateRect();
				return 0;
			}

			int idx = HitTest(ht);

			if (ht.bSBThumb != m_bHoverThumb)
			{
				m_bHoverThumb = ht.bSBThumb;
				m_pecThumb->SetReverse(!m_bHoverThumb);
				m_pecThumb->Begin(ECBF_CONTINUE);
				GetWnd()->WakeRenderThread();
			}

			if (idx != m_idxHot)
			{
				std::swap(m_idxHot, idx);
				if (idx >= 0)
					RedrawItem(idx);
				if (m_idxHot >= 0)
					RedrawItem(m_idxHot);
			}
		}
		return 0;

		case WM_MOUSELEAVE:
		{
			if (m_idxHot >= 0)
			{
				int idx = -1;
				std::swap(m_idxHot, idx);
				RedrawItem(idx);
			}

			if (!m_bThumbDrag)
				if (m_bHoverThumb)
				{
					m_bHoverThumb = FALSE;
					m_pecThumb->SetReverse(TRUE);
					m_pecThumb->Begin(ECBF_CONTINUE);
					GetWnd()->WakeRenderThread();
				}
		}
		return 0;

		case WM_MOUSEWHEEL:
		{
			m_psv->OnMouseWheel2(-GET_WHEEL_DELTA_WPARAM(wParam) / WHEEL_DELTA);
			GetWnd()->WakeRenderThread();
		}
		return 0;

		case WM_SIZE:
			if (m_idxInsertMark >= 0)
				UpdateInsertMarkGeometry();
			return 0;

		case WM_LBUTTONDOWN:
		{
			SetFocus();
			LEHITTEST ht{ ECK_GET_PT_LPARAM(lParam) };
			ClientToElem(ht.pt);
			int idx = HitTest(ht);

			int idxChangedBegin = -1, idxChangedEnd = -1;
			if (idx >= 0)
			{
				if (!(wParam & MK_CONTROL))
					DeselectAll(idxChangedBegin, idxChangedEnd);
				SelectItemForClick(idx);
				SetRedraw(FALSE);
				if (idxChangedBegin >= 0)
					RedrawItem(idxChangedBegin, idxChangedEnd);
				if (idx < idxChangedBegin || idx > idxChangedEnd)
					RedrawItem(idx);
				SetRedraw(TRUE);
			}
			else if (ht.bSBThumb)
			{
				m_bThumbDrag = TRUE;
				m_psv->OnLButtonDown(ht.pt.y - m_cyTopExtra);
				SetCapture();
			}
		}
		return 0;

		case WM_LBUTTONUP:
		{
			if (m_bThumbDrag)
			{
				m_bThumbDrag = FALSE;
				m_pWnd->ReleaseCaptureElem();
				m_psv->OnLButtonUp();
			}
		}
		return 0;

		case WM_CREATE:
		{
			m_pecThumb = new CEasingCurve{};
			InitEasingCurve(m_pecThumb);
			m_pecThumb->SetAnProc(Easing::OutSine);
			m_pecThumb->SetRange(0.f, 1.f);
			m_pecThumb->SetDuration(160);
			m_pecThumb->SetAnProc(Easing::OutSine);
			m_pecThumb->SetCallBack([](float f, float fOld, LPARAM lParam)
				{
					auto p = (CList*)lParam;
					p->RedrawThumb();
				});

			m_psv = new CInertialScrollView{};
			m_psv->SetCallBack([](int iPos, int iPrevPos, LPARAM lParam)
				{
					auto pThis = (CList*)lParam;
					pThis->ReCalcTopItem();
					pThis->InvalidateRect();
				}, (LPARAM)this);
			GetWnd()->RegisterTimeLine(m_psv);

			UpdateDpiSizeF(m_DsF, m_pWnd->GetDpiValue());

			m_pDC->CreateSolidColorBrush({}, &m_pBrush);
			m_pDC->QueryInterface(&m_pDC1);
		}
		return 0;

		case WM_DESTROY:
		{
			m_idxTop = 0;
			m_idxHot = -1;
			m_idxSel = -1;
			m_idxInsertMark = -1;

			m_cyItem = 0;
			m_cyPadding = 0;
			m_oyTopItem = 0;

			SafeRelease(m_pBrush);
			SafeRelease(m_pTf);
			SafeRelease(m_pDC1);
			for (auto& e : m_vItem)
				SafeRelease(e.pLayout);
			m_vItem.clear();
			SafeRelease(m_pGrInsertMark);
			m_psv->SetRange(0, 0);
			m_pImgList = NULL;
			m_bHoverThumb = FALSE;
			m_bThumbDrag = FALSE;
			m_bSingleSel = FALSE;

			SafeRelease(m_pecThumb);
			SafeRelease(m_psv);
		}
		return 0;
		}
		return CElem::OnEvent(uMsg, wParam, lParam);
	}

	int GetItemY(int idx) const
	{
		return m_oyTopItem + (idx - m_idxTop) * (m_cyItem + m_cyPadding);
	}

	int ItemFromY(int y) const
	{
		return m_idxTop + (y - m_oyTopItem) / (m_cyItem + m_cyPadding);
	}

	void SetItemHeight(int cy)
	{
		m_cyItem = cy;
		ReCalcScroll();
	}

	void SetItemPadding(int cy)
	{
		m_cyPadding = cy;
		ReCalcScroll();
	}

	void GetItemRect(int idx, RECT& rc) const
	{
		rc.left = 0;
		rc.right = GetViewWidth();
		rc.top = GetItemY(idx);
		rc.bottom = rc.top + m_cyItem;
	}

	void GetItemRect(int idx, D2D1_RECT_F& rc) const
	{
		rc.left = 0.f;
		rc.right = GetViewWidthF();
		rc.top = (float)GetItemY(idx);
		rc.bottom = rc.top + m_cyItem;
	}

	// 元素坐标
	int HitTest(LEHITTEST& leht) const
	{
		RECT rc{ 0,0,m_rc.right - m_rc.left,m_rc.bottom - m_rc.top };
		if (!PtInRect(rc, leht.pt) || m_vItem.empty())
			return -1;
		if (GetSBThumbRect(rc))
			if (PtInRect(rc, leht.pt))
			{
				leht.bSBThumb = TRUE;
				return -1;
			}

		const int idx = ItemFromY(leht.pt.y);
		if (idx >= 0 && idx < m_vItem.size())
			return idx;
		else
			return -1;
	}

	void DeselectAll(int& idxChangedBegin, int& idxChangedEnd)
	{
		if (m_bSingleSel)
			m_idxSel = -1;
		else
		{
			int idx0 = -1, idx1 = 0;
			EckCounter((int)m_vItem.size(), i)
			{
				auto& e = m_vItem[i];
				if (e.uFlags & LEIF_SELECTED)
				{
					e.uFlags &= ~LEIF_SELECTED;
					if (idx0 < 0)
						idx0 = i;
					idx1 = i;
				}
			}

			idxChangedBegin = idx0;
			idxChangedEnd = idx1;
		}
	}

	void SelectItemForClick(int idx)
	{
		m_idxFocus = idx;
		m_idxMark = idx;
		if (m_bSingleSel)
			m_idxSel = idx;
		else
			m_vItem[idx].uFlags |= LEIF_SELECTED;
	}

	void SetImageList(CD2dImageList* pImgList)
	{
		m_pImgList = pImgList;
	}

	void SetTextFormat(IDWriteTextFormat* pTf)
	{
		std::swap(m_pTf, pTf);
		if (m_pTf)
			m_pTf->AddRef();
		if (pTf)
			pTf->Release();
	}

	void GetInsertMarkRect(D2D1_RECT_F& rc) const
	{
		if (m_idxInsertMark < 0)
		{
			rc = {};
			return;
		}
		rc.left = 0.f;
		rc.right = GetViewWidthF();
		rc.top = GetItemY(m_idxInsertMark) - m_DsF.cyInsertMark * 2.f;
		rc.bottom = rc.top + m_DsF.cyInsertMark * 5.f;
	}

	void SetInsertMark(int idx)
	{
		if (!m_pGrInsertMark)
			UpdateInsertMarkGeometry();
		m_idxInsertMark = idx;
		D2D1_RECT_F rc;
		GetInsertMarkRect(rc);
		ElemToClient(rc);
		InvalidateRectF(&rc);
	}

	void InvalidateCache(int idx)
	{
		if (idx < 0)
			for (auto& e : m_vItem)
				SafeRelease(e.pLayout);
		else
		{
			EckAssert(idx < m_vItem.size());
			SafeRelease(m_vItem[idx].pLayout);
		}
	}

	void SetImageSize(int cxy)
	{
		if (cxy < 0)
			m_cxyImage = m_cyItem - (int)(m_pWnd->GetDs().CommMargin * 2.f);
		else
			m_cxyImage = cxy;
	}

	void SetTopExtraSpace(int cy)
	{
		m_cyTopExtra = cy;
		ReCalcScroll();
		ReCalcTopItem();
	}

	void SetBottomExtraSpace(int cy)
	{
		m_cyBottomExtra = cy;
		ReCalcScroll();
		ReCalcTopItem();
	}

	void RedrawItem(int idxBegin, int idxEnd)
	{
		RECT rc
		{
			0,
			GetItemY(idxBegin),
			GetViewWidth(),
			GetItemY(idxEnd) + m_cyItem
		};
		ElemToClient(rc);
		InvalidateRect(&rc);
	}

	void RedrawItem(int idx)
	{
		RECT rc;
		GetItemRect(idx, rc);
		ElemToClient(rc);
		InvalidateRect(&rc);
	}
};
ECK_DUI_NAMESPACE_END
ECK_NAMESPACE_END