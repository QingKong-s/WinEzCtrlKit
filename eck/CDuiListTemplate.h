#pragma once
#include "CDuiScrollBar.h"
#include "CDuiHeader.h"
#include "CD2DImageList.h"

#if !ECKCXX20
#error "EckDui requires C++20"
#endif

ECK_NAMESPACE_BEGIN
ECK_DUI_NAMESPACE_BEGIN
struct LTN_ITEM : DUINMHDR
{
	int idx;
	int idxGroup;
};

struct LE_HITTEST
{
	POINT pt;
	int idxGroup;
	BOOLEAN bHitGroupHeader;
	BOOLEAN bHitGroupImage;
};

// 项目标志
enum
{
	LEIF_SELECTED = (1u << 0),
};


class CListTemplate :public CElem
{
public:
	enum class Type : BYTE
	{
		List,
		Icon,
		Report,
	};
	enum : int
	{
		CyInsertMark = 3,
		CyDefHeader = 30,
		CyGroupLine = 1,
	};
	enum class ListPart
	{
		GroupHeader,
		GroupText,
		Item,
		GroupImg,
	};
protected:
	struct SUBITEM
	{
		ComPtr<IDWriteTextLayout> pLayout{};
	};
	struct ITEM
	{
		ComPtr<IDWriteTextLayout> pLayout{};
		UINT uFlags{};
		int y{};
		std::vector<SUBITEM> vSubItem{};
	};
	struct GROUP
	{
		ComPtr<IDWriteTextLayout> pLayout{};
		UINT uFlags{};
		int y{};
		std::vector<ITEM> Item{};
	};

	std::vector<ITEM> m_vItem{};// 项目
	std::vector<GROUP> m_Group{};// 组
	CScrollBar m_SBV{}, m_SBH{};
	CHeader m_Header{};
	ID2D1SolidColorBrush* m_pBrush{};
	CInertialScrollView* m_psvV{};
	CInertialScrollView* m_psvH{};
	CD2DImageList* m_pImgList{};
	CD2DImageList* m_pImgListGroup{};
	IDWriteTextFormat* m_pTfGroup{};
	//---通用
	int m_idxHot{ -1 };			// 热点项
	int m_idxSel{ -1 };			// 选中的项的索引，仅用于单选
	int m_idxInsertMark{ -1 };	// 插入标记应当显示在哪一项之前
	int m_idxFocus{ -1 };		// 焦点项
	int m_idxMark{ -1 };		// 标记项

	int m_cyTopExtra{};			// 顶部空白
	int m_cyBottomExtra{};		// 底部空白

	int m_cyItem{ 40 };			// 项目高度
	int m_cyPadding{ 3 };		// 项目间距

	int m_oyTopItem{};			// 小于等于零的值，指示第一可见项的遮挡高度
	int m_idxTop{};				// 第一个可见项
	//---分组模式
	int m_idxHotItemGroup{ -1 };	// 热点项所在的组
	int m_idxSelItemGroup{ -1 };	// 选中项所在的组
	int m_idxInsertMarkGroup{ -1 };	// 插入标记所在的组
	int m_idxFocusItemGroup{ -1 };	// 焦点项所在的组
	int m_idxMarkItemGroup{ -1 };	// 标记项所在的组
	int m_cyGroupHeader{ 40 };	// 组头高度
	int m_idxTopGroup{};		// 第一个可见组
	int m_idxHotGroup{ -1 };	// 热点组
	int m_cxGroupImage{};		// 组图片宽度
	int m_cyGroupImage{};		// 组图片高度
	int m_cyGroupItemTopPadding{};		// 项目区顶部空白
	int m_cyGroupItemBottomPadding{};	// 项目区底部空白
	//---图标模式
	int m_cxItem{ 40 };			// 项目宽度
	int m_cxPadding{ 3 };		// 水平项目间距
	int m_cItemPerRow{};		// 每行项目数
	//---
	POINT m_ptDragSelStart{};	// 拖动选择起始点
	RECT m_rcDragSel{};			// 当前选择矩形
	int m_dCursorToItemMax{};	// 鼠标指针到项目的最大距离

	Type m_eView{ Type::List };	// 视图类型

	BITBOOL m_bSingleSel : 1{};	// 单选
	BITBOOL m_bGroup : 1{};		// 分组
	BITBOOL m_bGroupImage : 1{};// 显示组图片

	BITBOOL m_bDraggingSel : 1{};	// 正在拖动选择


	virtual void GRPaintGroup(int idxGroup, const D2D1_RECT_F& rcPaint) {}

	virtual void LVPaintSubItem(int idx, int idxSub, int idxGroup,
		const D2D1_RECT_F& rcSub, const D2D1_RECT_F& rcPaint) {
	}

