#pragma once
#include "DuiBase.h"

ECK_NAMESPACE_BEGIN
ECK_DUI_NAMESPACE_BEGIN
enum : UINT
{

};

struct HEE_DISPINFO : DUINMHDR
{
	DispInfoMask uMask;
	int idx;

	PCWSTR pszText;
	int cchText;

	int idxImg;
	ID2D1Bitmap* pImg;
};

struct HEE_ITEMNOTIFY : DUINMHDR
{
	int idx;
};
struct HEE_DRAG : HEE_ITEMNOTIFY
{
	BOOL bDragDivider;
};

struct HE_HITTEST
{
	POINT pt;
	BOOLEAN bHitDivider;
};

class CHeader : public CElem, public CFixedTimeLine
{
public:
	enum : int
	{
		CxDividerWidthHalf = 6,
	};
private:
	struct ITEM
	{
		ComPtr<IDWriteTextLayout> pLayout;
		int x{};
		int cx{};
		float k{};
	};
	ID2D1SolidColorBrush* m_pBrush{};
	std::vector<ITEM> m_vItem{};
	std::vector<int> m_vMapToIdx{};
	int m_idxHot{ -1 };
	int m_idxPressed{ -1 };
	int m_idxDrag{ -1 };
	int m_idxInsertMark{ -1 };
	int m_cxItemOld{};// 拖动分隔条时用
	BITBOOL m_bDraggable : 1{};

	BITBOOL m_bHitDivider : 1{};
	BITBOOL m_bDragging : 1{};
	BITBOOL m_bDraggingDivider : 1{};
	BITBOOL m_bAnimating : 1{};

	void PaintItem(int idx, ITEM& e, const D2D1_RECT_F& rcClip)
	{
		const float Padding = GetTheme()->GetMetrics(Metrics::Padding);
		D2D1_RECT_F rcItem;
		State eState;
		if (m_idxPressed == idx)
			eState = State::Selected;
		else if (m_idxHot == idx)
			eState = State::Hot;
		else
			eState = State::Normal;
		rcItem.top = 0;
		rcItem.bottom = GetHeightF();
		rcItem.left = (float)e.x;
		rcItem.right = float(e.x + e.cx);
		GetTheme()->DrawBackground(Part::HeaderItem, eState,
			rcItem, nullptr);

		if (!e.pLayout.Get())
		{
			HEE_DISPINFO nm;
			nm.uCode = HEE_GETDISPINFO;
			nm.uMask = DIM_TEXT;
			nm.idx = idx;
			GenElemNotify(&nm);
			if (nm.pszText)
			{
				EckAssert(nm.cchText > 0);
				eck::g_pDwFactory->CreateTextLayout(nm.pszText, nm.cchText,
					GetTextFormat(), float(e.cx - Padding * 2), GetHeightF(), &e.pLayout);
			}
		}
		if (e.pLayout.Get())
		{
			D2D1_COLOR_F cr;
			GetTheme()->GetSysColor(SysColor::Text, cr);
			m_pBrush->SetColor(cr);
			m_pDC->DrawTextLayout({ float(e.x + Padding),0.f }, e.pLayout.Get(),
				m_pBrush, DrawTextLayoutFlags);
		}
	}

	void ReCalcItemX()
	{
		if (!GetItemCount())
			return;
		if (m_bDraggable)
		{

		}
		else
		{
			m_vItem[0].x = 0;
			for (int i{ 1 }; i < GetItemCount(); ++i)
				m_vItem[i].x = m_vItem[i - 1].x + m_vItem[i - 1].cx;
		}
	}

	BOOL OnSetCursor()
	{
		if (m_bHitDivider)
		{
			SetCursor(LoadCursorW(nullptr, IDC_SIZEWE));
			return TRUE;
		}
		return FALSE;
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
			int xMax{};
			for (int i{}; i < GetItemCount(); ++i)
			{
				auto& e = m_vItem[i];
				if (e.x > (int)ps.rcfClipInElem.right ||
					e.x + e.cx < (int)ps.rcfClipInElem.left)
					continue;
				if (e.x + e.cx > xMax)
					xMax = e.x + e.cx;
				PaintItem(i, e, ps.rcfClipInElem);
			}
			D2D1_RECT_F rcItem;
			rcItem.top = ps.rcfClipInElem.top;
			rcItem.bottom = ps.rcfClipInElem.bottom;
			rcItem.left = (float)xMax;
			rcItem.right = ps.rcfClipInElem.right;
			GetTheme()->DrawBackground(Part::HeaderItem, State::Normal,
				rcItem, nullptr);
			ECK_DUI_DBG_DRAW_FRAME;
			EndPaint(ps);
		}
		return 0;

