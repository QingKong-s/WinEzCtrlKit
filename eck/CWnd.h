/*
* WinEzCtrlKit Library
*
* CWnd.h ： 窗口基类
*
* Copyright(C) 2023 QingKong
*/
#pragma once
#include "ECK.h"
#include "WndHelper.h"
#include "CRefStr.h"

#include <optional>

#include <assert.h>

ECK_NAMESPACE_BEGIN
inline constexpr int
DATAVER_STD_1 = 1;
#pragma pack(push, ECK_CTRLDATA_ALIGN)
struct CREATEDATA_STD
{
	int iVer_Std;
	int cchText;
	DWORD dwStyle;
	DWORD dwExStyle;
	// WCHAR szText[];

	EckInline PCWSTR Text() const
	{
		if (cchText)
			return (PCWSTR)PtrSkipType(this);
		else
			return NULL;
	}
};
#pragma pack(pop)

#ifdef ECK_CTRL_DESIGN_INTERFACE
struct DESIGNDATA_WND
{
	BITBOOL bVisible : 1;
	BITBOOL bEnable : 1;
	CRefStrW rsName;
};
#endif

class CWnd
{
protected:
	HWND m_hWnd = NULL;

	EckInline HWND DefAttach(HWND hWnd)
	{
		HWND hOld = m_hWnd;
		m_hWnd = hWnd;
		return hOld;
	}
public:
#ifdef ECK_CTRL_DESIGN_INTERFACE
	DESIGNDATA_WND m_DDBase{};
#endif
	
	enum class ManageOp
	{
		// 依附句柄，返回先前窗口句柄
		Attach,
		// 拆离句柄，返回窗口句柄
		Detach,
		// 父窗口已更改，不使用返回值
		ChangeParent,
		// 修改绑定，成功返回非0，失败返回0
		Bind
	};

	CWnd()
	{

	}

	CWnd(HWND hWnd) :m_hWnd(hWnd)
	{

	}

	virtual ~CWnd()
	{

	}

	virtual HWND Manage(ManageOp iType, HWND hWnd);

	virtual HWND Create(PCWSTR pszText, DWORD dwStyle, DWORD dwExStyle,
		int x, int y, int cx, int cy, HWND hParent, int nID, PCVOID pData = NULL)
	{
		assert(FALSE);
		return NULL;
	}

	virtual CRefBin SerializeData(SIZE_T cbExtra = 0, SIZE_T* pcbSize = NULL);

	EckInline static PCVOID SkipBaseData(PCVOID p)
	{
		return (PCBYTE)p +
			sizeof(CREATEDATA_STD) +
			(((const CREATEDATA_STD*)p)->cchText + 1) * sizeof(WCHAR);
	}

	HWND ReCreate(EckOptNul(DWORD, dwNewStyle), EckOptNul(DWORD, dwNewExStyle), EckOptNul(RECT, rcPos));

	EckInline HWND GetHWND() const
	{
		return m_hWnd;
	}

	EckInline void FrameChanged() const
	{
		SetWindowPos(m_hWnd, NULL, 0, 0, 0, 0, SWP_NOZORDER | SWP_NOSIZE | SWP_NOMOVE | SWP_NOACTIVATE | SWP_FRAMECHANGED);
	}

	EckInline void SetRedraw(BOOL bRedraw) const
	{
		SendMessageW(m_hWnd, WM_SETREDRAW, bRedraw, 0);
	}

	EckInline BOOL Redraw() const
	{
		return InvalidateRect(m_hWnd, NULL, FALSE);
	}

	EckInline BOOL Redraw(const RECT& rc) const
	{
		return InvalidateRect(m_hWnd, &rc, FALSE);
	}

	EckInline operator HWND() const
	{
		return m_hWnd;
	}

	/// <summary>
	/// 置边框类型
	/// </summary>
	/// <param name="iFrame">0 - 无边框  1 - 凹入式  2 - 凸出式  3 - 浅凹入式  4 - 镜框式  5 - 单线边框式</param>
	void SetFrameType(int iFrame) const;

	int GetFrameType() const;

	void SetScrollBar(int i) const;

	int GetScrollBar() const;

	EckInline LRESULT SendMsg(UINT uMsg, WPARAM wParam, LPARAM lParam) const
	{
		return SendMessageW(m_hWnd, uMsg, wParam, lParam);
	}

	EckInline DWORD GetStyle() const
	{
		return (DWORD)GetWindowLongPtrW(m_hWnd, GWL_STYLE);
	}

	EckInline DWORD GetExStyle() const
	{
		return (DWORD)GetWindowLongPtrW(m_hWnd, GWL_EXSTYLE);
	}

	EckInline DWORD ModifyStyle(DWORD dwNew, DWORD dwMask, int idx = GWL_STYLE) const
	{
		return ModifyWindowStyle(m_hWnd, dwNew, dwMask, idx);
	}

	EckInline DWORD SetStyle(DWORD dwStyle) const
	{
		return (DWORD)SetWindowLongPtrW(m_hWnd, GWL_STYLE, dwStyle);
	}

	EckInline DWORD SetExStyle(DWORD dwStyle) const
	{
		return (DWORD)SetWindowLongPtrW(m_hWnd, GWL_EXSTYLE, dwStyle);
	}

	EckInline CRefStrW GetText() const
	{
		CRefStrW rs;
		int cch = GetWindowTextLengthW(m_hWnd);
		if (cch)
		{
			rs.ReSize(cch);
			GetWindowTextW(m_hWnd, rs.Data(), cch + 1);
		}
		return rs;
	}

	EckInline BOOL SetText(PCWSTR pszText) const
	{
		return SetWindowTextW(m_hWnd, pszText);
	}

	EckInline HRESULT SetExplorerTheme() const
	{
		return SetWindowTheme(m_hWnd, L"Explorer", NULL);
	}

	EckInline BOOL Move(int x, int y, int cx, int cy, BOOL bNoActive = FALSE) const
	{
		return SetWindowPos(m_hWnd, NULL, x, y, cx, cy, SWP_NOZORDER | (bNoActive ? SWP_NOACTIVATE : 0));
	}

	EckInline BOOL Destroy()
	{
		BOOL b = DestroyWindow(m_hWnd);
		m_hWnd = NULL;
		return b;
	}

	EckInline void SetFont(HFONT hFont, BOOL bRedraw = FALSE) const
	{
		SendMsg(WM_SETFONT, (WPARAM)hFont, bRedraw);
	}

	EckInline BOOL Show(int nCmdShow) const
	{
		return ShowWindow(m_hWnd, nCmdShow);
	}

	EckInline BOOL Enable(BOOL bEnable) const
	{
		return EnableWindow(m_hWnd, bEnable);
	}

	EckInline BOOL IsVisible() const
	{
		return IsWindowVisible(m_hWnd);
	}
};

class COwnWnd :public CWnd
{
protected:
	HFONT m_hFont = NULL;
	CRefStrW m_rsText{};

	EckInline void OnOwnWndMsg(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		switch (uMsg)
		{
		case WM_CREATE:
		{
			auto pcs = (CREATESTRUCTW*)lParam;
			m_rsText = pcs->lpszName;
		}
		break;
		case WM_SETFONT:
			m_hFont = (HFONT)wParam;
			break;
		case WM_SETTEXT:
			m_rsText = (PWSTR)lParam;
			break;
		case WM_GETTEXT:
			if (wParam > 0)
				m_rsText.CopyTo((PWSTR)lParam, (int)wParam - 1);
			break;
		}
	}
};
ECK_NAMESPACE_END