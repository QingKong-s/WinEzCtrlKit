/*
* WinEzCtrlKit Library
*
* CFrameLayout.h ： 帧布局
*
* Copyright(C) 2024 QingKong
*/
#pragma once
#include "CLayoutBase.h"

ECK_NAMESPACE_BEGIN
class CFrameLayout final :public CLayoutBase
{
public:
	ECK_RTTI(CFrameLayout);
private:
	struct ITEM :public ITEMBASE
	{
		ITEM() = default;
		constexpr ITEM(ILayout* pCtrl, const MARGINS& Margin, UINT uFlags, short cx, short cy)
			: ITEMBASE{ pCtrl, Margin, uFlags, cx, cy } {}
	};

	std::vector<ITEM> m_vItem{};
public:
	size_t Add(ILayout* pCtrl, const MARGINS& Margin = {}, UINT uFlags = 0u)
	{
		const auto size = pCtrl->LoGetSize();
		m_vItem.emplace_back(pCtrl, Margin, uFlags, (short)size.first, (short)size.second);
		return m_vItem.size() - 1;
	}

	void LoCommit() override
	{
		int x, y, cx, cy;
		HDWP hDwp = PreArrange(m_vItem.size());
		for (const auto& e : m_vItem)
		{
			CalcCtrlPosSize(e, { m_x, m_y, m_cx, m_cy },
				x, y, cx, cy);
			MoveCtrlPosSize(e, hDwp, x, y, cx, cy);
		}
		PostArrange(hDwp);
	}

	void LoOnDpiChanged(int iDpi) override
	{
		Refresh();
		for (auto& e : m_vItem)
		{
			ReCalcDpiSize(e, iDpi);
			e.pCtrl->LoOnDpiChanged(iDpi);
		}
		m_iDpi = iDpi;
	}

	void LoInitDpi(int iDpi) override
	{
		m_iDpi = iDpi;
		for (auto& e : m_vItem)
			e.pCtrl->LoInitDpi(iDpi);
	}

	void ShowFrame(int idx)
	{
		EckAssert(idx >= 0 && idx < (int)m_vItem.size());
		int i{};
		m_vItem[idx].pCtrl->LoShow(TRUE);
		for (; i < idx; ++i)
			m_vItem[i].pCtrl->LoShow(FALSE);
		for (i = idx + 1; i < (int)m_vItem.size(); ++i)
			m_vItem[i].pCtrl->LoShow(FALSE);
	}

	void Refresh() override
	{
		for (auto& e : m_vItem)
		{
			const auto size = e.pCtrl->LoGetSize();
			e.cx = (short)size.first;
			e.cy = (short)size.second;
		}
	}

	void Clear() override
	{
		CLayoutBase::Clear();
		m_vItem.clear();
	}

	EckInline constexpr auto& GetList() { return m_vItem; }

	void LoShow(BOOL bShow) override
	{
		for (const auto& e : GetList())
			e.pCtrl->LoShow(bShow);
	}
};
ECK_RTTI_IMPL_BASE_INLINE(CFrameLayout, CLayoutBase);
ECK_NAMESPACE_END