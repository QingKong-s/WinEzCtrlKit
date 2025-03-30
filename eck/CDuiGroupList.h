#pragma once
#include "CDuiScrollBar.h"

ECK_NAMESPACE_BEGIN
ECK_DUI_NAMESPACE_BEGIN
struct GL_ITEMINFO
{
	PCWSTR pszText;
	int cchText;
	int idxItem;
	int idxGroup;
};

struct GL_GROUPINFO
{
	PCWSTR pszText;
	int cchText;
	int idxItem;
	ID2D1DeviceContext* pDC;
	ID2D1Bitmap1* pBmp;
};

struct GL_GETDISPINFO
{
	Dui::DUINMHDR nmhdr;
	BOOL bItem;
	union
	{
		GL_ITEMINFO Item;
		GL_GROUPINFO Group;
	};
};

struct GL_HITTEST
{
	int idxGroup;
	BITBOOL bHitCover : 1;
	BITBOOL bHitGroupHeader : 1;
};


class CGroupList :public CElem
{
public:
	enum class Part
	{
		GroupHeader,
		GroupText,
		Item,
		Cover,
	};
private:
	enum :UINT
	{
		ITF_LAYOUT_CACHE_VALID = 1u << 0,
		ITF_SELECTED = 1u << 1,
	};

	struct ITEM
	{
		int y{};
		UINT uFlags{};
		IDWriteTextLayout* pLayout{};
	};

	struct GROUPITEM
	{
		int y{};
		UINT uFlags{};
		std::vector<ITEM> Item{};
		IDWriteTextLayout* pLayout{};
		float cxText{};
	};

	CScrollBar m_SB{};

	IDWriteTextFormat* m_pTextFormat{};
	IDWriteTextFormat* m_pTfGroupTitle{};

	ID2D1SolidColorBrush* m_pBr{};

	CInertialScrollView* m_psv{};

	std::vector<ITEM> m_Item{};
	std::vector<GROUPITEM> m_Group{};

	int m_cyItem{};				// 项目高度
	int m_cyGroupHeader{ 40 };	// 组头高度

	int m_idxTop{};				// 第一个可见项目
	int m_idxTopGroup{};		// 第一个可见组
	int m_idxHot{ -1 };			// 热点项
	int m_idxHotItemGroup{ -1 };// 热点项所在的组
	int m_idxHotGroup{ -1 };	// 热点组

	int m_oyTop = 0;			// 第一个可见组或项目的遮挡高度

	int m_cxCover = 0;			// 图片宽度

	BOOL m_bGroup = TRUE;		// 是否启用组

	constexpr static int c_iMinLinePerGroup = 3;

	int m_iDpi = USER_DEFAULT_SCREEN_DPI;
	ECK_DS_BEGIN(DPIS)
		ECK_DS_ENTRY_F(cxTextPadding, 4.f)
		ECK_DS_ENTRY_F(EmText, 12.f)
		ECK_DS_ENTRY_F(EmGroupText, 16.f)
		ECK_DS_ENTRY_F(cyGroupLine, 1.f)
		;
	ECK_DS_END_VAR(m_DsF);

