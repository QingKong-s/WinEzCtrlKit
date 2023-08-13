#pragma once
#include "CWnd.h"
#include "CSubclassMgr.h"

ECK_NAMESPACE_BEGIN
template<int TType>
class CScrollBarBase :public CWnd
{
public:
	BOOL m_bDisableNoScroll = FALSE;

	/// <summary>
	/// �������ü�ͷ
	/// </summary>
	/// <param name="iOp">ESB_����</param>
	/// <returns>�ɹ�����TRUE��ʧ�ܷ���FALSE</returns>
	EckInline BOOL EnableArrows(int iOp)
	{
		EnableScrollBar(m_hWnd, TType, iOp);
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
		GetScrollInfo(m_hWnd, TType, &si);
		return si.nPos;
	}

	EckInline int GetTrackPos()
	{
		SCROLLINFO si;
		si.cbSize = sizeof(SCROLLINFO);
		si.fMask = SIF_TRACKPOS;
		GetScrollInfo(m_hWnd, TType, &si);
		return si.nTrackPos;
	}

	EckInline BOOL GetRange(int* piMin = NULL, int* piMax = NULL)
	{
		SCROLLINFO si;
		si.cbSize = sizeof(SCROLLINFO);
		si.fMask = SIF_RANGE;
		BOOL b = GetScrollInfo(m_hWnd, TType, &si);
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
		GetScrollInfo(m_hWnd, TType, &si);
		return si.nPage;
	}

	EckInline BOOL GetInfo(SCROLLINFO* psi)
	{
		psi->cbSize = sizeof(SCROLLINFO);
		return GetScrollInfo(m_hWnd, TType, psi);
	}

	EckInline void SetPos(int iPos, BOOL bRedraw = TRUE)
	{
		SCROLLINFO si;
		si.cbSize = sizeof(SCROLLINFO);
		si.fMask = SIF_POS | (m_bDisableNoScroll ? SIF_DISABLENOSCROLL : 0);
		si.nPos = iPos;
		SetScrollInfo(m_hWnd, TType, &si, bRedraw);
	}

	EckInline void SetRange(int iMin, int iMax, BOOL bRedraw = TRUE)
	{
		SCROLLINFO si;
		si.cbSize = sizeof(SCROLLINFO);
		si.fMask = SIF_RANGE | (m_bDisableNoScroll ? SIF_DISABLENOSCROLL : 0);
		SetScrollInfo(m_hWnd, TType, &si, bRedraw);
		si.nMin = iMin;
		si.nMax = iMax;
	}

	EckInline void SetMin(int iMin, BOOL bRedraw = TRUE)
	{
		SCROLLINFO si;
		si.cbSize = sizeof(SCROLLINFO);
		si.fMask = SIF_RANGE;
		GetScrollInfo(m_hWnd, TType, &si);
		si.nMin = iMin;
		si.fMask |= (m_bDisableNoScroll ? SIF_DISABLENOSCROLL : 0);
		SetScrollInfo(m_hWnd, TType, &si, bRedraw);
	}

	EckInline void SetMax(int iMax, BOOL bRedraw = TRUE)
	{
		SCROLLINFO si;
		si.cbSize = sizeof(SCROLLINFO);
		si.fMask = SIF_RANGE;
		GetScrollInfo(m_hWnd, TType, &si);
		si.nMax = iMax;
		si.fMask |= (m_bDisableNoScroll ? SIF_DISABLENOSCROLL : 0);
		SetScrollInfo(m_hWnd, TType, &si, bRedraw);
	}

	EckInline void SetPage(int iPage, BOOL bRedraw = TRUE)
	{
		SCROLLINFO si;
		si.cbSize = sizeof(SCROLLINFO);
		si.fMask = SIF_PAGE | (m_bDisableNoScroll ? SIF_DISABLENOSCROLL : 0);
		si.nPage = iPage;
		SetScrollInfo(m_hWnd, TType, &si, bRedraw);
	}

	EckInline void SetInfo(SCROLLINFO* psi, BOOL bRedraw = TRUE)
	{
		psi->cbSize = sizeof(SCROLLINFO);
		psi->fMask |= (m_bDisableNoScroll ? SIF_DISABLENOSCROLL : 0);
		SetScrollInfo(m_hWnd, TType, psi, bRedraw);
	}
};

class CScrollBar :public CScrollBarBase<SB_CTL>
{
public:
	EckInline HWND Create(PCWSTR pszText, DWORD dwStyle, DWORD dwExStyle,
		int x, int y, int cx, int cy, HWND hParent, UINT nID)
	{
		dwStyle |= WS_CHILD;
		m_hWnd = CreateWindowExW(dwExStyle, WC_SCROLLBARW, pszText, dwStyle,
			x, y, cx, cy, hParent, i32ToP<HMENU>(nID), NULL, NULL);
		return m_hWnd;
	}

	/// <summary>
	/// �������ü�ͷ
	/// </summary>
	/// <returns>����TRUE��ʾΪ��ֱ������������FALSEΪˮƽ������</returns>
	EckInline BOOL GetDirection()
	{
		
	}
};

template<int TType>
class CScrollBarNc :public CScrollBarBase<TType>
{
public:
	EckInline BOOL ShowScrollBar(BOOL bShow)
	{
		return ::ShowScrollBar(this->m_hWnd, TType, bShow);
	}

	EckInline BOOL ShowScrollBar(BOOL bShow, int iBarType)
	{
		return ::ShowScrollBar(this->m_hWnd, iBarType, bShow);
	}
};

typedef CScrollBarNc<SB_HORZ> CScrollBarWndH;
typedef CScrollBarNc<SB_VERT> CScrollBarWndV;
ECK_NAMESPACE_END