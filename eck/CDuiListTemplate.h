#pragma once
#include "CDuiScrollBar.h"
#include "CSelRange.h"

ECK_NAMESPACE_BEGIN
ECK_DUI_NAMESPACE_BEGIN
struct LTN_ITEM : DUINMHDR
{
	int idxItem;
};

class CListTemplate :public CElem
{
protected:
	CSelRange m_SelRange{};
	CScrollBar m_SB{};
	CInertialScrollView* m_psv{};
	int m_cItem{};
	int m_cyPadding{ 6 };
	int m_cyItem{ 40 };
	int m_idxSel{ -1 };
	int m_idxHot{ -1 };
	int m_cyTopPadding{};
	int m_cyBottomPadding{};
	BITBOOL m_bSingleSel : 1{ TRUE };

	virtual void PaintItem(int idx, const D2D1_RECT_F& rcItem, const D2D1_RECT_F& rcPaint)
	{
		State eState;
		if (IsItemSel(idx))
			if (m_idxHot == idx)
				eState = State::HotSelected;
			else
				eState = State::Selected;
		else
			if (m_idxHot == idx)
				eState = State::Hot;
			else
				eState = State::Normal;
		if (eState != State::Normal)
			GetTheme()->DrawBackground(Part::ListItem, eState, rcItem);
	}

	void ReCalcScroll()
	{
		ECK_DUILOCK;
		m_psv->SetMinThumbSize(CxyMinScrollThumb);
		m_psv->SetRange(-m_cyTopPadding,
			GetItemCount() * (m_cyItem + m_cyPadding) + m_cyBottomPadding);
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

			int idxBegin = HitTestY((int)ps.rcfClipInElem.top);
			if (idxBegin < 0)
				idxBegin = 0;
			int idxEnd = HitTestY((int)ps.rcfClipInElem.bottom);
			if (idxEnd >= m_cItem && m_cItem)
				idxEnd = m_cItem - 1;
			if (idxBegin >= 0 && idxBegin <= idxEnd)
			{
				D2D1_RECT_F rcItem;
				rcItem.left = 0.f;
				rcItem.top = float(idxBegin * (m_cyItem + m_cyPadding)) - m_psv->GetPos();
				rcItem.right = GetWidthF();
				rcItem.bottom = rcItem.top + (float)m_cyItem;
				for (int idx = idxBegin; idx <= idxEnd; ++idx)
				{
					PaintItem(idx, rcItem, ps.rcfClipInElem);
					OffsetRect(rcItem, 0.f, float(m_cyItem + m_cyPadding));
				}
			}

			ECK_DUI_DBG_DRAW_FRAME;
			EndPaint(ps);
		}
		return 0;

		case WM_MOUSEMOVE:
		{
			POINT pt ECK_GET_PT_LPARAM(lParam);
			ClientToElem(pt);
			int idx = HitTest(pt);
			if (idx != m_idxHot)
			{
				ECK_DUILOCK;
				std::swap(m_idxHot, idx);
				if (m_idxHot >= 0)
					InvalidateItem(m_idxHot, FALSE);
				if (idx >= 0)
					InvalidateItem(idx, FALSE);
				if (m_idxHot >= 0 || idx >= 0)
					GetWnd()->WakeRenderThread();
			}
		}
		return 0;

		case WM_MOUSELEAVE:
		{
			int idx{ -1 };
			if (m_idxHot != idx)
			{
				std::swap(m_idxHot, idx);
				InvalidateItem(idx);
			}
		}
		return 0;

		case WM_LBUTTONDOWN:
		{
			POINT pt ECK_GET_PT_LPARAM(lParam);
			ClientToElem(pt);
			int idx = HitTest(pt);
			ECK_DUILOCK;
			LTN_ITEM nm;
			nm.uCode = LTE_ITEMCLICK;
			nm.idxItem = idx;
			GenElemNotify(&nm);
			if (m_bSingleSel)
			{
				std::swap(m_idxSel, idx);
				BOOL bInvalidate{};
				if (m_idxSel >= 0)
				{
					InvalidateItem(m_idxSel, FALSE);
					bInvalidate = TRUE;
				}
				if (idx >= 0 && m_idxSel != idx)
				{
					InvalidateItem(idx, FALSE);
					bInvalidate = TRUE;
				}
				if (bInvalidate)
					GetWnd()->WakeRenderThread();
			}
			else
			{
				int idx0, idx1;
				DeselectAll(idx0, idx1);
				BOOL bInvalidate{};
				if (idx0 >= 0)
				{
					InvalidateItemRange(idx0, idx1, FALSE);
					bInvalidate = TRUE;
				}
				if (idx >= 0)
				{
					m_SelRange.IncludeRange(idx, idx);
					InvalidateItem(idx, FALSE);
					bInvalidate = TRUE;
				}
				if (bInvalidate)
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
		{
			const auto cxSB = (int)GetTheme()->GetMetrics(Metrics::CxVScroll);
			m_SB.SetRect({
				GetWidth() - cxSB,
				m_cyTopPadding,
				GetWidth(),
				GetHeight() - m_cyBottomPadding });
			m_psv->SetPage(GetHeight());
		}
		return 0;

		case WM_CREATE:
		{
			m_SB.Create(nullptr, DES_VISIBLE, 0,
				0, 0, 0, 0, this, GetWnd());
			m_psv = m_SB.GetScrollView();
			m_psv->SetCallBack([](int iPos, int iPrevPos, LPARAM lParam)
				{
					((CListTemplate*)lParam)->InvalidateRect();
				}, (LPARAM)this);
			m_psv->AddRef();
		}
		break;

		case WM_DESTROY:
		{
			SafeRelease(m_psv);
		}
		}
		return __super::OnEvent(uMsg, wParam, lParam);
	}

