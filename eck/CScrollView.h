#pragma once
#include "IScroll.h"

ECK_NAMESPACE_BEGIN
template<class T>
	requires (std::is_signed_v<T> || std::is_floating_point_v<T>)
class CScrollViewT : public IScroll
{
public:
	constexpr static T InvalidValue = std::numeric_limits<T>::max();
protected:
	T m_Min{};
	T m_Max{};
	T m_Page{};
	T m_Pos{};

	T m_ViewSize{};
	T m_MinThumbSize{};

	T m_oxyThumbCursor{};
#ifdef _DEBUG
	BOOL m_bLBtnDown = FALSE;
#endif // _DEBUG

	BOOL constexpr RangeChanged()
	{
		if (!IsValid())
		{
			SetPosUncheck(GetMin());
			return TRUE;
		}
		if (GetPos() < GetMin())
			SetPosUncheck(GetMin());
		else if (GetPos() > GetMaxWithPage())
			SetPosUncheck(GetMaxWithPage());
		else
			return FALSE;
		return TRUE;
	}
public:
	EckInline constexpr T GetMin() const { return m_Min; }
	EckInline constexpr void SetMin(T iMin)
	{
		m_Min = iMin;
		RangeChanged();
	}

	EckInline constexpr T GetMax() const { return m_Max; }
	EckInline constexpr T GetMaxWithPage() const { return m_Max - m_Page + 1; }
	EckInline constexpr void SetMax(T Max)
	{
		m_Max = Max;
		RangeChanged();
	}

	EckInline constexpr T GetPage() const { return m_Page; }
	EckInline constexpr void SetPage(T Page)
	{
		m_Page = Page;
		RangeChanged();
	}

	EckInline constexpr T GetPos() const { return m_Pos; }
	EckInline constexpr BOOL SetPos(T Pos)
	{
		m_Pos = Pos;
		return RangeChanged();
	}
	EckInline constexpr void SetPosUncheck(T iPos) { m_Pos = iPos; }

	EckInline constexpr void SetRange(T Min, T Max)
	{
		m_Min = Min;
		m_Max = Max;
		RangeChanged();
	}
	EckInline constexpr T GetRangeDistance() const
	{
		return GetMaxWithPage() - GetMin();
	}

	EckInline constexpr BOOL IsValid() const { return GetRangeDistance() > 0; }

	constexpr void OnMouseWheel(T Delta)
	{
		if (!IsValid())
			return;
		T iPos = GetPos() + Delta;
		if (iPos < GetMin())
			iPos = GetMin();
		else if (iPos > GetMaxWithPage())
			iPos = GetMaxWithPage();

		SetPos(iPos);
	}

	constexpr float GetPrecent() const
	{
		const T d = GetMaxWithPage() - GetMin();
		if (d <= 0)
			return 0.f;
		else
			return float(GetPos() - GetMin()) / (float)d;
	}

	EckInline constexpr void SetViewSize(T ViewSize) { m_ViewSize = ViewSize; }
	EckInline constexpr T GetViewSize() const { return m_ViewSize; }

	EckInline constexpr void SetMinThumbSize(T MinThumbSize) { m_MinThumbSize = MinThumbSize; }
	EckInline constexpr T GetMinThumbSize() const { return m_MinThumbSize; }

	EckInline constexpr T GetThumbSize() const
	{
		const T d = GetMax() - GetMin();
		if (d <= 0)
			return InvalidValue;
		const T i = GetPage() * GetViewSize() / d;
		return std::max(GetMinThumbSize(), i);
	}

	EckInline constexpr T GetThumbPos(T ThumbSize) const
	{
		const T d = GetRangeDistance();
		if (d <= 0)
			return InvalidValue;
		return (GetPos() - GetMin()) * (GetViewSize() - ThumbSize) / d;
	}
	EckInline constexpr T GetThumbPos() const { return GetThumbPos(GetThumbSize()); }

	EckInline constexpr void OnLButtonDown(T xy)
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

	EckInline constexpr void OnMouseMove(T xy)
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

	constexpr void SetScrollInfo(const SCROLLINFO& si)
	{
		if (si.fMask & SIF_RANGE)
		{
			m_Min = (T)si.nMin;
			m_Max = (T)si.nMax;
		}
		if (si.fMask & SIF_PAGE)
			m_Page = (T)si.nPage;
		if (si.fMask & SIF_POS)
			m_Pos = (T)si.nPos;
		RangeChanged();
	}

	constexpr void GetScrollInfo(_Inout_ SCROLLINFO& si) const
	{
		if (si.fMask & SIF_RANGE)
		{
			si.nMin = (int)m_Min;
			si.nMax = (int)m_Max;
		}
		if (si.fMask & SIF_PAGE)
			si.nPage = (UINT)m_Page;
		if (si.fMask & SIF_POS)
			si.nPos = (int)m_Pos;
	}

	// 是否有必要显示滚动条
	EckInline constexpr BOOL IsVisible() const
	{
		return IsValid() && GetViewSize() > GetMinThumbSize();
	}

	BOOL ScrIsValid() override { return IsValid(); }
	BOOL ScrIsVisible() override { return IsVisible(); }
	void ScrSetScrollInfo(const SCROLLINFO& si) override { SetScrollInfo(si); }
	void ScrGetScrollInfo(_Inout_ SCROLLINFO& si) override { GetScrollInfo(si); }
	void ScrSetViewSize(int iViewSize) override { SetViewSize((T)iViewSize); }
	int ScrGetViewSize() override { return (int)GetViewSize(); }
};

using CScrollView = CScrollViewT<int>;
using CScrollViewF = CScrollViewT<float>;
ECK_NAMESPACE_END