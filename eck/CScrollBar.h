#pragma once
#include "CWnd.h"

ECK_NAMESPACE_BEGIN
class CScrollBar :public CWnd
{
public:
	ECK_RTTI(CScrollBar);
	ECK_CWND_NOSINGLEOWNER(CScrollBar);
	ECK_CWND_CREATE_CLS(WC_SCROLLBARW);

	ECK_CWNDPROP_STYLE(BottomAlign, SBS_BOTTOMALIGN);
	ECK_CWNDPROP_STYLE(Horz, SBS_HORZ);
	ECK_CWNDPROP_STYLE(LeftAlign, SBS_LEFTALIGN);
	ECK_CWNDPROP_STYLE(RightAlign, SBS_RIGHTALIGN);
	ECK_CWNDPROP_STYLE(SizeBox, SBS_SIZEBOX);
	ECK_CWNDPROP_STYLE(SizeBoxBottomRightAlign, SBS_SIZEBOXBOTTOMRIGHTALIGN);
	ECK_CWNDPROP_STYLE(SizeBoxTopLeftAlign, SBS_SIZEBOXTOPLEFTALIGN);
	ECK_CWNDPROP_STYLE(SizeGrip, SBS_SIZEGRIP);
	ECK_CWNDPROP_STYLE(TopAlign, SBS_TOPALIGN);
	ECK_CWNDPROP_STYLE(Vert, SBS_VERT);
private:
	BOOL m_bDisableNoScroll{};
public:
	/// <summary>
	/// 禁用启用箭头
	/// </summary>
	/// <param name="iOp">ESB_常量</param>
	/// <returns>成功返回TRUE，失败返回FALSE</returns>
	EckInline BOOL EnableArrows(int iOp)
	{
		return EnableScrollBar(m_hWnd, SB_CTL, iOp);
	}

	EckInline BOOL EnableArrows(int iOp, int iBarType)
	{
		return EnableScrollBar(m_hWnd, iBarType, iOp);
	}

	EckInline int GetPos()
	{
		SCROLLINFO si;
		si.cbSize = sizeof(SCROLLINFO);
		si.fMask = SIF_POS;
		GetScrollInfo(m_hWnd, SB_CTL, &si);
		return si.nPos;
	}

	EckInline int GetTrackPos()
	{
		SCROLLINFO si;
		si.cbSize = sizeof(SCROLLINFO);
		si.fMask = SIF_TRACKPOS;
		GetScrollInfo(m_hWnd, SB_CTL, &si);
		return si.nTrackPos;
	}

	EckInline BOOL GetRange(int* piMin = nullptr, int* piMax = nullptr)
	{
		SCROLLINFO si;
		si.cbSize = sizeof(SCROLLINFO);
		si.fMask = SIF_RANGE;
		BOOL b = GetScrollInfo(m_hWnd, SB_CTL, &si);
		if (piMin)
			*piMin = si.nMin;
		if (piMax)
			*piMax = si.nMax;
		return b;
	}

	EckInline int GetPage()
	{
		SCROLLINFO si;
		si.cbSize = sizeof(SCROLLINFO);
		si.fMask = SIF_PAGE;
		GetScrollInfo(m_hWnd, SB_CTL, &si);
		return si.nPage;
	}

	EckInline BOOL GetInfo(SCROLLINFO* psi)
	{
		return GetScrollInfo(m_hWnd, SB_CTL, psi);
	}

	EckInline void SetPos(int iPos, BOOL bRedraw = TRUE)
	{
		SCROLLINFO si;
		si.cbSize = sizeof(SCROLLINFO);
		si.fMask = SIF_POS | (m_bDisableNoScroll ? SIF_DISABLENOSCROLL : 0);
		si.nPos = iPos;
		SetScrollInfo(m_hWnd, SB_CTL, &si, bRedraw);
	}

	EckInline void SetRange(int iMin, int iMax, BOOL bRedraw = TRUE)
	{
		SCROLLINFO si;
		si.cbSize = sizeof(SCROLLINFO);
		si.fMask = SIF_RANGE | (m_bDisableNoScroll ? SIF_DISABLENOSCROLL : 0);
		SetScrollInfo(m_hWnd, SB_CTL, &si, bRedraw);
		si.nMin = iMin;
		si.nMax = iMax;
	}

	EckInline void SetMin(int iMin, BOOL bRedraw = TRUE)
	{
		SCROLLINFO si;
		si.cbSize = sizeof(SCROLLINFO);
		si.fMask = SIF_RANGE;
		GetScrollInfo(m_hWnd, SB_CTL, &si);
		si.nMin = iMin;
		si.fMask |= (m_bDisableNoScroll ? SIF_DISABLENOSCROLL : 0);
		SetScrollInfo(m_hWnd, SB_CTL, &si, bRedraw);
	}

	EckInline void SetMax(int iMax, BOOL bRedraw = TRUE)
	{
		SCROLLINFO si;
		si.cbSize = sizeof(SCROLLINFO);
		si.fMask = SIF_RANGE;
		GetScrollInfo(m_hWnd, SB_CTL, &si);
		si.nMax = iMax;
		si.fMask |= (m_bDisableNoScroll ? SIF_DISABLENOSCROLL : 0);
		SetScrollInfo(m_hWnd, SB_CTL, &si, bRedraw);
	}

	EckInline void SetPage(int iPage, BOOL bRedraw = TRUE)
	{
		SCROLLINFO si;
		si.cbSize = sizeof(SCROLLINFO);
		si.fMask = SIF_PAGE | (m_bDisableNoScroll ? SIF_DISABLENOSCROLL : 0);
		si.nPage = iPage;
		SetScrollInfo(m_hWnd, SB_CTL, &si, bRedraw);
	}

	EckInline void SetInfo(SCROLLINFO* psi, BOOL bRedraw = TRUE)
	{
		psi->fMask |= (m_bDisableNoScroll ? SIF_DISABLENOSCROLL : 0);
		SetScrollInfo(m_hWnd, SB_CTL, psi, bRedraw);
	}

	EckInline void SetDisableNoScroll(BOOL bDisable, BOOL bImmdSet = FALSE)
	{
		m_bDisableNoScroll = bDisable;
		if (bImmdSet)
		{
			SCROLLINFO si;
			si.cbSize = sizeof(SCROLLINFO);
			si.fMask = SIF_ALL;
			GetInfo(&si);
			SetInfo(&si, TRUE);
		}
	}

	EckInline BOOL GetDisableNoScroll() const { return m_bDisableNoScroll; }
};
ECK_RTTI_IMPL_BASE_INLINE(CScrollBar, CWnd);
ECK_NAMESPACE_END