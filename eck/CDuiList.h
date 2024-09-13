/*
* WinEzCtrlKit Library
*
* CDuiList.h ： DUI列表
*
* Copyright(C) 2024 QingKong
*/
#pragma once
#if ECKCXX20
#include "DuiBase.h"
#include "CInertialScrollView.h"
#include "CD2dImageList.h"

#include "CDuiScrollBar.h"

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
	DUINMHDR nmhdr{ LEE_GETDISPINFO };

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
};

enum
{
	LEIF_SELECTED = (1u << 0),
};

enum class ListType
{
	List,
	Icon,
};


class CList :public CElem
{
private:
	struct ITEM
	{
		IDWriteTextLayout* pLayout = NULL;
		UINT uFlags = 0;
		float cxText = 0.f;
		float cyText = 0.f;
	};

	std::vector<ITEM> m_vItem{};// 项目
	//-------------------通用
	int m_idxHot = -1;			// 热点项
	int m_idxSel = -1;			// 选中的项的索引，仅用于单选
	int m_idxInsertMark = -1;	// 插入标记应当显示在哪一项之前
	int m_idxFocus = -1;		// 焦点项
	int m_idxMark = -1;			// 标记项

	int m_cyTopExtra = 0;
	int m_cyBottomExtra = 0;

	int m_cyItem = 0;			// 项目高度
	int m_cyPadding = 0;		// 项目间距

	int m_cxImage = 0;			// 图像大小
	int m_cyImage = 0;			// 图像大小


	int m_oyTopItem = 0;		// 小于等于零的值，指示第一可见项的遮挡高度
	int m_idxTop = 0;			// 第一个可见项
	//-------------------列表模式

	//-------------------图标模式
	int m_cxItem = 0;
	int m_cxPadding = 0;
	int m_cItemPerRow = 0;

	POINT m_ptDragSelStart{};
	RECT m_rcDragSel{};
	int m_dCursorToItemMax = 0;


	ID2D1DeviceContext1* m_pDC1 = NULL;// for geometry realization

	ID2D1SolidColorBrush* m_pBrush = NULL;
	IDWriteTextFormat* m_pTf = NULL;
	ID2D1GeometryRealization* m_pGrInsertMark = NULL;

	CScrollBar m_SB{};
	CInertialScrollView* m_psv = NULL;
	CD2dImageList* m_pImgList = NULL;

	ListType m_eView = ListType::Icon;

	BITBOOL m_bSingleSel : 1 = FALSE;

	BITBOOL m_bDraggingSel : 1 = FALSE;

	ECK_DS_BEGIN(DPIS)
		ECK_DS_ENTRY_F(cxSBThumb, 8.f)
		ECK_DS_ENTRY_F(cxSBThumbSmall, 4.f)
		ECK_DS_ENTRY_F(minSBThumbSize, 20.f)
		ECK_DS_ENTRY_F(cyInsertMark, 2.f)
		;
	ECK_DS_END_VAR(m_DsF);

