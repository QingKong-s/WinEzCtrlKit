/*
* WinEzCtrlKit Library
*
* CScrollView.h ： 虚拟滚动条
*
* Copyright(C) 2023-2024 QingKong
*/
#pragma once
#include "Utility.h"

ECK_NAMESPACE_BEGIN
class CScrollView
{
protected:
	int m_iMin = 0;
	int m_iMax = 0;
	int m_iPage = 0;
	int m_iPos = 0;

	int m_iViewSize = 0;
	int m_iMinThumbSize = 0;

	int m_oxyThumbCursor = 0;
#ifdef _DEBUG
	BOOL m_bLBtnDown = FALSE;
#endif // _DEBUG

	void constexpr RangeChanged()
	{
		if (!IsValid())
		{
			SetPosUncheck(GetMin());
			return;
		}
		if (GetPos() < GetMin())
			SetPosUncheck(GetMin());
		else if (GetPos() > GetMaxWithPage())
			SetPosUncheck(GetMaxWithPage());
	}
public:
	EckInline constexpr int GetMin() const { return m_iMin; }

	EckInline constexpr void SetMin(int iMin)
	{
		m_iMin = iMin;
		RangeChanged();
	}

	EckInline constexpr int GetMax() const { return m_iMax; }

	EckInline constexpr int GetMaxWithPage() const { return m_iMax - m_iPage + 1; }

	EckInline constexpr void SetMax(int iMax)
	{
		m_iMax = iMax;
		RangeChanged();
	}

	EckInline constexpr int GetPage() const { return m_iPage; }

	EckInline constexpr void SetPage(int iPage)
	{
		m_iPage = iPage;
		RangeChanged();
	}

	EckInline constexpr int GetPos() const { return m_iPos; }

	EckInline constexpr void SetPos(int iPos)
	{
		m_iPos = iPos;
		RangeChanged();
	}

	EckInline constexpr void SetPosUncheck(int iPos)
	{
		m_iPos = iPos;
	}

	EckInline constexpr void SetRange(int iMin, int iMax)
	{
		m_iMin = iMin;
		m_iMax = iMax;
		RangeChanged();
	}

	EckInline constexpr int GetRangeDistance() const
	{
		return GetMaxWithPage() - GetMin();
	}

	EckInline constexpr BOOL IsValid() const { return GetRangeDistance() > 0; }

	constexpr void OnMouseWheel(int iDelta)
	{
		if (!IsValid())
			return;
		int iPos = GetPos() + iDelta;
		if (iPos < GetMin())
			iPos = GetMin();
		else if (iPos > GetMaxWithPage())
			iPos = GetMaxWithPage();

		SetPos(iPos);
	}

	constexpr float GetPrecent() const
	{
		const int d = GetMaxWithPage() - GetMin();
		if (d <= 0)
			return 0.f;
		else
			return (float)(GetPos() - GetMin()) / (float)d;
	}

	EckInline constexpr void SetViewSize(int iViewSize) { m_iViewSize = iViewSize; }

	EckInline constexpr int GetViewSize() const { return m_iViewSize; }

	EckInline constexpr void SetMinThumbSize(int iMinThumbSize) { m_iMinThumbSize = iMinThumbSize; }

	EckInline constexpr int GetMinThumbSize() const { return m_iMinThumbSize; }

	EckInline constexpr int GetThumbSize() const
	{
		const int d = GetMax() - GetMin();
		if (d <= 0)
			return std::numeric_limits<int>::min();
		const int i = GetPage() * GetViewSize() / d;
		return std::max(GetMinThumbSize(), i);
	}

	EckInline constexpr int GetThumbPos(int iThumbSize) const
	{
		const int d = GetRangeDistance();
		if (d <= 0)
			return std::numeric_limits<int>::min();
		return (GetPos() - GetMin()) * (GetViewSize() - iThumbSize) / d;
	}

	EckInline constexpr int GetThumbPos() const { return GetThumbPos(GetThumbSize()); }

	EckInline constexpr void OnLButtonDown(int xy)
	{
#ifdef _DEBUG
		EckAssert(!m_bLBtnDown);
		m_bLBtnDown = TRUE;
#endif // _DEBUG
		if (!IsValid())
			return;
		EckAssert(xy >= GetThumbPos() && xy <= GetThumbPos() + GetThumbSize());
		m_oxyThumbCursor = xy - GetThumbPos();
	}

	EckInline constexpr void OnMouseMove(int xy)
	{
		EckAssert(m_bLBtnDown);
		if (!IsValid())
			return;
		SetPos(GetMin() +
			(xy - m_oxyThumbCursor) *
			GetRangeDistance() /
			(GetViewSize() - GetThumbSize()));
	}

	EckInline constexpr void OnLButtonUp()
	{
#ifdef _DEBUG
		EckAssert(m_bLBtnDown);
		m_bLBtnDown = FALSE;
#endif // _DEBUG
	}
};
ECK_NAMESPACE_END