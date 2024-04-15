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
#include "CMemWalker.h"
#include "CException.h"
#include "ILayout.h"

#include <optional>
#include <functional>

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
		int x, int y, int cx, int cy, HWND hParent, int nID, ::eck::PCVOID pData = NULL)	\
	{																				\
		return Create(pszText, dwStyle, dwExStyle, x, y, cx, cy,					\
			hParent, ::eck::i32ToP<HMENU>(nID), pData);								\
	}

#define ECK_CWND_SINGLEOWNER													\
	[[nodiscard]] EckInline static constexpr BOOL IsSingleOwner() { return TRUE; }	\
	void Attach(HWND hWnd) override													\
	{																				\
		throw ::eck::CAttachSingleOwnerWndException{};								\
	}																				\
	HWND Detach() override															\
	{																				\
		throw ::eck::CDetachSingleOwnerWndException{};								\
	}																				\

#define ECK_CWND_NOSINGLEOWNER(Class)												\
	[[nodiscard]] EckInline static constexpr BOOL IsSingleOwner() { return FALSE; }	\
	Class() = default;																\
	Class(HWND hWnd) { m_hWnd = hWnd; }


class CWnd :public ILayout
{
	friend HHOOK BeginCbtHook(CWnd* pCurrWnd, FWndCreating pfnCreatingProc);
public:
	using FMsgHook = std::function<LRESULT(HWND, UINT, WPARAM, LPARAM, BOOL& bProcessed)>;

#ifdef ECK_CTRL_DESIGN_INTERFACE
	DESIGNDATA_WND m_DDBase{};
#endif
protected:
	HWND m_hWnd = NULL;
	WNDPROC m_pfnRealProc = DefWindowProcW;
	std::vector<FMsgHook> m_vFnMsgHook{};
	HDWP m_hDwpLayout = NULL;

	static void WndCreatingSetLong(HWND hWnd, CBT_CREATEWNDW* pcs, ECKTHREADCTX* pThreadCtx)
	{
		SetWindowLongPtrW(hWnd, 0, (LONG_PTR)pThreadCtx->pCurrWnd);
	}

	EckInline HWND IntCreate(DWORD dwExStyle, PCWSTR pszClass, PCWSTR pszText, DWORD dwStyle,
		int x, int y, int cx, int cy, HWND hParent, HMENU hMenu, HINSTANCE hInst, void* pParam,
		FWndCreating pfnCreatingProc = NULL)
	{
		BeginCbtHook(this, pfnCreatingProc);
#ifdef _DEBUG
		CreateWindowExW(dwExStyle, pszClass, pszText, dwStyle,
			x, y, cx, cy, hParent, hMenu, hInst, pParam);
		if (!m_hWnd)
		{
			EckDbgPrintFormatMessage(GetLastError());
			EckDbgBreak();
		}
		if (IsWindow(m_hWnd))
			return m_hWnd;
		else
			return m_hWnd = NULL;
#else
		return CreateWindowExW(dwExStyle, pszClass, pszText, dwStyle,
			x, y, cx, cy, hParent, hMenu, hInst, pParam);
#endif // _DEBUG
	}

	template<class T>
	EckInline void FillNmhdr(T& nm, UINT uCode) const
	{
		static_assert(sizeof(T) >= sizeof(NMHDR));
		auto p = (NMHDR*)&nm;
		p->hwndFrom = GetHWND();
		p->code = uCode;
		p->idFrom = GetDlgCtrlID(GetHWND());
	}

	template<class T>
	EckInline LRESULT SendNotify(T& nm) const
	{
		return SendMessageW(GetParent(GetHWND()), WM_NOTIFY, ((NMHDR*)&nm)->idFrom, (LPARAM)&nm);
	}

	template<class T>
	EckInline LRESULT FillNmhdrAndSendNotify(T& nm, UINT uCode) const
	{
		FillNmhdr(nm, uCode);
		return SendNotify(nm);
	}

	LRESULT DefNotifyMsg(HWND hParent, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		return CWndFromHWND(hParent)->OnMsg(hParent, uMsg, wParam, lParam);
	}

	EckInline LRESULT CallMsgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		EckCounter(m_vFnMsgHook.size(), i)
		{
			BOOL bProcessed;
			const auto lResult = m_vFnMsgHook[i].operator()(hWnd, uMsg, wParam, lParam, bProcessed);
			if (bProcessed)
				return lResult;
		}
		return OnMsg(hWnd, uMsg, wParam, lParam);
	}
public:
	ECKPROP_R(GetHWND)					HWND		HWnd;			// 窗口句柄
	ECKPROP(GetFont, SetFont)			HFONT		HFont;			// 字体句柄
	ECKPROP(GetStyle, SetStyle)			DWORD		Style;			// 窗口样式
	ECKPROP(GetExStyle, SetExStyle)		DWORD		ExStyle;		// 扩展窗口样式
	ECKPROP(GetText, SetText)			CRefStrW	Text;			// 标题
	ECKPROP(GetFrameType, SetFrameType) int			FrameType;		// 边框类型
	ECKPROP(GetScrollBar, SetScrollBar) int			ScrollBarType;	// 滚动条类型
	ECKPROP(IsVisible, SetVisibility)	BOOL		Visible;		// 可视
	ECKPROP(IsEnabled, Enable)			BOOL		Enabled;		// 可用（未禁止）
	ECKPROP_W(SetRedraw)				BOOL		Redrawable;		// 可重画
	ECKPROP(GetLeft, SetLeft)			int			Left;			// 左边，相对父窗口
	ECKPROP(GetTop, SetTop)				int			Top;			// 顶边，相对父窗口
	ECKPROP(GetWidth, SetWidth)			int			Width;			// 宽度
	ECKPROP(GetHeight, SetHeight)		int			Height;			// 高度
	ECKPROP(GetPosition, SetPosition)	POINT		Position;		// 位置
	ECKPROP(GetSize, SetSize)			SIZE		Size;			// 尺寸
	ECKPROP_R(GetClientWidth)			int			ClientWidth;
	ECKPROP_R(GetClientHeight)			int			ClientHeight;


	static LRESULT CALLBACK EckWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		const auto pCtx = GetThreadCtx();
		const auto p = pCtx->WmAt(hWnd);
		EckAssert(p);

		CWnd* pChild;
		BOOL bProcessed = FALSE;
		switch (uMsg)
		{
		case WM_NOTIFY:
			if (pChild = pCtx->WmAt(((NMHDR*)lParam)->hwndFrom))
			{
				const auto lResult = pChild->OnNotifyMsg(hWnd, uMsg, wParam, lParam, bProcessed);
				if (bProcessed)
					return lResult;
			}
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
			if (pChild = pCtx->WmAt((HWND)lParam))
			{
				const auto lResult = pChild->OnNotifyMsg(hWnd, uMsg, wParam, lParam, bProcessed);
				if (bProcessed)
					return lResult;
			}
			break;
		case WM_DRAWITEM:
			if (pChild = pCtx->WmAt(((DRAWITEMSTRUCT*)lParam)->hwndItem))
			{
				const auto lResult = pChild->OnNotifyMsg(hWnd, uMsg, wParam, lParam, bProcessed);
				if (bProcessed)
					return lResult;
			}
			break;
		case WM_MEASUREITEM:
			if (pChild = pCtx->WmAt(GetDlgItem(hWnd, ((MEASUREITEMSTRUCT*)lParam)->CtlID)))
			{
				const auto lResult = pChild->OnNotifyMsg(hWnd, uMsg, wParam, lParam, bProcessed);
				if (bProcessed)
					return lResult;
			}
			break;
		case WM_DELETEITEM:
			if (pChild = pCtx->WmAt(((DELETEITEMSTRUCT*)lParam)->hwndItem))
			{
				const auto lResult = pChild->OnNotifyMsg(hWnd, uMsg, wParam, lParam, bProcessed);
				if (bProcessed)
					return lResult;
			}
			break;
		case WM_COMPAREITEM:
			if (pChild = pCtx->WmAt(((COMPAREITEMSTRUCT*)lParam)->hwndItem))
			{
				const auto lResult = pChild->OnNotifyMsg(hWnd, uMsg, wParam, lParam, bProcessed);
				if (bProcessed)
					return lResult;
			}
			break;
		case WM_NCDESTROY:// 窗口生命周期中的最后一个消息，在这里解绑HWND和CWnd，从窗口映射中清除无效内容
		{
			const auto lResult = p->CallMsgProc(hWnd, uMsg, wParam, lParam);
			(void)p->CWnd::Detach();// 控件类可能不允许拆离，必须使用基类拆离
			return lResult;
		}
		}
		return p->CallMsgProc(hWnd, uMsg, wParam, lParam);
	}

	/// <summary>
	/// 是否可更改句柄所有权
	/// </summary>
	/// <returns>若类与HWND没有强联系则返回FALSE，否则返回TRUE，此时不能执行依附拆离等操作</returns>
	[[nodiscard]] EckInline static constexpr BOOL IsSingleOwner() { return FALSE; }

	CWnd() = default;

	/// <summary>
	/// 构造自句柄。
	/// 不插入窗口映射，仅用于临时使用
	/// </summary>
	/// <param name="hWnd"></param>
	CWnd(HWND hWnd) :m_hWnd{ hWnd } {}

	virtual ~CWnd()
	{
#ifdef _DEBUG
		// 对于已添加进映射的窗口，CWnd的生命周期必须在窗口生命周期之内
		if (m_hWnd)
			EckAssert(((GetThreadCtx()->WmAt(m_hWnd) == this) ? (!m_hWnd) : TRUE));
#endif // _DEBUG
	}

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
	[[nodiscard]] virtual HWND Detach()
	{
		HWND hWnd = NULL;
		std::swap(hWnd, m_hWnd);
		const auto pCtx = GetThreadCtx();
		EckAssert(pCtx->WmAt(hWnd) == this);// 检查匹配性
		pCtx->WmRemove(hWnd);
		return hWnd;
	}

	EckInline void AttachNew(HWND hWnd)
	{
		Attach(hWnd);
		m_pfnRealProc = SetWindowProc(hWnd, EckWndProc);
	}

	EckInline void DetachNew()
	{
		SetWindowProc(Detach(), m_pfnRealProc);
	}

	/// <summary>
	/// 创建窗口
	/// </summary>
	EckInline HWND Create(PCWSTR pszText, DWORD dwStyle, DWORD dwExStyle,
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
		throw CConstMsgException(L"调用了CWnd的Create方法");
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
		//switch (uMsg)
		//{
		//case WM_CTLCOLORBTN:
		//case WM_CTLCOLOREDIT:
		//case WM_CTLCOLORDLG:
		//case WM_CTLCOLORLISTBOX:
		//case WM_CTLCOLORSCROLLBAR:
		//case WM_CTLCOLORSTATIC:
		//	SetDCBrushColor((HDC)wParam, 0x383838);
		//	return (LRESULT)GetStockBrush(DC_BRUSH);
		//	;
		//}
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
	/// <param name="bProcessed">若设为TRUE，则不再将当前消息交由父窗口处理，调用函数前保证其为FALSE</param>
	/// <returns>消息返回值</returns>
	EckInline virtual LRESULT OnNotifyMsg(HWND hParent, UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bProcessed)
	{
		return 0;
	}

	/// <summary>
	/// 取理想尺寸
	/// </summary>
	/// <param name="cx">理想宽度</param>
	/// <param name="cy">理想高度</param>
	void LoGetAppropriateSize(int& cx, int& cy) override
	{
		RECT rc;
		GetWindowRect(m_hWnd, &rc);
		cx = rc.right - rc.left;
		cy = rc.bottom - rc.top;
	}

	std::pair<int, int> LoGetPos()
	{
		const auto pt{ GetPosition() };
		return { pt.x,pt.y };
	}

	std::pair<int, int> LoGetSize()
	{
		const auto size{ GetSize() };
		return { size.cx,size.cy };
	}

	HWND LoGetHWND() override
	{
		return GetHWND();
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

		Destroy();
		return Create(NULL, 0, 0,
			rcPos.value().left, rcPos.value().top, rcPos.value().right, rcPos.value().bottom,
			hParent, iID, rb.Data());
	}

	/// <summary>
	/// 取窗口句柄
	/// </summary>
	[[nodiscard]] EckInline HWND GetHWND() const { return m_hWnd; }

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
		return RedrawWindow(m_hWnd, NULL, NULL, RDW_INVALIDATE | RDW_ERASE);
	}

	/// <summary>
	/// 区域重画
	/// </summary>
	EckInline BOOL Redraw(const RECT& rc) const
	{
		return RedrawWindow(m_hWnd, &rc, NULL, RDW_INVALIDATE | RDW_ERASE);
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
	[[nodiscard]] int GetFrameType() const
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
	[[nodiscard]] int GetScrollBar() const
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

	EckInline LRESULT SendMsg(UINT uMsg, WPARAM wParam, LPARAM lParam) const
	{
		return SendMessageW(m_hWnd, uMsg, wParam, lParam);
	}

	EckInline LRESULT PostMsg(UINT uMsg, WPARAM wParam, LPARAM lParam) const
	{
		return PostMessageW(m_hWnd, uMsg, wParam, lParam);
	}

	/// <summary>
	/// 取窗口样式
	/// </summary>
	[[nodiscard]] EckInline DWORD GetStyle() const
	{
		return (DWORD)GetWindowLongPtrW(m_hWnd, GWL_STYLE);
	}

	/// <summary>
	/// 取扩展样式
	/// </summary>
	[[nodiscard]] EckInline DWORD GetExStyle() const
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
	[[nodiscard]] EckInline CRefStrW GetText() const
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

	EckInline HRESULT SetItemsViewTheme() const
	{
		return SetWindowTheme(m_hWnd, L"ItemsView", NULL);
	}

	EckInline HRESULT SetTheme(PCWSTR pszAppName, PCWSTR pszSubList = NULL) const
	{
		return SetWindowTheme(m_hWnd, pszAppName, pszSubList);
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
		auto b = DestroyWindow(m_hWnd);
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
	[[nodiscard]] EckInline HFONT GetFont() const
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
	[[nodiscard]] EckInline BOOL IsEnabled() const
	{
		return IsWindowEnabled(m_hWnd);
	}

	/// <summary>
	/// 是否可见
	/// </summary>
	[[nodiscard]] EckInline BOOL IsVisible() const
	{
		return IsWindowVisible(m_hWnd);
	}

	/// <summary>
	/// 取长型
	/// </summary>
	/// <param name="i">GWL_常量</param>
	[[nodiscard]] EckInline LONG_PTR GetLong(int i) const
	{
		return GetWindowLongPtrW(m_hWnd, i);
	}

	/// <summary>
	/// 置长型
	/// </summary>
	/// <param name="i">GWL_常量</param>
	/// <param name="l">新长型</param>
	/// <returns>旧长型</returns>
	[[nodiscard]] EckInline LONG_PTR SetLong(int i, LONG_PTR l) const
	{
		return SetWindowLongPtrW(m_hWnd, i, l);
	}

	[[nodiscard]] EckInline CRefStrW GetClsName() const
	{
		CRefStrW rs(256);
		rs.ReSize(GetClassNameW(GetHWND(), rs.Data(), 256 + 1));
		return rs;
	}

	void SetLeft(int i) const
	{
		RECT rc;
		GetWindowRect(m_hWnd, &rc);
		ScreenToClient(GetParent(m_hWnd), (POINT*)&rc);
		SetWindowPos(m_hWnd, NULL, i, rc.top, 0, 0, SWP_NOZORDER | SWP_NOSIZE | SWP_NOACTIVATE);
	}

	[[nodiscard]] int GetLeft() const
	{
		RECT rc;
		GetWindowRect(m_hWnd, &rc);
		ScreenToClient(GetParent(m_hWnd), (POINT*)&rc);
		return rc.left;
	}

	void SetTop(int i) const
	{
		RECT rc;
		GetWindowRect(m_hWnd, &rc);
		ScreenToClient(GetParent(m_hWnd), (POINT*)&rc);
		SetWindowPos(m_hWnd, NULL, rc.left, i, 0, 0, SWP_NOZORDER | SWP_NOSIZE | SWP_NOACTIVATE);
	}

	[[nodiscard]] int GetTop() const
	{
		RECT rc;
		GetWindowRect(m_hWnd, &rc);
		ScreenToClient(GetParent(m_hWnd), (POINT*)&rc);
		return rc.top;
	}

	void SetWidth(int i) const
	{
		RECT rc;
		GetWindowRect(m_hWnd, &rc);
		SetWindowPos(m_hWnd, NULL, 0, 0, i, rc.bottom - rc.top,
			SWP_NOZORDER | SWP_NOMOVE | SWP_NOACTIVATE);
	}

	[[nodiscard]] int GetWidth() const
	{
		RECT rc;
		GetWindowRect(m_hWnd, &rc);
		return rc.right - rc.left;
	}

	void SetHeight(int i) const
	{
		RECT rc;
		GetWindowRect(m_hWnd, &rc);
		SetWindowPos(m_hWnd, NULL, 0, 0, rc.right - rc.left, i,
			SWP_NOZORDER | SWP_NOMOVE | SWP_NOACTIVATE);
	}

	[[nodiscard]] int GetHeight() const
	{
		RECT rc;
		GetWindowRect(m_hWnd, &rc);
		return rc.bottom - rc.top;
	}

	EckInline void SetPosition(POINT pt) const
	{
		SetWindowPos(m_hWnd, NULL, pt.x, pt.y, 0, 0, SWP_NOZORDER | SWP_NOSIZE | SWP_NOACTIVATE);
	}

	[[nodiscard]] EckInline POINT GetPosition() const
	{
		RECT rc;
		GetWindowRect(m_hWnd, &rc);
		ScreenToClient(GetParent(m_hWnd), (POINT*)&rc);
		return { rc.left, rc.top };
	}

	EckInline void SetSize(SIZE sz) const
	{
		SetWindowPos(m_hWnd, NULL, 0, 0, sz.cx, sz.cy, SWP_NOZORDER | SWP_NOMOVE | SWP_NOACTIVATE);
	}

	[[nodiscard]] EckInline SIZE GetSize() const
	{
		RECT rc;
		GetWindowRect(m_hWnd, &rc);
		return { rc.right - rc.left, rc.bottom - rc.top };
	}

	EckInline BOOL EnableArrows(int iOp, int iBarType)
	{
		EnableScrollBar(m_hWnd, iBarType, iOp);
	}

	EckInline int GetSbPos(int iType) const
	{
		SCROLLINFO si;
		si.cbSize = sizeof(SCROLLINFO);
		si.fMask = SIF_POS;
		GetScrollInfo(m_hWnd, iType, &si);
		return si.nPos;
	}

	EckInline int GetSbTrackPos(int iType) const
	{
		SCROLLINFO si;
		si.cbSize = sizeof(SCROLLINFO);
		si.fMask = SIF_TRACKPOS;
		GetScrollInfo(m_hWnd, iType, &si);
		return si.nTrackPos;
	}

	EckInline BOOL GetSbRange(int iType, int* piMin = NULL, int* piMax = NULL) const
	{
		SCROLLINFO si;
		si.cbSize = sizeof(SCROLLINFO);
		si.fMask = SIF_RANGE;
		BOOL b = GetScrollInfo(m_hWnd, iType, &si);
		if (piMin)
			*piMin = si.nMin;
		if (piMax)
			*piMax = si.nMax;
		return b;
	}

	EckInline int GetSbPage(int iType) const
	{
		SCROLLINFO si;
		si.cbSize = sizeof(SCROLLINFO);
		si.fMask = SIF_PAGE;
		GetScrollInfo(m_hWnd, iType, &si);
		return si.nPage;
	}

	EckInline BOOL GetSbInfo(int iType, SCROLLINFO* psi) const
	{
		psi->cbSize = sizeof(SCROLLINFO);
		return GetScrollInfo(m_hWnd, iType, psi);
	}

	EckInline void SetSbPos(int iType, int iPos, BOOL bRedraw = TRUE) const
	{
		SCROLLINFO si;
		si.cbSize = sizeof(SCROLLINFO);
		si.fMask = SIF_POS;
		si.nPos = iPos;
		SetScrollInfo(m_hWnd, iType, &si, bRedraw);
	}

	EckInline void SetSbRange(int iType, int iMin, int iMax, BOOL bRedraw = TRUE) const
	{
		SCROLLINFO si;
		si.cbSize = sizeof(SCROLLINFO);
		si.fMask = SIF_RANGE;
		SetScrollInfo(m_hWnd, iType, &si, bRedraw);
		si.nMin = iMin;
		si.nMax = iMax;
	}

	EckInline void SetSbMin(int iType, int iMin, BOOL bRedraw = TRUE) const
	{
		SCROLLINFO si;
		si.cbSize = sizeof(SCROLLINFO);
		si.fMask = SIF_RANGE;
		GetScrollInfo(m_hWnd, iType, &si);
		si.nMin = iMin;
		SetScrollInfo(m_hWnd, iType, &si, bRedraw);
	}

	EckInline void SetSbMax(int iType, int iMax, BOOL bRedraw = TRUE) const
	{
		SCROLLINFO si;
		si.cbSize = sizeof(SCROLLINFO);
		si.fMask = SIF_RANGE;
		GetScrollInfo(m_hWnd, iType, &si);
		si.nMax = iMax;
		SetScrollInfo(m_hWnd, iType, &si, bRedraw);
	}

	EckInline void SetSbPage(int iType, int iPage, BOOL bRedraw = TRUE) const
	{
		SCROLLINFO si;
		si.cbSize = sizeof(SCROLLINFO);
		si.fMask = SIF_PAGE;
		si.nPage = iPage;
		SetScrollInfo(m_hWnd, iType, &si, bRedraw);
	}

	EckInline void SetSbInfo(int iType, SCROLLINFO* psi, BOOL bRedraw = TRUE) const
	{
		SetScrollInfo(m_hWnd, iType, psi, bRedraw);
	}

	EckInline [[nodiscard]] int GetClientWidth() const
	{
		RECT rc;
		GetClientRect(m_hWnd, &rc);
		return rc.right;
	}

	EckInline [[nodiscard]] int GetClientHeight() const
	{
		RECT rc;
		GetClientRect(m_hWnd, &rc);
		return rc.bottom;
	}

	EckInline [[nodiscard]] BOOL IsValid() const
	{
		return IsWindow(m_hWnd);
	}

	EckInline WNDPROC SetWndProc(WNDPROC pfnWndProc)
	{
		std::swap(m_pfnRealProc, pfnWndProc);
		return pfnWndProc;
	}

	EckInline void InstallMsgHook(const FMsgHook& fn, int idx = -1)
	{
		if (idx >= 0)
			m_vFnMsgHook.emplace(m_vFnMsgHook.begin() + idx, fn);
		else
			m_vFnMsgHook.emplace_back(fn);
	}

	EckInline void UnInstallMsgHook(int idx)
	{
		m_vFnMsgHook.erase(m_vFnMsgHook.begin() + idx);
	}
};

EckInline void AttachDlgItems(HWND hDlg, int cItem, CWnd* const* pWnd, const int* iId)
{
	EckCounter(cItem, i)
		pWnd[i]->AttachNew(GetDlgItem(hDlg, iId[i]));
}
ECK_NAMESPACE_END