	void SetItemCount(int cItem)
	{
		ECK_DUILOCK;
		m_cItem = cItem;
		if (m_bSingleSel)
		{
			if (m_idxSel >= m_cItem)
				m_idxSel = -1;
		}
		else
			m_SelRange.OnSetItemCount(m_cItem);
		if (m_idxHot >= m_cItem)
			m_idxHot = -1;
		ReCalcScroll();
	}
	EckInline constexpr int GetItemCount() const { return m_cItem; }

	constexpr int HitTestY(int y, BOOL bExcludePadding = FALSE) const
	{
		y += m_psv->GetPos();
		const int idx = y / (m_cyItem + m_cyPadding);
		if (bExcludePadding && y > idx * (m_cyItem + m_cyPadding) - m_cyPadding)
			return -1;
		else
			return idx;
	}

	constexpr int HitTest(POINT pt) const
	{
		if (pt.x < 0 || pt.x > GetWidth() || pt.y < 0 || pt.y > GetHeight())
			return -1;
		const int idx = HitTestY(pt.y);
		if (idx >= m_cItem)
			return -1;
		return idx;
	}

	EckInline constexpr BOOL IsItemSel(int idx) const
	{
		if (m_bSingleSel)
			return idx == m_idxSel;
		else
			return m_SelRange.IsSelected(idx);
	}

	EckInline constexpr int GetFirstVisibleItem() const
	{
		if (!m_cItem)
			return -1;
		return m_psv->GetPos() / (m_cyItem + m_cyPadding);
	}

	EckInline constexpr int GetLastVisibleItem() const
	{
		const int idx = (m_psv->GetPos() + GetHeight()) / (m_cyItem + m_cyPadding);
		return idx < m_cItem ? idx : m_cItem - 1;
	}

	void DeselectAll(int& idxChanged0, int& idxChanged1)
	{
		ECK_DUILOCK;
		if (m_bSingleSel)
			if (m_idxSel >= 0)
			{
				m_idxSel = -1;
				idxChanged0 = idxChanged1 = m_idxSel;
			}
			else
				idxChanged0 = idxChanged1 = -1;
		else
			m_SelRange.Clear(GetFirstVisibleItem(), GetLastVisibleItem(),
				idxChanged0, idxChanged1);
	}

	constexpr void GetItemRect(int idx, D2D1_RECT_F& rcItem) const
	{
		rcItem.left = 0.f;
		rcItem.top = float(idx * (m_cyItem + m_cyPadding)) - m_psv->GetPos();
		rcItem.right = GetWidthF();
		rcItem.bottom = rcItem.top + float(m_cyItem);
	}

	constexpr void GetItemRect(int idx0, int idx1, D2D1_RECT_F& rcItem) const
	{
		rcItem.left = 0.f;
		rcItem.top = float(idx0 * (m_cyItem + m_cyPadding)) - m_psv->GetPos();
		rcItem.right = GetWidthF();
		rcItem.bottom = float((idx1 + 1) * (m_cyItem + m_cyPadding)) -
			m_psv->GetPos() + m_cyItem;
	}

	constexpr void GetItemRect(int idx, RECT& rcItem) const
	{
		rcItem.left = 0;
		rcItem.top = idx * (m_cyItem + m_cyPadding) - m_psv->GetPos();
		rcItem.right = GetWidth();
		rcItem.bottom = rcItem.top + m_cyItem;
	}

	constexpr void GetItemRect(int idx0, int idx1, RECT& rcItem) const
	{
		rcItem.left = 0;
		rcItem.top = idx0 * (m_cyItem + m_cyPadding) - m_psv->GetPos();
		rcItem.right = GetWidth();
		rcItem.bottom = (idx1 + 1) * (m_cyItem + m_cyPadding) - m_psv->GetPos();
	}

	EckInline void InvalidateItem(int idx, BOOL bUpdateNow = TRUE)
	{
		RECT rc;
		GetItemRect(idx, rc);
		ElemToClient(rc);
		InvalidateRect(rc, bUpdateNow);
	}

	EckInline void InvalidateItemRange(int idx0, int idx1, BOOL bUpdateNow = TRUE)
	{
		RECT rc;
		GetItemRect(idx0, rc);
		rc.bottom = (idx1 + 1) * (m_cyItem + m_cyPadding) - m_psv->GetPos();
		ElemToClient(rc);
		InvalidateRect(rc, bUpdateNow);
	}

	EckInlineCe void SetItemHeight(int cyItem) { m_cyItem = cyItem; }
	EckInlineNdCe int GetItemHeight() const { return m_cyItem; }

	EckInlineCe void SetItemPadding(int cyPadding) { m_cyPadding = cyPadding; }
	EckInlineNdCe int GetItemPadding() const { return m_cyPadding; }

	EckInlineCe void SetSingleSel(BOOL bSingleSel) { m_bSingleSel = bSingleSel; }
	EckInlineNdCe BOOL GetSingleSel() const { return m_bSingleSel; }

	EckInline void SetTopPadding(int cyTopPadding)
	{
		ECK_DUILOCK;
		m_cyTopPadding = cyTopPadding;
		ReCalcScroll();
	}
	EckInlineNdCe int GetTopPadding() const { return m_cyTopPadding; }

	EckInline void SetBottomPadding(int cyBottomPadding)
	{
		ECK_DUILOCK;
		m_cyBottomPadding = cyBottomPadding;
		ReCalcScroll();
	}
	EckInlineNdCe int GetBottomPadding() const { return m_cyBottomPadding; }
};
ECK_DUI_NAMESPACE_END
ECK_NAMESPACE_END