/*
* WinEzCtrlKit Library
*
* CColorPicker.h ： 颜色选择器
* 使用所有者绘制的超类化组合框实现的颜色选择器
*
* Copyright(C) 2023 QingKong
*/
#pragma once
#include "CWnd.h"
#include "CSubclassMgr.h"
#include "Utility.h"

#include <memory>

#include <CommCtrl.h>

ECK_NAMESPACE_BEGIN
class CColorPicker :public CWnd
{
private:
	struct PARENTSCCTX
	{
		int iDpi;
	};
	WND_RECORDER_DECL(CColorPicker)
	SUBCLASS_REF_MGR_DECL(CColorPicker, PARENTSCCTX*)

	UINT m_uNotifyMsg = 0;
	COLORREF m_crCustom = CLR_INVALID;
	COLORREF m_crCCDlgCustom[16]{};
	HWND m_hParent = NULL;
	PARENTSCCTX m_ParentCtx{};

	static ATOM m_atomColorPicker;
	static WNDPROC m_pfnColorPickerDefProc;

	static LRESULT CALLBACK SubclassProc_Parent(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
		UINT_PTR uIdSubclass, DWORD_PTR dwRefData);

	static LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
public:
	static ATOM RegisterWndClass(HINSTANCE hInstance);

	HWND Create(PCWSTR pszText, DWORD dwStyle, DWORD dwExStyle,
		int x, int y, int cx, int cy, HWND hParent, int nID, PCVOID pData = NULL);

	COLORREF GetColor();

	BOOL SetColor(COLORREF cr);

	EckInline void SelColor(HWND hWnd, int idx)
	{
		SendMsg(CB_SETCURSEL, idx, 0);
	}

	EckInline void SetNotifyMsg(UINT uMsg)
	{
		m_uNotifyMsg = uMsg;
	}
};
ECK_NAMESPACE_END