	BOOL DrawGroupItem(int idxGroup, int idxItem, const D2D1_RECT_F& rcPaint)
	{
		auto& e = m_Group[idxGroup].Item[idxItem];
		D2D1_RECT_F rcItem;
		GetGroupPartRect(rcItem, Part::Item, idxGroup, idxItem);

		if (rcItem.bottom <= rcPaint.top || rcItem.top >= rcPaint.bottom)
			return FALSE;

		if (!(e.uFlags & ITF_LAYOUT_CACHE_VALID))
		{
			GL_GETDISPINFO sldi{};
			sldi.nmhdr.uCode = GLE_GETDISPINFO;
			sldi.bItem = TRUE;
			sldi.Item.cchText = -1;
			sldi.Item.idxItem = idxItem;
			sldi.Item.idxGroup = idxGroup;
			GenElemNotify(&sldi);

			SafeRelease(e.pLayout);
			g_pDwFactory->CreateTextLayout(sldi.Item.pszText, sldi.Item.cchText, m_pTextFormat,
				GetWidthF() - (float)m_cxCover, (float)m_cyItem, &e.pLayout);
			e.uFlags |= ITF_LAYOUT_CACHE_VALID;
		}

		if (e.pLayout)
		{
			D2D1_COLOR_F cr;
			if (idxItem == m_idxHot && idxGroup == m_idxHotItemGroup)
			{
				if (e.uFlags & ITF_SELECTED)
					GetTheme()->GetColor(Dui::Part::ListItem, Dui::State::HotSelected,
						eck::ClrPart::Bk, cr);
				else
					GetTheme()->GetColor(Dui::Part::ListItem, Dui::State::Hot,
						eck::ClrPart::Bk, cr);
				m_pBr->SetColor(cr);
				m_pDC->FillRectangle(rcItem, m_pBr);
			}
			else if (e.uFlags & ITF_SELECTED)
			{
				GetTheme()->GetColor(Dui::Part::ListItem, Dui::State::Selected,
					eck::ClrPart::Bk, cr);
				m_pBr->SetColor(cr);
				m_pDC->FillRectangle(rcItem, m_pBr);
			}

			rcItem.left += m_DsF.cxTextPadding;
			GetTheme()->GetSysColor(Dui::SysColor::Text, cr);
			m_pBr->SetColor(cr);
			m_pDC->DrawTextLayout({ rcItem.left,rcItem.top }, e.pLayout,
				m_pBr, D2D1_DRAW_TEXT_OPTIONS_NONE);
			rcItem.left -= m_DsF.cxTextPadding;
		}
		return TRUE;
	}

	BOOL DrawGroup(int idxGroup, const D2D1_RECT_F& rcPaint)
	{
		auto& e = m_Group[idxGroup];

		D2D1_RECT_F rcText;
		GetGroupPartRect(rcText, Part::GroupHeader, idxGroup, 0);

		D2D1_RECT_F rcCover;
		GetGroupPartRect(rcCover, Part::Cover, idxGroup, 0);

		const BOOL bText = !(rcText.bottom <= rcPaint.top || rcText.top >= rcPaint.bottom);
		const BOOL bCover = !(rcCover.bottom <= rcPaint.top || rcCover.top >= rcPaint.bottom ||
			rcCover.left >= rcPaint.right || rcCover.right <= rcPaint.left);
		if (!bText && !bCover)
			return FALSE;

		GL_GETDISPINFO sldi{};
		if (bText || bCover)
		{
			sldi.nmhdr.uCode = GLE_GETDISPINFO;
			sldi.bItem = FALSE;
			sldi.Group.cchText = -1;
			sldi.Group.idxItem = idxGroup;
			sldi.Group.pDC = m_pDC;
			GenElemNotify(&sldi);
		}

		if (bText)
		{
			if (!(e.uFlags & ITF_LAYOUT_CACHE_VALID))
			{
				SafeRelease(e.pLayout);
				g_pDwFactory->CreateTextLayout(sldi.Group.pszText, sldi.Group.cchText, m_pTfGroupTitle,
					GetWidthF(), (float)m_cyGroupHeader, &e.pLayout);
				DWRITE_TEXT_METRICS tm;
				e.pLayout->GetMetrics(&tm);
				e.cxText = tm.width;
				e.uFlags |= ITF_LAYOUT_CACHE_VALID;
			}
			if (sldi.Group.pszText)
			{
				D2D1_COLOR_F cr;
				GetTheme()->GetSysColor(Dui::SysColor::Text, cr);
				m_pBr->SetColor(cr);
#pragma warning(suppress: 6387)// 可能为nullptr
				m_pDC->DrawTextLayout({ rcText.left,rcText.top }, e.pLayout,
					m_pBr, D2D1_DRAW_TEXT_OPTIONS_ENABLE_COLOR_FONT);

				const float yLine = rcText.top + (float)(m_cyGroupHeader / 2);
				m_pDC->DrawLine({ rcText.left + m_DsF.cxTextPadding * 2.f + e.cxText,yLine },
					{ GetWidthF() - m_DsF.cxTextPadding,yLine }, m_pBr, m_DsF.cyGroupLine);
			}
		}

		if (bCover && sldi.Group.pBmp)
			m_pDC->DrawBitmap(sldi.Group.pBmp, &rcCover);
		return TRUE;
	}

