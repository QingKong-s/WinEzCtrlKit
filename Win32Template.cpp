#define ___HEADER_PRIVATE_MAIN___
#include "Win32Template.h"
#include "Resource.h"

#include <dwmapi.h>
#include <ntverp.h>

#include "eck/Env.h"
#include "eck/DbgHelper.h"
#include "eck/DlgHelper.h"

#include "eck/ECK.h"
#include "eck/CButton.h"
#include "eck/CEdit.h"
#include "eck/CLabel.h"
#include "eck/CColorPicker.h"
#include "eck/ImageHelper.h"
#include "eck/CListBoxExt.h"
#include "eck/CCommDlg.h"
#include "eck/CDirBox.h"
#include "eck/CScrollBar.h"
#include "eck/CComboBox.h"
#include "eck/CListView.h"
#include "eck/CTaskGroupList.h"
#include "eck/CArray.h"


using namespace std::literals;

INT_PTR CALLBACK DlgProc_1(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	LPNMHDR     lpnmhdr;
	static eck::CCommandLink cl1,cl2;
	switch (uMsg)
	{
	case WM_INITDIALOG:
	{
		cl1.Create(L"立即浏览 Internet(&I)", WS_VISIBLE | BS_DEFCOMMANDLINK, 0, 80, 300, 600, 80, hDlg, 101);
		cl2.Create(L"设置新连接(&S)", WS_VISIBLE, 0, 80, 400, 600, 80, hDlg, 102);
	}
	break;
	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		BeginPaint(hDlg, &ps);
		FillRect(ps.hdc, &ps.rcPaint, (HBRUSH)GetSysColorBrush(COLOR_WINDOW));
		EndPaint(hDlg, &ps);
	
	}
	return TRUE;
	case WM_NOTIFY:
		lpnmhdr = (NMHDR FAR*)lParam;

		switch (lpnmhdr->code)
		{
		case PSN_APPLY:   //sent when OK or Apply button pressed
			break;

		case PSN_RESET:   //sent when Cancel button pressed
			break;

		case PSN_SETACTIVE:
		{
			PropSheet_SetHeaderTitle(GetParent(hDlg), 0, L"你已经连接到Internet");
			PropSheet_SetWizButtons(GetParent(hDlg), PSWIZB_BACK | PSWIZB_NEXT);
		}
			return FALSE;

		default:
			break;
		}
		break;

	default:
		break;
	}

	return FALSE;
}

