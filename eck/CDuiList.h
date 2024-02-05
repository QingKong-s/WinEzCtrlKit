/*
* WinEzCtrlKit Library
*
* CDuiList.h ： DUI列表
*
* Copyright(C) 2024 QingKong
*/
#pragma once
#include "DuiBase.h"
#include "CScrollView.h"
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
	};

	std::vector<ITEM> m_vItem{};// 项目
	int m_idxTop = 0;			// 第一个可见项
	int m_idxHot = -1;			// 热点项
	int m_idxSel = -1;			// 选中的项的索引，仅用于单选
	int m_idxInsertMark = -1;	// 插入标记应当显示在哪一项之前

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

	CInertialScrollView m_Sv{};	// 滚动视图
	CD2dImageList* m_pImgList = NULL;

	BITBOOL m_bHoverThumb : 1 = FALSE;
	BITBOOL m_bThumbDrag : 1 = FALSE;
	BITBOOL m_bSingleSel : 1 = FALSE;

	ECK_DS_BEGIN(DPIS)
		ECK_DS_ENTRY_F(cxSBThumb, 8.f)
		ECK_DS_ENTRY_F(cxSBThumbSmall, 4.f)
		ECK_DS_ENTRY_F(minSBThumbSize, 20.f)
		ECK_DS_ENTRY_F(cyInsertMark, 2.f)
		;
	ECK_DS_END_VAR(m_DsF);

	void DrawItem(int idx, const D2D1_RECT_F& rcReal)
	{
		auto& e = m_vItem[idx];
		LEEDISPINFO es{ LEIM_TEXT | LEIM_IMAGE,idx };
		GenElemNotify(LEE_GETDISPINFO, 0, (LPARAM)&es);
		if (!e.pLayout && es.pszText)
		{
			EckAssert(es.cchText > 0);
			g_pDwFactory->CreateTextLayout(es.pszText, es.cchText, m_pTf,
				GetWidthF() - m_pWnd->GetDs().CommMargin * 2, m_cyItem, &e.pLayout);
		}

		const float fRad = m_pWnd->GetDs().CommRrcRadius;
		D2D1_ROUNDED_RECT rrc{ { rcReal.left,rcReal.top + GetItemY(idx),rcReal.right },fRad,fRad };
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
		const float yImage = (m_cyItem - m_cxyImage) / 2;

		auto rcOld = rrc.rect;

		rrc.rect.left += xImage;
		rrc.rect.right = rrc.rect.left + m_cxyImage;
		rrc.rect.top += yImage;
		rrc.rect.bottom = rrc.rect.top + m_cxyImage;
		if(es.pImg)
			m_pDC->DrawBitmap(es.pImg, rrc.rect, 1.f, D2D1_INTERPOLATION_MODE_LINEAR);
		else if (m_pImgList && es.idxImg >= 0)
			m_pImgList->Draw(es.idxImg, rrc.rect);

		if (e.pLayout)
		{
			m_pBrush->SetColor(D2D1::ColorF(0x000000));
			m_pDC->DrawTextLayout(D2D1::Point2F(
				rrc.rect.right + xImage, 
				rcOld.top),
				e.pLayout, m_pBrush);
		}
	}

	void ReCalcScroll()
	{
		m_Sv.SetMinThumbSize(m_DsF.minSBThumbSize);
		m_Sv.SetViewSize(m_rc.bottom - m_rc.top);
		m_Sv.SetRange(-m_cyTopExtra, m_vItem.size() * (m_cyItem + m_cyPadding) + m_cyBottomExtra);
		m_Sv.SetPage(m_rc.bottom - m_rc.top);
	}

	void ReCalcTopItem()
	{
		m_idxTop = m_Sv.GetPos() / (m_cyItem + m_cyPadding);
		m_oyTopItem = m_idxTop * (m_cyItem + m_cyPadding) - m_Sv.GetPos();
	}

	BOOL GetSBThumbRectFromClientRect(D2D1_RECT_F& rc)
	{
		const int cyThumb = m_Sv.GetThumbSize();
		const int yThumb = m_Sv.GetThumbPos(cyThumb);
		if (cyThumb < 0 || yThumb < 0)
			return FALSE;
		rc.left = rc.right - m_DsF.cxSBThumb;
		rc.top += yThumb;
		rc.right = rc.left + m_DsF.cxSBThumb;
		rc.bottom = rc.top + cyThumb;
		return TRUE;
	}

	// 元素坐标
	BOOL GetSBThumbRect(RECT& rc) const
	{
		const int cyThumb = m_Sv.GetThumbSize();
		const int yThumb = m_Sv.GetThumbPos(cyThumb);
		if (cyThumb < 0 || yThumb < 0)
			return FALSE;
		rc.left = m_rc.right - m_rc.left - m_DsF.cxSBThumb;
		rc.top = yThumb;
		rc.right = rc.left + m_DsF.cxSBThumb;
		rc.bottom = rc.top + cyThumb;
		return TRUE;
	}

	// 客户坐标
	void PaintScrollBar(const D2D1_RECT_F& rcReal)
	{
		const int cxThumb = (m_bHoverThumb ? m_DsF.cxSBThumb : m_DsF.cxSBThumbSmall);
		D2D1_ROUNDED_RECT rrc{ rcReal,cxThumb / 2.f,m_DsF.cxSBThumb / 2.f };
		if (!GetSBThumbRectFromClientRect(rrc.rect))
			return;
		if (!m_bHoverThumb)
			rrc.rect.right = rrc.rect.left + m_DsF.cxSBThumbSmall;
		m_pBrush->SetColor(D2D1::ColorF(m_bHoverThumb ? 0xaeb0aa : 0xc8c9c4));
		m_pDC->FillRoundedRectangle(rrc, m_pBrush);
	}

	void RedrawThumb()
	{
		D2D1_RECT_F rc = m_rcf;
		if (m_pParent)
			m_pParent->ElemToParent(rc);
		GetSBThumbRectFromClientRect(rc);
		m_pWnd->Redraw(rc);
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
		Redraw();
	}

	LRESULT OnEvent(UINT uMsg, WPARAM wParam, LPARAM lParam) override
	{
		switch (uMsg)
		{
		case WM_MOUSEMOVE:
		{
			LEHITTEST ht{ ECK_GET_PT_LPARAM(lParam) };
			ParentToElem(ht.pt);

			if (m_bThumbDrag)
			{
				m_Sv.OnMouseMove(ht.pt.y);
				ReCalcTopItem();
				Redraw();
				return 0;
			}

			int idx = HitTest(ht);
			
			if (ht.bSBThumb != m_bHoverThumb)
			{
				m_bHoverThumb = ht.bSBThumb;
				RedrawThumb();
			}

			if (idx != m_idxHot)
			{
				std::swap(m_idxHot, idx);
				RECT rc;
				if (idx >= 0)
				{
					GetItemRect(idx, rc);
					ElemToParent(rc);
					m_pWnd->Redraw(rc);
				}
				if (m_idxHot >= 0)
				{
					GetItemRect(m_idxHot, rc);
					ElemToParent(rc);
					m_pWnd->Redraw(rc);
				}
			}
		}
		return 0;

		case WM_MOUSELEAVE:
		{
			if (m_idxHot >= 0)
			{
				int idx = -1;
				std::swap(m_idxHot, idx);
				RECT rc;
				GetItemRect(idx, rc);
				ElemToParent(rc);
				m_pWnd->Redraw(rc);
			}

			if (m_bHoverThumb)
			{
				m_bHoverThumb = FALSE;
				RedrawThumb();
			}
		}
		return 0;

		case WM_MOUSEWHEEL:
		{
			m_Sv.OnMouseWheel2(-GET_WHEEL_DELTA_WPARAM(wParam) / WHEEL_DELTA,
				[](int iPos, int iPrevPos, LPARAM lParam)
				{
					auto pThis = (CList*)lParam;
					pThis->ReCalcTopItem();
					pThis->Redraw();
				}, (LPARAM)this);
		}
		return 0;

		case WM_SIZE:
			if (m_idxInsertMark >= 0)
				UpdateInsertMarkGeometry();
			return 0;

		case WM_LBUTTONDOWN:
		{
			LEHITTEST ht{ ECK_GET_PT_LPARAM(lParam) };
			ParentToElem(ht.pt);
			int idx = HitTest(ht);
			if (idx >= 0)
			{
				if (!(wParam & MK_CONTROL))
					DeselectAll();
				SelectItemForClick(idx);
				Redraw();
			}
			else if (ht.bSBThumb)
			{
				m_bThumbDrag = TRUE;
				m_Sv.OnLButtonDown(ht.pt.y);
				m_pWnd->SetCaptureElem(this);
			}
		}
		return 0;

		case WM_LBUTTONUP:
		{
			if (m_bThumbDrag)
			{
				m_bThumbDrag = FALSE;
				m_pWnd->ReleaseCaptureElem();
				m_Sv.OnLButtonUp();
			}
		}
		return 0;

		case WM_CREATE:
		{
			UpdateDpiSizeF(m_DsF, m_pWnd->GetDpiValue());
			m_Sv.SetHWND(m_pWnd->HWnd);
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
			m_Sv.SetRange(0, 0);
			m_pImgList = NULL;
			m_bHoverThumb = FALSE;
			m_bThumbDrag = FALSE;
			m_bSingleSel = FALSE;
		}
		return 0;
		}
		return CElem::OnEvent(uMsg, wParam, lParam);
	}

	void OnRedraw(const D2D1_RECT_F& rcClip, float ox, float oy) override
	{
		ECK_DUI_BEGINREDRAW;
		m_pBrush->SetColor(D2D1::ColorF(0xffffff));
		m_pDC->FillRectangle(rcClip, m_pBrush);
		if (!m_vItem.empty())
		{
			auto rcClipElem = MakeRect(rcClip);
			ParentToElem(rcClipElem);
			const int idxBegin = std::max(ItemFromY(rcClipElem.top - oy), 0);
			const int idxEnd = std::min(ItemFromY(rcClipElem.bottom - oy) + 1, (int)m_vItem.size() - 1);
			for (size_t i = idxBegin; i <= idxEnd; ++i)
				DrawItem(i, rc);
			PaintScrollBar(rc);

			if (m_idxInsertMark >= 0)
			{
				D2D1_RECT_F rcIm;
				GetInsertMarkRect(rcIm);
				if (rcIm.bottom > 0.f && rcIm.top < GetHeightF())
				{
					m_pDC1->SetTransform(D2D1::Matrix3x2F::Translation(rc.left, rc.top + rcIm.top));
					m_pBrush->SetColor(D2D1::ColorF(0x000000));
					m_pDC1->DrawGeometryRealization(m_pGrInsertMark, m_pBrush);
					m_pDC1->SetTransform(D2D1::Matrix3x2F::Identity());
				}
			}
		}
		ECK_DUI_ENDREDRAW;
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
		rc.right = m_rc.right - m_rc.left;
		rc.top = GetItemY(idx);
		rc.bottom = rc.top + m_cyItem;
	}

	void GetItemRect(int idx, D2D1_RECT_F& rc) const
	{
		rc.left = 0.f;
		rc.right = m_rcf.right - m_rcf.left;
		rc.top = GetItemY(idx);
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

	void DeselectAll()
	{
		if (m_bSingleSel)
			m_idxSel = -1;
		else
			for (auto& e : m_vItem)
				e.uFlags &= ~LEIF_SELECTED;
	}

	void SelectItemForClick(int idx)
	{
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
		rc.right = m_rc.right - m_rc.left;
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
		m_pWnd->Redraw(rc);
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
			m_cxyImage = m_cyItem - m_pWnd->GetDs().CommMargin * 2;
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
};
ECK_DUI_NAMESPACE_END
ECK_NAMESPACE_END