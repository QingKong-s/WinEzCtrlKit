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
#include "eck\SystemHelper.h"
#include "eck\CAnimationBox.h"
#include "eck\CForm.h"
#include "eck\CTreeList.h"
#include "eck\CInputBox.h"

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
	eck::CTreeList m_TL{};

	eck::CFlowLayout m_lot{};

	HFONT m_hFont = eck::EzFont(L"微软雅黑", 9);
	HBITMAP m_hbm = NULL;
public:
	__declspec(noinline) void Test()
	{
		using namespace eck;
		using namespace eck::Literals;
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

		//CRefStrA rsa("测试字符串");
		//auto bstr1 = rsa.ToBSTR();
		//auto bstr2 = rs.ToBSTR();
		//EckDbgPrintWithPos(L"测试");

		//CRefStrT<WCHAR, CCharTraits<WCHAR>, CAllocatorVA<WCHAR, int>> rsva(L"测试字符串");
		//
		//rsva.AppendFormat(L"整数 = %d，字符串 = %s。", 100, L"我是字符串");
		//rsva.MakeRepeatedStrSequence(L"123456", 6, 1000);

		//GpImage* pbmp;
		//GdipLoadImageFromFile(LR"(E:\Desktop\Temp\111111.bmp)", &pbmp);
		//auto rb = SaveGpImage(pbmp, ImageType::Png);
		//WriteToFile(LR"(E:\Desktop\111111.png)", rb);

		//auto hIcon = CreateHICON(LR"(E:\Desktop\Temp\111111.jpg)");
		//TrayAdd(101, hIcon, L"我是提示");

		//auto a= BytesToInteger<DWORD>(0xaa, 0xbb, 0xcc, 0xdd);

		//auto b = Rand(0x111111, 0xffffff);
		//a = ColorrefToARGB(b, 0x4B);
		//CRefStrW rs{};
		//rs.Format(L"%08X", b);
		//rs.PushBack(L"\n");
		//rs.AppendFormat(L"%08X", a);
		////MsgBox(rs.Data());

		//EckDbgPrint(L"123"_rs < L"456");

		//IWICBitmap* pBitmap;
		//IWICBitmapDecoder* pDecoder;
		//CreateWicBitmapDecoder(LR"(E:\Desktop\Temp\111111.jpg)", pDecoder);
		//CreateWicBitmap(pBitmap, pDecoder);

		//CRefBin rb = SaveWicBitmap(pBitmap);
		//WriteToFile(LR"(E:\Desktop\123.png)", rb);
		//EckDbgBreak();

		CInputBox ib;
		INPUTBOXOPT opt
		{
			L"输入框测试",
			L"此 API 不参与 DPI 虚拟化",
			L"BeginPaint 函数准备用于绘制的指定窗口，并使用有关绘制的信息填充 PAINTSTRUCT 结构。",
			L"BeginPaint 函数会自动设置设备上下文的剪辑区域，以排除更新区域之外的任何区域。 更新区域由 InvalidateRect 或 InvalidateRgn"
			" 函数以及系统在调整大小、移动、创建、滚动或影响工作区的任何其他操作后设置。"
			" 如果更新区域标记为要擦除， BeginPaint 会将 WM_ERASEBKGND 消息发送到窗口。"
			"\n应用程序不应调用 BeginPaint ，除非响应 WM_PAINT 消息。 对 BeginPaint 的每个调用都必须具有对 EndPaint 函数的相应调用。"
			"\n如果插入点位于要绘制的区域， BeginPaint 会自动隐藏插入点以防止擦除它。"
			"\n如果窗口的 类具有背景画笔， 则 BeginPaint 使用该画笔在返回之前擦除更新区域的背景。",
			{},
			IPBF_CENTERSCREEN | IPBF_FIXWIDTH //IPBF_MULTILINE
			,0,
			0,
			1000,
			0,
		};

		ib.DlgBox(HWnd, &opt);
		MsgBox(opt.rsInput.Data());
	}

	BOOL PreTranslateMessage(const MSG& Msg) override
	{
		return CForm::PreTranslateMessage(Msg);
	}
	struct WNDDATA
	{
		eck::TLNODE Node{};
		HWND hWnd{};
		eck::CRefStrW rs{};
		eck::CRefStrW rs2{};
		eck::CRefStrW rs3{};
		std::vector<WNDDATA*> Children{};
	};
	void EnumWnd(HWND hWnd, WNDDATA* data)
	{
		auto h = GetWindow(hWnd, GW_CHILD);
		while (h)
		{
			if (IsWindowVisible(h))
			{
				EnumWnd(h, data->Children.emplace_back(new WNDDATA{ {},h }));
			}
			h = GetWindow(h, GW_HWNDNEXT);
		}
		data->Node.cChildren = data->Children.size();
	}

	LRESULT OnMsg(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) override
	{

		static std::vector<WNDDATA*> data{};
		switch (uMsg)
		{
		case WM_CREATE:
		{
			//m_Btn.Create(L"按钮测试", WS_CHILD | WS_VISIBLE, 0, 0, 0, 300, 70, hWnd, 101);
			//m_lot.Add(&m_Btn, eck::FLF_FIXWIDTH | eck::FLF_FIXHEIGHT);

			//m_Edit.Create(L"编辑框", WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL, 0,
			//	0, 0, 200, 100, hWnd, 102);
			//m_Edit.SetFrameType(1);
			//m_lot.Add(&m_Edit, eck::FLF_FIXWIDTH | eck::FLF_FIXHEIGHT);

			//m_Label.Create(L"我是标签", WS_CHILD | WS_VISIBLE | WS_BORDER, 0, 0, 0, 300, 200, hWnd, 103);
			//m_hbm = eck::CreateHBITMAP(LR"(E:\Desktop\Temp\111111.jpg)");
			//m_Label.SetPic(m_hbm);
			//m_lot.Add(&m_Label);

			//m_AB.Create(NULL, WS_CHILD | WS_VISIBLE | WS_BORDER, 0, 0, 0, 1100, 700, hWnd, 104);
			//m_lot.Add(&m_AB);
			//auto pDC = m_AB.GetDC();

			//IWICBitmapDecoder* pDecoder;
			//eck::CreateWicBitmapDecoder(LR"(F:\Test\1.png)", pDecoder, eck::g_pWicFactory);
			//IWICBitmap* pWicBmp;
			//eck::CreateWicBitmap(pWicBmp, pDecoder, eck::g_pWicFactory);
			//ID2D1Bitmap1* pBitmap;
			//pDC->CreateBitmapFromWicBitmap(pWicBmp, &pBitmap);
			//auto pSpirit = new eck::CAbSpiritImage(pBitmap);
			//pSpirit->SetPos({ 200,400 });
			////m_AB.AddSpirit(pSpirit);
			////pSpirit->AutoMarch({ 10.f,20,600.f,0,FALSE });

			//eck::CreateWicBitmapDecoder(LR"(F:\Test\2.jpg)", pDecoder, eck::g_pWicFactory);
			//eck::CreateWicBitmap(pWicBmp, pDecoder, eck::g_pWicFactory);
			//pDC->CreateBitmapFromWicBitmap(pWicBmp, &pBitmap);

			//pSpirit = new eck::CAbSpiritImage(pBitmap);
			//pSpirit->SetPos({ 400,200 });

			//m_AB.AddSpirit(pSpirit);
			//pSpirit->Turn(-eck::Deg2Rad(90.f));
			//pSpirit->AutoMarch({ 10.f,20,600.f,0,FALSE });
			//eck::CListBoxNew::FLbnProc pfnn = [](HWND hWnd, UINT uCode, LPARAM lParam, LPARAM lRefData)->LRESULT
			//	{
			//		static std::vector<eck::CRefStrW> v{};
			//		switch (uCode)
			//		{
			//		case eck::CListBoxNew::NCode::GetDispInfo:
			//		{
			//			if (!v.size())
			//			{
			//				v.resize(100);
			//				EckCounter(100, i)
			//				{
			//					v[i] = eck::ToStr(i) + L"测试测试";
			//				}
			//			}
			//			auto p = (eck::LBNITEM*)lParam;
			//			p->pszText = v[p->idxItem].Data();
			//			p->cchText = v[p->idxItem].Size();
			//		}
			//		return 0;
			//		}

			//		return 0;
			//	};
			//m_LBN.SetProc(pfnn);
			//m_LBN.Create(NULL, WS_CHILD | WS_VISIBLE | WS_BORDER | WS_VSCROLL, 0,
			//	0, 0, 700, 600, hWnd, 105);
			//m_LBN.SetItemCount(100);
			//m_lot.Add(&m_LBN, eck::FLF_FIXWIDTH | eck::FLF_FIXHEIGHT);
			data.push_back(new WNDDATA{});
			EnumWnd(GetDesktopWindow(), data[0]);
			m_TL.Create(NULL, WS_CHILD | WS_VISIBLE | WS_BORDER, 0,
				0, 0, 1200, 1000, hWnd, 106);
			m_TL.BuildTree();
			eck::SetFontForWndAndCtrl(hWnd, m_hFont);


			Test();
		}
		return 0;
		case WM_NOTIFY:
		{
			auto pnm = (NMHDR*)lParam;
			if (pnm->hwndFrom == m_TL.HWnd)
			{
				switch (pnm->code)
				{
				case eck::NM_TL_FILLCHILDREN:
				{
					auto p = (eck::NMTLFILLCHILDREN*)lParam;
					if (!p->pParent)
					{
						p->pParent = (eck::TLNODE*)data[0];
						p->pChildren = (eck::TLNODE**)data[0]->Children.data();
					}
					else
					{
						auto pd = (WNDDATA*)p->pParent;
						p->pChildren = (eck::TLNODE**)pd->Children.data();
					}
				}
				return 0;
				case eck::NM_TL_GETDISPINFO:
				{
					auto p = (eck::NMTLGETDISPINFO*)lParam;
					auto pd = (WNDDATA*)p->Item.pNode;
					switch (p->Item.idxSubItem)
					{
					case 0:
						pd->rs.Format(L"0x%08X", pd->hWnd);
						p->Item.pszText = pd->rs.Data();
						p->Item.cchText = pd->rs.Size();
						break;
					case 1:
						pd->rs2 = eck::CWnd(pd->hWnd).GetClsName();
						p->Item.pszText = pd->rs2.Data();
						p->Item.cchText = pd->rs2.Size();
						break;
					case 2:
						pd->rs3 = eck::CWnd(pd->hWnd).GetText();
						p->Item.pszText = pd->rs3.Data();
						p->Item.cchText = pd->rs3.Size();
						break;
					}
				}
				return 0;
				}
			}
		}
		break;
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
				//TrayPopBalloon(101, L"气球测试", L"我是标题");
				//m_Edit.SetClr(2, eck::Colorref::Azure);
				//BkColor = eck::Colorref::DeepGray;
				//EscClose = TRUE;
				//TotalMove = TRUE;
				//Redraw();
			}
		}
		break;
		}
		return CForm::OnMsg(hWnd, uMsg, wParam, lParam);
	}

	static ATOM RegisterWndClass()
	{
		return eck::EzRegisterWndClass(WCN_TEST);
	}

	ECK_CWND_CREATE;
	HWND Create(PCWSTR pszText, DWORD dwStyle, DWORD dwExStyle,
		int x, int y, int cx, int cy, HWND hParent, HMENU hMenu, PCVOID pData = NULL) override
	{
		IntCreate(dwExStyle, WCN_TEST, pszText, dwStyle,
			x, y, cx, cy, hParent, hMenu, eck::g_hInstance, this);
		EckDbgPrintWndMap();
		return m_hWnd;
	}
};