		case WM_SETCURSOR:
			return OnSetCursor();
		case WM_MOUSEMOVE:
		{
			ECK_DUILOCK;
			HE_HITTEST ht{ ECK_GET_PT_LPARAM(lParam) };
			ClientToElem(ht.pt);
			auto idx = HitTest(ht);
			if (m_bDraggingDivider)
			{
				const auto cxNew = ht.pt.x - m_vItem[m_idxDrag].x;
				if (cxNew != m_vItem[m_idxDrag].cx)
				{
					m_vItem[m_idxDrag].cx = cxNew;
					ReCalcItemX();
					RECT rc;
					GetItemRect(m_idxDrag, rc);
					rc.right = GetWidth();
					ElemToClient(rc);
					InvalidateRect(rc);
					HEE_ITEMNOTIFY nm;
					nm.uCode = HEE_WIDTHCHANGED;
					nm.idx = m_idxDrag;
					GenElemNotify(&nm);
				}
			}
			else if (m_bDragging)
			{

			}
			else
			{
				if (m_bHitDivider != ht.bHitDivider)
				{
					m_bHitDivider = ht.bHitDivider;
					OnSetCursor();
				}
				std::swap(m_idxHot, idx);
				if (m_idxHot >= 0)
					InvalidateItem(m_idxHot);
				if (idx >= 0)
					InvalidateItem(idx);
			}
		}
		break;
		case WM_MOUSELEAVE:
		{
			ECK_DUILOCK;
			int idx{ -1 };
			std::swap(m_idxHot, idx);
			if (idx >= 0)
				InvalidateItem(idx);
		}
		break;
		case WM_LBUTTONDOWN:
		{
			ECK_DUILOCK;
			HE_HITTEST ht{ ECK_GET_PT_LPARAM(lParam) };
			ClientToElem(ht.pt);
			const auto idx = HitTest(ht);
			if (idx >= 0)
			{
				SetCapture();
				m_idxDrag = idx;
				if (ht.bHitDivider)
				{
					m_bDraggingDivider = TRUE;
					m_cxItemOld = GetItem(idx).cx;
				}
				else
					m_bDragging = TRUE;
				HEE_DRAG nm;
				nm.uCode = HEE_BEGINDRAG;
				nm.bDragDivider = ht.bHitDivider;
				nm.idx = idx;
				GenElemNotify(&nm);
			}
		}
		break;
		case WM_LBUTTONUP:
		{
			if (m_idxDrag >= 0)
			{
				ReleaseCapture();
				HEE_DRAG nm;
				nm.uCode = HEE_ENDDRAG;
				nm.bDragDivider = m_bDraggingDivider;
				nm.idx = m_idxDrag;
				m_idxDrag = -1;
				m_bDragging = FALSE;
				m_bDraggingDivider = FALSE;
				GenElemNotify(&nm);
			}
		}
		break;
		case WM_CREATE:
			m_pDC->CreateSolidColorBrush({}, &m_pBrush);
			break;
		case WM_DESTROY:
			SafeRelease(m_pBrush);
			break;
		}

		return __super::OnEvent(uMsg, wParam, lParam);
	}

	void Tick(int iMs)
	{

	}

	BOOL IsValid() { return m_bAnimating; }

	void InvalidateCache(int idx = -1) noexcept
	{
		ECK_DUILOCK;
		if (idx < 0)
			for (auto& e : m_vItem)
				e.pLayout.Clear();
		else
			m_vItem[idx].pLayout.Clear();
	}

	void SetItemCount(int cItem, _In_reads_opt_(cItem) const int* pcx = nullptr) noexcept
	{
		ECK_DUILOCK;
		m_vItem.resize(cItem);
		if (m_bDraggable)
			m_vMapToIdx.resize(cItem);
		if (pcx)
		{
			for (int i{}; i < cItem; ++i)
				m_vItem[i].cx = pcx[i];
			ReCalcItemX();
		}
	}
	EckInlineNdCe int GetItemCount() const noexcept { return (int)m_vItem.size(); }

	EckInlineNdCe int OrderToIndex(int iOrder) const noexcept
	{
		if (m_bDraggable)
			return m_vMapToIdx[iOrder];
		return iOrder;
	}

	constexpr [[nodiscard]] int HitTest(HE_HITTEST& ht) const noexcept
	{
		for (int i{}; i < GetItemCount(); ++i)
		{
			const auto& e = m_vItem[i];
			if (ht.pt.x >= e.x &&
				ht.pt.x < e.x + e.cx + CxDividerWidthHalf)
			{
				if (ht.pt.x > e.x + e.cx - CxDividerWidthHalf)
					ht.bHitDivider = TRUE;// 分隔条左界
				return i;
			}
		}
		return -1;
	}

	void GetItemRect(int idx, RECT& rcItem) const noexcept
	{
		const auto& e = m_vItem[idx];
		rcItem.left = e.x;
		rcItem.right = e.x + e.cx;
		rcItem.top = 0;
		rcItem.bottom = GetHeight();
	}

	void InvalidateItem(int idx)
	{
		RECT rcItem;
		GetItemRect(idx, rcItem);
		ElemToClient(rcItem);
		InvalidateRect(rcItem);
	}

	int GetContentWidth() const noexcept
	{
		if (!GetItemCount())
			return 0;
		auto& e = GetItem(OrderToIndex(GetItemCount() - 1));
		return e.x + e.cx;
	}
	EckInlineNdCe const ITEM& GetItem(int idx) const noexcept { return m_vItem[idx]; }
};
ECK_DUI_NAMESPACE_END
ECK_NAMESPACE_END