	void DrawItem(int idxItem, D2D1_RECT_F& rcItem, const D2D1_RECT_F& rcPaint)
	{
		auto& e = m_Item[idxItem];

		if (!(e.uFlags & ITF_LAYOUT_CACHE_VALID))
		{
			GL_GETDISPINFO sldi{};
			sldi.nmhdr.uCode = GLE_GETDISPINFO;
			sldi.bItem = TRUE;
			sldi.Item.cchText = -1;
			sldi.Item.idxItem = idxItem;
			sldi.Item.idxGroup = -1;
			GenElemNotify(&sldi);

			SafeRelease(e.pLayout);
			g_pDwFactory->CreateTextLayout(sldi.Item.pszText, sldi.Item.cchText, m_pTextFormat,
				GetWidthF() - (float)m_cxCover, (float)m_cyItem, &e.pLayout);
			e.uFlags |= ITF_LAYOUT_CACHE_VALID;
		}

		if (e.pLayout)
		{
			D2D1_COLOR_F cr;
			if (idxItem == m_idxHot)
			{
				if (e.uFlags & ITF_SELECTED)
					GetTheme()->GetColor(Dui::Part::ListItem, Dui::State::HotSelected,
						eck::ClrPart::Bk, cr);
				else
					GetTheme()->GetColor(Dui::Part::ListItem, Dui::State::Hot,
						eck::ClrPart::Bk, cr);
				m_pBr->SetColor(cr);
				m_pDC->FillRectangle(rcItem, m_pBr);
			}
			else if (e.uFlags & ITF_SELECTED)
			{
				GetTheme()->GetColor(Dui::Part::ListItem, Dui::State::Selected,
					eck::ClrPart::Bk, cr);
				m_pBr->SetColor(cr);
				m_pDC->FillRectangle(rcItem, m_pBr);
			}

			rcItem.left += m_DsF.cxTextPadding;
			GetTheme()->GetSysColor(Dui::SysColor::Text, cr);
			m_pBr->SetColor(cr);
			m_pDC->DrawTextLayout({ rcItem.left,rcItem.top }, e.pLayout,
				m_pBr, D2D1_DRAW_TEXT_OPTIONS_NONE);
			rcItem.left -= m_DsF.cxTextPadding;
		}
	}
public:
	LRESULT OnEvent(UINT uMsg, WPARAM wParam, LPARAM lParam) override
	{
		switch (uMsg)
		{
		case WM_PAINT:
		{
			Dui::ELEMPAINTSTRU eps;
			BeginPaint(eps, wParam, lParam);

			if (m_bGroup)
			{
				if (m_Group.empty())
				{
					EndPaint(eps);
					return 0;
				}

				const int iSbPos = m_psv->GetPos();
				auto it = std::lower_bound(m_Group.begin() + m_idxTopGroup, m_Group.end(),
					eps.prcClip->top + iSbPos, [](const GROUPITEM& x, int iPos)
					{
						return x.y < iPos;
					});
				if (it != m_Group.begin())
					--it;

				for (int i = (int)std::distance(m_Group.begin(), it); i < GetGroupCount(); ++i)
				{
					const auto& e = m_Group[i];
					if (e.y >= (int)eps.rcfClipInElem.bottom + iSbPos)
						break;
					DrawGroup(i, eps.rcfClipInElem);
					for (int j = (i == m_idxTopGroup ? m_idxTop : 0); j < (int)e.Item.size(); ++j)
					{
						const auto& f = e.Item[j];
						if (f.y >= (int)eps.rcfClipInElem.bottom + iSbPos)
							break;
						DrawGroupItem(i, j, eps.rcfClipInElem);
					}
				}
			}
			else
			{
				const auto idxTop = (DWORD)std::max(m_idxTop + (int)eps.prcClip->top / m_cyItem,
					m_idxTop);
				const auto idxBottom = (DWORD)std::min(m_idxTop + (int)eps.prcClip->bottom / m_cyItem,
					GetItemCount() - 1);
				D2D1_RECT_F rcItem;
				GetItemRect(rcItem, idxTop);

				for (DWORD i = idxTop; i <= idxBottom; ++i)
				{
					DrawItem(i, rcItem, eps.rcfClipInElem);
					eck::OffsetRect(rcItem, 0.f, (float)m_cyItem);
				}
			}
			EndPaint(eps);
		}
		return 0;

		case WM_SIZE:
		{
			m_SB.SetRect({ GetWidth() - (int)GetTheme()->GetMetrics(Dui::Metrics::CxVScroll),0,GetWidth(),GetHeight() });
			m_psv->SetPage(GetHeight());
		}
		return 0;

		case WM_MOUSEMOVE:
		{
			POINT pt ECK_GET_PT_LPARAM(lParam);
			ClientToElem(pt);
			int idxHot;
			GL_HITTEST slht;
			idxHot = HitTest(pt, slht);
			if (m_bGroup)
			{
				if (idxHot != m_idxHot || m_idxHotItemGroup != slht.idxGroup)
				{
					std::swap(idxHot, m_idxHot);
					std::swap(slht.idxGroup, m_idxHotItemGroup);
					if (slht.idxGroup >= 0 && idxHot >= 0)
						InvalidateItem(slht.idxGroup, idxHot);
					if (m_idxHotItemGroup >= 0 && m_idxHot >= 0)
						InvalidateItem(m_idxHotItemGroup, m_idxHot);
				}
			}
			else
			{
				if (idxHot != m_idxHot)
				{
					std::swap(idxHot, m_idxHot);
					if (idxHot >= 0)
						InvalidateItem(idxHot);
					if (m_idxHot >= 0)
						InvalidateItem(m_idxHot);
				}
			}
		}
		return 0;

		case WM_MOUSELEAVE:
		{
			int idx = -1;
			if (m_bGroup)
			{
				int idxGroup = -1;
				std::swap(idx, m_idxHot);
				std::swap(idxGroup, m_idxHotItemGroup);
				if (idxGroup >= 0 && idx >= 0)
					InvalidateItem(idxGroup, idx);
			}
			else
			{
				std::swap(idx, m_idxHot);
				if (idx >= 0)
					InvalidateItem(idx);
			}
		}
		return 0;

		case WM_MOUSEWHEEL:
			m_psv->OnMouseWheel2(-GET_WHEEL_DELTA_WPARAM(wParam) / WHEEL_DELTA);
			GetWnd()->WakeRenderThread();
			return 0;

		case WM_NOTIFY:
		{
			if ((Dui::CElem*)wParam == &m_SB)
			{
				switch (((Dui::DUINMHDR*)lParam)->uCode)
				{
				case Dui::EE_VSCROLL:
					ECK_DUILOCK;
					CalcTopItem();
					InvalidateRect();
					return TRUE;
				}
			}
		}
		return 0;

		case WM_LBUTTONDOWN:
		{
			int y0{ INT_MAX }, y1, x;
			if (GetAsyncKeyState(VK_CONTROL) & 0x8000)
				;
			else
				DeselectAll(y0, y1, x);

			POINT pt ECK_GET_PT_LPARAM(lParam);
			ClientToElem(pt);
			int idxHot;
			GL_HITTEST slht;
			if (m_bGroup)
			{
				idxHot = HitTest(pt, slht);
				if (slht.idxGroup >= 0 && idxHot >= 0)
				{
					m_Group[slht.idxGroup].Item[idxHot].uFlags |= ITF_SELECTED;
					if (y0 == INT_MAX)
						InvalidateItem(slht.idxGroup, idxHot);
					else
					{
						RECT rc{ x, y0, GetWidth(), y1 };
						RECT rcItem;
						GetGroupPartRect(rcItem, Part::Item, slht.idxGroup, idxHot);
						eck::UnionRect(rc, rc, rcItem);
						InvalidateRect(rc);
					}
				}
				else
				{
					if (y0 != INT_MAX)
					{
						const RECT rc{ x, y0, GetWidth(), y1 };
						InvalidateRect(rc);
					}
				}
			}
		}
		return 0;

		case WM_LBUTTONUP:
		{

		}
		return 0;

		case WM_CREATE:
		{
			eck::UpdateDpiSizeF(m_DsF, GetWnd()->GetDpiValue());

			m_SB.Create(nullptr, Dui::DES_VISIBLE | Dui::DES_TRANSPARENT, 0,
				0, 0, 0, 0, this, GetWnd());
			m_SB.SetRect({ GetWidth() - (int)(int)GetTheme()->GetMetrics(Dui::Metrics::CxVScroll),0,GetWidth(),GetHeight() });
			m_psv = m_SB.GetScrollView();
			m_psv->AddRef();
			m_psv->SetCallBack([](int iPos, int iPrevPos, LPARAM lParam)
				{
					const auto p = (CGroupList*)lParam;
					p->CalcTopItem();
					p->InvalidateRect();
				}, (LPARAM)this);

			g_pDwFactory->CreateTextFormat(L"微软雅黑", nullptr, DWRITE_FONT_WEIGHT_NORMAL,
				DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, m_DsF.EmText, L"zh-cn", &m_pTextFormat);
			m_pTextFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
			m_pTextFormat->SetWordWrapping(DWRITE_WORD_WRAPPING_NO_WRAP);
			IDWriteInlineObject* pEllipsisTrimming{};
			g_pDwFactory->CreateEllipsisTrimmingSign(m_pTextFormat, &pEllipsisTrimming);
			DWRITE_TRIMMING Opt{ .granularity = DWRITE_TRIMMING_GRANULARITY_CHARACTER };
			m_pTextFormat->SetTrimming(&Opt, pEllipsisTrimming);
			pEllipsisTrimming->Release();

			g_pDwFactory->CreateTextFormat(L"微软雅黑", nullptr, DWRITE_FONT_WEIGHT_NORMAL,
				DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, m_DsF.EmGroupText, L"zh-cn", &m_pTfGroupTitle);
			m_pTfGroupTitle->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
			m_pTfGroupTitle->SetWordWrapping(DWRITE_WORD_WRAPPING_NO_WRAP);
			g_pDwFactory->CreateEllipsisTrimmingSign(m_pTfGroupTitle, &pEllipsisTrimming);
			m_pTfGroupTitle->SetTrimming(&Opt, pEllipsisTrimming);
			pEllipsisTrimming->Release();

			m_pDC->CreateSolidColorBrush({}, &m_pBr);
		}
		break;

		case WM_DESTROY:
		{
			SafeRelease(m_pTextFormat);
			SafeRelease(m_pTfGroupTitle);
			SafeRelease(m_pBr);
			SafeRelease(m_psv);
		}
		break;
		}
		return __super::OnEvent(uMsg, wParam, lParam);
	}