	void LVPaintItem(int idx, const D2D1_RECT_F& rcPaint, const LEEDISPINFO& es)
	{
		auto& e = m_vItem[idx];
		if (!e.pLayout && es.pszText)
		{
			EckAssert(es.cchText > 0);
			g_pDwFactory->CreateTextLayout(es.pszText, es.cchText, m_pTf,
				GetWidthF() - m_pWnd->GetDs().CommMargin * 3 - m_cxImage, (float)m_cyItem, &e.pLayout);
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
		D2D1_RECT_F rc{ 0.f,(float)GetItemY(idx),GetWidthF() };
		rc.bottom = rc.top + m_cyItem;

		const auto& cr = m_pColorTheme->Get();
		BOOL bDoNotFill = FALSE;
		if ((e.uFlags & LEIF_SELECTED) || (m_bSingleSel && m_idxSel == idx))
			if (m_idxHot == idx)
				m_pBrush->SetColor(cr.crBkHotSel);
			else
				m_pBrush->SetColor(cr.crBkSelected);
		else if (m_idxHot == idx)
			m_pBrush->SetColor(cr.crBkHot);
		else
			bDoNotFill = TRUE;

		if (!bDoNotFill)
			m_pDC->FillRectangle(rc, m_pBrush);

		const float xImage = m_pWnd->GetDs().CommMargin;
		const float yImage = (float)((m_cyItem - m_cyImage) / 2);

		auto rcOld = rc;

		rc.left += xImage;
		rc.right = rc.left + m_cxImage;
		rc.top += yImage;
		rc.bottom = rc.top + m_cyImage;
		if (!(rc.right <= rcPaint.left || rc.left >= rcPaint.right))
			if (es.pImg)
				m_pDC->DrawBitmap(es.pImg, rc, 1.f, D2D1_INTERPOLATION_MODE_LINEAR);
			else if (m_pImgList && es.idxImg >= 0)
				m_pImgList->Draw(es.idxImg, rc);

		const float xText = rc.right + xImage;
		if (e.pLayout && !(xText + e.cxText <= rcPaint.left || xText >= rcPaint.right))
		{
			m_pBrush->SetColor(cr.crTextNormal);
			m_pDC->DrawTextLayout(D2D1::Point2F(xText, rcOld.top), e.pLayout, m_pBrush);
		}
	}

	void IVPaintItem(int idx, const D2D1_RECT_F& rcPaint, const LEEDISPINFO& es)
	{
		auto& e = m_vItem[idx];

		D2D1_RECT_F rc;
		GetItemRect(idx, rc);

		const auto& cr = m_pColorTheme->Get();
		BOOL bDoNotFill = FALSE;
		if ((e.uFlags & LEIF_SELECTED) || (m_bSingleSel && m_idxSel == idx))
			if (m_idxHot == idx)
				m_pBrush->SetColor(cr.crBkHotSel);
			else
				m_pBrush->SetColor(cr.crBkSelected);
		else if (m_idxHot == idx)
			m_pBrush->SetColor(cr.crBkHot);
		else
			bDoNotFill = TRUE;

		if (!bDoNotFill)
			m_pDC->FillRectangle(rc, m_pBrush);

		D2D1_RECT_F rcImg
		{
			rc.left + (m_cxItem - m_cxImage) / 2.f,
			rc.top + m_pWnd->GetDs().CommMargin
		};
		rcImg.right = rcImg.left + m_cxImage;
		rcImg.bottom = rcImg.top + m_cyImage;
		if (!(rcImg.right <= rcPaint.left || rcImg.left >= rcPaint.right))
			if (es.pImg)
				m_pDC->DrawBitmap(es.pImg, rcImg, 1.f, D2D1_INTERPOLATION_MODE_LINEAR);
			else if (m_pImgList && es.idxImg >= 0)
				m_pImgList->Draw(es.idxImg, rcImg);

		if (!e.pLayout && es.pszText)
		{
			EckAssert(es.cchText > 0);
			g_pDwFactory->CreateTextLayout(es.pszText, es.cchText, m_pTf,
				(float)m_cxItem, (float)(rc.bottom - rcImg.bottom), &e.pLayout);

			if (e.pLayout)
			{
				e.pLayout->SetWordWrapping(DWRITE_WORD_WRAPPING_WRAP);
				e.pLayout->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
				e.pLayout->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_NEAR);
				DWRITE_TEXT_METRICS tm;
				e.pLayout->GetMetrics(&tm);
				e.cxText = tm.width;
				e.cyText = tm.height;
			}
			else
				e.cxText = e.cyText = 0.f;
		}

		if (e.pLayout)
		{
			m_pBrush->SetColor(cr.crTextNormal);
			m_pDC->DrawTextLayout(D2D1::Point2F(rc.left, rcImg.bottom + m_pWnd->GetDs().CommMargin), 
				e.pLayout, m_pBrush);
		}
	}

	EckInline void DrawItem(int idx, const D2D1_RECT_F& rcPaint)
	{
		LEEDISPINFO es{ {LEE_GETDISPINFO},LEIM_TEXT | LEIM_IMAGE,idx };
		GenElemNotify(&es);
		switch (m_eView)
		{
		case ListType::List:
			LVPaintItem(idx, rcPaint, es);
			break;
		case ListType::Icon:
			IVPaintItem(idx, rcPaint, es);
			break;
		default:
			ECK_UNREACHABLE;
		}
	}