	virtual void LVPaintItem(int idx, int idxGroup, const D2D1_RECT_F& rcPaint)
	{
		RECT rc;
		if (m_bGroup)
		{
			GetGroupPartRect(ListPart::Item, idx, idxGroup, rc);
			State eState;
			if ((m_Group[idxGroup].Item[idx].uFlags & LEIF_SELECTED) ||
				(m_bSingleSel && m_idxSel == idx && m_idxSelItemGroup == idxGroup))
				if (idx == m_idxHot && idxGroup == m_idxHotItemGroup)
					eState = State::HotSelected;
				else
					eState = State::Selected;
			else if (idx == m_idxHot && idxGroup == m_idxHotItemGroup)
				eState = State::Hot;
			else
				eState = State::None;
			if (eState != State::None)
				GetTheme()->DrawBackground(Part::ListItem,
					eState, MakeD2DRcF(rc), nullptr);
			GetGroupSubItemRect(idx, 0, idxGroup, rc);
			LVPaintSubItem(idx, 0, idxGroup, MakeD2DRcF(rc), rcPaint);
			if (m_eView == Type::Report)
				for (int i = 1; i < m_Header.GetItemCount(); i++)
				{
					GetGroupSubItemRect(idx, i, idxGroup, rc);
					LVPaintSubItem(idx, i, idxGroup, MakeD2DRcF(rc), rcPaint);
				}
		}
		else
		{
			GetItemRect(idx, rc);
			State eState;
			if ((m_vItem[idx].uFlags & LEIF_SELECTED) ||
				(m_bSingleSel && m_idxSel == idx))
				if (m_idxHot == idx)
					eState = State::HotSelected;
				else
					eState = State::Selected;
			else if (m_idxHot == idx)
				eState = State::Hot;
			else
				eState = State::None;
			if (eState != State::None)
				GetTheme()->DrawBackground(Part::ListItem,
					eState, MakeD2DRcF(rc), nullptr);

			GetSubItemRect(idx, 0, rc);
			LVPaintSubItem(idx, 0, -1, MakeD2DRcF(rc), rcPaint);
			if (m_eView == Type::Report)
				for (int i = 1; i < m_Header.GetItemCount(); i++)
				{
					GetSubItemRect(idx, i, rc);
					LVPaintSubItem(idx, i, -1, MakeD2DRcF(rc), rcPaint);
				}
		}
	}

	virtual void IVPaintItem(int idx, const D2D1_RECT_F& rcPaint) {}

	virtual void PostPaint(ELEMPAINTSTRU& ps) {}

	void ReCalcHScroll()
	{
		if (m_eView == Type::Report)
		{
			m_psvH->SetPage(GetWidth());
			m_psvH->SetRange(0, m_Header.GetContentWidth() + GetRealGroupImageWidth());
			m_SBH.SetVisible(m_psvH->IsVisible());
		}
	}

