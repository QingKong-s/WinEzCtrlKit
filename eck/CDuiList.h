#pragma once
#include "CDuiScrollBar.h"
#include "CDuiHeader.h"
#include "CD2DImageList.h"

#if !ECKCXX20
#error "EckDui requires C++20"
#endif

ECK_NAMESPACE_BEGIN
ECK_DUI_NAMESPACE_BEGIN
struct LEE_DISPINFO : DUINMHDR
{
	DispInfoMask uMask;
	int idx;
	int idxSub;

	PCWSTR pszText;
	int cchText;

	int idxImg;
	ID2D1Bitmap* pImg;

};

struct LEHITTEST
{
	POINT pt;
};

enum
{
	LEIF_SELECTED = (1u << 0),
};

enum class ListType : BYTE
{
	List,
	Icon,
};


class CList :public CElem
{
public:
	enum : int
	{
		CyInsertMark = 3,
		CyDefHeader = 30,
	};
private:
	struct SUBITEM
	{
		ComPtr<IDWriteTextLayout> pLayout{};
	};
	struct ITEM
	{
		ComPtr<IDWriteTextLayout> pLayout{};
		UINT uFlags{};
		std::vector<SUBITEM> vSubItem{};
	};

	std::vector<ITEM> m_vItem{};// 项目
	CScrollBar m_SBV{}, m_SBH{};
	CHeader m_Header{};
	ID2D1SolidColorBrush* m_pBrush{};
	CInertialScrollView* m_psvV{};
	CInertialScrollView* m_psvH{};
	CD2DImageList* m_pImgList{};
	//---通用
	int m_idxHot{ -1 };			// 热点项
	int m_idxSel{ -1 };			// 选中的项的索引，仅用于单选
	int m_idxInsertMark{ -1 };	// 插入标记应当显示在哪一项之前
	int m_idxFocus{ -1 };		// 焦点项
	int m_idxMark{ -1 };		// 标记项

	int m_cyTopExtra{};		// 顶部空白
	int m_cyBottomExtra{};	// 底部空白

	int m_cyItem{};			// 项目高度
	int m_cyPadding{};		// 项目间距

	int m_oyTopItem{};		// 小于等于零的值，指示第一可见项的遮挡高度
	int m_idxTop{};			// 第一个可见项
	//---列表模式
	//---图标模式
	int m_cxItem{};			// 项目宽度
	int m_cxPadding{};		// 水平项目间距
	int m_cItemPerRow{};	// 每行项目数
	//---
	POINT m_ptDragSelStart{};	// 拖动选择起始点
	RECT m_rcDragSel{};			// 当前选择矩形
	int m_dCursorToItemMax{};	// 鼠标指针到项目的最大距离

	ListType m_eView{ ListType::Icon };

	BITBOOL m_bSingleSel : 1{};
	BITBOOL m_bReport : 1{};

	BITBOOL m_bDraggingSel : 1{};