	void ReCalcScroll()
	{
		if (!m_cyItem || !m_cxItem)
			return;
		m_psv->SetMinThumbSize((int)m_DsF.minSBThumbSize);
		m_psv->SetPage(GetHeight());
		switch (m_eView)
		{
		case ListType::List:
			m_psv->SetRange(-m_cyTopExtra, (int)m_vItem.size() * (m_cyItem + m_cyPadding) + m_cyBottomExtra);
			break;
		case ListType::Icon:
		{
			m_cItemPerRow = (GetWidth() + m_cxPadding) / (m_cxItem + m_cxPadding);
			const int cItemV = ((int)m_vItem.size() - 1) / m_cItemPerRow + 1;
			m_psv->SetRange(-m_cyTopExtra, cItemV * (m_cyItem + m_cyPadding) + m_cyBottomExtra);
		}
		break;
		default:
			ECK_UNREACHABLE;
		}
	}

	void ReCalcTopItem()
	{
		if (!m_cyItem || !m_cxItem)
			return;
		switch (m_eView)
		{
		case ListType::List:
			m_idxTop = m_psv->GetPos() / (m_cyItem + m_cyPadding);
			m_oyTopItem = m_idxTop * (m_cyItem + m_cyPadding) - m_psv->GetPos();
			break;
		case ListType::Icon:
		{
			const int cItemV = m_psv->GetPos() / (m_cyItem + m_cyPadding);
			m_idxTop = cItemV * m_cItemPerRow;
			m_oyTopItem = cItemV * (m_cyItem + m_cyPadding) - m_psv->GetPos();
		}
		break;
		default:
			ECK_UNREACHABLE;
		}
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

		pPath->Release();
	}

	EckInline int IVLogItemFromX(int x) const
	{
		return x / (m_cxItem + m_cxPadding);
	}

	EckInline int IVLogItemFromY(int y) const
	{
		return (y - m_oyTopItem) / (m_cyItem + m_cyPadding);
	}

	EckInline std::pair<int, int> IVGetItemXY(int idx) const
	{
		const int idxV = (idx - m_idxTop) / m_cItemPerRow;
		return
		{
			((idx - m_idxTop) % m_cItemPerRow) * (m_cxItem + m_cxPadding),
			m_oyTopItem + idxV * (m_cyItem + m_cyPadding)
		};
	}