	void CalcTopItem()
	{
		if (m_bGroup)
		{
			if (!m_Group.size())
			{
				m_idxTopGroup = m_idxTop = 0;
				return;
			}
			const auto it = std::lower_bound(m_Group.begin(), m_Group.end(), m_psv->GetPos(),
				[](const GROUPITEM& x, int iPos)
				{
					return x.y < iPos;
				});

#ifdef _DEBUG
			if (it == m_Group.end())
				EckDbgBreak();
#endif

			if (it == m_Group.begin())
				m_idxTopGroup = 0;
			else
				m_idxTopGroup = (int)std::distance(m_Group.begin(), it - 1);
			EckDbgPrint(m_idxTopGroup);
			const auto& e = m_Group[m_idxTopGroup];
			m_oyTop = e.y - m_psv->GetPos();
			m_idxTop = (m_psv->GetPos() - (e.y + m_cyGroupHeader)) / m_cyItem;
		}
		else
		{
			m_idxTop = m_psv->GetPos() / m_cyItem;
			m_oyTop = m_psv->GetPos() - m_idxTop * m_cyItem;
		}
	}

	constexpr BOOL GetGroup() const { return m_bGroup; }

	constexpr void SetGroup(BOOL bGroup) { m_bGroup = bGroup; }

	constexpr void SetItemCount(int cItems) { m_Item.resize(cItems); }
	constexpr int GetItemCount() const { return (int)m_Item.size(); }