int CALLBACK PSProc(HWND hDlg, UINT uMsg, LPARAM lParam)
{
	switch (uMsg)
	{
	case PSCB_INITIALIZED:
	{
		eck::EnableWindowMica(hDlg, DWMSBT_MAINWINDOW);
	}
	break;
	}
	return 0;
}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ PWSTR pszCmdLine, _In_ int nCmdShow)
{
	CoInitialize(NULL);     
	if (eck::Init(hInstance) != eck::InitStatus::Ok)
	{
		MessageBoxW(NULL, L"初始化失败", L"", 0);
		return 1;
	}
	using namespace eck;
	using namespace eck::Literals;

	//PCWSTR psz = L"1000|3000|4000";
	//std::vector<eck::CRefStrW> h{};
	//eck::SplitStr(psz, L"|", h);
	//for (auto& x : h)
	//{
	//	EckDbgPrint(x);
	//}

	//CRefBin rb1{ 3 };
	//CRefBin rb2{ 3 },
	//	rb3{ 10,10,11,12 };
	//auto pos = FindBinRev(rb1, rb2);
	//EckDbgPrintFmt(L"%I64u", pos);
	//rb1.Replace(2, 3, rb2);
	//rb1.ReplaceSubBin(rb2, rb3, 0, 1);
	//rb3 = CRefBin({ 1,2,3 }) + CRefBin({ 4,5,6 });
	//rb3 = rb1 + rb2 + rb3;
	//std::vector<CRefBin> t{};
	//SplitBin(rb1, rb2, t);
	//for (auto& x : t)
	//{
	//	EckDbgPrint(BinToFriendlyString(x.m_pStream, x.m_cb, 1));
	//}
	//EckDbgPrint(BinToFriendlyString(rb3, rb3.m_cb, 1));


	//CArray<int> arr(2, 4, 4);
	///*auto aaa = arr[1][2].e;
	//*(aaa) = 0xFFFFFFFF;*/
	////arr[1][2] = 1;
	////int a = arr[1][2];
	//arr[1][2] = 0xFFFFFFFF;
	//arr.ReDim(TRUE, 3, 3i64, 3i64, 3i64);
	//auto pp = arr[2][2][2].AddressOf();
	//arr[2][2][2] = 0xCCCCCCCC;


	//CRefStrW rs = L"1";
	//int poss = FindStrRev(rs, L"1");
	//EckDbgPrintFmt(L"%d", poss);
	//return 0;
	
	
	//PCWSTR psz = L"1000|||3000||4000";
	//std::vector<eck::CRefStrW> a{};
	//eck::SplitStr(psz, L"||||", a);
	//for (auto& x : a)
	//{
	//	//EckDbgPrint(x);
	//}

	//WCHAR sz[] = L"200||400";
	//std::vector<PWSTR> b{};
	//eck::SplitStr(sz, L"|", b);
	//for (auto x : b)
	//{
	//	EckDbgPrint(x);
	//}

	//PCWSTR psz = L"1哈哈哈A";
	//auto rs = eck::ToFullWidth(psz);
	//EckDbgPrint(psz);
	//EckDbgPrint(rs);


	//using namespace eck;
	//PCWSTR psz = L"       1      2   3   ";
	//CRefStrW rs = AllTrimStr(psz);
	//EckDbgPrint(rs);
	//return 0;

	//EckDbgPrint(CalcNextAlignBoundaryDistance(0, (void*)5, 4));
	//return 0;

	DLGTDLG Dlg{ 1,0xFFFF,0,0,WS_OVERLAPPEDWINDOW | WS_VISIBLE|DS_SETFONT ,3,0,0,240,100 };
	Dlg.Caption = L"测试对话框";
	Dlg.Font = { 9,400,FALSE,GB2312_CHARSET };
	Dlg.rsFontName = L"微软雅黑";

	std::vector<DLGTITEM> Items;
	DLGTITEM a;

	a = { 0,0,WS_VISIBLE | WS_CHILD,20,20,70,26,101 };
	a.Class = WC_BUTTONW;
	a.Caption = L"测试按钮87578678678";
	Items.push_back(std::move(a));

	a = { 0,0,WS_VISIBLE | WS_CHILD,90,20,80,20,102 };
	a.Class = TRACKBAR_CLASSW;
	Items.push_back(std::move(a));

	a = { 0,0,WS_VISIBLE | WS_CHILD,20,60,80,20,102 };
	a.Class = WC_EDITW;
	a.Caption = L"测试编辑框";
	Items.push_back(std::move(a));

	CRefBin rb = SerializeDialogTemplate(Dlg, Items);

	//HWND hDlg = CreateDialogIndirectParamW(NULL, (DLGTEMPLATE*)rb.m_pStream, NULL, DlgProc_About, 0);
	//Dlg = {};
	//Items.clear();
	//DeserializeDialogTemplate(rb, Dlg, Items);
	////HWND hDlg = CreateWindowFromDialogTemplate(Dlg, Items, NULL, NULL, hInstance, NULL, NULL, NULL, 0);

	PROPSHEETPAGEW psp[3]{};
	psp[0].dwSize = sizeof(PROPSHEETPAGEW);
	psp[0].dwFlags = PSP_HIDEHEADER;
	psp[0].hInstance = hInstance;
	psp[0].pszTemplate = MAKEINTRESOURCEW(IDD_ABOUT);
	psp[0].pszTitle = L"测试测试测试测试1";
	psp[0].pszHeaderSubTitle = L"标题标题";
	psp[0].pfnDlgProc = DlgProc_1;

	psp[1].dwSize = sizeof(PROPSHEETPAGEW);
	psp[1].dwFlags = PSP_DLGINDIRECT | PSP_USEHEADERTITLE|PSP_USEHEADERSUBTITLE;
	psp[1].hInstance = hInstance;
	psp[1].pResource = (DLGTEMPLATE*)rb.m_pStream;
	psp[1].pszTitle = L"测试测试测试测试2";
	psp[1].pszHeaderSubTitle = L"标题标题";

	psp[2].dwSize = sizeof(PROPSHEETPAGEW);
	psp[2].dwFlags = PSP_HIDEHEADER;
	psp[2].hInstance = hInstance;
	psp[2].pszTemplate = MAKEINTRESOURCEW(IDD_ABOUT);
	psp[2].pszTitle = L"测试测试测试测试3";

	PROPSHEETHEADERW psh{};

	psh.dwSize = sizeof(psh);
	psh.dwFlags = PSH_USEHICON | PSH_PROPSHEETPAGE | PSH_USECALLBACK |
		PSH_AEROWIZARD | PSH_WIZARD | PSH_HEADER;
		//PSH_WIZARD97;
	psh.hInstance = hInstance;
	psh.hIcon = LoadIconW(NULL, IDI_WINLOGO);
	psh.pszCaption = L"连接到 Internet";
	psh.nPages = ARRAYSIZE(psp);
	psh.nStartPage = 0;
	psh.ppsp = psp;
	psh.pfnCallback = PSProc;

	//PropertySheetW(&psh);

	//eck::EnableWindowMica((HWND)0x00010112,DWMSBT_TRANSIENTWINDOW);

	MSG msg;
	//ShowIt();
	//while (GetMessageW(&msg, NULL, 0, 0))
	//{
	//	TranslateMessage(&msg);
	//	DispatchMessageW(&msg);
	//}

	//return (int)msg.wParam;


	WNDCLASSEXW wcex{ sizeof(WNDCLASSEX) };
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc_Main;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIconW(NULL, IDI_APPLICATION);
	wcex.hCursor = LoadCursorW(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
	wcex.lpszClassName = WCN_MAIN;
	//wcex.cbClsExtra =
	//wcex.cbWndExtra =
	//wcex.lpszMenuName =
	//wcex.hIconSm = LoadIconW(hInstance, MAKEINTRESOURCE()); 
	if (!RegisterClassExW(&wcex))
		return 1;

	HWND hWnd = CreateWindowExW(0, WCN_MAIN, L"示例Win32程序", WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);
	if (!hWnd)
		return 1;
	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	while (GetMessageW(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessageW(&msg);
	}
	return (int)msg.wParam;
}

HFONT g_hFont;

eck::CPushButton* g_Btn;
eck::CEdit* g_Edit;
eck::CLabel* g_Label;
eck::CColorPicker* g_CC;
eck::CListBoxExt* g_LBExt;
eck::CDirBox* g_DirBox;
eck::CScrollBarWndH* g_SBNcH;
eck::CScrollBar* g_SB;
eck::CListView* g_LV;
eck::CTaskGroupList* g_TGL;

int g_iDpi = USER_DEFAULT_SCREEN_DPI;
LRESULT CALLBACK WndProc_Main(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	static HICON hIcon;
	//LRESULT lr;
	//DwmDefWindowProc(hWnd, uMsg, wParam, lParam, &lr);
	//using namespace eck;
	switch (uMsg)
	{
	case WM_SIZE:
	{
	}
	return 0;

	case WM_COMMAND:
	{
		ECK_COMMAND_BEGIN(uCtrlID, hCtrl)
			ECK_COMMAND_CASE(BN_CLICKED)
		{
			switch (uCtrlID)
			{
			case 101:
				g_Label->SetGradientMode(5);
				//MessageBoxW(hWnd, g_Btn->GetText(), L"测试", 0);
				//MessageBoxW(hWnd, L"按钮单击", L"测试", 0);
				break;
			}
		}
		return 0;
		ECK_COMMAND_END()
	}
	break;

	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC hDC = BeginPaint(hWnd, &ps);
		//FillRect(ps.hdc, &ps.rcPaint, (HBRUSH)GetStockObject(BLACK_BRUSH));
		EndPaint(hWnd, &ps);
	}
	return 0;

	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;

	case WM_USER:
	{
		if (wParam == 104)
		{
			g_Label->SetClr(1, (COLORREF)lParam);
		}
	}
	return 0;

	case WM_USER + 1:
	{
		if (wParam == eck::TGLNM_CLICK)
		{
			auto pi = (eck::TGLCLICKINFO*)lParam;
			std::wstring s =
				L"项目索引："s + std::to_wstring(pi->idxItem) +
				L"，部件ID：" + std::to_wstring(pi->uPart) +
				L"，子任务索引：" + std::to_wstring(pi->idxSubTask);
			MessageBoxW(hWnd, s.c_str(), L"", 0);
		}
	}
	return 0;

	//case WM_NCCALCSIZE:
	//{
	//	MARGINS margins{ 3,3,40,3 };
	//	return eck::OnNcCalcSize(wParam, lParam, &margins);
	//}

	//case WM_NCPAINT:
	//{
	//	RECT rc;
	//	GetWindowRect(hWnd, &rc);
	//	eck::ScreenToClient(hWnd, &rc);
	//	HDC hDC = GetWindowDC(hWnd);
	//	FillRect(hDC, &rc, GetStockBrush(BLACK_BRUSH));
	//	LRESULT lResult = DefWindowProcW(hWnd, uMsg, wParam, lParam);
	//	DrawIconEx(hDC, 600, 0, hIcon, 40, 40, 0, 0, DI_NORMAL);
	//	ReleaseDC(hWnd, hDC);
	//}
	//return 0;

	case WM_CREATE:
	{
		hIcon = eck::CreateHICON(LR"(E:\Desktop\图标9.ico)");
		g_iDpi = eck::GetDpi(hWnd);
		g_hFont = eck::EzFont(L"微软雅黑", 9);

		//eck::CWnd w;
		//w.Attach(hWnd);
		//w.FrameChanged();

		//MARGINS Margins{ 0,0,eck::DpiScale(60,g_iDpi),0 };
		////DwmExtendFrameIntoClientArea(hWnd, &Margins);
		//eck::EnableWindowMica(hWnd);

		//BOOL b = TRUE;
		//DwmSetWindowAttribute(hWnd, DWMWA_ALLOW_NCPAINT, &b, sizeof(b));

		//eck::SetFontForWndAndCtrl(hWnd, g_hFont);
		//return 0;

		g_Btn = new eck::CPushButton;
		g_Btn->Create(L"测试按钮", WS_VISIBLE | BS_PUSHBUTTON, 0, 20, 30, 300, 180, hWnd, 101);

		g_Edit = new eck::CEdit;
		g_Edit->Create(L"示例编辑框", WS_VISIBLE, WS_EX_CLIENTEDGE, 400, 20, 400, 60, hWnd, 102);
		//g_Edit->SetClr(2, 0xFF);
		//g_Edit->SetClr(1, 0xFF);

		g_Label = new eck::CLabel;
		g_Label->Create(L"标签", WS_VISIBLE | WS_BORDER, 0, 20, 230, 400, 200, hWnd, 103);
		g_Label->SetClr(2, CLR_DEFAULT);
		auto rb = eck::ReadInFile(LR"(E:\Desktop\Temp\Zombatar_1.jpg)");
		HBITMAP hBitmap = eck::CreateHBITMAP(rb, rb.m_cb);
		g_Label->SetPic(hBitmap);
		//g_Label->SetTransparent(TRUE);

		g_CC = new eck::CColorPicker;
		g_CC->SetNotifyMsg(WM_USER);
		g_CC->Create(NULL, WS_VISIBLE, 0, 20, 450, 160, 400, hWnd, 104);

		g_LBExt = new eck::CListBoxExt;
		g_LBExt->Create(NULL, WS_VISIBLE, WS_EX_CLIENTEDGE, 500, 80, 300, 400, hWnd, 105);
		g_LBExt->SetToolTip(TRUE);
		g_LBExt->SetCheckBoxMode(1);
		g_LBExt->SetItemHeight(0, 30);
		eck::LBITEMCOMMINFO CommInfo{};
		g_LBExt->SetRedraw(FALSE);
		for (int i = 0; i < 40; ++i)
		{
			if (i == 5)
			{
				CommInfo.bDisabled = TRUE;
			}
			else
			{
				CommInfo.bDisabled = FALSE;
			}
			g_LBExt->AddString(
				(L"测试测试"s + std::to_wstring(i)).c_str(),
				(L"我是提示"s + std::to_wstring(i)).c_str(),
				CommInfo);
		}
		g_LBExt->SetRedraw(TRUE);

		g_DirBox = new eck::CDirBox;
		g_DirBox->SetDir(L"E:");
		g_DirBox->SetFileShowing(TRUE);
		g_DirBox->Create(NULL, WS_VISIBLE | TVS_HASBUTTONS | TVS_FULLROWSELECT | TVS_LINESATROOT | TVS_TRACKSELECT,
			WS_EX_CLIENTEDGE, 820, 80, 400, 400, hWnd, 106);

		g_DirBox->SetExplorerTheme();
		g_DirBox->SetTVExtStyle(TVS_EX_DOUBLEBUFFER);


		eck::CTaskDialog td;
		TASKDIALOGCONFIG tdc;
		tdc.dwFlags = TDF_ALLOW_DIALOG_CANCELLATION | TDF_USE_COMMAND_LINKS | TDF_SHOW_PROGRESS_BAR | TDF_CALLBACK_TIMER;
		tdc.pszMainInstruction = L"测试测试";
		tdc.pszContent = L"内容测试";
		tdc.hMainIcon = (HICON)TD_INFORMATION_ICON;
		td.m_aBtn.push_back({ 101,L"按钮1" });
		td.m_aBtn.push_back({ 102,L"按钮2" });
		td.m_aBtn.push_back({ 103,L"按钮3" });
		td.m_aBtn.push_back({ 104,L"按钮4" });
		td.m_aRadioBtn.push_back({ 201,L"单选框1" });
		td.m_aRadioBtn.push_back({ 202,L"单选框2" });
		tdc.lpCallbackData = (LONG_PTR)&td;
		tdc.pfCallback = [](HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, LONG_PTR lRefData)->HRESULT
		{
			auto p = (eck::CTaskDialog*)lRefData;
			switch (uMsg)
			{
			case TDN_CREATED:
				p->SetPBRange(0, 100);
				break;
			case TDN_RADIO_BUTTON_CLICKED:
				if (wParam == 201)
				{
					p->SetPBState(PBST_ERROR);
					p->SetShieldIcon(TRUE, 102);
				}
				else
				{
					p->SetPBState(PBST_NORMAL);
					p->SetShieldIcon(FALSE, 102);
				}
				break;

			case TDN_TIMER:
				p->SetPBPos((int)wParam / 200);
				if (wParam / 200 > 100)
					return S_FALSE;
				return S_OK;
			}
			return S_OK;
		};
		td.Show(&tdc);

		g_SBNcH = new eck::CScrollBarWndH;
		g_SBNcH->Attach(hWnd);
		g_SBNcH->ShowScrollBar(FALSE);

		g_SB = new eck::CScrollBar;
		g_SB->Create(NULL, WS_VISIBLE, 0, 20, 560, 400, 36, hWnd, 107);

		g_LV = new eck::CListView;
		g_LV->Create(NULL, WS_VISIBLE|LVS_REPORT, WS_EX_CLIENTEDGE, 20, 600, 600, 400, hWnd, 108);
		g_LV->SetExplorerTheme();
		g_LV->SetLVExtendStyle(LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER);

		g_LV->InsertColumn(L"第一列");

		EckCounter(20, i)
		{
			g_LV->InsertItem((L"测试测试测试"s + std::to_wstring(i)).c_str(), i);
		}


		HIMAGELIST hIL = ImageList_Create(48, 48, ILC_COLOR32, 10, 10);
		HBITMAP hbm = eck::CreateHBITMAP(LR"(E:\Desktop\图标9.ico)");
		ImageList_Add(hIL, hbm, NULL);

		g_TGL = new eck::CTaskGroupList;
		g_TGL->Create(NULL, WS_VISIBLE, WS_EX_CLIENTEDGE, 700, 480, 700, 500, hWnd, 109);
		g_TGL->SetTaskImageList(hIL);
		g_TGL->SetNotifyMsg(WM_USER + 1);
		eck::TGLSUBTASK SubTask[3]{};
		std::wstring s[3];
		EckCounter(10, i)
		{
			s[0] = (L"子任务"s + std::to_wstring(i) + L"___1"s);
			s[1] = (L"子任务"s + std::to_wstring(i) + L"___2"s);
			s[2] = (L"子任务"s + std::to_wstring(i) + L"___3"s);
			SubTask[0].pszText = s[0].c_str();
			SubTask[1].pszText = s[1].c_str();
			SubTask[2].pszText = s[2].c_str();
			g_TGL->InsertTask((L"测试节标题"s + std::to_wstring(i)).c_str(), -1, 0, SubTask, ARRAYSIZE(SubTask));
		}
		g_TGL->RecalcColumnIdealWidth();
		eck::SetFontForWndAndCtrl(hWnd, g_hFont);
	}
	return 0;

	case WM_DPICHANGED:
	{
		int iDpi = LOWORD(wParam);
		eck::OnDpiChanged_Parent_PreMonV2(hWnd, iDpi, g_iDpi);
		g_iDpi = iDpi;
	}
	break;
	}

	return DefWindowProcW(hWnd, uMsg, wParam, lParam);
}

INT_PTR CALLBACK DlgProc_About(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_INITDIALOG:
	{
		EckDbgPrint(L"54644");
	}
	return TRUE;

	case WM_COMMAND:
	{
	}
	break;

	case WM_CLOSE:
		EndDialog(hDlg, 0);
		PostQuitMessage(0);
		return TRUE;

	case WM_DESTROY:
		
		return TRUE;
	}

	return FALSE;
}

#undef ___HEADER_PRIVATE_MAIN___