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
	ILayout* m_pParent{};
	HDWP m_hDwp{};
public:
	void LoSetPos(int x, int y)
	{
		m_x = x;
		m_y = y;
	}

	void LoSetSize(int cx, int cy)
	{
		m_cx = cx;
		m_cy = cy;
	}

	void LoSetPosSize(int x, int y, int cx, int cy)
	{
		m_x = x;
		m_y = y;
		m_cx = cx;
		m_cy = cy;
	}

	std::pair<int, int> LoGetPos()
	{
		return { m_x,m_y };
	}

	std::pair<int, int> LoGetSize()
	{
		return { m_cx,m_cy };
	}

	void LoBeginDeferPos() {}

	EckInline void Arrange(int cx, int cy)
	{
		LoSetPosSize(0, 0, cx, cy);
		LoCommit();
	}

	EckInline void LoSetParent(ILayout* p) override { m_pParent = p; }

	EckInline HDWP LoGetCurrHDWP() override { return m_hDwp; }

	virtual void Clear()
	{
		m_x = m_y = m_cx = m_cy = 0;
		m_pParent = NULL;
		m_hDwp = NULL;
	}
};
ECK_NAMESPACE_END