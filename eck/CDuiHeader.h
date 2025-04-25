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

class CHeader : public CElem
{
public:
	enum : int
	{
		CxTextPadding = 3,
	};
private:
	struct ITEM
	{
		ComPtr<IDWriteTextLayout> pLayout;
		UINT uFlags;
		int x;
		int cx;
	};
	ID2D1SolidColorBrush* m_pBrush{};
	std::vector<ITEM> m_vItem{};
	std::vector<int> m_vMapToIdx{};
	int m_idxHot{ -1 };
	int m_idxPressed{ -1 };

	BITBOOL m_bDraggable : 1{};

	void PaintItem(int idx, ITEM& e, const D2D1_RECT_F& rcClip)
	{
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
					GetTextFormat(), float(e.cx - CxTextPadding * 2), GetHeightF(), &e.pLayout);
			}
		}
		if (e.pLayout.Get())
		{
			D2D1_COLOR_F cr;
			GetTheme()->GetSysColor(SysColor::Text, cr);
			m_pBrush->SetColor(cr);
			m_pDC->DrawTextLayout({ float(e.x + CxTextPadding),0.f }, e.pLayout.Get(),
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

		case WM_MOUSEMOVE:
		{
			ECK_DUILOCK;
			POINT pt ECK_GET_PT_LPARAM(lParam);
			ClientToElem(pt);
			auto idx = HitTestX(pt.x);
			std::swap(m_idxHot, idx);
			if (m_idxHot >= 0)
				InvalidateItem(m_idxHot);
			if (idx >= 0)
				InvalidateItem(idx);
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
		case WM_CREATE:
			m_pDC->CreateSolidColorBrush({}, &m_pBrush);
			break;
		case WM_DESTROY:
			SafeRelease(m_pBrush);
			break;
		}

		return __super::OnEvent(uMsg, wParam, lParam);
	}

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

	constexpr [[nodiscard]] int HitTestX(int x) const noexcept
	{
		for (int i{}; i < GetItemCount(); ++i)
		{
			if (x >= m_vItem[i].x && x < m_vItem[i].x + m_vItem[i].cx)
				return i;
		}
		return -1;
	}

	void GetItemRect(int idx,RECT& rcItem) const noexcept
	{
		const auto& e = m_vItem[idx];
		rcItem.left = e.x;
		rcItem.right = e.x + e.cx;
		rcItem.top = 0;
		rcItem.bottom = GetHeightF();
	}

	void InvalidateItem(int idx)
	{
		RECT rcItem;
		GetItemRect(idx,rcItem);
		ElemToClient(rcItem);
		InvalidateRect(rcItem);
	}
};
ECK_DUI_NAMESPACE_END
ECK_NAMESPACE_END