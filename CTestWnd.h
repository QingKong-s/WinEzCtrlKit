﻿#pragma once
#include "eck\CWnd.h"
#include "eck\DbgHelper.h"
#include "eck\DlgHelper.h"
#include "eck\ECK.h"
#include "eck\CButton.h"
#include "eck\CEditExt.h"
#include "eck\CLabel.h"
#include "eck\CColorPicker.h"
#include "eck\ImageHelper.h"
#include "eck\CListBoxExt.h"
#include "eck\CCommDlg.h"
#include "eck\CDirBox.h"
#include "eck\CScrollBar.h"
#include "eck\CComboBox.h"
#include "eck\CListView.h"
#include "eck\CTaskGroupList.h"
#include "eck\CArray.h"
#include "eck\CResSet.h"
#include "eck\CFormTable.h"
#include "eck\CTabHeader.h"
#include "eck\CSplitBar.h"
#include "eck\CDrawPanel.h"
#include "eck\CLunarCalendar.h"
#include "eck\CListBoxNew.h"
#include "eck\CMenu.h"
#include "eck\CFlowLayout.h"

#define WCN_TEST L"TesttttttttttttttWndddddddddd"

using eck::PCVOID;

class CTestWnd :public eck::CWnd
{
private:
	eck::CPushButton m_Btn;
	eck::CEditExt m_Edit;
	eck::CLabel m_Label;
	eck::CColorPicker m_CC;
	eck::CListBoxExt m_LBExt;
	eck::CDirBox m_DirBox;
	eck::CScrollBarWndH m_SBNcH;
	eck::CScrollBar m_SB;
	eck::CListView m_LV;
	eck::CTaskGroupList m_TGL;
	eck::CTabHeader m_TH;
	eck::CSplitBar m_SPB;
	eck::CSplitBar m_SPBH;
	eck::CDrawPanel m_DP;
	eck::CDrawPanelD2D m_DPD2D;
	eck::CListBoxNew m_LBN;

	eck::CFlowLayout m_lot{};

	HFONT m_hFont = eck::EzFont(L"微软雅黑", 9);
	HBITMAP m_hbm = NULL;
public:
	LRESULT OnMsg(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) override
	{
		switch (uMsg)
		{
		case WM_CREATE:
		{
			m_Btn.Create(L"按钮测试", WS_CHILD | WS_VISIBLE, 0, 0, 0, 300, 70, hWnd, 101);
			m_lot.Add(&m_Btn, eck::FLF_FIXWIDTH | eck::FLF_FIXHEIGHT);
			m_Edit.Create(L"编辑框", WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL, WS_EX_CLIENTEDGE,
				0, 0, 200, 100, hWnd, 102);
			m_lot.Add(&m_Edit, eck::FLF_FIXWIDTH | eck::FLF_FIXHEIGHT);
			m_Label.Create(L"我是标签", WS_CHILD | WS_VISIBLE | WS_BORDER, 0, 0, 0, 300, 200, hWnd, 103);
			m_hbm = eck::CreateHBITMAP(LR"(E:\Desktop\Temp\111111.jpg)");
			m_Label.SetPic(m_hbm);
			m_lot.Add(&m_Label);

			eck::SetFontForWndAndCtrl(hWnd, m_hFont);
		}
		return 0;
		case WM_SIZE:
		{
			const int cx = LOWORD(lParam), cy = HIWORD(lParam);
			m_lot.Arrange(cx, cy);
		}
		return 0;
		case WM_COMMAND:
		{
			if ((HWND)lParam == m_Btn.GetHWND() && HIWORD(wParam) == BN_CLICKED)
				EckDbgPrintWndMap();
		}
		break;
		}
		return CWnd::OnMsg(hWnd, uMsg, wParam, lParam);
	}

	static ATOM RegisterWndClass()
	{
		WNDCLASSW wc{};
		wc.cbWndExtra = sizeof(void*);
		wc.hCursor = LoadCursorW(NULL, IDC_ARROW);
		wc.hInstance = eck::g_hInstance;
		wc.lpfnWndProc = DefWindowProcW;
		wc.lpszClassName = WCN_TEST;
		wc.style = CS_DBLCLKS | CS_VREDRAW | CS_HREDRAW;
		wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
		return RegisterClassW(&wc);
	}

	EckInline HWND Create(PCWSTR pszText, DWORD dwStyle, DWORD dwExStyle,
		int x, int y, int cx, int cy, HWND hParent, int nID, PCVOID pData = NULL) override
	{
		m_hWnd = IntCreate(dwExStyle, WCN_TEST, pszText, dwStyle,
			x, y, cx, cy, hParent, eck::i32ToP<HMENU>(nID), eck::g_hInstance, this);
		return m_hWnd;
	}
};