	void ReCalcScroll()
	{
		if (!m_cyItem || !m_cxItem)
			return;
		ReCalcHScroll();
		m_psvV->SetPage(GetHeight());
		if (m_bGroup)
			return;
		switch (m_eView)
		{
		case Type::List:
		case Type::Report:
		{
			m_psvV->SetRange(-m_cyTopExtra, GetItemCount() *
				(m_cyItem + m_cyPadding) + m_cyBottomExtra);
		}
		break;
		case Type::Icon:
		{
			m_cItemPerRow = (GetWidth() + m_cxPadding) / (m_cxItem + m_cxPadding);
			const int cItemV = (GetItemCount() - 1) / m_cItemPerRow + 1;
			m_psvV->SetRange(-m_cyTopExtra, cItemV *
				(m_cyItem + m_cyPadding) + m_cyBottomExtra);
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
		case Type::List:
		case Type::Report:
		{
			if (m_bGroup)
			{
				if (m_Group.empty())
				{
					m_idxTopGroup = m_idxTop = 0;
					return;
				}
				const auto it = std::lower_bound(m_Group.begin(), m_Group.end(),
					m_psvV->GetPos(), [](const GROUP& x, int iPos)
					{
						return x.y < iPos;
					});
				EckAssert(it != m_Group.end());
				if (it == m_Group.begin())
					m_idxTopGroup = 0;
				else
					m_idxTopGroup = (int)std::distance(m_Group.begin(), it - 1);
				EckDbgPrint(m_idxTopGroup);
				const auto& e = m_Group[m_idxTopGroup];
				m_oyTopItem = e.y - m_psvV->GetPos();
				m_idxTop = (m_psvV->GetPos() -
					(e.y + m_cyGroupHeader + m_cyGroupItemTopPadding)) / m_cyItem;
				if (m_idxTop < 0)
					m_idxTop = 0;
			}
			else
			{
				m_idxTop = m_psvV->GetPos() / (m_cyItem + m_cyPadding);
				m_oyTopItem = m_idxTop * (m_cyItem + m_cyPadding) - m_psvV->GetPos();
			}
		}
		break;
		case Type::Icon:
		{
			const int cItemV = m_psvV->GetPos() / (m_cyItem + m_cyPadding);
			m_idxTop = cItemV * m_cItemPerRow;
			m_oyTopItem = cItemV * (m_cyItem + m_cyPadding) - m_psvV->GetPos();
		}
		break;
		default: ECK_UNREACHABLE;
		}
	}

	void ArrangeHeader(BOOL bRePos = FALSE)
	{
		auto cxContent = m_Header.GetContentWidth();
		cxContent = std::max(cxContent, GetWidth());
		if (bRePos)
		{
			RECT rc;
			rc.left = -m_psvH->GetPos();
			if (m_bGroup && m_bGroupImage)
				rc.left += m_cxGroupImage;
			rc.right = rc.left + cxContent;
			rc.top = 0;
			rc.bottom = m_Header.GetHeight();
			m_Header.SetRect(rc);
		}
		else
		{
			if (m_Header.GetWidth() != cxContent)
				m_Header.SetSize(cxContent, m_Header.GetHeight());
		}
	}

	EckInlineNdCe int GetRealGroupImageWidth() const
	{
		return (m_bGroupImage && m_bGroup) ? m_cxGroupImage : 0;
	}

	// 从X坐标计算逻辑项目索引，索引相对当前可见范围
	EckInline int IVLogItemFromX(int x) const
	{
		EckAssert(m_eView == Type::Icon);
		return x / (m_cxItem + m_cxPadding);
	}
	// 从Y坐标计算逻辑项目索引，索引相对当前可见范围
	// m_idxTop所在行的下一行记为0，上一行记为-1
	EckInline int IVLogItemFromY(int y) const
	{
		EckAssert(m_eView == Type::Icon);
		const auto i = (y - m_oyTopItem) / (m_cyItem + m_cyPadding);
		return (y - m_oyTopItem < 0) ? i - 1 : i;
	}

	EckInline std::pair<int, int> IVGetItemXY(int idx) const
	{
		EckAssert(m_eView == Type::Icon);
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
		EckAssert(m_eView == Type::List || m_eView == Type::Report);
		return m_oyTopItem + (idx - m_idxTop) * (m_cyItem + m_cyPadding);
	}
	// 由Y坐标得到索引
	EckInline int LVItemFromY(int y) const
	{
		EckAssert(m_eView == Type::List || m_eView == Type::Report);
		return m_idxTop + (y - m_oyTopItem) / (m_cyItem + m_cyPadding);
	}

	void CalcItemRangeInRect(const RECT& rc, _Out_ int& idxBegin, _Out_ int& idxEnd)
	{
		switch (m_eView)
		{
		case Type::List:
		case Type::Report:
			idxBegin = LVItemFromY(rc.top);
			idxBegin = std::clamp(idxBegin, 0, GetItemCount() - 1);
			idxEnd = LVItemFromY(rc.bottom);
			idxEnd = std::clamp(idxEnd, 0, GetItemCount() - 1);
			break;
		case Type::Icon:
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

	// 由索引得到组，坐标相对元素
	int GRGroupFromY(int y, _Out_ int& idxItemInGroup) const
	{
		y += m_psvV->GetPos();
		auto it = std::lower_bound(m_Group.begin(), m_Group.end(), y,
			[](const GROUP& x, int iPos)
			{
				return x.y < iPos;
			});
		if (it == m_Group.begin())
			;
		else if (it == m_Group.end())
			it = (m_Group.rbegin() + 1).base();
		else
			--it;
		int idxGroup = (int)std::distance(m_Group.begin(), it);
		const int yGroupBottom = it->y +
			m_cyGroupHeader + m_cyGroupItemTopPadding;
		const int yInItem = y - yGroupBottom;
		idxItemInGroup = yInItem / (m_cyItem + m_cyPadding);
		if (idxItemInGroup < 0)
			idxItemInGroup = 0;
		else if (idxItemInGroup >= (int)it->Item.size())
		{
			if (idxGroup + 1 >= GetGroupCount())
				idxItemInGroup = (int)it->Item.size() - 1;
			else
			{
				++idxGroup;
				idxItemInGroup = 0;
			}
		}
		return idxGroup;
	}

	void CalcItemRangeInRect(const RECT& rc, _Out_ int& idxBegin, _Out_ int& idxEnd,
		_Out_ int& idxGroupBegin, _Out_ int& idxGroupEnd)
	{
		idxGroupBegin = GRGroupFromY(rc.top, idxBegin);
		idxGroupEnd = GRGroupFromY(rc.bottom, idxEnd);
	}

	void GRDragSelMouseMove(POINT pt, WPARAM wParam)
	{
		const auto dy = m_psvV->GetPos();
		EckAssert(m_bDraggingSel);
		RECT rcOld{ m_rcDragSel };
		OffsetRect(rcOld, 0, -dy);

		m_rcDragSel = MakeRect(pt, POINT{ m_ptDragSelStart.x,m_ptDragSelStart.y - dy });

		int idxBegin, idxEnd, idxGroupBegin, idxGroupEnd;
		RECT rcItem;
		RECT rcJudge;
		UnionRect(rcJudge, rcOld, m_rcDragSel);
		if (IsRectEmpty(rcJudge))
			goto Skip;

		CalcItemRangeInRect(rcJudge, idxBegin, idxEnd, idxGroupBegin, idxGroupEnd);
		if (idxBegin < 0 || idxEnd < 0)
			goto Skip;
		if (idxGroupBegin < 0)
			idxGroupBegin = 0;
		if (idxGroupEnd >= GetGroupCount())
			idxGroupEnd = GetGroupCount() - 1;
		for (int i = idxGroupBegin; i <= idxGroupEnd; ++i)
		{
			auto& f = m_Group[i];
			int j0, j1;
			if (i == idxGroupBegin)
				j0 = idxBegin;
			else
				j0 = 0;
			if (i == idxGroupEnd)
				j1 = idxEnd;
			else
				j1 = (int)f.Item.size() - 1;
			for (int j = j0; j <= j1; ++j)
			{
				auto& e = f.Item[j];
				GetGroupPartRect(ListPart::Item, j, i, rcItem);
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
		}
	Skip:
		OffsetRect(m_rcDragSel, 0, dy);
		InvalidateRect();
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

	void OnPaint(WPARAM wParam, LPARAM lParam)
	{
		ELEMPAINTSTRU ps;
		BeginPaint(ps, wParam, lParam);

		switch (m_eView)
		{
		case Type::Report:
			if (!m_Header.GetItemCount())
				goto EndPaintLabel;
			[[fallthrough]];
		case Type::List:
		{
			if (m_bGroup)
			{
				if (m_Group.empty())
					goto EndPaintLabel;

				const int iSbPos = m_psvV->GetPos();
				auto it = std::lower_bound(m_Group.begin() + m_idxTopGroup, m_Group.end(),
					(int)ps.rcfClipInElem.top + iSbPos, [](const GROUP& x, int iPos)
					{
						return x.y < iPos;
					});
				if (it != m_Group.begin())
					--it;

				for (int i = (int)std::distance(m_Group.begin(), it); i < GetGroupCount(); ++i)
				{
					const auto& e = m_Group[i];
					if (e.y >= (int)ps.rcfClipInElem.bottom + iSbPos)
						break;
					GRPaintGroup(i, ps.rcfClipInElem);
					for (int j = (i == m_idxTopGroup ? m_idxTop : 0); j < (int)e.Item.size(); ++j)
					{
						const auto& f = e.Item[j];
						if (f.y >= (int)ps.rcfClipInElem.bottom + iSbPos)
							break;
						LVPaintItem(j, i, ps.rcfClipInElem);
					}
				}
			}
			else
			{
				if (!GetItemCount())
					goto EndPaintLabel;
				const int idxBegin = std::max(LVItemFromY((int)ps.rcfClipInElem.top), 0);
				const int idxEnd = std::min(LVItemFromY((int)ps.rcfClipInElem.bottom), GetItemCount() - 1);
				for (int i = idxBegin; i <= idxEnd; ++i)
					LVPaintItem(i, -1, ps.rcfClipInElem);
			}
		}
		break;
		case Type::Icon:
		{
			if (!GetItemCount())
				goto EndPaintLabel;
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

					IVPaintItem(i, ps.rcfClipInElem);
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

		if (m_bDraggingSel)
		{
			auto rc{ MakeD2DRcF(m_rcDragSel) };
			OffsetRect(rc, 0.f, (float)-m_psvV->GetPos());
			GetTheme()->DrawBackground(Part::ListSelRect,
				State::None, rc, nullptr);
		}

		PostPaint(ps);
	EndPaintLabel:
		ECK_DUI_DBG_DRAW_FRAME;
		EndPaint(ps);
	}

	void IVGetItemRangeRect(int idxBegin, int idxEnd, _Out_ RECT& rc)
	{
		EckAssert(m_eView == Type::Icon);
		if (idxBegin == idxEnd)
			RedrawItem(idxBegin);
		else
		{
			auto [x1, y1] = IVGetItemXY(idxBegin);
			auto [x2, y2] = IVGetItemXY(idxEnd);
			if (y1 == y2)
			{
				rc =
				{
					x1,
					y1,
					x2 + m_cxItem,
					y2 + m_cyItem
				};
			}
			else
			{
				rc =
				{
					0,
					y1,
					GetWidth(),
					y2 + m_cyItem
				};
			}
		}
	}
public:
	LRESULT OnEvent(UINT uMsg, WPARAM wParam, LPARAM lParam) override
	{
		switch (uMsg)
		{
		case WM_PAINT:
			OnPaint(wParam, lParam);
			return 0;

		case WM_MOUSEMOVE:
		{
			ECK_DUILOCK;
			LE_HITTEST ht{ ECK_GET_PT_LPARAM(lParam) };
			ClientToElem(ht.pt);
			if (m_bDraggingSel)
			{
				if (ht.pt.x < 0) ht.pt.x = 0;
				if (ht.pt.y < 0) ht.pt.y = 0;
				if (ht.pt.x >= GetWidth()) ht.pt.x = GetWidth() - 1;
				if (ht.pt.y >= GetHeight()) ht.pt.y = GetHeight() - 1;
				if (m_bGroup)
					GRDragSelMouseMove(ht.pt, wParam);
				else
					DragSelMouseMove(ht.pt, wParam);
				return 0;
			}

			int idx = HitTest(ht);
			if (m_bGroup)
			{
				if (idx != m_idxHot || m_idxHotItemGroup != ht.idxGroup)
				{
					std::swap(idx, m_idxHot);
					std::swap(ht.idxGroup, m_idxHotItemGroup);
					if (ht.idxGroup >= 0 && idx >= 0)
						RedrawGroupItem(ht.idxGroup, idx);
					if (m_idxHotItemGroup >= 0 && m_idxHot >= 0)
						RedrawGroupItem(m_idxHotItemGroup, m_idxHot);
				}
			}
			else if (idx != m_idxHot)
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
			if (m_bGroup)
			{
				int idx = -1, idxGroup = -1;
				std::swap(idx, m_idxHot);
				std::swap(idxGroup, m_idxHotItemGroup);
				if (idxGroup >= 0 && idx >= 0)
					RedrawGroupItem(idxGroup, idx);
			}
			else if (m_idxHot >= 0)
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
			if (wParam & MK_SHIFT)
				m_psvH->OnMouseWheel2(-GET_WHEEL_DELTA_WPARAM(wParam) / WHEEL_DELTA);
			else
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
			else if ((wParam == (WPARAM)&m_SBH) &&
				(((DUINMHDR*)lParam)->uCode == EE_HSCROLL))
			{
				m_Header.SetPos(-m_psvH->GetPos() + GetRealGroupImageWidth(), 0);
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
					EckAssert(m_eView == Type::Report);
					const auto* const p = (HEE_ITEMNOTIFY*)lParam;
					ArrangeHeader(TRUE);
					ReCalcHScroll();
					RECT rc;
					m_Header.GetItemRect(p->idx, rc);
					if (m_bGroup)
					{
						if (p->idx)
							for (auto& e : m_Group)
								for (auto& f : e.Item)
									f.vSubItem[p->idx - 1].pLayout.Clear();
						else
							for (auto& e : m_Group)
								for (auto& f : e.Item)
									f.pLayout.Clear();
						rc.left += GetRealGroupImageWidth();
					}
					else
					{
						if (p->idx)
							for (auto& e : m_vItem)
								e.vSubItem[p->idx - 1].pLayout.Clear();
						else
							for (auto& e : m_vItem)
								e.pLayout.Clear();
					}
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
			m_SBV.SetRect({
				GetWidth() - cxSB,
				m_cyTopExtra,
				GetWidth(),
				GetHeight() - m_cyBottomExtra });
			m_SBH.SetRect({ 0,GetHeight() - cxSB,GetWidth(),GetHeight() });
			ArrangeHeader();
		}
		return 0;

		case WM_LBUTTONDOWN:
		{
			ECK_DUILOCK;
			POINT pt ECK_GET_PT_LPARAM(lParam);
			SetFocus();
			LE_HITTEST ht{ pt };
			ClientToElem(ht.pt);
			int idx = HitTest(ht);

			RECT rcInvalid;
			if (idx >= 0)
			{
				if (!(wParam & MK_CONTROL))
					DeselectAll(rcInvalid);
				RECT rcItem;
				if (m_bGroup)
				{
					SelectItemForClick(idx, ht.idxGroup);
					GetGroupPartRect(ListPart::Item, idx, ht.idxGroup, rcItem);
				}
				else
				{
					SelectItemForClick(idx);
					GetItemRect(idx, rcItem);
				}
				UnionRect(rcInvalid, rcInvalid, rcItem);
				ElemToClient(rcInvalid);
				InvalidateRect(rcInvalid);
			}
			else
			{
				ClientToScreen(GetWnd()->HWnd, &pt);
				if (IsMouseMovedBeforeDragging(GetWnd()->HWnd, pt.x, pt.y))
				{
					if (!GetWnd()->IsValid())
						return 0;
					SetCapture();
					if (!(wParam & (MK_CONTROL | MK_SHIFT)))
					{
						RECT rcInvalid;
						DeselectAll(rcInvalid);
						ElemToClient(rcInvalid);
						InvalidateRect(rcInvalid);
					}
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
					DeselectAll(rcInvalid);
					ElemToClient(rcInvalid);
					InvalidateRect(rcInvalid);
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

			m_Header.Create(nullptr, /*DES_VISIBLE*/0, 0,
				0, 0, 0, CyDefHeader, this);

			m_SBV.Create(nullptr, DES_VISIBLE, 0,
				0, 0, 0, 0, this);
			m_psvV = m_SBV.GetScrollView();
			m_psvV->AddRef();
			m_psvV->SetMinThumbSize(CxyMinScrollThumb);
			m_psvV->SetCallBack([](int iPos, int iPrevPos, LPARAM lParam)
				{
					auto pThis = (CListTemplate*)lParam;
					pThis->ReCalcTopItem();
					pThis->InvalidateRect();
				}, (LPARAM)this);
			m_psvV->SetDelta(80);

			m_SBH.Create(nullptr, 0, 0,
				0, 0, 0, 0, this);
			m_SBH.SetHorizontal(TRUE);
			m_psvH = m_SBH.GetScrollView();
			m_psvH->AddRef();
			m_psvH->SetMinThumbSize(CxyMinScrollThumb);
			m_psvH->SetCallBack([](int iPos, int iPrevPos, LPARAM lParam)
				{
					auto pThis = (CListTemplate*)lParam;
					pThis->m_Header.SetPos(-iPos + pThis->GetRealGroupImageWidth(), 0);
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
		if (m_eView == Type::Report)
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
		case Type::List:
			rc.left = 0;
			rc.right = GetWidth();
			rc.top = LVGetItemY(idx);
			rc.bottom = rc.top + m_cyItem;
			break;
		case Type::Report:
		{
			rc.left = -m_psvH->GetPos();
			rc.right = rc.left + m_Header.GetContentWidth();
			rc.top = LVGetItemY(idx);
			rc.bottom = rc.top + m_cyItem;
		}
		break;
		case Type::Icon:
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
		RECT rc2;
		GetItemRect(idx, rc2);
		rc = MakeD2DRcF(rc2);
	}

	void GetSubItemRect(int idx, int idxSub, RECT& rc) const noexcept
	{
		EckAssert(idx < GetItemCount());
		if (m_eView != Type::Report)
		{
			GetItemRect(idx, rc);
			return;
		}
		m_Header.GetItemRect(idxSub, rc);
		rc.left -= m_psvH->GetPos();
		rc.right -= m_psvH->GetPos();
		rc.top = LVGetItemY(idx);
		rc.bottom = rc.top + m_cyItem;
	}

	void GetGroupPartRect(ListPart ePart, int idxItemInGroup,
		int idxGroup, _Out_ RECT& rc) const noexcept
	{
		auto& e = m_Group[idxGroup];
		int dx, cxContent;
		switch (m_eView)
		{
		case Type::List:
			dx = 0;
			cxContent = GetWidth();
			break;
		case Type::Report:
			dx = -m_psvH->GetPos();
			cxContent = m_Header.GetContentWidth();
			break;
		default: ECK_UNREACHABLE;
		}
		switch (ePart)
		{
		case ListPart::GroupHeader:
			rc =
			{
				dx,
				e.y - m_psvV->GetPos(),
				dx + cxContent,
				e.y - m_psvV->GetPos() + m_cyGroupHeader,
			};
			break;
		case ListPart::GroupText:
			rc = {};
			EckDbgBreak();
			break;
		case ListPart::Item:
			rc =
			{
				dx + m_cxGroupImage,
				e.Item[idxItemInGroup].y - m_psvV->GetPos(),
				dx + m_cxGroupImage + cxContent,
				e.Item[idxItemInGroup].y - m_psvV->GetPos() + m_cyItem,
			};
			break;
		case ListPart::GroupImg:
		{
			const auto y = e.y - m_psvV->GetPos() +
				m_cyGroupHeader + m_cyGroupItemTopPadding;
			rc = { dx,y,dx + m_cxGroupImage,y + m_cyGroupImage };
		}
		break;
		default: ECK_UNREACHABLE;
		}
	}

	EckInline void GetGroupPartRect(ListPart ePart, int idxItemInGroup,
		int idxGroup, _Out_ D2D1_RECT_F& rc) const noexcept
	{
		RECT rc2;
		GetGroupPartRect(ePart, idxItemInGroup, idxGroup, rc2);
		rc = MakeD2DRcF(rc2);
	}

	void GetGroupSubItemRect(int idx, int idxSub, int idxGroup, RECT& rc) const noexcept
	{
		EckAssert(idxGroup < GetGroupCount());
		EckAssert(idx < (int)m_Group[idxGroup].Item.size());
		GetGroupPartRect(ListPart::Item, idx, idxGroup, rc);
		if (m_eView != Type::Report)
			return;
		RECT rc2;
		m_Header.GetItemRect(idxSub, rc2);
		if (m_bGroupImage)
		{
			rc2.left += m_cxGroupImage;
			rc2.right += m_cxGroupImage;
		}
		rc.left = rc2.left - m_psvH->GetPos();
		rc.right = rc2.right - m_psvH->GetPos();
	}

	int HitTest(LE_HITTEST& leht) const
	{
		leht.bHitGroupHeader = FALSE;
		leht.bHitGroupImage = FALSE;
		if (!PtInRect(GetViewRect(), leht.pt))
			return -1;

		switch (m_eView)
		{
		case Type::List:
		case Type::Report:
		{
			if (m_bGroup)
			{
				if (!GetGroupCount())
					return -1;
				//---测试组
				const int y = leht.pt.y + m_psvV->GetPos();
				auto it = std::lower_bound(m_Group.begin() + m_idxTopGroup, m_Group.end(), y,
					[](const GROUP& x, int iPos)
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
				leht.idxGroup = idxGroup;
				// 测试组部件
				const int yGroupBottom = it->y +
					m_cyGroupHeader + m_cyGroupItemTopPadding;
				if (y > it->y && y < yGroupBottom)
				{
					leht.bHitGroupHeader = TRUE;
					return -1;
				}
				if (m_bGroupImage && leht.pt.x < m_cxGroupImage)
				{
					if (y >= yGroupBottom &&
						y < yGroupBottom + m_cyGroupImage)
						leht.bHitGroupImage = TRUE;
					return -1;
				}
				//---测试组项
				const int yInItem = y - yGroupBottom;
				if (yInItem >= 0)
				{
					int idx = yInItem / (m_cyItem + m_cyPadding);
					if (yInItem > (idx + 1) * (m_cyItem + m_cyPadding) - m_cyPadding)
						return -1;// 命中项目间隔
					if (idx < (int)it->Item.size())
						return idx;
				}
				return -1;
			}
			if (!GetItemCount())
				return -1;
			if (m_eView == Type::Report &&
				leht.pt.x > m_Header.GetContentWidth())
				return -1;
			const int idx = LVItemFromY(leht.pt.y);
			if (leht.pt.y >= LVGetItemY(idx) + m_cyItem)
				return -1;// 命中项目间隔
			if (idx >= 0 && idx < GetItemCount())
				return idx;
			else
				return -1;
		}
		break;

		case Type::Icon:
		{
			if (!GetItemCount())
				return -1;
			const int idxX = IVLogItemFromX(leht.pt.x);
			if (idxX < 0 || idxX >= m_cItemPerRow)
				return -1;
			const int idxY = IVLogItemFromY(leht.pt.y);
			const int idx = m_idxTop + idxX + idxY * m_cItemPerRow;
			if (idx < 0 || idx >= GetItemCount())
				return -1;
			const auto [x, y] = IVGetItemXY(idx);
			if (leht.pt.x >= x + m_cxItem ||
				leht.pt.y >= y + m_cyItem)
				return -1;// 命中项目间隔
			return idx;
		}
		break;
		default: ECK_UNREACHABLE;
		}
	}

	void DeselectAll(_Out_ RECT& rcInvalid)
	{
		if (m_bGroup)
		{
			const int iSbPos = m_psvV->GetPos();
			const int cy = GetHeight();
			int y0{ INT_MAX }, y1{ INT_MIN }, x{ INT_MAX };
			for (auto& e : m_Group)
			{
				if (e.uFlags & LEIF_SELECTED)
				{
					e.uFlags &= ~LEIF_SELECTED;
					const int y = e.y - iSbPos;
					if ((y > -m_cyGroupHeader && y < GetHeight()))
					{
						if (y0 == INT_MAX)
							y0 = y;
						y1 = std::max(y1, y + m_cyGroupHeader);
						x = std::min(x, 0);
					}
				}
				for (auto& f : e.Item)
				{
					if (f.uFlags & LEIF_SELECTED)
					{
						f.uFlags &= ~LEIF_SELECTED;
						const int y = f.y - iSbPos;
						if ((y > -m_cyItem && y < GetHeight()))
						{
							if (y0 == INT_MAX)
								y0 = y;
							y1 = std::max(y1, y + m_cyItem);
						}
						x = std::min(x, m_cxGroupImage);
					}
				}
			}
			rcInvalid = { x, y0, GetWidth(), y1 };
		}
		else
		{
			if (m_bSingleSel)
			{
				if (m_idxSel >= 0)
				{
					GetItemRect(m_idxSel, rcInvalid);
					m_idxSel = -1;
				}
				else
					rcInvalid = {};
			}
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
				if (idx0 >= 0)
				{
					if (m_eView == Type::Icon)
						IVGetItemRangeRect(idx0, idx1, rcInvalid);
					else
					{
						GetItemRect(idx0, rcInvalid);
						RECT rc;
						GetItemRect(idx1, rc);
						UnionRect(rcInvalid, rcInvalid, rc);
					}
				}
				else
					rcInvalid = {};
			}
		}
	}

	void SelectItemForClick(int idx)
	{
		LTN_ITEM nm{ LTE_ITEMCLICK };
		nm.idx = idx;
		nm.idxGroup = -1;
		GenElemNotify(&nm);
		m_idxFocus = idx;
		m_idxMark = idx;
		if (m_bSingleSel)
			m_idxSel = idx;
		else
			m_vItem[idx].uFlags |= LEIF_SELECTED;
	}

	void SelectItemForClick(int idx, int idxGroup)
	{
		LTN_ITEM nm{ LTE_ITEMCLICK };
		nm.idx = idx;
		nm.idxGroup = idxGroup;
		GenElemNotify(&nm);
		m_idxFocus = idx;
		m_idxFocusItemGroup = idxGroup;
		m_idxMark = idx;
		m_idxMarkItemGroup = idxGroup;
		if (m_bSingleSel)
		{
			m_idxSel = idx;
			m_idxSelItemGroup = idxGroup;
		}
		else
			m_Group[idxGroup].Item[idx].uFlags |= LEIF_SELECTED;
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

	void SetGroupImageList(CD2DImageList* pImgList)
	{
		ECK_DUILOCK;
		std::swap(m_pImgListGroup, pImgList);
		if (m_pImgListGroup)
			m_pImgListGroup->AddRef();
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
		RECT rc;
		switch (m_eView)
		{
		case Type::List:
		{
			rc =
			{
				0,
				LVGetItemY(idxBegin),
				GetWidth(),
				LVGetItemY(idxEnd) + m_cyItem
			};
		}
		break;
		case Type::Report:
		{
			rc =
			{
				-m_psvH->GetPos(),
				LVGetItemY(idxBegin),
				std::min(GetWidth(), m_Header.GetContentWidth()),
				LVGetItemY(idxEnd) + m_cyItem
			};
		}
		break;
		case Type::Icon:
			IVGetItemRangeRect(idxBegin, idxEnd, rc);
			break;
		default: ECK_UNREACHABLE;
		}
		ElemToClient(rc);
		InvalidateRect(rc);
	}

	void RedrawItem(int idx)
	{
		RECT rc;
		GetItemRect(idx, rc);
		ElemToClient(rc);
		InvalidateRect(rc);
	}

	void RedrawGroupItem(int idxGroup, int idxItemInGroup)
	{
		EckAssert(GetGroup());
		RECT rc;
		GetGroupPartRect(ListPart::Item, idxItemInGroup, idxGroup, rc);
		ElemToClient(rc);
		InvalidateRect(rc);
	}

	void ReCalc(int idxGroupBegin = 0)
	{
		if (m_bGroup)
		{
			int y = 0;
			for (size_t i = idxGroupBegin; i < m_Group.size(); ++i)
			{
				auto& Group = m_Group[i];
				Group.y = y;
				y += (m_cyGroupHeader + m_cyGroupItemTopPadding);
				const auto yImg = y;
				for (auto& Item : Group.Item)
				{
					Item.y = y;
					y += (m_cyItem + m_cyPadding);
				}
				y -= m_cyPadding;;
				y += m_cyGroupItemBottomPadding;
				if (m_bGroupImage && y < yImg + m_cyGroupImage)
					y = yImg + m_cyGroupImage;
			}
			m_psvV->SetRange(-m_cyTopExtra, y + m_cyBottomExtra);
		}
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
		EckAssert(m_eView == Type::Report);
		m_Header.SetItemCount(cItem, pcx);
		if (m_bGroup)
			for (auto& e : m_Group)
				for (auto& f : e.Item)
					f.vSubItem.resize(cItem - 1);
		else
			for (auto& e : m_vItem)
				e.vSubItem.resize(cItem - 1);
	}

	EckInline void SetGroupCount(int cGroups)
	{
		ECK_DUILOCK;
		m_Group.resize(cGroups);
	}
	EckInlineNdCe int GetGroupCount() const { return (int)m_Group.size(); }

	EckInline void SetGroupItemCount(int idxGroup, int cItems)
	{
		ECK_DUILOCK;
		auto& v = m_Group[idxGroup].Item;
		if (m_eView == Type::Report)
		{
			const auto idxBegin = std::max(0, (int)v.size() - 1);
			v.resize(cItems);
			for (int i = idxBegin; i < cItems; ++i)
				v[i].vSubItem.resize(m_Header.GetItemCount());
		}
		else
			v.resize(cItems);
	}
	EckInlineNdCe int GetGroupItemCount(int idxGroup) const { return (int)m_Group[idxGroup].Item.size(); }

	void SetGroupTextFormat(IDWriteTextFormat* pTf)
	{
		ECK_DUILOCK;
		std::swap(m_pTfGroup, pTf);
		if (m_pTfGroup)
			m_pTfGroup->AddRef();
		if (pTf)
			pTf->Release();
	}

	void SetView(Type eView) noexcept
	{
		ECK_DUILOCK;
		m_eView = eView;
		if (m_eView == Type::Report)
			m_Header.SetVisible(TRUE);
		else
			m_Header.SetVisible(FALSE);
	}
	EckInlineNdCe Type GetView() const noexcept { return m_eView; }

	EckInline void UpdateHeaderLayout() noexcept { ArrangeHeader(TRUE); }

	EckInlineCe void SetGroupImageWidth(int cx) noexcept { m_cxGroupImage = cx; }
	EckInlineNdCe int GetGroupImageWidth() const noexcept { return m_cxGroupImage; }

	EckInlineCe void SetGroupImageHeight(int cy) noexcept { m_cyGroupImage = cy; }
	EckInlineNdCe int GetGroupImageHeight() const noexcept { return m_cyGroupImage; }

	EckInlineNdCe auto& GetScrollBarV() noexcept { return m_SBV; }
	EckInlineNdCe auto& GetScrollBarH() noexcept { return m_SBH; }
	EckInlineNdCe auto& GetHeader() noexcept { return m_Header; }

	EckInlineCe void SetItemHeight(int cy) noexcept { m_cyItem = cy; }
	EckInlineNdCe int GetItemHeight() const noexcept { return m_cyItem; }

	EckInlineCe void SetItemPadding(int cy) noexcept { m_cyPadding = cy; }
	EckInlineNdCe int GetItemPadding() const noexcept { return m_cyPadding; }

	EckInlineCe void SetItemWidth(int cx) noexcept
	{
		EckAssert(m_eView == Type::Icon);
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

	EckInlineCe void SetGroup(BOOL bGroup) noexcept { m_bGroup = bGroup; }
	EckInlineNdCe BOOL GetGroup() const noexcept { return m_bGroup; }

	EckInlineCe void SetGroupImage(BOOL b) noexcept { m_bGroupImage = b; }
	EckInlineNdCe BOOL GetGroupImage() const noexcept { return m_bGroupImage; }

	EckInlineCe void SetGroupItemTopPadding(int cy) noexcept { m_cyGroupItemTopPadding = cy; }
	EckInlineNdCe int GetGroupItemTopPadding() const noexcept { return m_cyGroupItemTopPadding; }

	EckInlineCe void SetGroupItemBottomPadding(int cy) noexcept { m_cyGroupItemBottomPadding = cy; }
	EckInlineNdCe int GetGroupItemBottomPadding() const noexcept { return m_cyGroupItemBottomPadding; }
};
ECK_DUI_NAMESPACE_END
ECK_NAMESPACE_END