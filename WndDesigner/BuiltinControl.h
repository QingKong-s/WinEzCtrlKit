#pragma once
enum : UINT
{
	BICF_NONE = 0,
};
struct BUILTIN_CTRL
{
	std::wstring_view svClsName;
	std::wstring_view svDisplayName;
	int idxIcon;
	SIZE sizeDef;
	DWORD dwDefStyle;	// 总有WS_CHILD | WS_VISIBLE
	DWORD dwDefExStyle;
	UINT uFlags;		// BICF_
};

constexpr BUILTIN_CTRL BuiltInCtrls[]
{
	{ L"CButton",		L"Button",		 0,{ 80,32 },BS_PUSHBUTTON },
	{ L"CButton",		L"RadioBtn",	 1,{ 80,32 },BS_AUTORADIOBUTTON },
	{ L"CButton",		L"CheckBtn",	 2,{ 80,32 },BS_AUTOCHECKBOX },
	{ L"CButton",		L"TriStateBtn",	68,{ 80,32 },BS_AUTO3STATE },
	{ L"CStatic",		L"Static",		 3,{ 100,32 } },
	{ L"CLabel",		L"Label",		 3,{ 100,32 } },
	{ L"CButton",		L"GroupBox",	 4,{ 100,120 },BS_GROUPBOX },
	{ L"CPictureBox",	L"PicBox",		 5,{ 100,120 } },
	{ L"CEdit",			L"Edit",		 7,{ 100,120 },0,WS_EX_CLIENTEDGE },
	{ L"CEditExt",		L"EditExt",		 7,{ 80,24 },0,WS_EX_CLIENTEDGE },
	{ L"CListBox",		L"ListBox",		 8,{ 100,120 },0,WS_EX_CLIENTEDGE },
	{ L"CListBoxNew",	L"ListBoxNew",	 8,{ 100,120 },0,WS_EX_CLIENTEDGE },
	{ L"CComboBox",		L"ComboBox",	 9,{ 100,24 } },
	{ L"CComboBoxEx",	L"ComboBoxEx",	 9,{ 100,24 } },
	{ L"CComboBoxNew",	L"ComboBoxNew",	 9,{ 100,24 } },
	{ L"CToolBar",		L"ToolBar",		10,{ 100,40 } },
	{ L"CTab",			L"Tab",			12,{ 100,120 } },
	{ L"CStatusBar",	L"StatusBar",	13,{ 100,40 } },
	{ L"CScrollBar",	L"ScrollBar",	14,{ 100,32 } },
	{ L"CProgressBar",	L"ProgressBar",	15,{ 100,32 } },
	{ L"CTrackBar",		L"TrackBar",	16,{ 100,32 } },
	{ L"CDateTimePicker",L"DateTimePicker",18,{ 100,32 } },
	{ L"CMonthCal",		L"MonthCal",	19,{ 100,120 } },
	{ L"CListVew",		L"ListView",	20,{ 100,120 } },
	{ L"CHeader",		L"Header",		21,{ 100,32 } },
	{ L"CTreeView",		L"TreeView",	22,{ 100,120 } },
	{ L"CIPEdit",		L"IPEdit",		23,{ 100,24 } },
	{ L"CUpDown",		L"UpDown",		24,{ 32,40 } },
	{ L"CHotKey",		L"HotKey",		25,{ 100,32 } },
	{ L"CLink",			L"Link",		27,{ 100,32 } },
	{ L"CRichEdit",		L"RichEdit",	28,{ 100,120 },0,WS_EX_CLIENTEDGE },
	{ L"CSplitBar",		L"SplitBar",	32,{ 18,100 } },
	{ L"CHitter",		L"Hitter",		44,{ 32,32 } },
};