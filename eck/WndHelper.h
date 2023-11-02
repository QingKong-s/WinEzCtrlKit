/*
* WinEzCtrlKit Library
*
* WndHelper.h ： 窗口操作帮助函数
*
* Copyright(C) 2023 QingKong
*/
#pragma once
#include "ECK.h"
#include "Utility.h"


#define ECK_DS_BEGIN(StructName) struct StructName {
#define ECK_DS_END_VAR(VarName) } VarName{};
#define ECK_DS_END() };
#define ECK_DS_ENTRY(Name, Size) const int o_##Name = Size; int Name = Size;
#define ECK_DS_ENTRY_F(Name, Size) const float o_##Name = Size; float Name = Size;

#ifndef ECKMACRO_NO_ADD_HANDLE_MSG
#define HANDLE_WM_MOUSELEAVE(hWnd, wParam, lParam, fn) \
	((fn)((hWnd)), 0L)

#endif

#define HANDLE_WM_DPICHANGED(hWnd, wParam, lParam, fn) \
	((fn)((hWnd), (int)(short)LOWORD(wParam), (int)(short)HIWORD(wParam), (RECT*)(lParam)), 0L)

ECK_NAMESPACE_BEGIN

#define ECK_COMMAND_BEGIN(CtrlIdVarName, CtrlHandleVarName) \
	UINT CtrlIdVarName = LOWORD(wParam); \
	HWND CtrlHandleVarName = (HWND)lParam; \
	switch (HIWORD(wParam)) {

#define ECK_COMMAND_CASE(uCode) \
	case uCode:

#define ECK_COMMAND_END() \
	}


#define ECK_NOTIFY_BEGIN(PNMHDRVarName) \
	NMHDR* PNMHDRVarName=(NMHDR*)lParam; \
	switch(PNMHDRVarName->code) {

#define ECK_NOTIFY_CASE(uCode) \
	case uCode:

#define ECK_NOTIFY_END() \
	}

EckInline DWORD ModifyWindowStyle(HWND hWnd, DWORD dwNew, DWORD dwMask, int idx = GWL_STYLE)
{
	DWORD dwStyle = (DWORD)GetWindowLongPtrW(hWnd, idx);
	dwStyle &= ~dwMask;
	dwStyle |= dwNew;
	SetWindowLongPtrW(hWnd, idx, dwStyle);
	return dwStyle;
}

#define SUBCLASS_MGR_DECL(Class) \
	static std::unordered_map<HWND, Class*> m_WndRecord; \
	static eck::CSubclassMgr<Class*> m_SM;
#define SUBCLASS_MGR_INIT(Class, uSCID, SubclassProc) \
	std::unordered_map<HWND, Class*> Class::m_WndRecord{}; \
	eck::CSubclassMgr<Class*> Class::m_SM(m_WndRecord, SubclassProc, uSCID);

#define SUBCLASS_REF_MGR_DECL(Class, CtxType) \
	static eck::CSubclassMgrRef<CtxType>::TRefRecorder::TRecord m_WndRefRecord; \
	static eck::CSubclassMgrRef<CtxType> m_SMRef;
#define SUBCLASS_REF_MGR_INIT(Class, CtxType, uSCID, SubclassProc, Deleter) \
	eck::CSubclassMgrRef<CtxType>::TRefRecorder::TRecord Class::m_WndRefRecord{}; \
	eck::CSubclassMgrRef<CtxType> Class::m_SMRef(m_WndRefRecord, SubclassProc, uSCID, Deleter);

#define WND_RECORDER_DECL(Class) \
	static std::unordered_map<HWND, Class*> m_WndRecord; \
	static eck::CWndRecorder<Class*> m_Recorder;
#define WND_RECORDER_INIT(Class) \
	std::unordered_map<HWND, Class*> Class::m_WndRecord{}; \
	eck::CWndRecorder<Class*> Class::m_Recorder(m_WndRecord);



EckInline int GetDpi(HWND hWnd)
{
#if _WIN32_WINNT >= _WIN32_WINNT_WIN10
	if (hWnd)
		return GetDpiForWindow(hWnd);
	else
		return GetDpiForSystem();
#else
	HDC hDC = GetDC(hWnd);
	int i = GetDeviceCaps(hDC, LOGPIXELSX);
	ReleaseDC(hWnd, hDC);
	return i;
#endif
}

