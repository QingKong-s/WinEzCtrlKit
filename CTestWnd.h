#pragma once
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
#include "eck\SystemHelper.h"
#include "eck\CAnimationBox.h"
#include "eck\CForm.h"

#define WCN_TEST L"TesttttttttttttttWndddddddddd"

using eck::PCVOID;

class CTestWnd :public eck::CForm
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
	eck::CAnimationBox m_AB{};

	eck::CFlowLayout m_lot{};

	HFONT m_hFont = eck::EzFont(L"微软雅黑", 9);
	HBITMAP m_hbm = NULL;
public:
	void Test()
	{
		using namespace eck;
		//CHAR szA[]{ "123你好45" };
		//EckDbgPrint(eck::CalcDbcsStringCharCount(szA, ARRAYSIZE(szA) - 1));
		//EckDbgBreak();
		//EckDbgPrint(L"-----------------------");

		//eck::CRegKey key(LR"(HKCU\Software\Test1)");
		//eck::CRegKey key2{};
		//key2.Create(LR"(HKCU\Qk\Software\Test1)");
		//key2.SetValue(L"测试值", 1);
		//EckDbgPrint(L"-----------------------");
		//key2.Open(
		//	LR"(HKLM\SOFTWARE\Microsoft\VisualStudio\Debugger\JIT\{F200A7E7-DEA5-11D0-B854-00A0244A1DE2})",
		//	KEY_READ);
		//EckDbgPrint(key2.GetValueStr(NULL,L"JITSettings"));
		//EckDbgPrint(L"-----------------------");
		//key2.Open(
		//	LR"(HKLM\SOFTWARE\Microsoft\Windows)",
		//	KEY_READ);
		//key2.EnumKey([](eck::CRefStrW& rsName)->BOOL
		//	{
		//		EckDbgPrint(rsName);
		//		return FALSE;
		//	});
		//EckDbgPrint(L"-----------------------");
		//key2.Open(
		//	LR"(HKLM\SOFTWARE\Microsoft\Windows\CurrentVersion)",
		//	KEY_READ);
		//key2.EnumValue([](eck::CRefStrW& rsName, DWORD dwType)->BOOL
		//	{
		//		EckDbgPrint(rsName);
		//		return FALSE;
		//	});

		//CPUINFO ci{};
		//GetCpuInfo(ci);
		////EckDbgPrint(L"-----------------------");
		////EckDbgPrint(ci.rsVendor);
		////EckDbgPrint(ci.rsBrand);
		////EckDbgPrint(ci.rsSerialNum);
		////EckDbgPrint(ci.rsDescription);
		//////EckDbgPrint(ci.rsVendor);
		////EckDbgPrint(L"-----------------------");
		//VARIANT var0{};
		//WmiQueryClassProp(L"SELECT Description FROM Win32_Processor", L"Description", var0);
		////EckDbgPrint( var0.bstrVal+2);

		//FILEVERINFO fvi{};
		//GetFileVerInfo(LR"(C:\Program Files\bilibili\哔哩哔哩.exe)", fvi);

		//eck::CRefBin rb;
		//KeyboardEvent(VK_CONTROL, 'A');

		//CRefBin rbOlePic = ReadInFile(LR"(E:\Desktop\Temp\111111.bmp)");
		//IStream* pStream = new CStreamView(rbOlePic);
		//IPicture* pPic;
		//auto hr=OleLoadPicture(pStream, rbOlePic.Size(), TRUE, IID_PPV_ARGS(&pPic));
		//EckDbgPrintFormatMessage(hr);

		//CRefStrW rs(L"测试测试123你好");
		//rs.AppendFormat(L"整数 = %d，字符串 = %s。", 100, L"我是字符串");
		//rs.Format(L"浮点 = %f", 120.3f);

		CRefStrA rsa("测试字符串");
		//auto bstr1 = rsa.ToBSTR();
		//auto bstr2 = rs.ToBSTR();
		EckDbgPrintWithPos(L"测试");
		EckDbgBreak();
	}

	BOOL PreTranslateMessage(const MSG& Msg) override
	{
		return CForm::PreTranslateMessage(Msg);
	}

	LRESULT OnMsg(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) override
	{
		switch (uMsg)
		{
		case WM_CREATE:
		{
			m_Btn.Create(L"按钮测试", WS_CHILD | WS_VISIBLE, 0, 0, 0, 300, 70, hWnd, 101);
			m_lot.Add(&m_Btn, eck::FLF_FIXWIDTH | eck::FLF_FIXHEIGHT);

			m_Edit.Create(L"编辑框", WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL, 0,
				0, 0, 200, 100, hWnd, 102);
			m_Edit.SetFrameType(1);
			m_lot.Add(&m_Edit, eck::FLF_FIXWIDTH | eck::FLF_FIXHEIGHT);

			m_Label.Create(L"我是标签", WS_CHILD | WS_VISIBLE | WS_BORDER, 0, 0, 0, 300, 200, hWnd, 103);
			m_hbm = eck::CreateHBITMAP(LR"(E:\Desktop\Temp\111111.jpg)");
			m_Label.SetPic(m_hbm);
			m_lot.Add(&m_Label);

			m_AB.Create(NULL, WS_CHILD | WS_VISIBLE | WS_BORDER, 0, 0, 0, 1100, 700, hWnd, 104);
			m_lot.Add(&m_AB);
			auto pDC = m_AB.GetDC();

			IWICBitmapDecoder* pDecoder;
			eck::CreateWicBitmapDecoder(LR"(F:\Test\1.png)", pDecoder, eck::g_pWicFactory);
			IWICBitmap* pWicBmp;
			eck::CreateWicBitmap(pWicBmp, pDecoder, eck::g_pWicFactory);
			ID2D1Bitmap1* pBitmap;
			pDC->CreateBitmapFromWicBitmap(pWicBmp, &pBitmap);

			auto pSpirit = new eck::CAbSpiritImage(pBitmap);
			pSpirit->SetPos({ 200,400 });

			//m_AB.AddSpirit(pSpirit);
			//pSpirit->AutoMarch({ 10.f,20,600.f,0,FALSE });


			eck::CreateWicBitmapDecoder(LR"(F:\Test\2.jpg)", pDecoder, eck::g_pWicFactory);
			eck::CreateWicBitmap(pWicBmp, pDecoder, eck::g_pWicFactory);
			pDC->CreateBitmapFromWicBitmap(pWicBmp, &pBitmap);

			pSpirit = new eck::CAbSpiritImage(pBitmap);
			pSpirit->SetPos({ 400,200 });

			//m_AB.AddSpirit(pSpirit);
			//pSpirit->Turn(-eck::Deg2Rad(90.f));
			//pSpirit->AutoMarch({ 10.f,20,600.f,0,FALSE });

			eck::SetFontForWndAndCtrl(hWnd, m_hFont);

			Test();
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
			{
				BkColor = eck::Colorref::DeepGray;
				EscClose = TRUE;
				TotalMove = TRUE;
				Redraw();
			}
		}
		break;
		}
		return CForm::OnMsg(hWnd, uMsg, wParam, lParam);
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

	ECK_CWND_CREATE;
	HWND Create(PCWSTR pszText, DWORD dwStyle, DWORD dwExStyle,
		int x, int y, int cx, int cy, HWND hParent, HMENU hMenu, PCVOID pData = NULL) override
	{
		m_hWnd = IntCreate(dwExStyle, WCN_TEST, pszText, dwStyle,
			x, y, cx, cy, hParent, hMenu, eck::g_hInstance, this);
		EckDbgPrintWndMap();
		return m_hWnd;
	}
};