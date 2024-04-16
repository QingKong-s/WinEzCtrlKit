/*
* WinEzCtrlKit Library
*
* CScrollView.h ： 滚动视图
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

	void RangeChanged()
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
	EckInline int GetMin() const { return m_iMin; }

	void SetMin(int iMin)
	{
		m_iMin = iMin;
		RangeChanged();
	}

	EckInline int GetMax() const { return m_iMax; }

	EckInline int GetMaxWithPage() const { return m_iMax - m_iPage + 1; }

	void SetMax(int iMax)
	{
		m_iMax = iMax;
		RangeChanged();
	}

	EckInline int GetPage() const { return m_iPage; }

	void SetPage(int iPage)
	{
		m_iPage = iPage;
		RangeChanged();
	}

	EckInline int GetPos() const { return m_iPos; }

	void SetPos(int iPos)
	{
		m_iPos = iPos;
		RangeChanged();
	}

	void SetPosUncheck(int iPos)
	{
		m_iPos = iPos;
	}

	void SetRange(int iMin, int iMax)
	{
		m_iMin = iMin;
		m_iMax = iMax;
		RangeChanged();
	}

	int GetRangeDistance() const
	{
		return GetMaxWithPage() - GetMin();
	}

	BOOL IsValid() const { return GetRangeDistance() > 0; }

	void OnMouseWheel(int iDelta)
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

	float GetPrecent() const
	{
		const int d = GetMaxWithPage() - GetMin();
		if (d <= 0)
			return 0.f;
		else
			return (float)(GetPos() - GetMin()) / (float)d;
	}

	EckInline void SetViewSize(int iViewSize) { m_iViewSize = iViewSize; }

	EckInline int GetViewSize() const { return m_iViewSize; }

	EckInline void SetMinThumbSize(int iMinThumbSize) { m_iMinThumbSize = iMinThumbSize; }

	EckInline int GetMinThumbSize() const { return m_iMinThumbSize; }

	EckInline int GetThumbSize() const
	{
		const int d = GetMax() - GetMin();
		if (d <= 0)
			return std::numeric_limits<int>::min();
		const int i = GetPage() * GetViewSize() / d;
		return std::max(GetMinThumbSize(), i);
	}

	EckInline int GetThumbPos(int iThumbSize) const
	{
		const int d = GetRangeDistance();
		if (d <= 0)
			return std::numeric_limits<int>::min();
		return (GetPos() - GetMin()) * (GetViewSize() - iThumbSize) / d;
	}

	EckInline int GetThumbPos() const { return GetThumbPos(GetThumbSize()); }

	EckInline void OnLButtonDown(int xy)
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

	EckInline void OnMouseMove(int xy)
	{
		EckAssert(m_bLBtnDown);
		if (!IsValid())
			return;
		SetPos(GetMin() +
			(xy - m_oxyThumbCursor) *
			GetRangeDistance() /
			(GetViewSize() - GetThumbSize()));
	}

	EckInline void OnLButtonUp()
	{
#ifdef _DEBUG
		EckAssert(m_bLBtnDown);
		m_bLBtnDown = FALSE;
#endif // _DEBUG
	}
};
ECK_NAMESPACE_END