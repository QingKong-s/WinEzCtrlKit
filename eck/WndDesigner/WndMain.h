#pragma once
#include <Windows.h>

#define WCN_WDMAIN L"Eck.WndClass.WndDesigner.Main"

#define PROP_DESIGNSEL L"Eck.Prop.Sel"

#define IDMI_WW_COPY	101
#define IDMI_WW_CUT		102
#define IDMI_WW_PASTE	103
#define IDMI_WW_DEL		104
//#define IDMI_WW_COPY	105


#define CBFVER_DESIGN_CTRL_1 1

#pragma pack(push, 4)
struct CBINFOCTRLHEADER
{
	int iVer;
	int cCtrls;
};

struct CBINFOCTRL
{
	DWORD cbData;
	int cchText;
	int cChildren;
	int idxInfo;
	RECT rc;// 左顶宽高，相对于最靠左上角的控件
	DWORD dwStyle;
	DWORD dwExStyle;
};
// WCHAR szText[];
// BYTE byData[];
#pragma pack(pop)

struct VALIDSELCTRL
{
	HWND hWnd;
	int cChildren;
};

//extern int g_iDpi;


LRESULT CALLBACK WndProc_WDMain(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void ShowIt();

LRESULT CALLBACK SubclassProc_Ctrl(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData);