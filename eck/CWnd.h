﻿/*
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
	CRefStrW rsName;
	BITBOOL bVisible : 1;
	BITBOOL bEnable : 1;
};
#endif

#define ECK_CWND_CREATE																\
	HWND Create(PCWSTR pszText, DWORD dwStyle, DWORD dwExStyle,						\
		int x, int y, int cx, int cy, HWND hParent, int nID, PCVOID pData = NULL)	\
	{																				\
		return Create(pszText, dwStyle, dwExStyle, x, y, cx, cy,					\
			hParent, ::eck::i32ToP<HMENU>(nID), pData);								\
	}																				\
	HWND Create(PCWSTR pszText, DWORD dwStyle, DWORD dwExStyle,						\
		int x, int y, int cx, int cy, HWND hParent, HMENU hMenu, PCVOID pData = NULL) override

class CWnd
{
public:
	WNDPROC m_pfnRealProc = DefWindowProcW;
protected:
	struct CREATEPARAM
	{
		CWnd* pWnd;
		void* pParam;
	};
	HWND m_hWnd = NULL;

	static void WndCreatingSetLong(HWND hWnd, CBT_CREATEWNDW* pcs, ECKTHREADCTX* pThreadCtx)
	{
		SetWindowLongPtrW(hWnd, 0, (LONG_PTR)pThreadCtx->pCurrWnd);
	}

	EckInline HWND IntCreate(DWORD dwExStyle, PCWSTR pszClass, PCWSTR pszText, DWORD dwStyle,
		int x, int y, int cx, int cy, HWND hParent, HMENU hMenu, HINSTANCE hInst, void* pParam,
		FWndCreating pfnCreatingProc = NULL)
	{
		BeginCbtHook(this, pfnCreatingProc);
		const auto hWnd = CreateWindowExW(dwExStyle, pszClass, pszText, dwStyle,
			x, y, cx, cy, hParent, hMenu, hInst, pParam);
		return hWnd;
	}
public:
#ifdef ECK_CTRL_DESIGN_INTERFACE
	DESIGNDATA_WND m_DDBase{};
#endif
public:
	ECKPROP_R(GetHWND) HWND HWnd;
	ECKPROP(GetFont, SetFont) HFONT HFont;
	ECKPROP(GetStyle, SetStyle) DWORD Style;
	ECKPROP(GetExStyle, SetExStyle) DWORD ExStyle;
	ECKPROP_R(GetText) CRefStrW Text;
	ECKPROP(GetFrameType, SetFrameType) int FrameType;
	ECKPROP(GetScrollBar, SetScrollBar) int ScrollBarType;
	ECKPROP(IsVisible, SetVisibility) BOOL Visible;
	ECKPROP(IsEnabled, Enable) BOOL Enabled;
	ECKPROP_W(SetRedraw) BOOL Redrawable;

	/// <summary>
	/// EckWndProc
	/// </summary>
	static LRESULT CALLBACK WndProcMsgReflection(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		auto pCtx = GetThreadCtx();
		auto p = pCtx->WmAt(hWnd);
		EckAssert(p);

		CWnd* pChild;
		LRESULT lResult = 0;
		switch (uMsg)
		{
		case WM_NOTIFY:
			pChild = pCtx->WmAt(((NMHDR*)lParam)->hwndFrom);
			if (pChild && pChild->OnNotifyMsg(hWnd, uMsg, wParam, lParam, lResult))
				return lResult;
			break;
		case WM_HSCROLL:
		case WM_VSCROLL:
		case WM_COMMAND:
		case WM_CHARTOITEM:
		case WM_VKEYTOITEM:
		case WM_CTLCOLORMSGBOX:
		case WM_CTLCOLOREDIT:
		case WM_CTLCOLORLISTBOX:
		case WM_CTLCOLORBTN:
		case WM_CTLCOLORDLG:
		case WM_CTLCOLORSCROLLBAR:
		case WM_CTLCOLORSTATIC:
			pChild = pCtx->WmAt((HWND)lParam);
			if (pChild && pChild->OnNotifyMsg(hWnd, uMsg, wParam, lParam, lResult))
				return lResult;
			break;
		case WM_DRAWITEM:
			pChild = pCtx->WmAt(((DRAWITEMSTRUCT*)lParam)->hwndItem);
			if (pChild && pChild->OnNotifyMsg(hWnd, uMsg, wParam, lParam, lResult))
				return lResult;
			break;
		case WM_MEASUREITEM:
			pChild = pCtx->WmAt(GetDlgItem(hWnd, ((MEASUREITEMSTRUCT*)lParam)->CtlID));
			if (pChild && pChild->OnNotifyMsg(hWnd, uMsg, wParam, lParam, lResult))
				return lResult;
			break;
		case WM_DELETEITEM:
			pChild = pCtx->WmAt(((DELETEITEMSTRUCT*)lParam)->hwndItem);
			if (pChild && pChild->OnNotifyMsg(hWnd, uMsg, wParam, lParam, lResult))
				return lResult;
			break;
		case WM_COMPAREITEM:
			pChild = pCtx->WmAt(((COMPAREITEMSTRUCT*)lParam)->hwndItem);
			if (pChild && pChild->OnNotifyMsg(hWnd, uMsg, wParam, lParam, lResult))
				return lResult;
			break;
		}
		return p->OnMsg(hWnd, uMsg, wParam, lParam);
	}

	/// <summary>
	/// 是否可更改句柄所有权
	/// </summary>
	/// <returns>若类与HWND没有强联系则返回FALSE，否则返回TRUE，此时不能执行依附拆离等操作</returns>
	EckInline static constexpr BOOL IsSingleOwner() { return FALSE; }

	CWnd() {}

	/// <summary>
	/// 构造自句柄。
	/// 不插入窗口映射，仅用于临时使用
	/// </summary>
	/// <param name="hWnd"></param>
	CWnd(HWND hWnd) :m_hWnd(hWnd) {}

	virtual ~CWnd() {}

	/// <summary>
	/// 依附句柄。
	/// 函数将新句柄插入窗口映射，调用此函数必须满足两个前提：本类不能持有句柄；新句柄未在窗口映射中
	/// </summary>
	/// <param name="hWnd">新句柄</param>
	virtual void Attach(HWND hWnd)
	{
		EckAssert(!m_hWnd);// 当前类必须未持有句柄
		const auto pCtx = GetThreadCtx();
		EckAssert(!pCtx->WmAt(hWnd));// 新句柄必须未被CWnd持有
		m_hWnd = hWnd;
		pCtx->WmAdd(hWnd, this);
	}

	/// <summary>
	/// 拆离句柄。
	/// 函数将从窗口映射中移除句柄
	/// </summary>
	/// <returns>本类持有的旧句柄</returns>
	virtual HWND Detach()
	{
		HWND hWnd = NULL;
		std::swap(hWnd, m_hWnd);
		const auto pCtx = GetThreadCtx();
		EckAssert(pCtx->WmAt(hWnd) == this);// 检查匹配性
		pCtx->WmRemove(hWnd);
		return hWnd;
	}

	/// <summary>
	/// 创建窗口
	/// </summary>
	HWND Create(PCWSTR pszText, DWORD dwStyle, DWORD dwExStyle,
		int x, int y, int cx, int cy, HWND hParent, int nID, PCVOID pData = NULL)
	{
		return Create(pszText, dwStyle, dwExStyle, x, y, cx, cy,
			hParent, i32ToP<HMENU>(nID), pData);
	}

	/// <summary>
	/// 创建窗口。
	/// 不能调用基类的此方法
	/// </summary>
	virtual HWND Create(PCWSTR pszText, DWORD dwStyle, DWORD dwExStyle,
		int x, int y, int cx, int cy, HWND hParent, HMENU hMenu, PCVOID pData = NULL)
	{
		EckDbgBreak();
		return NULL;
	}

	/// <summary>
	/// 序列化数据。
	/// 子类若要存储额外数据，一般情况下应首先调用基类的此方法，然后再序列化自己的数据
	/// </summary>
	/// <param name="rb">字节集</param>
	virtual void SerializeData(CRefBin& rb)
	{
		CRefStrW rsText = GetText();
		const SIZE_T cbSize = sizeof(CREATEDATA_STD) + rsText.ByteSize();
		CMemWriter w(rb.PushBack(cbSize), cbSize);
		CREATEDATA_STD* p;
		w.SkipPointer(p);
		p->iVer_Std = DATAVER_STD_1;
		p->cchText = rsText.Size();
		p->dwStyle = GetStyle();
		p->dwExStyle = GetExStyle();

		w << rsText;
	}

	/// <summary>
	/// 消息处理函数
	/// </summary>
	EckInline virtual LRESULT OnMsg(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		return CallWindowProcW(m_pfnRealProc, hWnd, uMsg, wParam, lParam);
	}

	/// <summary>
	/// 消息过滤器
	/// </summary>
	/// <param name="Msg">MSG结构</param>
	/// <returns>若要禁止派发该消息则返回TRUE，否则返回FALSE</returns>
	EckInline virtual BOOL PreTranslateMessage(const MSG& Msg)
	{
		return FALSE;
	}

	/// <summary>
	/// 父窗口通知类消息映射。
	/// 父窗口接收到的通知消息将路由到本方法，一般情况下无需手动调用本方法。
	/// 路由的消息有以下四种：所有者项目系列（WM_XxxITEM）、
	/// 标准通知系列（WM_COMMAND、WM_NOTIFY）、着色系列（WM_CTLCOLORXxx）、
	/// 滚动条系列（WM_VSCROLL、WM_HSCROLL）
	/// </summary>
	/// <param name="hParent">父窗口句柄</param>
	/// <param name="uMsg">消息</param>
	/// <param name="wParam">wParam</param>
	/// <param name="lParam">lParam</param>
	/// <param name="lResult">消息返回值，调用本方法前保证其为0，仅当本方法返回TRUE时有效</param>
	/// <returns>若返回TRUE，则不再将当前消息交由父窗口处理</returns>
	EckInline virtual BOOL OnNotifyMsg(HWND hParent, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT& lResult)
	{
		return FALSE;
	}

	/// <summary>
	/// 取理想尺寸
	/// </summary>
	/// <param name="cx">理想宽度</param>
	/// <param name="cy">理想高度</param>
	EckInline virtual void GetAppropriateSize(int& cx, int& cy) const
	{
		RECT rc;
		GetWindowRect(m_hWnd, &rc);
		cx = rc.right - rc.left;
		cy = rc.bottom - rc.top;
	}

	EckInline static PCVOID SkipBaseData(PCVOID p)
	{
		return (PCBYTE)p +
			sizeof(CREATEDATA_STD) +
			(((const CREATEDATA_STD*)p)->cchText + 1) * sizeof(WCHAR);
	}

	/// <summary>
	/// 重新创建窗口。
	/// 函数先序列化数据，然后销毁当前窗口，最后创建新窗口，期间将自动调整窗口映射
	/// </summary>
	/// <param name="dwNewStyle">可选的窗口样式，将覆盖序列化数据中的样式</param>
	/// <param name="dwNewExStyle">可选的扩展窗口样式，将覆盖序列化数据中的样式</param>
	/// <param name="rcPos">可选的位置（左顶宽高），将覆盖旧位置</param>
	/// <returns>窗口句柄</returns>
	HWND ReCreate(EckOptNul(DWORD, dwNewStyle), EckOptNul(DWORD, dwNewExStyle), EckOptNul(RECT, rcPos))
	{
		CRefBin rb{};
		SerializeData(rb);
		HWND hParent = GetParent(m_hWnd);
		int iID = GetDlgCtrlID(m_hWnd);

		if (!rcPos.has_value())
		{
			RECT rc;
			GetWindowRect(m_hWnd, &rc);
			ScreenToClient(hParent, &rc);
			rcPos = rc;
		}

		auto pData = (CREATEDATA_STD*)rb.Data();
		if (dwNewStyle.has_value())
			pData->dwStyle = dwNewStyle.value();
		if (dwNewExStyle.has_value())
			pData->dwExStyle = dwNewExStyle.value();

		DestroyWindow(m_hWnd);
		return Create(NULL, 0, 0,
			rcPos.value().left, rcPos.value().top, rcPos.value().right, rcPos.value().bottom,
			hParent, iID, rb.Data());
	}

	/// <summary>
	/// 取窗口句柄
	/// </summary>
	EckInline HWND GetHWND() const { return m_hWnd; }

	/// <summary>
	/// 边框已更改
	/// </summary>
	EckInline void FrameChanged() const
	{
		SetWindowPos(m_hWnd, NULL, 0, 0, 0, 0,
			SWP_NOZORDER | SWP_NOSIZE | SWP_NOMOVE | SWP_NOACTIVATE | SWP_FRAMECHANGED);
	}

	/// <summary>
	/// 允许/禁止重画
	/// </summary>
	EckInline void SetRedraw(BOOL bRedraw) const
	{
		SendMsg(WM_SETREDRAW, bRedraw, 0);
	}

	/// <summary>
	/// 重画
	/// </summary>
	EckInline BOOL Redraw() const
	{
		return InvalidateRect(m_hWnd, NULL, FALSE);
	}

	/// <summary>
	/// 区域重画
	/// </summary>
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
	void SetFrameType(int iFrame) const
	{
		DWORD dwStyle = GetStyle() & ~WS_BORDER;
		DWORD dwExStyle = GetExStyle()
			& ~(WS_EX_WINDOWEDGE | WS_EX_STATICEDGE | WS_EX_DLGMODALFRAME | WS_EX_CLIENTEDGE);

		switch (iFrame)
		{
		case 0: break;// 无边框
		case 1: dwExStyle |= WS_EX_CLIENTEDGE; break;// 凹入式
		case 2: dwExStyle |= (WS_EX_WINDOWEDGE | WS_EX_DLGMODALFRAME); break;// 凸出式
		case 3: dwExStyle |= WS_EX_STATICEDGE; break;// 浅凹入式
		case 4: dwExStyle |= (WS_EX_DLGMODALFRAME | WS_EX_CLIENTEDGE); break;// 镜框式
		case 5: dwStyle |= WS_BORDER; break;// 单线边框式
		}

		SetStyle(dwStyle);
		SetExStyle(dwExStyle);
	}

	/// <summary>
	/// 取边框类型
	/// </summary>
	int GetFrameType() const
	{
		const DWORD dwStyle = GetStyle();
		const DWORD dwExStyle = GetExStyle();
		if (IsBitSet(dwExStyle, WS_EX_DLGMODALFRAME))
		{
			if (IsBitSet(dwExStyle, WS_EX_CLIENTEDGE))
				return 4;// 镜框式
			if (IsBitSet(dwExStyle, WS_EX_WINDOWEDGE))
				return 2;// 凸出式
		}

		if (IsBitSet(dwExStyle, WS_EX_CLIENTEDGE))
			return 1;// 凹入式
		if (IsBitSet(dwExStyle, WS_EX_STATICEDGE))
			return 3;// 浅凹入式
		if (IsBitSet(dwStyle, WS_BORDER))
			return 5;// 单线边框式

		return 0;// 无边框
	}

	/// <summary>
	/// 置滚动条类型
	/// </summary>
	void SetScrollBar(int i) const
	{
		switch (i)
		{
		case 0:
			ShowScrollBar(m_hWnd, SB_VERT, FALSE);
			ShowScrollBar(m_hWnd, SB_HORZ, FALSE);
			break;
		case 1:
			ShowScrollBar(m_hWnd, SB_VERT, FALSE);
			ShowScrollBar(m_hWnd, SB_HORZ, TRUE);
			break;
		case 2:
			ShowScrollBar(m_hWnd, SB_VERT, TRUE);
			ShowScrollBar(m_hWnd, SB_HORZ, FALSE);
			break;
		case 3:
			ShowScrollBar(m_hWnd, SB_VERT, TRUE);
			ShowScrollBar(m_hWnd, SB_HORZ, TRUE);
			break;
		}
	}

	/// <summary>
	/// 取滚动条类型
	/// </summary>
	int GetScrollBar() const
	{
		const BOOL bVSB = IsBitSet(GetWindowLongPtrW(m_hWnd, GWL_STYLE), WS_VSCROLL);
		const BOOL bHSB = IsBitSet(GetWindowLongPtrW(m_hWnd, GWL_STYLE), WS_HSCROLL);
		if (bVSB)
			if (bHSB)
				return 3;
			else
				return 2;
		if (bHSB)
			return 1;

		return 0;
	}

	/// <summary>
	/// 发送消息
	/// </summary>
	EckInline LRESULT SendMsg(UINT uMsg, WPARAM wParam, LPARAM lParam) const
	{
		return SendMessageW(m_hWnd, uMsg, wParam, lParam);
	}

	/// <summary>
	/// 取窗口样式
	/// </summary>
	EckInline DWORD GetStyle() const
	{
		return (DWORD)GetWindowLongPtrW(m_hWnd, GWL_STYLE);
	}

	/// <summary>
	/// 取扩展样式
	/// </summary>
	EckInline DWORD GetExStyle() const
	{
		return (DWORD)GetWindowLongPtrW(m_hWnd, GWL_EXSTYLE);
	}

	/// <summary>
	/// 修改样式
	/// </summary>
	/// <param name="dwNew">新样式</param>
	/// <param name="dwMask">掩码</param>
	/// <param name="idx">GWL_常量</param>
	/// <returns>旧样式</returns>
	EckInline DWORD ModifyStyle(DWORD dwNew, DWORD dwMask, int idx = GWL_STYLE) const
	{
		return ModifyWindowStyle(m_hWnd, dwNew, dwMask, idx);
	}

	/// <summary>
	/// 置样式
	/// </summary>
	/// <param name="dwStyle">样式</param>
	/// <returns>旧样式</returns>
	EckInline DWORD SetStyle(DWORD dwStyle) const
	{
		return (DWORD)SetWindowLongPtrW(m_hWnd, GWL_STYLE, dwStyle);
	}

	/// <summary>
	/// 置扩展样式
	/// </summary>
	/// <param name="dwStyle">样式</param>
	/// <returns>旧样式</returns>
	EckInline DWORD SetExStyle(DWORD dwStyle) const
	{
		return (DWORD)SetWindowLongPtrW(m_hWnd, GWL_EXSTYLE, dwStyle);
	}

	/// <summary>
	/// 取标题
	/// </summary>
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

	/// <summary>
	/// 取标题
	/// </summary>
	/// <param name="pszBuf">缓冲区</param>
	/// <param name="cchBuf">缓冲区大小</param>
	/// <returns>复制的字符数</returns>
	EckInline int GetText(PWSTR pszBuf, int cchBuf) const
	{
		return GetWindowTextW(m_hWnd, pszBuf, cchBuf);
	}

	/// <summary>
	/// 置标题
	/// </summary>
	EckInline BOOL SetText(PCWSTR pszText) const
	{
		return SetWindowTextW(m_hWnd, pszText);
	}

	/// <summary>
	/// 置视觉样式
	/// </summary>
	/// <returns></returns>
	EckInline HRESULT SetExplorerTheme() const
	{
		return SetWindowTheme(m_hWnd, L"Explorer", NULL);
	}

	/// <summary>
	/// 移动
	/// </summary>
	EckInline BOOL Move(int x, int y, int cx, int cy, BOOL bNoActive = FALSE) const
	{
		return SetWindowPos(m_hWnd, NULL, x, y, cx, cy, SWP_NOZORDER | (bNoActive ? SWP_NOACTIVATE : 0));
	}

	/// <summary>
	/// 销毁
	/// </summary>
	/// <returns></returns>
	EckInline BOOL Destroy()
	{
		EckAssert(IsWindow(m_hWnd));
		BOOL b = DestroyWindow(m_hWnd);
		if (b)
		{
			const auto pCtx = GetThreadCtx();
			EckAssert(pCtx->WmAt(m_hWnd) == this);// 验证匹配性
			pCtx->WmRemove(m_hWnd);
			m_hWnd = NULL;
		}
		return b;
	}

	/// <summary>
	/// 置字体
	/// </summary>
	/// <param name="hFont">字体句柄</param>
	/// <param name="bRedraw">是否重画</param>
	EckInline void SetFont(HFONT hFont, BOOL bRedraw = FALSE) const
	{
		SendMsg(WM_SETFONT, (WPARAM)hFont, bRedraw);
	}

	/// <summary>
	/// 取字体
	/// </summary>
	EckInline HFONT GetFont() const
	{
		return (HFONT)SendMsg(WM_GETFONT, 0, 0);
	}

	/// <summary>
	/// 显示
	/// </summary>
	/// <param name="nCmdShow">SW_常量</param>
	EckInline BOOL Show(int nCmdShow) const
	{
		return ShowWindow(m_hWnd, nCmdShow);
	}

	EckInline void SetVisibility(BOOL bVisible) const
	{
		Show(bVisible ? SW_SHOW : SW_HIDE);
	}

	/// <summary>
	/// 允许/禁止
	/// </summary>
	EckInline BOOL Enable(BOOL bEnable) const
	{
		return EnableWindow(m_hWnd, bEnable);
	}

	/// <summary>
	/// 是否允许
	/// </summary>
	EckInline BOOL IsEnabled() const
	{
		return IsWindowEnabled(m_hWnd);
	}

	/// <summary>
	/// 是否可见
	/// </summary>
	EckInline BOOL IsVisible() const
	{
		return IsWindowVisible(m_hWnd);
	}

	/// <summary>
	/// 取长型
	/// </summary>
	/// <param name="i">GWL_常量</param>
	EckInline LONG_PTR GetLong(int i) const
	{
		return GetWindowLongPtrW(m_hWnd, i);
	}

	/// <summary>
	/// 置长型
	/// </summary>
	/// <param name="i">GWL_常量</param>
	/// <param name="l">新长型</param>
	/// <returns>旧长型</returns>
	EckInline LONG_PTR SetLong(int i, LONG_PTR l) const
	{
		return SetWindowLongPtrW(m_hWnd, i, l);
	}
};
ECK_NAMESPACE_END