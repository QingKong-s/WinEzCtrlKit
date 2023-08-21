/*
* WinEzCtrlKit Library
*
* WndHelper.h �� ���ڲ�����������
*
* Copyright(C) 2023 QingKong
*/
#pragma once
#include "ECK.h"
#include "Utility.h"

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

EckInline int DpiScale(int i, int iDpi)
{
	return i * iDpi / USER_DEFAULT_SCREEN_DPI;
}

EckInline int DpiScale(int i, int iDpiNew, int iDpiOld)
{
	return i * iDpiNew / iDpiOld;
}

EckInline void DpiScale(RECT& rc, int iDpiNew, int iDpiOld)
{
	rc.left = rc.left * iDpiNew / iDpiOld;
	rc.top = rc.top * iDpiNew / iDpiOld;
	rc.right = rc.right * iDpiNew / iDpiOld;
	rc.bottom = rc.bottom * iDpiNew / iDpiOld;
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
/// ��������
/// </summary>
/// <param name="pszFontName">��������</param>
/// <param name="iPoint">����</param>
/// <param name="iWeight">Ȩ��</param>
/// <param name="bItalic">�Ƿ���б</param>
/// <param name="bUnderline">�Ƿ��»���</param>
/// <param name="bStrikeOut">�Ƿ�ɾ����</param>
/// <param name="hWnd">����߶�ʱ�Ĳ��մ��ڣ���ʹ�ô˴��ڵ�DC��������Ĭ��ʹ�����洰��</param>
/// <returns>������</returns>
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
/// ���ô������塣
/// ����ö�ٴ��ڵ������Ӵ��ڲ�Ϥ����������
/// </summary>
/// <param name="hWnd">���ھ��</param>
/// <param name="hFont">������</param>
/// <param name="bRedraw">�Ƿ��ػ�</param>
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
/// ����DPI�����ġ�
/// ��������'ÿ��ʾ��DPI��֪V2'��֪ģʽ��
/// �����ݹ�������ڵ��������ﴰ�ڲ�����DeferWindowPosϤ�������ߴ硣
/// ���ڵ�������Դ�����������ʵ�ִ����޸�
/// </summary>
/// <param name="hWnd">���ھ��</param>
/// <param name="iDpiNew">���ĺ��DPI</param>
/// <param name="iDpiOld">����ǰ��DPI</param>
/// <returns>�ɹ�����TRUE��ʧ�ܷ���FALSE</returns>
BOOL OnDpiChanged_Parent_PreMonV2(HWND hWnd, int iDpiNew, int iDpiOld);

/// <summary>
/// ���¾�DPI���´�������
/// </summary>
/// <param name="hFont">����</param>
/// <param name="iDpiNew">��DPI</param>
/// <param name="iDpiOld">��DPI</param>
/// <param name="bDeletePrevFont">�Ƿ�ɾ��hFontָʾ�����壬Ĭ��FALSE</param>
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
/// ���¾�DPI�������ô�������
/// </summary>
/// <param name="hWnd">���ھ��</param>
/// <param name="iDpiNew">��DPI</param>
/// <param name="iDpiOld">��DPI</param>
/// <param name="bRedraw">�Ƿ��ػ���Ĭ��TRUE</param>
/// <param name="bDeletePrevFont">�Ƿ�ɾ����ǰ�Ĵ������壬Ĭ��FALSE</param>
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

// ����һ��ͼ���б���������ListView�иߣ�����DPI����
#define EckMakeLVItemHeightImageList(cy) ImageList_Create(1, (cy), 0, 1, 0)
// ͨ������ͼ���б�������ListView�иߣ�����DPI����
#define EckLVSetItemHeight(hLV,cy) ((HIMAGELIST)SendMessageW((hLV), LVM_SETIMAGELIST, \
					LVSIL_SMALL, (LPARAM)EckMakeLVItemHeightImageList(cy)))

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
ECK_NAMESPACE_END