EckInline int DpiScale(int i, int iDpiNew, int iDpiOld)
{
	return i * iDpiNew / iDpiOld;
}

EckInline int DpiScale(int i, int iDpi)
{
	return DpiScale(i, iDpi, USER_DEFAULT_SCREEN_DPI);
}

EckInline float DpiScaleF(float i, int iDpiNew, int iDpiOld)
{
	return i * iDpiNew / iDpiOld;
}

EckInline float DpiScaleF(float i, int iDpi)
{
	return DpiScaleF(i, iDpi, USER_DEFAULT_SCREEN_DPI);
}

EckInline void DpiScale(RECT& rc, int iDpiNew, int iDpiOld)
{
	rc.left = rc.left * iDpiNew / iDpiOld;
	rc.top = rc.top * iDpiNew / iDpiOld;
	rc.right = rc.right * iDpiNew / iDpiOld;
	rc.bottom = rc.bottom * iDpiNew / iDpiOld;
}

EckInline void DpiScale(RECT& rc, int iDpi)
{
	DpiScale(rc, iDpi, USER_DEFAULT_SCREEN_DPI);
}

EckInline void SetPrevDpiProp(HWND hWnd, int iDpi)
{
	SetPropW(hWnd, PROP_PREVDPI, i32ToP<HANDLE>(iDpi));
}

EckInline int GetPrevDpiProp(HWND hWnd)
{
	return pToI32<int>(GetPropW(hWnd, PROP_PREVDPI));
}

EckInline void RemovePrevDpiProp(HWND hWnd)
{
	RemovePropW(hWnd, PROP_PREVDPI);
}

/// <summary>
/// 创建字体
/// </summary>
/// <param name="pszFontName">字体名称</param>
/// <param name="iPoint">点数</param>
/// <param name="iWeight">权重</param>
/// <param name="bItalic">是否倾斜</param>
/// <param name="bUnderline">是否下划线</param>
/// <param name="bStrikeOut">是否删除线</param>
/// <param name="hWnd">计算高度时的参照窗口，将使用此窗口的DC来度量，默认使用桌面窗口</param>
/// <returns>字体句柄</returns>
EckInline HFONT EzFont(PCWSTR pszFontName, int iPoint, int iWeight = 400, BOOL bItalic = FALSE,
	BOOL bUnderline = FALSE, BOOL bStrikeOut = FALSE, HWND hWnd = NULL, DWORD dwCharSet = GB2312_CHARSET)
{
	HDC hDC = GetDC(hWnd);
	int iSize;
	iSize = -MulDiv(iPoint, GetDeviceCaps(hDC, LOGPIXELSY), 72);
	ReleaseDC(hWnd, hDC);
	return CreateFontW(iSize, 0, 0, 0, iWeight, bItalic, bUnderline,
		bStrikeOut, dwCharSet, 0, 0, 0, 0, pszFontName);
}

/// <summary>
/// 设置窗口字体。
/// 函数枚举窗口的所有子窗口并悉数设置字体
/// </summary>
/// <param name="hWnd">窗口句柄</param>
/// <param name="hFont">字体句柄</param>
/// <param name="bRedraw">是否重画</param>
EckInline void SetFontForWndAndCtrl(HWND hWnd, HFONT hFont, BOOL bRedraw = FALSE)
{
	EnumChildWindows(hWnd,
		[](HWND hWnd, LPARAM lParam)->BOOL
		{
			SendMessageW(hWnd, WM_SETFONT, lParam, FALSE);
			return TRUE;
		}, (LPARAM)hFont);
	if (bRedraw)
		RedrawWindow(hWnd, NULL, NULL, RDW_INVALIDATE | RDW_ALLCHILDREN | RDW_UPDATENOW);
}

/// <summary>
/// 窗口DPI被更改。
/// 仅适用于'每显示器DPI感知V2'感知模式。
/// 函数递归遍历窗口的所有子孙窗口并调用DeferWindowPos悉数调整尺寸。
/// 窗口的其他资源如字体等由其实现代码修改
/// </summary>
/// <param name="hWnd">窗口句柄</param>
/// <param name="iDpiNew">更改后的DPI</param>
/// <param name="iDpiOld">更改前的DPI</param>
/// <returns>成功返回TRUE，失败返回FALSE</returns>
BOOL OnDpiChanged_Parent_PreMonV2(HWND hWnd, int iDpiNew, int iDpiOld);