	D2D1_SIZE_F GetImageSize(const LEE_DISPINFO& es)
	{
		if (es.pImg)
			return es.pImg->GetSize();
		else if (m_pImgList && es.idxImg >= 0)
		{
			int cx, cy;
			m_pImgList->GetImageSize(cx, cy);
			return { (float)cx, (float)cy };
		}
		else
			return {};
	}
	void LVPaintSubItem(int idx, int idxSub, const D2D1_RECT_F& rcSub,
		const D2D1_RECT_F& rcPaint)
	{
		LEE_DISPINFO es{ LEE_GETDISPINFO };
		es.idx = idx;
		es.idxSub = idxSub;
		es.uMask = DIM_TEXT | DIM_IMAGE;
		GenElemNotify(&es);

		D2D1_SIZE_F sizeImg = GetImageSize(es);

		const float Padding = GetTheme()->GetMetrics(Metrics::SmallPadding);
		auto& e = m_vItem[idx];
		auto& pTl = (idxSub ? e.vSubItem[idxSub - 1].pLayout : e.pLayout);
		if (!pTl.Get() && es.pszText)
		{
			EckAssert(es.cchText > 0);
			g_pDwFactory->CreateTextLayout(es.pszText, es.cchText, GetTextFormat(),
				rcSub.right - rcSub.left - Padding * 3 - sizeImg.width,
				(float)m_cyItem, &pTl);
		}

		State eState;
		if ((e.uFlags & LEIF_SELECTED) || (m_bSingleSel && m_idxSel == idx))
			if (m_idxHot == idx)
				eState = State::HotSelected;
			else
				eState = State::Selected;
		else if (m_idxHot == idx)
			eState = State::Hot;
		else
			eState = State::None;

		if (eState != State::None)
			GetTheme()->DrawBackground(Part::ListItem, eState, rcSub, nullptr);

		const float xImage = Padding;
		const float yImage = (float)((m_cyItem - sizeImg.height) / 2);
		float xText;
		if (es.pImg || (m_pImgList && es.idxImg >= 0))
		{
			auto rc{ rcSub };
			rc.left += xImage;
			rc.right = rc.left + sizeImg.width;
			rc.top += yImage;
			rc.bottom = rc.top + sizeImg.height;
			xText = rc.right + Padding;
			if (!(rc.right <= rcPaint.left || rc.left >= rcPaint.right))
				if (es.pImg)
					m_pDC->DrawBitmap(es.pImg, rc);
				else/* if (m_pImgList && es.idxImg >= 0)*/
					m_pImgList->Draw(es.idxImg, rc);
		}
		else
			xText = rcSub.left + Padding;
		if (pTl.Get())
		{
			DWRITE_TEXT_METRICS tm;
			pTl->GetMetrics(&tm);
			if (!(xText + tm.width <= rcPaint.left || xText >= rcPaint.right))
			{
				D2D1_COLOR_F cr;
				GetTheme()->GetSysColor(SysColor::Text, cr);
				m_pBrush->SetColor(cr);
				m_pDC->DrawTextLayout({ xText, rcSub.top }, pTl.Get(), m_pBrush);
			}
		}
	}

	void LVPaintItem(int idx, const D2D1_RECT_F& rcPaint)
	{
		RECT rc;
		GetSubItemRect(idx, 0, rc);
		LVPaintSubItem(idx, 0, MakeD2DRcF(rc), rcPaint);
		if (m_bReport)
			for (int i = 1; i < m_Header.GetItemCount(); i++)
			{
				GetSubItemRect(idx, i, rc);
				LVPaintSubItem(idx, i, MakeD2DRcF(rc), rcPaint);
			}
	}

	void IVPaintItem(int idx, const D2D1_RECT_F& rcPaint)
	{
		LEE_DISPINFO es{ LEE_GETDISPINFO };
		es.idx = idx;
		es.uMask = DIM_TEXT | DIM_IMAGE;
		GenElemNotify(&es);

		D2D1_SIZE_F sizeImg = GetImageSize(es);
		auto& e = m_vItem[idx];

		RECT rcTmp;
		GetItemRect(idx, rcTmp);
		auto rc{ MakeD2DRcF(rcTmp) };

		State eState;
		if ((e.uFlags & LEIF_SELECTED) || (m_bSingleSel && m_idxSel == idx))
			if (m_idxHot == idx)
				eState = State::HotSelected;
			else
				eState = State::Selected;
		else if (m_idxHot == idx)
			eState = State::Hot;
		else
			eState = State::None;

		if (eState != State::None)
			GetTheme()->DrawBackground(Part::ListItem, eState, rc, nullptr);

		const float Padding = GetTheme()->GetMetrics(Metrics::SmallPadding);
		D2D1_RECT_F rcImg;
		rcImg.left = rc.left + (m_cxItem - sizeImg.width) / 2.f;
		rcImg.top = rc.top + Padding;
		rcImg.right = rcImg.left + sizeImg.width;
		rcImg.bottom = rcImg.top + sizeImg.height;

		if (!(rcImg.right <= rcPaint.left || rcImg.left >= rcPaint.right))
			if (es.pImg)
				m_pDC->DrawBitmap(es.pImg, rcImg, 1.f, D2D1_INTERPOLATION_MODE_LINEAR);
			else if (m_pImgList && es.idxImg >= 0)
				m_pImgList->Draw(es.idxImg, rcImg);

		if (!e.pLayout.Get() && es.pszText)
		{
			EckAssert(es.cchText > 0);
			g_pDwFactory->CreateTextLayout(es.pszText, es.cchText, GetTextFormat(),
				(float)m_cxItem, float(rc.bottom - rcImg.bottom), &e.pLayout);

			if (e.pLayout.Get())
			{
				e.pLayout->SetWordWrapping(DWRITE_WORD_WRAPPING_NO_WRAP);
				e.pLayout->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
				e.pLayout->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_NEAR);
			}
		}

