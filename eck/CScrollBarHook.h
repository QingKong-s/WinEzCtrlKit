#pragma once
#include "CWnd.h"
#include "CCriticalSection.h"

ECK_NAMESPACE_BEGIN
enum class SBHT
{
	INVALID,

	ARROWUP,
	ARROWDOWN,
	ARROWLEFT,
	ARROWRIGHT,

	PAGEUP,
	PAGEDOWN,
	PAGELEFT,
	PAGERIGHT,

	THUMBV,
	THUMBH
};

enum class SBHOVERSTATE
{
	NONE,
	VSB,
	HSB
};

class CScrollBarHook
{
private:
	CWnd* m_pWnd{};
	SCROLLINFO m_si[2]{};// 0 垂直  1 水平

	RECT m_rcWnd{};// 窗口矩形（相对自身窗口）
	int m_iNcLBtnDownPos{ HTNOWHERE };

	int m_cxVSB{};
	int m_cyHSB{};
	int m_cxSBArrow{};
	int m_cySBArrow{};

	POINT m_ptSBDown{};
	int m_oyVSB{};
	int m_oxHSB{};

	int m_iDPI{ USER_DEFAULT_SCREEN_DPI };

	SBHT m_idxLBtnDownPart{ SBHT::INVALID };
	SBHT m_idxHotPart{ SBHT::INVALID };

	RECT m_rcVSB{};
	RECT m_rcHSB{};
	RECT m_BorderSize{};
	int m_yThumb{};
	int m_xThumb{};
	int m_cyVThumb{};
	int m_cxHThumb{};

	int m_cxBorder{};

	int m_iDefThumbSize{};

	SBHOVERSTATE m_iHoverState{ SBHOVERSTATE::NONE };

	BOOL m_bEnableVSB{ TRUE };
	BOOL m_bEnableHSB{ TRUE };

	HTHEME m_hTheme{};

	static int s_cHookRef;
	static CCriticalSection s_cs;

	using FSetScrollInfo = int(WINAPI*)(HWND, int, const SCROLLINFO*, BOOL);
	using FGetScrollInfo = BOOL(WINAPI*)(HWND, int, SCROLLINFO*);
	using FShowScrollBar = BOOL(WINAPI*)(HWND, int, BOOL);

	static FSetScrollInfo s_pfnSetScrollInfo;
	static FGetScrollInfo s_pfnGetScrollInfo;
	static FShowScrollBar s_pfnShowScrollBar;

	static int WINAPI NewSetScrollInfo(HWND hWnd, int nBar, const SCROLLINFO* psi, BOOL bRedraw)
	{

		return s_pfnSetScrollInfo(hWnd, nBar, psi, bRedraw);
	}

	static BOOL WINAPI NewGetScrollInfo(HWND hWnd, int nBar, SCROLLINFO* psi)
	{

		return s_pfnGetScrollInfo(hWnd, nBar, psi);
	}

	static BOOL WINAPI NewShowScrollBar(HWND hWnd, int nBar, BOOL bShow)
	{

		return s_pfnShowScrollBar(hWnd, nBar, bShow);
	}
public:
	BOOL BindWnd(CWnd* pWnd)
	{
		s_cs.Enter();
		if (!s_cHookRef)
		{
			s_cHookRef = 1;
			DetourTransactionBegin();
			DetourAttach(&s_pfnSetScrollInfo, NewSetScrollInfo);
			DetourAttach(&s_pfnGetScrollInfo, NewGetScrollInfo);
			DetourAttach(&s_pfnShowScrollBar, NewShowScrollBar);
			DetourTransactionCommit();
		}
		s_cs.Leave();

		//pWnd->InstallMsgHook([this](HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)->LRESULT
		//	{

		//	});

	}

};

inline int								CScrollBarHook::s_cHookRef{};
inline CCriticalSection					CScrollBarHook::s_cs{};
inline CScrollBarHook::FSetScrollInfo	CScrollBarHook::s_pfnSetScrollInfo{ SetScrollInfo };
inline CScrollBarHook::FGetScrollInfo	CScrollBarHook::s_pfnGetScrollInfo{ GetScrollInfo };
inline CScrollBarHook::FShowScrollBar	CScrollBarHook::s_pfnShowScrollBar{ ShowScrollBar };
ECK_NAMESPACE_END