	constexpr void SetGroupCount(int cGroups) { m_Group.resize(cGroups); }
	constexpr int GetGroupCount() const { return (int)m_Group.size(); }

	constexpr void SetGroupItemCount(int idxGroup, int cItems) { m_Group[idxGroup].Item.resize(cItems); }
	constexpr int GetGroupItemCount(int idxGroup) const { return (int)m_Group[idxGroup].Item.size(); }

	/// <summary>
	/// 重新计算项目位置
	/// </summary>
	/// <param name="idxBegin">若当前启用组，则指示起始组索引；否则指示起始项索引</param>
	void ReCalc(int idxBegin)
	{
		if (m_bGroup)
		{
			int y = 0;
			for (int i = idxBegin; i < m_Group.size(); ++i)
			{
				auto& Group = m_Group[i];
				Group.y = y;
				y += m_cyGroupHeader;
				for (auto& Item : Group.Item)
				{
					Item.y = y;
					y += m_cyItem;
				}

				if (Group.Item.size() < c_iMinLinePerGroup)
					y += ((c_iMinLinePerGroup - (int)Group.Item.size()) * m_cyItem);
			}
			m_psv->SetMax(y);
		}
		else
		{
			m_psv->SetMax((int)m_Item.size() * m_cyItem);
		}
	}

