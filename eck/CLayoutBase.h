/*
* WinEzCtrlKit Library
*
* CLayoutBase.h ： 布局基类
*
* Copyright(C) 2024 QingKong
*/
#pragma once
#include "ILayout.h"

ECK_NAMESPACE_BEGIN
class CLayoutBase :public ILayout
{
protected:
	int m_x{},
		m_y{},
		m_cx{},
		m_cy{};
	BOOL m_bUseDwp{ TRUE };
#ifdef _DEBUG
	BOOL m_bPrePostBalanced{};
#endif
public:
	void LoSetPos(int x, int y) override
	{
		m_x = x;
		m_y = y;
	}

	void LoSetSize(int cx, int cy) override
	{
		m_cx = cx;
		m_cy = cy;
	}

	void LoSetPosSize(int x, int y, int cx, int cy) override
	{
		m_x = x;
		m_y = y;
		m_cx = cx;
		m_cy = cy;
	}

	std::pair<int, int> LoGetPos() override
	{
		return { m_x,m_y };
	}

	std::pair<int, int> LoGetSize() override
	{
		return { m_cx,m_cy };
	}

	void LoShow(BOOL bShow) override {}

	void LoGetAppropriateSize(int& cx, int& cy) override
	{
		cx = m_cx;
		cy = m_cy;
	}

	virtual void Clear()
	{
		m_x = m_y = m_cx = m_cy = 0;
		m_bUseDwp = TRUE;
	}

	EckInline void Arrange(int cx, int cy)
	{
		LoSetPosSize(0, 0, cx, cy);
		LoCommit();
	}

	EckInline void Arrange(int x, int y, int cx, int cy)
	{
		LoSetPosSize(x, y, cx, cy);
		LoCommit();
	}

	EckInline constexpr void SetUseDwp(BOOL b) { m_bUseDwp = b; }

	EckInline constexpr BOOL GetUseDwp() const { return m_bUseDwp; }

	HDWP PreArrange(size_t cWindows = 2u)
	{
#ifdef _DEBUG
		EckAssert(!m_bPrePostBalanced);
		m_bPrePostBalanced = TRUE;
#endif
		if (m_bUseDwp)
			return BeginDeferWindowPos((int)cWindows);
		else
			return NULL;
	}

	void PostArrange(HDWP hDwp)
	{
#ifdef _DEBUG
		EckAssert(m_bPrePostBalanced);
		m_bPrePostBalanced = FALSE;
#endif
		EndDeferWindowPos(hDwp);
	}
};
ECK_NAMESPACE_END