/// <summary>
/// 按新旧DPI重新创建字体
/// </summary>
/// <param name="hFont">字体</param>
/// <param name="iDpiNew">新DPI</param>
/// <param name="iDpiOld">旧DPI</param>
/// <param name="bDeletePrevFont">是否删除hFont指示的字体，默认FALSE</param>
/// <returns></returns>
EckInline HFONT ReCreateFontForDpiChanged(HFONT hFont, int iDpiNew, int iDpiOld, BOOL bDeletePrevFont = FALSE)
{
	LOGFONTW lf;
	GetObjectW(hFont, sizeof(lf), &lf);
	if (bDeletePrevFont)
		DeleteObject(hFont);
	lf.lfHeight = DpiScale(lf.lfHeight, iDpiNew, iDpiOld);
	return CreateFontIndirectW(&lf);
}

/// <summary>
/// 按新旧DPI重新设置窗口字体
/// </summary>
/// <param name="hWnd">窗口句柄</param>
/// <param name="iDpiNew">新DPI</param>
/// <param name="iDpiOld">旧DPI</param>
/// <param name="bRedraw">是否重画，默认TRUE</param>
/// <param name="bDeletePrevFont">是否删除先前的窗口字体，默认FALSE</param>
/// <returns></returns>
EckInline HFONT ResetFontForDpiChanged(HWND hWnd, int iDpiNew, int iDpiOld, BOOL bRedraw = TRUE, BOOL bDeletePrevFont = FALSE)
{
	HFONT hFontNew = ReCreateFontForDpiChanged(
		(HFONT)SendMessageW(hWnd, WM_GETFONT, 0, 0), 
		iDpiNew, iDpiOld, bDeletePrevFont);
	SendMessageW(hWnd, WM_SETFONT, (WPARAM)hFontNew, bRedraw);
	return hFontNew;
}

BOOL GetWindowClientRect(HWND hWnd, HWND hParent, RECT* prc);

/// <summary>
/// 通过设置图像列表来设置ListView行高
/// </summary>
/// <param name="hLV"></param>
/// <param name="cy"></param>
/// <returns></returns>
EckInline HIMAGELIST LVSetItemHeight(HWND hLV, int cy)
{
	return (HIMAGELIST)SendMessageW((hLV), LVM_SETIMAGELIST, LVSIL_SMALL, (LPARAM)ImageList_Create(1, (cy), 0, 1, 0));
}

EckInline BOOL BitBltPs(const PAINTSTRUCT* pps, HDC hdcSrc)
{
	return BitBlt(pps->hdc,
		pps->rcPaint.left,
		pps->rcPaint.top,
		pps->rcPaint.right - pps->rcPaint.left,
		pps->rcPaint.bottom - pps->rcPaint.top,
		hdcSrc,
		pps->rcPaint.left,
		pps->rcPaint.top,
		SRCCOPY);
}

EckInline WNDPROC SetWindowProc(HWND hWnd, WNDPROC pfnWndProc)
{
	return (WNDPROC)SetWindowLongPtrW(hWnd, GWLP_WNDPROC, (LONG_PTR)pfnWndProc);
}

#ifdef ECKMACRO_NO_WIN11_22621
EckInline HRESULT EnableWindowMica(HWND hWnd, DWORD uType = 2)
{
	return E_FAIL;
}
#else
EckInline HRESULT EnableWindowMica(HWND hWnd, DWM_SYSTEMBACKDROP_TYPE uType = DWMSBT_MAINWINDOW)
{
	return DwmSetWindowAttribute(hWnd, DWMWA_SYSTEMBACKDROP_TYPE, &uType, sizeof(uType));
}
#endif

LRESULT OnNcCalcSize(WPARAM wParam, LPARAM lParam, MARGINS* pMargins);

template<class T>
EckInline void UpdateDpiSize(T& Dpis, int iDpi)
{
	for (int* p = ((int*)&Dpis) + 1; p < PtrSkipType(&Dpis); p += 2)
		*p = DpiScale(*(p - 1), iDpi);
}

template<class T>
EckInline void UpdateDpiSizeF(T& Dpis, int iDpi)
{
	for (float* p = ((float*)&Dpis) + 1; p < PtrSkipType(&Dpis); p += 2)
		*p = DpiScaleF(*(p - 1), iDpi);
}

HWND GetThreadFirstWindow(DWORD dwTid);

HWND GetSafeOwner(HWND hParent, HWND* phWndTop);
ECK_NAMESPACE_END