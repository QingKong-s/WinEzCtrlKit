#define ___HEADER_PRIVATE_MAIN___
#include "Win32Template.h"
#include "Resource.h"

#include <dwmapi.h>

#include <format>

#include "eck/Env.h"
#include "CTestWnd.h"


using namespace std::literals;

INT_PTR CALLBACK DlgProc_1(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	LPNMHDR     lpnmhdr;
	static eck::CCommandLink cl1, cl2;
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
	CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
	//CoInitialize(NULL);
	if (eck::Init(hInstance) != eck::InitStatus::Ok)
	{
		MessageBoxW(NULL, L"初始化失败", L"", 0);
		return 1;
	}

	using namespace eck;
	using namespace eck::Literals;

	CRefStrW rsi(L"123456789");
	rsi.Insert(2, L"插入一段文本");
	EckDbgPrint(rsi);
	rsi.Erase(4, 5);
	EckDbgPrint(rsi);
	//EckDbgBreak();

	auto date = CeToLunar({ 1902,6,20 });
	EckDbgPrint(std::format(L"{},{},{},{}", date.wYear, date.byMonth, date.byDay, date.bLeapMonth).c_str());
	date = CeToLunar({ 1902,1,1 });
	EckDbgPrint(std::format(L"{},{},{},{}", date.wYear, date.byMonth, date.byDay, date.bLeapMonth).c_str());
	date = CeToLunar({ 1902,1,6 });
	EckDbgPrint(std::format(L"{},{},{},{}", date.wYear, date.byMonth, date.byDay, date.bLeapMonth).c_str());
	date = CeToLunar({ 1902,1,11 });
	EckDbgPrint(std::format(L"{},{},{},{}", date.wYear, date.byMonth, date.byDay, date.bLeapMonth).c_str());
	date = CeToLunar({ 1902,2,3 });
	EckDbgPrint(std::format(L"{},{},{},{}", date.wYear, date.byMonth, date.byDay, date.bLeapMonth).c_str());
	EckDbgPrint(NumToNaYin(GetNaYin(L"甲子")));
	EckDbgPrint(NumToNaYin(GetNaYin(6)));
	EckDbgPrint(NumToJieQi(GetJieQi({ 1902,5,22 })));
	EckDbgPrint(NumToShuXiang(GetShuXiang(2023)));


	//EckDbgPrint(rb0.At<int>(7));
	//EckAssert(rb0.At<int>(7) == 0);
	auto bbbb = GetClipboardString();
	EckDbgPrint(bbbb.Data());
	//SetClipboardString(L"测试测试啊啊啊啊啊啊啊啊啊啊");
	HDC hDC = GetDC(GetDesktopWindow());
	//HBITMAP hbm = (HBITMAP)GetCurrentObject(hDC, OBJ_BITMAP);
	//auto tt = GetBmpData(hbm, hDC);

	CDib dib{};
	dib.Create(hDC);
	auto tt = dib.GetBmpData();
	//WriteToFile(LR"(E:\Desktop\1234567.bmp)", tt);

	auto pbmi = (BITMAPINFO*)malloc(sizeof(BITMAPINFOHEADER) + 246 * sizeof(RGBQUAD));
	ZeroMemory(pbmi, sizeof(BITMAPINFOHEADER));

	pbmi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	pbmi->bmiHeader.biWidth = (LONG)800;
	pbmi->bmiHeader.biHeight = -(LONG)600;
	pbmi->bmiHeader.biPlanes = 1;
	pbmi->bmiHeader.biBitCount = 8;
	pbmi->bmiHeader.biClrUsed = 246;
	auto pclr = pbmi->bmiColors;
	int r, g, b,i,inc;
	for (i = 0, g = 0, inc = 8; g <= 0xFF; ++i, g += inc)
	{
		pclr[i] = { (BYTE)g,(BYTE)g,(BYTE)g,0 };
		inc = (inc == 9 ? 8 : 9);
	}

	for (r = 0; r <= 0xFF; r += 0x33)
		for (g = 0; g <= 0xFF; g += 0x33)
			for (b = 0; b <= 0xFF; b += 0x33)
			{
				pclr[i] = { (BYTE)r,(BYTE)g,(BYTE)b,0 };
				++i;
			}

	auto hbm = CreateDIBSection(NULL, pbmi, DIB_RGB_COLORS, NULL, NULL, 0);
	hDC = CreateCompatibleDC(0);
	auto plp = (LOGPALETTE*)malloc(sizeof(LOGPALETTE) + 246 * sizeof(PALETTEENTRY));
	plp->palVersion = 0x300;
	plp->palNumEntries = 246;
	memcpy(plp->palPalEntry, pclr, 246 * sizeof(PALETTEENTRY));
	auto hpal = CreatePalette(plp);

	SelectObject(hDC, hbm);
	SelectPalette(hDC, hpal, FALSE);
	RealizePalette(hDC);

	EckCounter(246, i)
	{
		auto hbr = CreateSolidBrush(PALETTEINDEX(i));
		RECT rc{ i * 800 / 246,0,(i + 1) * 800 / 246,600 };
		FillRect(hDC, &rc, hbr);

	}

	CDib dib1;
	dib1.Create(hDC, 0, hpal);
	tt = dib1.GetBmpData();
	//WriteToFile(LR"(E:\Desktop\palette_test.bmp)", tt);

	//EckDbgBreak();
	//EckDbgBreak();
	//CResSet<int> ress;


	//BYTE by[20];
	//CMemWriter w(by);
	//w.SetValidRange(by, 10);
	//w << 1 << 2;
	//w << 3;

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


	//CArray<int> arr(4, 4);
	//arr[1][2] = 1;
	////int a = arr[1][2];
	//arr[1][2] = 0xFFFFFFFF;
	//arr.ReDim(3, 3, 3);
	//auto pp = arr[2][2][2].AddressOf();
	//arr[2][2][2] = 0xCCCCCCCC;

	//arr.ReDim(6, 6);
	//arr[0][0] = 123;
	//arr[2][2] = 456;

	//for (auto& x : arr)
	//{
	//	x = 1;
	//}

	//CArray<std::wstring> srr(4, 4);
	//EckCounter(4, i)
	//{
	//	EckCounter(4, j)
	//	{
	//		srr[i][j] = std::format(L"{},{}", i, j);
	//	}
	//}

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

	DLGTDLG Dlg{ 1,0xFFFF,0,0,WS_OVERLAPPEDWINDOW | WS_VISIBLE | DS_SETFONT ,3,0,0,240,100 };
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
	psp[1].dwFlags = PSP_DLGINDIRECT | PSP_USEHEADERTITLE | PSP_USEHEADERSUBTITLE;
	psp[1].hInstance = hInstance;
	psp[1].pResource = (DLGTEMPLATE*)rb.Data();
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


	//WNDCLASSEXW wcex{ sizeof(WNDCLASSEX) };
	//wcex.style = CS_HREDRAW | CS_VREDRAW;
	//wcex.lpfnWndProc = WndProc_Main;
	//wcex.hInstance = hInstance;
	//wcex.hIcon = LoadIconW(NULL, IDI_APPLICATION);
	//wcex.hCursor = LoadCursorW(NULL, IDC_ARROW);
	//wcex.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
	//wcex.lpszClassName = WCN_MAIN;
	//if (!RegisterClassExW(&wcex))
	//	return 1;

	//HWND hWnd = CreateWindowExW(0, WCN_MAIN, L"示例Win32程序", WS_OVERLAPPEDWINDOW,
	//	CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);
	//if (!hWnd)
	//	return 1;
	//ShowWindow(hWnd, nCmdShow);
	//UpdateWindow(hWnd);
	CTestWnd::RegisterWndClass();
	CTestWnd w;
	w.Create(L"示例Win32程序", WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN, 0, CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, 0);
	w.Show(SW_SHOW);

	while (GetMessageW(&msg, NULL, 0, 0))
	{
		if (!eck::PreTranslateMessage(msg))
		{
			TranslateMessage(&msg);
			DispatchMessageW(&msg);
		}
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
eck::CTabHeader* g_TH;
eck::CSplitBar* g_SPB;
eck::CSplitBar* g_SPBH;
eck::CDrawPanel* g_DP;
eck::CDrawPanelD2D* g_DPD2D;
eck::CListBoxNew* g_LBN;
eck::CMenu g_Menu
{
	{L"我是第一项",100},
	{L"我是第β项",101,MF_CHECKED},
	{L"我是第3项",102},
	{L"我是第D项",103,MF_GRAYED},
	{L"我是第戊项",104},
	{L"我是第サ项",105},
};


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
		switch (LOWORD(wParam))
		{
		case BN_CLICKED:
		{
			switch (HIWORD(wParam))
			{
			case 101:
				g_Label->SetGradientMode(5);
				//MessageBoxW(hWnd, g_Btn->GetText(), L"测试", 0);
				//MessageBoxW(hWnd, L"按钮单击", L"测试", 0);
				break;
			}
		}
		return 0;
		}
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
	case WM_LBUTTONDOWN:
	{
		POINT pt;
		GetCursorPos(&pt);
		g_Menu.TrackPopupMenu(hWnd, pt.x, pt.y);
	}
	break;
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
		//eck::CRefBin rbMenuData;
		//g_Menu.SerializeData(rbMenuData);
		//eck::WriteToFile(LR"(E:\Desktop\MenuData.bin)", rbMenuData);
		//EckDbgBreak();
		//g_Menu.CreatePopup();
		//rbMenuData = eck::ReadInFile(LR"(E:\Desktop\MenuData.bin)");
		//g_Menu.AppendItems(rbMenuData.Data(), rbMenuData.Size());
		
		//hIcon = eck::CreateHICON(LR"(E:\Desktop\图标9.ico)");

		//hIcon=eck::CreateHICON()
		eck::CComboBox cb;
		cb.DropDown = TRUE;
		g_iDpi = eck::GetDpi(hWnd);
		g_hFont = eck::EzFont(L"微软雅黑", 9);

		eck::CListBoxNew::FLbnProc pfnn = [](HWND hWnd, UINT uCode, LPARAM lParam, LPARAM lRefData)->LRESULT
			{
				static std::vector<eck::CRefStrW> v{};
				switch (uCode)
				{
				case eck::CListBoxNew::NCode::GetDispInfo:
				{
					if (!v.size())
					{
						v.resize(100);
						EckCounter(100, i)
						{
							v[i] = eck::ToStr(i) + L"测试测试";
						}
					}
					auto p = (eck::LBNEWITEM*)lParam;
					p->pszText = v[p->idxItem].Data();
					p->cchText = v[p->idxItem].Size();
				}
				return 0;
				}

				return 0;
			};

		//g_LBN = new eck::CListBoxNew;
		//g_LBN->SetProc(pfnn);
		//g_LBN->Create(NULL, WS_CHILD | WS_VISIBLE | WS_BORDER | WS_VSCROLL, 0, 100, 100, 600, 800, hWnd, 101);
		//g_LBN->SetItemCount(100);

		EckDbgPrint(g_Menu.GetItemString(101).Data());
		//EckDbgBreak();
		//EckDbgBreak();

		g_DP = new eck::CDrawPanel;
		g_DP->Create(0, WS_CHILD | WS_VISIBLE | WS_BORDER, 0, 10, 10, 900, 900, hWnd, 10001);
		HDC hDC = g_DP->GetHDC();
		RECT rcDP{ 0,0,900,900 };
		FillRect(hDC, &rcDP, (HBRUSH)GetStockObject(WHITE_BRUSH));

		//auto rbdib = eck::ReadInFile(LR"(E:\Desktop\1.bmp)");
		//eck::CDib dib;
		//auto hbm = dib.Create(rbdib.Data());
		//HDC hcdc = CreateCompatibleDC(hDC);
		//SelectObject(hcdc, hbm);
		//BitBlt(hDC, 0, 0, 900, 900, hcdc, 0, 0, SRCCOPY);
		g_DP->Redraw();
		//eck::CIcoFileReader ico;
		//auto rbIco = eck::ReadInFile(LR"(E:\Desktop\Tempo.ico)");
		//ico.AnalyzeData(rbIco.Data());
		//EckCounter(ico.GetIconCount(), i)
		//{
		//	EckDbgPrintFmt(L"cx = %d, cy = %d", (int)ico.At(i)->bWidth, (int)ico.At(i)->bHeight);
		//	DrawIconEx(hDC, 10, 10 + 50 * i, ico.CreateHICON(i), 0, 0, 0, NULL, DI_NORMAL);
		//}
		//EckDbgPrintFmt(L"cx = %d, cy = %d", (int)ico.At(4)->bWidth, (int)ico.At(4)->bHeight);

		//RECT rcEllipse{ 10,20,600,300 };
		//Ellipse(hDC, rcEllipse.left, rcEllipse.top, rcEllipse.right, rcEllipse.bottom);
		//POINT pt;
		//eck::PtFromEllipseAngle((rcEllipse.left + rcEllipse.right) / 2, (rcEllipse.top + rcEllipse.bottom) / 2,
		//	(rcEllipse.right - rcEllipse.left) / 2, (rcEllipse.bottom - rcEllipse.top) / 2,
		//	eck::Deg2Rad(30.f), (int&)pt.x, (int&)pt.y);
		//
		//HPEN hPen = CreatePen(0, 1, eck::Colorref::Red);
		//SelectObject(hDC, hPen);
		//MoveToEx(hDC, rcEllipse.left + (rcEllipse.right - rcEllipse.left) / 2,
		//	rcEllipse.top + (rcEllipse.bottom - rcEllipse.top) / 2, 0);
		//LineTo(hDC, pt.x, pt.y);
		

		//eck::DrawSpirograph(hDC, 260, 260, 260, 100, 30, 0.1);
		//eck::DrawButterflyCurve(hDC, 400, 400,4.f,100,100,0.01);

		//Ellipse(hDC, 100, 100, 600, 400);

		//eck::DrawArc(hDC, 100, 100, 600, 400, eck::Deg2Rad(30), eck::Deg2Rad(15));

		//eck::DrawRoseCurve(hDC, 400, 400);
		//eck::DrawRegularStar(hDC, 450, 450, 300, 10, eck::Deg2Rad(90.f), FALSE);
		//Rectangle(hDC, 600, 600, 880, 880);
		//float a, b, c;
		//eck::CalcLineEquation(900, 360, 200, 900, a, b, c);
		//XFORM xform = eck::XFORMReflection(a, b, c);
		//eck::XFORMTranslate(100,100, xform);
		//eck::XFORMRotate(eck::Deg2Rad(30), 100, 100, xform);
		//eck::XFORMScale(0.5, 0.5,100,100, xform);
		//eck::XFORMShear(tanf(eck::Deg2Rad(20)), 0,100,100, xform);
		//SetGraphicsMode(hDC, GM_ADVANCED);
		//auto bb = SetWorldTransform(hDC, &xform);
		//EckAssert(bb);
		//Rectangle(hDC, 600, 600, 880, 880);

		//ModifyWorldTransform(hDC, NULL, MWT_IDENTITY);
		//GpGraphics* pGraphics;
		//GdipCreateFromHDC(hDC, &pGraphics);
		//GdipSetSmoothingMode(pGraphics, SmoothingModeHighQuality);

		//POINT pt[]
		//{
		//	{100,100},
		//	{120,130},
		//	{210,230},
		//	{250,260},
		//	{340,200},
		//	{360,210},
		//	{400,350},
		//	{410,400},
		//	{480,410},
		//	{500,600}
		//};
		//constexpr int ioff = 4;
		//std::vector<POINT> vPt{};

		//eck::CalcBezierControlPoints(vPt, pt, ARRAYSIZE(pt));

		//SelectObject(hDC, CreatePen(0, 2, eck::Colorref::Green));
		//int j = 0;
		//EckCounter(vPt.size(), i)
		//{
		//	if (i == 0)
		//		continue;
		//	auto& pt0 = vPt[i];
		//	++j;
		//	if (j % 3 == 0)
		//	{
		//		MoveToEx(hDC, vPt[i - 1].x, vPt[i - 1].y, 0);
		//		LineTo(hDC, vPt[i - 2].x, vPt[i - 2].y);
		//		j = 0;
		//	}

		//	Ellipse(hDC, pt0.x - ioff, pt0.y - ioff, pt0.x + ioff, pt0.y + ioff);
		//}

		//SelectObject(hDC, CreatePen(0, 2, eck::Colorref::Black));
		////PolyBezier(hDC, vPt.data(), vPt.size());

		//SelectObject(hDC, CreatePen(0, 2, eck::Colorref::Red));
		//for (auto pt0 : pt)
		//{
		//	//Ellipse(hDC, pt0.x - ioff, pt0.y - ioff, pt0.x + ioff, pt0.y + ioff);
		//}

		//GpPen* pPen;
		//GdipCreatePen1(eck::ColorrefToARGB(eck::Colorref::Azure), 2.f, GpUnit::UnitPixel, &pPen);
		////eck::DrawSpirograph(pGraphics, pPen, 600, 600, 260, 100, 30, 0.1);
		////eck::DrawButterflyCurve(pGraphics, pPen, 400, 400, 4.f, 100, 100, 0.005);
		////eck::DrawRegularStar(g_DP->GetGraphics(), pPen, 450, 450, 300, 10);
		////eck::DrawRoseCurve(g_DP->GetGraphics(), pPen, 450, 450, 300.f, 10);

		//PCWSTR pszTest = LR"(F:\阿里云盘下载\图片\D5D0A9FFEEBE4385BC7BD3E9AB90F21B.jpg)";

		//IWICBitmapDecoder* pDecoder;
		//eck::CreateWicBitmapDecoder(pszTest, pDecoder, eck::g_pWicFactory);

		//std::vector<IWICBitmap*> vWicBmp{};
		//eck::CreateWicBitmap(vWicBmp, pDecoder, eck::g_pWicFactory);

		//HBITMAP hbmOrg = eck::CreateHBITMAP(vWicBmp[0]);
		//UINT cx, cy;
		//vWicBmp[0]->GetSize(&cx, &cy);
		//POINT ptS[]
		//{
		//	{0,0},
		//	{cx,0},
		//	{cx,cy},
		//	{0,cy},
		//};

		//POINT ptD[]
		//{
		//	{0,0},
		//	{cx+100,70},
		//	{cx,cy - 100},
		//	{10,cy+20},
		//	
		//};

		//int aaaa = 10;

		//using T = int*;
		//std::add_pointer_t<std::add_const_t<std::remove_pointer_t<T>>> a1 = &aaaa;
		//a1 = NULL;
		//

		//eck::CMifptpHBITMAP Bmp(hbmOrg);
		//eck::CMifptpHBITMAP NewBmp{};

		//eck::MakeImageFromPolygonToPolygon(Bmp, NewBmp, ptS, ptD, ARRAYSIZE(ptD));
		//HDC hCDC = CreateCompatibleDC(hDC);
		//SelectObject(hCDC, NewBmp.GetHBITMAP());
		//
		////BitBlt(hDC, 0, 0, NewBmp.GetWidth(), NewBmp.GetHeight(), hCDC, 0, 0, SRCCOPY);
		//SelectObject(hDC, GetStockObject(NULL_BRUSH));
		////Polygon(hDC, ptD, 4);

		//GpBitmap* pbmp;
		//GdipCreateBitmapFromFile(pszTest, &pbmp);
		//eck::CMifptpGpBitmap BmpGp(pbmp);
		//eck::CMifptpGpBitmap NewBmpGp{};
		//GpPoint ptS1[]
		//{
		//	{0,0},
		//	{cx,0},
		//	{cx,cy},
		//	{0,cy},
		//};

		//GpPoint ptD1[]
		//{
		//	{0,0},
		//	{cx + 100,70},
		//	{cx,cy - 100},
		//	{50,cy + 20},

		//};
		//auto time = GetTickCount64();
		//eck::MakeImageFromPolygonToPolygon(BmpGp, NewBmpGp, ptS1, ptD1, ARRAYSIZE(ptD1));
		//time = GetTickCount64() - time;
		////MessageBoxW(0, std::format(L"{}", time).c_str(), 0, 0);
		//GdipDrawImage(g_DP->GetGraphics(), NewBmpGp.GetGpBitmap(), 0, 0);


		//g_DP->Redraw();



		//g_DPD2D = new eck::CDrawPanelD2D;
		//g_DPD2D->Create(0, WS_CHILD | WS_VISIBLE | WS_BORDER, 0, 910, 10, 900, 900, hWnd, 10002);

		//auto pDC = g_DPD2D->GetDC();
		//ID2D1SolidColorBrush* pBrush;
		//pDC->CreateSolidColorBrush(D2D1::ColorF(0x66CCFF), &pBrush);
		//EckAssert(pBrush);
		//pDC->BeginDraw();
		//pDC->Clear(D2D1::ColorF(D2D1::ColorF::White));

		////eck::DrawSpirograph({ eck::g_pD2dFactory,pDC,pBrush,3.f,NULL,450,450,260,100,50 });
		////eck::DrawButterflyCurve({ eck::g_pD2dFactory,pDC,pBrush,3.f,NULL,200,200,4.f,50.f,50.f });
		////eck::DrawRoseCurve({ eck::g_pD2dFactory,pDC,pBrush,3.f,NULL,450,450 });

		//ID2D1Bitmap1* pBitmap;
		//pDC->CreateBitmapFromWicBitmap(vWicBmp[0], &pBitmap);
		//EckAssert(pBitmap);

		////D2D1_MATRIX_4X4_F M{};
		//D2D1_RECT_F rcF{ 300,300,700,700 };
		////auto Size = pBitmap->GetSize();
		////D2D1_RECT_F rcFSrc{ 0,0,Size.width,Size.height };
		////eck::CalcDistortMatrix(rcF, { {280,280},{700,310},{300,780},{750,700} }, M);
		////pDC->DrawBitmap(pBitmap, rcF, 1.f, D2D1_INTERPOLATION_MODE_CUBIC, rcFSrc, M);
		////rcF = { 600,600,880,880 };
		////pDC->DrawRectangle(rcF, pBrush, 3.f);

		////eck::CalcLineEquation(900, 360,200,900, a, b, c);
		////pDC->DrawLine({ 900, 360 }, { 200,900 }, pBrush);
		////pDC->SetTransform(eck::D2dMatrixReflection(a, b, c));
		////pDC->DrawRectangle(rcF, pBrush, 3.f);
		////pDC->SetTransform(D2D1::Matrix3x2F::Identity());
		////eck::DrawRegularStar({eck::g_pD2dFactory,pDC,pBrush,5.f,NULL,450,450,300,6});
		//
		////pDC->DrawEllipse({ 450,450,300,300 }, pBrush, 8.f);
		//pDC->EndDraw();
		//g_DPD2D->GetSwapChain()->Present(0, 0);

		//auto rbft = eck::ReadInFile(LR"(E:\Desktop\1.eckft)");
		//eck::CFormTable ft;
		//ft.LoadTable(rbft);// 解析窗体表
		//eck::LoadForm(hWnd, ft.GetFormBin(101));// 载入
		//eck::SetFontForWndAndCtrl(hWnd, g_hFont);
		//return 0;
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

		//g_Btn = new eck::CPushButton;
		//g_Btn->Create(L"测试按钮", WS_VISIBLE | BS_PUSHBUTTON, 0, 20, 30, 300, 180, hWnd, 101);

		//g_Edit = new eck::CEdit;
		//g_Edit->Create(L"示例编辑框", WS_VISIBLE, WS_EX_CLIENTEDGE, 400, 20, 400, 60, hWnd, 102);
		////g_Edit->SetClr(2, 0xFF);
		////g_Edit->SetClr(1, 0xFF);

		//g_Label = new eck::CLabel;
		//g_Label->Create(L"标签", WS_VISIBLE | WS_BORDER, 0, 20, 230, 400, 200, hWnd, 103);
		//g_Label->SetClr(2, CLR_DEFAULT);
		//auto rb = eck::ReadInFile(LR"(E:\Desktop\Temp\Zombatar_1.jpg)");
		//HBITMAP hBitmap = eck::CreateHBITMAP(rb, rb.m_cb);
		//g_Label->SetPic(hBitmap);
		////g_Label->SetTransparent(TRUE);

		//g_CC = new eck::CColorPicker;
		//g_CC->SetNotifyMsg(WM_USER);
		//g_CC->Create(NULL, WS_VISIBLE, 0, 20, 450, 160, 400, hWnd, 104);

		//g_LBExt = new eck::CListBoxExt;
		//g_LBExt->Create(NULL, WS_VISIBLE, WS_EX_CLIENTEDGE, 500, 80, 300, 400, hWnd, 105);
		//g_LBExt->SetToolTip(TRUE);
		//g_LBExt->SetCheckBoxMode(1);
		//g_LBExt->SetItemHeight(0, 30);
		//eck::LBITEMCOMMINFO CommInfo{};
		//g_LBExt->SetRedraw(FALSE);
		//for (int i = 0; i < 40; ++i)
		//{
		//	if (i == 5)
		//	{
		//		CommInfo.bDisabled = TRUE;
		//	}
		//	else
		//	{
		//		CommInfo.bDisabled = FALSE;
		//	}
		//	g_LBExt->AddString(
		//		(L"测试测试"s + std::to_wstring(i)).c_str(),
		//		(L"我是提示"s + std::to_wstring(i)).c_str(),
		//		CommInfo);
		//}
		//g_LBExt->SetRedraw(TRUE);

		//g_DirBox = new eck::CDirBox;
		//g_DirBox->SetDir(L"E:");
		//g_DirBox->SetFileShowing(TRUE);
		//g_DirBox->Create(NULL, WS_VISIBLE | TVS_HASBUTTONS | TVS_FULLROWSELECT | TVS_LINESATROOT | TVS_TRACKSELECT,
		//	WS_EX_CLIENTEDGE, 820, 80, 400, 400, hWnd, 106);

		//g_DirBox->SetExplorerTheme();
		//g_DirBox->SetTVExtStyle(TVS_EX_DOUBLEBUFFER);


		//eck::CTaskDialog td;
		//TASKDIALOGCONFIG tdc;
		//tdc.dwFlags = TDF_ALLOW_DIALOG_CANCELLATION | TDF_USE_COMMAND_LINKS | TDF_SHOW_PROGRESS_BAR | TDF_CALLBACK_TIMER;
		//tdc.pszMainInstruction = L"测试测试";
		//tdc.pszContent = L"内容测试";
		//tdc.hMainIcon = (HICON)TD_INFORMATION_ICON;
		//td.m_aBtn.push_back({ 101,L"按钮1" });
		//td.m_aBtn.push_back({ 102,L"按钮2" });
		//td.m_aBtn.push_back({ 103,L"按钮3" });
		//td.m_aBtn.push_back({ 104,L"按钮4" });
		//td.m_aRadioBtn.push_back({ 201,L"单选框1" });
		//td.m_aRadioBtn.push_back({ 202,L"单选框2" });
		//tdc.lpCallbackData = (LONG_PTR)&td;
		//tdc.pfCallback = [](HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, LONG_PTR lRefData)->HRESULT
		//{
		//	auto p = (eck::CTaskDialog*)lRefData;
		//	switch (uMsg)
		//	{
		//	case TDN_CREATED:
		//		p->SetPBRange(0, 100);
		//		break;
		//	case TDN_RADIO_BUTTON_CLICKED:
		//		if (wParam == 201)
		//		{
		//			p->SetPBState(PBST_ERROR);
		//			p->SetShieldIcon(TRUE, 102);
		//		}
		//		else
		//		{
		//			p->SetPBState(PBST_NORMAL);
		//			p->SetShieldIcon(FALSE, 102);
		//		}
		//		break;

		//	case TDN_TIMER:
		//		p->SetPBPos((int)wParam / 200);
		//		if (wParam / 200 > 100)
		//			return S_FALSE;
		//		return S_OK;
		//	}
		//	return S_OK;
		//};
		//td.Show(&tdc);

		/*g_SBNcH = new eck::CScrollBarWndH;
		g_SBNcH->Manage(eck::CWnd::ManageOp::Attach, hWnd);
		g_SBNcH->ShowScrollBar(FALSE);

		g_SB = new eck::CScrollBar;
		g_SB->Create(NULL, WS_VISIBLE, 0, 20, 560, 400, 36, hWnd, 107);

		g_LV = new eck::CListView;
		g_LV->Create(NULL, WS_VISIBLE | LVS_REPORT, WS_EX_CLIENTEDGE, 20, 600, 600, 400, hWnd, 108);
		g_LV->SetExplorerTheme();
		g_LV->SetLVExtendStyle(LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER);

		g_LV->InsertColumn(L"第一列");

		EckCounter(20, i)
		{
			g_LV->InsertItem((L"测试测试测试"s + std::to_wstring(i)).c_str(), i);
		}


		HIMAGELIST hIL = ImageList_Create(48, 48, ILC_COLOR32, 10, 10);
		HBITMAP hbm = eck::CreateHBITMAP(LR"(E:\Desktop\Temp\图标9.ico)");
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

		g_TH = new eck::CTabHeader;
		g_TH->Create(NULL, WS_VISIBLE | WS_CHILD, 0, 0, 0, 800, 100, hWnd, 201);
		eck::TABHEADERITEM thi;
		thi.uMask = eck::THIM_TEXT;
		thi.pszText = (PWSTR)L"测试项目";
		EckCounterNV(6)
		{
			g_TH->InsertItem(&thi);
		}

		g_SPB = new eck::CSplitBar;
		g_SPB->Create(NULL, WS_VISIBLE | WS_CHILD, 0, 50, 200, 20, 600, hWnd, 202);

		g_SPBH = new eck::CSplitBar;
		g_SPBH->Create(NULL, WS_VISIBLE | WS_CHILD, 0, 300, 400, 600, 20, hWnd, 202);
		g_SPBH->SetDirection(TRUE);*/
eck::SetFontForWndAndCtrl(hWnd, g_hFont);
	}
	return 0;

	case WM_DPICHANGED:
	{
		int iDpi = LOWORD(wParam);
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