	EckInline void DragSelMouseMove(POINT pt, WPARAM wParam)
	{
		EckAssert(m_bDraggingSel);
		RECT rcOld{ m_rcDragSel };

		m_rcDragSel = MakeRect(pt, m_ptDragSelStart);

		RECT rcJudge;
		UnionRect(rcJudge, rcOld, m_rcDragSel);

		int idxBegin, idxX, idxY;

		idxX = IVLogItemFromX(rcJudge.left + 1);
		if (idxX < 0 || idxX >= m_cItemPerRow)
			idxBegin = -1;
		else
		{
			idxY = IVLogItemFromY(rcJudge.top + 1);
			idxBegin = m_idxTop + idxX + idxY * m_cItemPerRow;
			if (idxBegin < 0 || idxBegin >= GetItemCount())
				idxBegin = -1;
		}

		RECT rcItem;
		if (idxBegin >= 0)
			for (int i = idxBegin; i < GetItemCount(); ++i)
			{
				auto& e = m_vItem[i];
				if (IVGetItemXY(i).first >= (int)rcJudge.right)// 需要下移一行
				{
					i = idxBegin + m_cItemPerRow;
					idxBegin = i;
					if (i >= GetItemCount())
						break;
				}

				if (IVGetItemXY(i).second >= (int)rcJudge.bottom)// Y方向判断完成
					break;

				GetItemRect(i, rcItem);
				const BOOL bIntersectOld = IsRectsIntersect(rcItem, rcOld);
				const BOOL bIntersectNew = IsRectsIntersect(rcItem, m_rcDragSel);

				if (wParam & MK_CONTROL)
				{
					if (bIntersectOld != bIntersectNew)
						e.uFlags ^= LEIF_SELECTED;// 翻转选中状态
				}
				else
				{
					if (bIntersectOld && !bIntersectNew)
						e.uFlags &= ~LEIF_SELECTED;// 先前选中但是现在未选中，清除选中状态
					else if (!bIntersectOld && bIntersectNew)
						e.uFlags |= LEIF_SELECTED;// 先前未选中但是现在选中，设置选中状态
					// mark设为离光标最远的选中项（标准ListView的行为）
					if (bIntersectNew && !(wParam & (MK_CONTROL | MK_SHIFT)))
					{
						const int d = (pt.x - rcItem.left) * (pt.x - rcItem.left) +
							(pt.y - rcItem.top) * (pt.y - rcItem.top);
						if (d > m_dCursorToItemMax)
						{
							m_dCursorToItemMax = d;
							m_idxMark = i;
						}
					}
				}
			}
		InvalidateRect();
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
		switch (uMsg)
		{
		case WM_PAINT:
		{
			ELEMPAINTSTRU ps;
			BeginPaint(ps, wParam, lParam);
			if (!m_vItem.empty())
			{
				switch (m_eView)
				{
				case ListType::List:
				{
					const int idxBegin = std::max(ItemFromY((int)ps.rcfClipInElem.top), 0);
					const int idxEnd = std::min(ItemFromY((int)ps.rcfClipInElem.bottom), (int)m_vItem.size() - 1);
					for (int i = idxBegin; i <= idxEnd; ++i)
						DrawItem(i, ps.rcfClipInElem);
				}
				break;
				case ListType::Icon:
				{
					int idxBegin, idxX, idxY;

					idxX = IVLogItemFromX((int)ps.rcfClipInElem.left + 1);
					if (idxX < 0 || idxX >= m_cItemPerRow)
						idxBegin = -1;
					else
					{
						idxY = IVLogItemFromY((int)ps.rcfClipInElem.top + 1);
						idxBegin = m_idxTop + idxX + idxY * m_cItemPerRow;
						if (idxBegin < 0 || idxBegin >= GetItemCount())
							idxBegin = -1;
					}

					if (idxBegin >= 0)
						for (int i = idxBegin; i < GetItemCount(); ++i)
						{
							if (IVGetItemXY(i).first >= (int)ps.rcfClipInElem.right)// 需要下移一行
							{
								i = idxBegin + m_cItemPerRow;
								idxBegin = i;
								if (i >= GetItemCount())
									break;
							}

							if (IVGetItemXY(i).second >= (int)ps.rcfClipInElem.bottom)// Y方向重画完成
								break;

							DrawItem(i, ps.rcfClipInElem);
						}
				}
				break;
				default:
					ECK_UNREACHABLE;
				}

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

			const auto& crs = GetColorTheme()->Get();

			if (m_bDraggingSel)
			{
				auto cr = crs.crBkHot;
				cr.a *= 0.7f;
				m_pBrush->SetColor(cr);
				m_pDC->FillRectangle(MakeD2DRcF(m_rcDragSel), m_pBrush);

				cr = crs.crBkSelected;
				cr.a *= 0.7f;
				m_pBrush->SetColor(cr);
				m_pDC->DrawRectangle(MakeD2DRcF(m_rcDragSel), m_pBrush, 3.f);
			}

			ECK_DUI_DBG_DRAW_FRAME;
			EndPaint(ps);
		}
		return 0;

		case WM_MOUSEMOVE:
		{
			ECK_DUILOCK;
			LEHITTEST ht{ ECK_GET_PT_LPARAM(lParam) };
			ClientToElem(ht.pt);

			if (m_bDraggingSel)
			{
				DragSelMouseMove(ht.pt, wParam);
				return 0;
			}

			int idx = HitTest(ht);

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
			ECK_DUILOCK;
			if (m_idxHot >= 0)
			{
				int idx = -1;
				std::swap(m_idxHot, idx);
				RedrawItem(idx);
			}
		}
		return 0;

		case WM_MOUSEWHEEL:
		{
			ECK_DUILOCK;
			m_psv->OnMouseWheel2(-GET_WHEEL_DELTA_WPARAM(wParam) / WHEEL_DELTA);
			GetWnd()->WakeRenderThread();
		}
		return 0;

		case WM_NOTIFY:
		{
			ECK_DUILOCK;
			if ((wParam == (WPARAM)&m_SB) &&
				(((DUINMHDR*)lParam)->uCode == EE_VSCROLL))
			{
				ReCalcTopItem();
				InvalidateRect();
				return TRUE;
			}
		}
		break;

		case WM_SIZE:
		{
			ECK_DUILOCK;
			ReCalcTopItem();
			ReCalcScroll();
			if (m_idxInsertMark >= 0)
				UpdateInsertMarkGeometry();
			const auto cxSB = (int)GetWnd()->GetDs().CommSBCxy;
			m_SB.SetRect({ GetWidth() - cxSB,0,GetWidth(),GetHeight() });
		}
		return 0;

		case WM_LBUTTONDOWN:
		{
			ECK_DUILOCK;
			POINT pt ECK_GET_PT_LPARAM(lParam);
			SetFocus();
			LEHITTEST ht{ pt };
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
			else
			{
				ClientToScreen(GetWnd()->HWnd, &pt);
				if (IsMouseMovedBeforeDragging(GetWnd()->HWnd, pt.x, pt.y))
				{
					if (!GetWnd()->IsValid())
						return 0;
					SetCapture();
					m_bDraggingSel = TRUE;
					m_rcDragSel = {};
					m_ptDragSelStart = ht.pt;
					m_dCursorToItemMax = INT_MIN;
				}
				else
				{
					DeselectAll(idxChangedBegin, idxChangedEnd);
					if (idxChangedBegin >= 0)
						RedrawItem(idxChangedBegin, idxChangedEnd);
				}
			}
		}
		return 0;

		case WM_LBUTTONUP:
		{
			ECK_DUILOCK;
			if (m_bDraggingSel)
			{
				m_bDraggingSel = FALSE;
				ReleaseCapture();
				InvalidateRect();
			}
		}
		return 0;

		case WM_CAPTURECHANGED:
		{
			ECK_DUILOCK;
			if (m_bDraggingSel)
			{
				m_bDraggingSel = FALSE;
				InvalidateRect();
			}
		}
		return 0;

		case WM_CREATE:
		{
			m_pColorTheme = GetWnd()->GetDefColorTheme()[CTI_LIST];
			m_pColorTheme->Ref();

			m_SB.Create(NULL, DES_VISIBLE|DES_COMPOSITED, 0,
				0, 0, 0, 0, this, GetWnd());
			m_psv = m_SB.GetScrollView();
			m_psv->AddRef();
			m_psv->SetCallBack([](int iPos, int iPrevPos, LPARAM lParam)
				{
					auto pThis = (CList*)lParam;
					pThis->ReCalcTopItem();
					pThis->InvalidateRect();
				}, (LPARAM)this);
			m_psv->SetDelta(DpiScale(80, GetWnd()->GetDpiValue()));

			UpdateDpiSizeF(m_DsF, m_pWnd->GetDpiValue());

			m_pDC->CreateSolidColorBrush({}, &m_pBrush);
			m_pDC->QueryInterface(&m_pDC1);
		}
		return 0;

		case WM_DESTROY:
		{
			ECK_DUILOCK;
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
			m_bSingleSel = FALSE;

			SafeRelease(m_psv);
		}
		return 0;
		}
		return CElem::OnEvent(uMsg, wParam, lParam);
	}

	EckInline int GetItemY(int idx) const
	{
		EckAssert(m_eView == ListType::List);
		return m_oyTopItem + (idx - m_idxTop) * (m_cyItem + m_cyPadding);
	}

	EckInline int ItemFromY(int y) const
	{
		EckAssert(m_eView == ListType::List);
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

	void SetItemWidth(int cx)
	{
		m_cxItem = cx;
		ReCalcScroll();
	}

	void SetItemPaddingH(int cx)
	{
		m_cxPadding = cx;
		ReCalcScroll();
	}

	void GetItemRect(int idx, RECT& rc) const
	{
		switch (m_eView)
		{
		case ListType::List:
			rc.left = 0;
			rc.right = GetWidth();
			rc.top = GetItemY(idx);
			rc.bottom = rc.top + m_cyItem;
			break;
		case ListType::Icon:
		{
			const auto xy = IVGetItemXY(idx);
			rc.left = xy.first;
			rc.right = rc.left + m_cxItem;
			rc.top = xy.second;
			rc.bottom = rc.top + m_cyItem;
		}
		break;
		default:
			ECK_UNREACHABLE;
		}
	}

	void GetItemRect(int idx, D2D1_RECT_F& rc) const
	{
		switch (m_eView)
		{
		case ListType::List:
			rc.left = 0;
			rc.right = GetWidthF();
			rc.top = (float)GetItemY(idx);
			rc.bottom = rc.top + m_cyItem;
			break;
		case ListType::Icon:
		{
			const auto xy = IVGetItemXY(idx);
			rc.left = (float)xy.first;
			rc.right = rc.left + m_cxItem;
			rc.top = (float)xy.second;
			rc.bottom = rc.top + m_cyItem;
		}
		break;
		default:
			ECK_UNREACHABLE;
		}
	}

	// 元素坐标
	int HitTest(LEHITTEST& leht) const
	{
		RECT rc{ 0,0,m_rc.right - m_rc.left,m_rc.bottom - m_rc.top };
		if (!PtInRect(rc, leht.pt) || m_vItem.empty())
			return -1;

		switch(m_eView)
		{
		case ListType::List:
		{
			const int idx = ItemFromY(leht.pt.y);
			if (idx >= 0 && idx < (int)m_vItem.size())
				return idx;
			else
				return -1;
		}
		break;

		case ListType::Icon:
		{
			const int idxX = IVLogItemFromX(leht.pt.x);
			if (idxX < 0 || idxX >= m_cItemPerRow)
				return -1;
			const int idxY = IVLogItemFromY(leht.pt.y);
			const int idx = m_idxTop + idxX + idxY * m_cItemPerRow;
			if (idx < 0 || idx >= GetItemCount())
				return -1;
			return idx;
		}
		break;
		default:
			ECK_UNREACHABLE;
		}
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
		pImgList->GetImageSize(m_cxImage, m_cyImage);
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
		rc.right = GetWidthF();
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
		InvalidateRect(rc);
	}

	void InvalidateCache(int idx)
	{
		if (idx < 0)
			for (auto& e : m_vItem)
				SafeRelease(e.pLayout);
		else
		{
			EckAssert(idx < (int)m_vItem.size());
			SafeRelease(m_vItem[idx].pLayout);
		}
	}

	//void SetImageSize(int cxy)
	//{
	//	if (cxy < 0)
	//		m_cxyImage = m_cyItem - (int)(m_pWnd->GetDs().CommMargin * 2.f);
	//	else
	//		m_cxyImage = cxy;
	//}

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
		EckAssert(idxEnd >= idxBegin);
		switch (m_eView)
		{
		case ListType::List:
		{
			RECT rc
			{
				0,
				GetItemY(idxBegin),
				GetWidth(),
				GetItemY(idxEnd) + m_cyItem
			};
			ElemToClient(rc);
			InvalidateRect(rc);
		}
		break;
		case ListType::Icon:
		{
			if (idxBegin == idxEnd)
				RedrawItem(idxBegin);
			else
			{
				auto [x1, y1] = IVGetItemXY(idxBegin);
				auto [x2, y2] = IVGetItemXY(idxEnd);
				if (y1 == y2)
				{
					RECT rc
					{
						x1,
						y1,
						x2 + m_cxItem,
						y2 + m_cyItem
					};
					ElemToClient(rc);
					InvalidateRect(rc);
				}
				else
				{
					RECT rc
					{
						0,
						y1,
						GetWidth(),
						y2 + m_cyItem
					};
					ElemToClient(rc);
					InvalidateRect(rc);
				}
			}
		}
		break;
		default:
			ECK_UNREACHABLE;
		}
	}

	void RedrawItem(int idx)
	{
		RECT rc;
		GetItemRect(idx, rc);
		ElemToClient(rc);
		InvalidateRect(rc);
	}

	int GetItemCount() const { return (int)m_vItem.size(); }
};
ECK_DUI_NAMESPACE_END
ECK_NAMESPACE_END
#else
#error "EckDui requires C++20"
#endif// ECKCXX20