	constexpr int GetItemHeight() const { return m_cyItem; }

	void SetItemHeight(int cyItem)
	{
		m_cyItem = cyItem;
		m_cxCover = cyItem * 3 - cyItem / 4;
		m_psv->SetDelta(m_cyItem * 3);
	}

	int HitTest(POINT pt, GL_HITTEST& pslht)
	{
		pslht.bHitCover = FALSE;
		pslht.bHitGroupHeader = FALSE;
		pslht.idxGroup = -1;
		if (pt.x < 0 || pt.x >= GetWidth() || pt.y < 0 || pt.y >= GetHeight())
			return -1;
		if (m_bGroup)
		{
			const int y = pt.y + m_psv->GetPos();
			auto it = std::lower_bound(m_Group.begin() + m_idxTop, m_Group.end(), y,
				[](const GROUPITEM& x, int iPos)
				{
					return x.y < iPos;
				});

			if (it == m_Group.begin())
				return -1;
			else if (it == m_Group.end())
				it = (m_Group.rbegin() + 1).base();
			else
				--it;
			const int idxGroup = (int)std::distance(m_Group.begin(), it);
			pslht.idxGroup = idxGroup;
			const int yGroupBottom = it->y + m_cyGroupHeader;
			if (y > it->y && y < yGroupBottom)
			{
				pslht.bHitGroupHeader = TRUE;
				return -1;
			}
			if (pt.x < m_cxCover && y >= yGroupBottom && y < yGroupBottom + m_cxCover)
			{
				pslht.bHitCover = TRUE;
				return -1;
			}

			const int yInItem = y - yGroupBottom;
			if (yInItem >= 0)
			{
				int idx = yInItem / m_cyItem;
				if (idx < (int)it->Item.size())
					return idx;
			}
			return -1;
		}
		else
		{
			const int idx = (pt.y + m_psv->GetPos()) / m_cyItem;
			if (idx >= (int)m_Item.size())
				return -1;
			else
				return idx;
		}
	}

