#pragma once
#include "CWnd.h"

ECK_NAMESPACE_BEGIN

class CScrollBar :public CWnd
{
private:
	BOOL m_bDisableNoScroll = FALSE;
public:
	ECK_STYLE_GETSET(BottomAlign, SBS_BOTTOMALIGN);
	ECK_STYLE_GETSET(Horz, SBS_HORZ);
	ECK_STYLE_GETSET(LeftAlign, SBS_LEFTALIGN);
	ECK_STYLE_GETSET(RightAlign, SBS_RIGHTALIGN);
	ECK_STYLE_GETSET(SizeBox, SBS_SIZEBOX);
	ECK_STYLE_GETSET(SizeBoxBottomRightAlign, SBS_SIZEBOXBOTTOMRIGHTALIGN);
	ECK_STYLE_GETSET(SizeBoxTopLeftAlign, SBS_SIZEBOXTOPLEFTALIGN);
	ECK_STYLE_GETSET(SizeGrip, SBS_SIZEGRIP);
	ECK_STYLE_GETSET(TopAlign, SBS_TOPALIGN);
	ECK_STYLE_GETSET(Vert, SBS_VERT);

	ECK_CWND_CREATE;
	HWND Create(PCWSTR pszText, DWORD dwStyle, DWORD dwExStyle,
		int x, int y, int cx, int cy, HWND hParent, HMENU hMenu, PCVOID pData = NULL) override
	{
		return IntCreate(dwExStyle, WC_SCROLLBARW, pszText, dwStyle,
			x, y, cx, cy, hParent, hMenu, NULL, NULL);
	}

	/// <summary>
	/// 禁用启用箭头
	/// </summary>
	/// <param name="iOp">ESB_常量</param>
	/// <returns>成功返回TRUE，失败返回FALSE</returns>
	EckInline BOOL EnableArrows(int iOp)
	{
		EnableScrollBar(m_hWnd, SB_CTL, iOp);
	}

	EckInline BOOL EnableArrows(int iOp, int iBarType)
	{
		EnableScrollBar(m_hWnd, iBarType, iOp);
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

	EckInline BOOL GetRange(int* piMin = NULL, int* piMax = NULL)
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
		psi->cbSize = sizeof(SCROLLINFO);
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
		psi->cbSize = sizeof(SCROLLINFO);
		psi->fMask |= (m_bDisableNoScroll ? SIF_DISABLENOSCROLL : 0);
		SetScrollInfo(m_hWnd, SB_CTL, psi, bRedraw);
	}
};
ECK_NAMESPACE_END