		if (e.pLayout.Get())
		{
			D2D1_COLOR_F cr;
			GetTheme()->GetSysColor(SysColor::Text, cr);
			m_pBrush->SetColor(cr);
			m_pDC->DrawTextLayout({ rc.left, rcImg.bottom + Padding },
				e.pLayout.Get(), m_pBrush);
		}
	}

	EckInline void DrawItem(int idx, const D2D1_RECT_F& rcPaint)
	{
		switch (m_eView)
		{
		case ListType::List:
			LVPaintItem(idx, rcPaint);
			break;
		case ListType::Icon:
			IVPaintItem(idx, rcPaint);
			break;
		default: ECK_UNREACHABLE;
		}
	}

	void ReCalcScroll()
	{
		if (!m_cyItem || !m_cxItem)
			return;
		m_psvV->SetMinThumbSize(CxyMinScrollThumb);
		m_psvV->SetPage(GetHeight());
		switch (m_eView)
		{
		case ListType::List:
			m_psvV->SetRange(-m_cyTopExtra, GetItemCount() * (m_cyItem + m_cyPadding) + m_cyBottomExtra);
			break;
		case ListType::Icon:
		{
			m_cItemPerRow = (GetWidth() + m_cxPadding) / (m_cxItem + m_cxPadding);
			const int cItemV = (GetItemCount() - 1) / m_cItemPerRow + 1;
			m_psvV->SetRange(-m_cyTopExtra, cItemV * (m_cyItem + m_cyPadding) + m_cyBottomExtra);
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
			m_idxTop = m_psvV->GetPos() / (m_cyItem + m_cyPadding);
			m_oyTopItem = m_idxTop * (m_cyItem + m_cyPadding) - m_psvV->GetPos();
			break;
		case ListType::Icon:
		{
			const int cItemV = m_psvV->GetPos() / (m_cyItem + m_cyPadding);
			m_idxTop = cItemV * m_cItemPerRow;
			m_oyTopItem = cItemV * (m_cyItem + m_cyPadding) - m_psvV->GetPos();
		}
		break;
		default:
			ECK_UNREACHABLE;
		}
	}

	// 从X坐标计算逻辑项目索引，索引相对当前可见范围
	EckInline int IVLogItemFromX(int x) const
	{
		return x / (m_cxItem + m_cxPadding);
	}
	// 从Y坐标计算逻辑项目索引，索引相对当前可见范围
	// m_idxTop所在行的下一行记为0，上一行记为-1
	EckInline int IVLogItemFromY(int y) const
	{
		const auto i = (y - m_oyTopItem) / (m_cyItem + m_cyPadding);
		return (y - m_oyTopItem < 0) ? i - 1 : i;
	}

	EckInline std::pair<int, int> IVGetItemXY(int idx) const
	{
		const int idxV = (idx - m_idxTop) / m_cItemPerRow;
		return
		{
			(abs(idx - m_idxTop) % m_cItemPerRow) * (m_cxItem + m_cxPadding),
			m_oyTopItem + idxV * (m_cyItem + m_cyPadding)
		};
	}

	// 由索引得到Y坐标
	EckInline int LVGetItemY(int idx) const
	{
		EckAssert(m_eView == ListType::List);
		return m_oyTopItem + (idx - m_idxTop) * (m_cyItem + m_cyPadding);
	}
	// 由Y坐标得到索引
	EckInline int LVItemFromY(int y) const
	{
		EckAssert(m_eView == ListType::List);
		return m_idxTop + (y - m_oyTopItem) / (m_cyItem + m_cyPadding);
	}

	void CalcItemRangeInRect(const RECT& rc, _Out_ int& idxBegin, _Out_ int& idxEnd)
	{
		switch (m_eView)
		{
		case ListType::List:
			idxBegin = LVItemFromY(rc.top);
			idxBegin = std::clamp(idxBegin, 0, GetItemCount() - 1);
			idxEnd = LVItemFromY(rc.bottom);
			idxEnd = std::clamp(idxEnd, 0, GetItemCount() - 1);
			break;
		case ListType::Icon:
		{
			int idxX = IVLogItemFromX(rc.left);
			idxX = std::clamp(idxX, 0, m_cItemPerRow - 1);
			int idxY = IVLogItemFromY(rc.top);
			idxBegin = m_idxTop + idxX + idxY * m_cItemPerRow;

			idxX = IVLogItemFromX(rc.right);
			idxX = std::clamp(idxX, 0, m_cItemPerRow - 1);
			idxY = IVLogItemFromY(rc.bottom);
			idxEnd = m_idxTop + idxX + idxY * m_cItemPerRow;
		}
		break;
		default: ECK_UNREACHABLE;
		}
	}

	void DragSelMouseMove(POINT pt, WPARAM wParam)
	{
		const auto dy = m_psvV->GetPos();
		EckAssert(m_bDraggingSel);
		RECT rcOld{ m_rcDragSel };
		OffsetRect(rcOld, 0, -dy);

		m_rcDragSel = MakeRect(pt, POINT{ m_ptDragSelStart.x,m_ptDragSelStart.y - dy });

		RECT rcJudge;
		UnionRect(rcJudge, rcOld, m_rcDragSel);

		int idxBegin, idxEnd;
		RECT rcItem;
		CalcItemRangeInRect(rcJudge, idxBegin, idxEnd);
		if (idxBegin < 0)
			idxBegin = 0;
		if (idxEnd >= GetItemCount())
			idxEnd = GetItemCount() - 1;
		for (int i = idxBegin; i <= idxEnd; ++i)
		{
			auto& e = m_vItem[i];

			if (m_eView == ListType::Icon)
			{
				const auto xy = IVGetItemXY(i);
				if (xy.first >= (int)rcJudge.right)// 需要下移一行
				{
					i = idxBegin + m_cItemPerRow;
					idxBegin = i;
					if (i >= GetItemCount())
						break;
				}
			}

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
		OffsetRect(m_rcDragSel, 0, dy);
		InvalidateRect();
	}
public:
	LRESULT OnEvent(UINT uMsg, WPARAM wParam, LPARAM lParam) override
	{
		switch (uMsg)
		{
		case WM_PAINT:
		{
			ELEMPAINTSTRU ps;
			BeginPaint(ps, wParam, lParam);
			if (GetItemCount() && (m_bReport ? m_Header.GetItemCount() : TRUE))
			{
				switch (m_eView)
				{
				case ListType::List:
				{
					const int idxBegin = std::max(LVItemFromY((int)ps.rcfClipInElem.top), 0);
					const int idxEnd = std::min(LVItemFromY((int)ps.rcfClipInElem.bottom), GetItemCount() - 1);
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
				default: ECK_UNREACHABLE;
				}

				if (m_idxInsertMark >= 0)
				{
					D2D1_RECT_F rcIm;
					GetInsertMarkRect(rcIm);
					if (rcIm.bottom > 0.f && rcIm.top < GetHeightF())
					{
						// TODO：插入标记
					}
				}
			}

			if (m_bDraggingSel)
			{
				auto rc{ MakeD2DRcF(m_rcDragSel) };
				OffsetRect(rc, 0.f, (float)-m_psvV->GetPos());
				GetTheme()->DrawBackground(Part::ListSelRect,
					State::None, rc, nullptr);
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
				if (ht.pt.x < 0) ht.pt.x = 0;
				if (ht.pt.y < 0) ht.pt.y = 0;
				if (ht.pt.x >= GetWidth()) ht.pt.x = GetWidth() - 1;
				if (ht.pt.y >= GetHeight()) ht.pt.y = GetHeight() - 1;
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
			m_psvV->OnMouseWheel2(-GET_WHEEL_DELTA_WPARAM(wParam) / WHEEL_DELTA);
			GetWnd()->WakeRenderThread();
		}
		return 0;

		case WM_NOTIFY:
		{
			ECK_DUILOCK;
			if ((wParam == (WPARAM)&m_SBV) &&
				(((DUINMHDR*)lParam)->uCode == EE_VSCROLL))
			{
				ReCalcTopItem();
				InvalidateRect();
				return TRUE;
			}
			else if ((wParam == (WPARAM)&m_Header))
				switch (((DUINMHDR*)lParam)->uCode)
				{
				case HEE_GETDISPINFO:
					return GenElemNotify((HEE_DISPINFO*)lParam);
				case HEE_WIDTHCHANGED:
				{
					EckAssert(m_eView == ListType::List && m_bReport);
					const auto* const p = (HEE_ITEMNOTIFY*)lParam;
					if (p->idx)
						for (auto& e : m_vItem)
							e.vSubItem[p->idx - 1].pLayout.Clear();
					else
					{
						for (auto& e : m_vItem)
							e.pLayout.Clear();
					}
					RECT rc;
					GetSubItemRect(0, p->idx, rc);
					rc.top = 0;
					rc.right = GetWidth();
					rc.bottom = GetHeight();
					ElemToClient(rc);
					InvalidateRect(rc);
				}
				return 0;
				}
		}
		break;

		case WM_SIZE:
		{
			ECK_DUILOCK;
			ReCalcTopItem();
			ReCalcScroll();
			const auto cxSB = (int)GetTheme()->GetMetrics(Metrics::CxVScroll);
			m_SBV.SetRect({ GetWidth() - cxSB,0,GetWidth(),GetHeight() });
			if (m_Header.IsValid())
				m_Header.SetSize(GetWidth(), m_Header.GetHeight());
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
					if (ht.pt.x < 0) ht.pt.x = 0;
					if (ht.pt.y < 0) ht.pt.y = 0;
					if (ht.pt.x >= GetWidth()) ht.pt.x = GetWidth() - 1;
					if (ht.pt.y >= GetHeight()) ht.pt.y = GetHeight() - 1;
					m_ptDragSelStart = ht.pt;
					m_ptDragSelStart.y += m_psvV->GetPos();
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
			if (m_bDraggingSel)
			{
				m_bDraggingSel = FALSE;
				InvalidateRect();
			}
		}
		return 0;

		case WM_CREATE:
		{
			m_pDC->CreateSolidColorBrush({}, &m_pBrush);

			m_Header.Create(nullptr, DES_VISIBLE, 0,
				0, 0, 0, CyDefHeader, this);

			m_SBV.Create(nullptr, DES_VISIBLE, 0,
				0, 0, 0, 0, this);
			m_psvV = m_SBV.GetScrollView();
			m_psvV->AddRef();
			m_psvV->SetCallBack([](int iPos, int iPrevPos, LPARAM lParam)
				{
					auto pThis = (CList*)lParam;
					pThis->ReCalcTopItem();
					pThis->InvalidateRect();
				}, (LPARAM)this);
			m_psvV->SetDelta(80);

			m_SBH.Create(nullptr, 0, 0,
				0, 0, 0, 0, this);

			m_psvH = m_SBH.GetScrollView();
			m_psvH->AddRef();
			m_psvH->SetCallBack([](int iPos, int iPrevPos, LPARAM lParam)
				{
					auto pThis = (CList*)lParam;
					pThis->ReCalcTopItem();
					pThis->InvalidateRect();
				}, (LPARAM)this);
			m_psvH->SetDelta(40);
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
			m_vItem.clear();
			m_pImgList = nullptr;
			m_bSingleSel = FALSE;

			SafeRelease(m_psvV);
			SafeRelease(m_psvH);
		}
		return 0;
		}
		return __super::OnEvent(uMsg, wParam, lParam);
	}

	EckInline void SetItemCount(int c) noexcept
	{
		ECK_DUILOCK;
		if (m_bReport)
		{
			const auto idxBegin = std::max(0, GetItemCount() - 1);
			m_vItem.resize(c);
			for (int i = idxBegin; i < c; ++i)
				m_vItem[i].vSubItem.resize(m_Header.GetItemCount());
		}
		else
			m_vItem.resize(c);
	}
	EckInlineNdCe int GetItemCount() const noexcept { return (int)m_vItem.size(); }

	void GetItemRect(int idx, RECT& rc) const
	{
		switch (m_eView)
		{
		case ListType::List:
			if (m_bReport)
			{
				rc.left = -m_psvH->GetPos();
				rc.right = rc.left + m_Header.GetContentWidth();
			}
			else
			{
				rc.left = 0;
				rc.right = GetWidth();
			}
			rc.top = LVGetItemY(idx);
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

	void GetSubItemRect(int idx, int idxSub, RECT& rc) const noexcept
	{
		EckAssert(idx < GetItemCount());
		if (m_eView != ListType::List || !m_bReport)
		{
			GetItemRect(idx, rc);
			return;
		}
		m_Header.GetItemRect(idxSub, rc);
		rc.top = LVGetItemY(idx);
		rc.bottom = rc.top + m_cyItem;
	}

	// 元素坐标
	int HitTest(LEHITTEST& leht) const
	{
		if (!PtInRect(GetViewRect(), leht.pt) || !GetItemCount())
			return -1;

		switch (m_eView)
		{
		case ListType::List:
		{
			if (m_bReport && leht.pt.x > m_Header.GetContentWidth())
				return -1;
			const int idx = LVItemFromY(leht.pt.y);
			if (idx >= 0 && idx < GetItemCount())
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
		default: ECK_UNREACHABLE;
		}
	}

	void DeselectAll(int& idxChangedBegin, int& idxChangedEnd)
	{
		if (m_bSingleSel)
			m_idxSel = -1;
		else
		{
			int idx0 = -1, idx1 = 0;
			EckCounter(GetItemCount(), i)
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

	void SetImageList(CD2DImageList* pImgList)
	{
		ECK_DUILOCK;
		std::swap(m_pImgList, pImgList);
		if (m_pImgList)
			m_pImgList->AddRef();
		if (pImgList)
			pImgList->Release();
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
		rc.top = LVGetItemY(m_idxInsertMark) - (float)CyInsertMark * 2.f;
		rc.bottom = rc.top + (float)CyInsertMark * 5.f;
	}

	void SetInsertMark(int idx, BOOL bRedraw = TRUE)
	{
		m_idxInsertMark = idx;
		if (bRedraw)
		{
			D2D1_RECT_F rc;
			GetInsertMarkRect(rc);
			ElemToClient(rc);
			InvalidateRect(rc);
		}
	}

	void InvalidateCache(int idx)
	{
		if (idx < 0)
			for (auto& e : m_vItem)
				e.pLayout.Clear();
		else
		{
			EckAssert(idx < GetItemCount());
			m_vItem[idx].pLayout.Clear();
		}
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
				LVGetItemY(idxBegin),
				GetWidth(),
				LVGetItemY(idxEnd) + m_cyItem
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

	void ReCalc()
	{
		ReCalcScroll();
		ReCalcTopItem();
	}

	EckInline void SetHeaderHeight(int cy)
	{
		m_Header.SetSize(m_Header.GetWidth(), cy);
	}
	EckInlineNd int GetHeaderHeight() const noexcept { return m_Header.GetHeight(); }

	void SetColumnCount(int cItem,
		_In_reads_opt_(cItem) const int* pcx = nullptr) noexcept
	{
		EckAssert(m_bReport);
		m_Header.SetItemCount(cItem, pcx);
		for (auto& e : m_vItem)
			e.vSubItem.resize(cItem - 1);
	}

	EckInlineCe void SetView(ListType eView) noexcept
	{
		m_eView = eView;
		switch (eView)
		{
		case ListType::Icon:
			m_bReport = FALSE;
			break;
		default: ECK_UNREACHABLE;
		}
	}
	EckInlineNdCe ListType GetView() const noexcept { return m_eView; }

	EckInlineNdCe auto& GetScrollBarV() noexcept { return m_SBV; }
	EckInlineNdCe auto& GetScrollBarH() noexcept { return m_SBH; }
	EckInlineNdCe auto& GetHeader() noexcept { return m_Header; }

	EckInlineCe void SetItemHeight(int cy) noexcept { m_cyItem = cy; }
	EckInlineNdCe int GetItemHeight() const noexcept { return m_cyItem; }

	EckInlineCe void SetItemPadding(int cy) noexcept { m_cyPadding = cy; }
	EckInlineNdCe int GetItemPadding() const noexcept { return m_cyPadding; }

	EckInlineCe void SetItemWidth(int cx) noexcept
	{
		EckAssert(m_eView == ListType::Icon);
		m_cxItem = cx;
	}
	EckInlineNdCe int GetItemWidth() const noexcept { return m_cxItem; }

	EckInlineCe void SetItemPaddingH(int cx) { m_cxPadding = cx; }
	EckInlineNdCe int GetItemPaddingH() const noexcept { return m_cxPadding; }

	EckInlineCe void SetSingleSel(BOOL bSingleSel) noexcept { m_bSingleSel = bSingleSel; }
	EckInlineNdCe BOOL GetSingleSel() const noexcept { return m_bSingleSel; }

	EckInlineCe void SetTopExtraSpace(int cy) noexcept { m_cyTopExtra = cy; }
	EckInlineNdCe int GetTopExtraSpace() const noexcept { return m_cyTopExtra; }

	EckInlineCe void SetBottomExtraSpace(int cy) noexcept { m_cyBottomExtra = cy; }
	EckInlineNdCe int GetBottomExtraSpace() const noexcept { return m_cyBottomExtra; }

	EckInline void SetReport(BOOL bReport) noexcept
	{
		EckAssert(m_eView == ListType::List);
		m_bReport = bReport;
	}
	EckInlineNdCe BOOL GetReport() const noexcept { return m_bReport; }
};
ECK_DUI_NAMESPACE_END
ECK_NAMESPACE_END