	void GetGroupPartRect(RECT& rc, Part ePart, int idxGroup, int idxItemInGroup)
	{
		auto& e = m_Group[idxGroup];
		switch (ePart)
		{
		case Part::GroupHeader:
			rc =
			{
				(int)m_DsF.cxTextPadding,
				e.y - m_psv->GetPos(),
				GetWidth(),
				e.y - m_psv->GetPos() + m_cyGroupHeader,
			};
			break;
		case Part::GroupText:
			EckDbgBreak();
			break;
		case Part::Item:
			rc =
			{
				m_cxCover,
				e.Item[idxItemInGroup].y - m_psv->GetPos(),
				GetWidth(),
				e.Item[idxItemInGroup].y - m_psv->GetPos() + m_cyItem,
			};
			break;
		case Part::Cover:
			rc =
			{
				0,
				e.y - m_psv->GetPos() + m_cyGroupHeader,
				m_cxCover,
				e.y - m_psv->GetPos() + m_cyGroupHeader + m_cxCover
			};
			break;
		default:
			EckDbgBreak();
			break;
		}
	}

	void GetGroupPartRect(D2D1_RECT_F& rc, Part ePart, int idxGroup, int idxItemInGroup)
	{
		RECT rc2;
		GetGroupPartRect(rc2, ePart, idxGroup, idxItemInGroup);
		rc = eck::MakeD2DRcF(rc2);
	}

	void GetItemRect(RECT& rc, int idxItem) 
	{
		rc =
		{
			0,
			idxItem * m_cyItem - m_psv->GetPos(),
			GetWidth(),
			(idxItem + 1) * m_cyItem - m_psv->GetPos(),
		};
	}

	void GetItemRect(D2D1_RECT_F& rc, int idxItem)
	{
		rc =
		{
			0.f,
			float(idxItem * m_cyItem - m_psv->GetPos()),
			GetWidthF(),
			float((idxItem + 1) * m_cyItem - m_psv->GetPos()),
		};
	}

	void InvalidateItem(int idxGroup, int idxItemInGroup)
	{
		EckAssert(GetGroup());
		RECT rc;
		GetGroupPartRect(rc, Part::Item, idxGroup, idxItemInGroup);
		ElemToClient(rc);
		InvalidateRect(rc);
	}

	void InvalidateItem(int idxItem)
	{
		EckAssert(!GetGroup());
		RECT rc
		{
			0,
			idxItem * m_cyItem - m_psv->GetPos(),
			GetWidth(),
			(idxItem + 1) * m_cyItem - m_psv->GetPos(),
		};
		ElemToClient(rc);
		InvalidateRect(rc);
	}

	void DeselectAll(int& y0, int& y1, int& x)
	{
		if (m_bGroup)
		{
			const int iSbPos = m_psv->GetPos();
			const int cy = GetHeight();
			int y0_{ INT_MAX }, y1_{ INT_MIN }, x_{ INT_MAX };
			for (auto& e : m_Group)
			{
				if (e.uFlags & ITF_SELECTED)
				{
					e.uFlags &= ~ITF_SELECTED;
					const int y = e.y - iSbPos;
					if ((y > -m_cyGroupHeader && y < GetHeight()))
					{
						if (y0_ == INT_MAX)
							y0_ = y;
						y1_ = std::max(y1_, y + m_cyGroupHeader);
						x_ = std::min(x_, 0);
					}
				}
				for (auto& f : e.Item)
				{
					if (f.uFlags & ITF_SELECTED)
					{
						f.uFlags &= ~ITF_SELECTED;
						const int y = f.y - iSbPos;
						if ((y > -m_cyItem && y < GetHeight()))
						{
							if (y0_ == INT_MAX)
								y0_ = y;
							y1_ = std::max(y1_, y + m_cyItem);
						}
						x_ = std::min(x_, m_cxCover);
					}
				}
			}
			y0 = y0_;
			y1 = y1_;
			x = x_;
		}
		else
		{

		}
	}
};
ECK_DUI_NAMESPACE_END
ECK_NAMESPACE_END