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
#include "eck\CTreeList.h"
#include "eck\CInputBox.h"
#include "eck\CFixedBlockCollection.h"
#include "eck\CListViewExt.h"
#include "eck\DuiBase.h"
#include "eck\CDuiLabel.h"
#include "eck\CDuiButton.h"
#include "eck\CDuiList.h"
#include "eck\CDuiTrackBar.h"
#include "eck\CDuiCircleButton.h"
#include "eck\CIni.h"
#include "eck\CTreeListExt.h"
#include "eck\CComboBoxNew.h"
#include "eck\CLinearLayout.h"
#include "eck\CDuiScrollBar.h"
#include "eck\CEditNcComp.h"
#include "eck\CSrwLock.h"
#include "eck\CFontPicker.h"
#include "eck\CColorPickBlock.h"
#include "eck\CDuiEdit.h"
#include "eck\CTab.h"

#define WCN_TEST L"CTestWindow"

using eck::PCVOID;
using eck::PCBYTE;
using namespace eck::Literals;

class CTestDui :public eck::Dui::CDuiWnd
{
public:
	eck::Dui::CLabel m_Label{};
	eck::Dui::CLabel m_Label2{};
	eck::Dui::CLabel m_Label3{};
	eck::Dui::CButton m_Btn{};
	eck::Dui::CList m_List{};
	eck::Dui::CTrackBar m_TB{};
	eck::Dui::CCircleButton m_CBtn{};
	eck::Dui::CScrollBar m_SB{};
	enum
	{
		cximg = 160,
		cyimg = 160,
	};
	eck::CD2dImageList m_il{ cximg, cyimg };
	eck::CEasingCurve m_ec{};

	struct ITEM
	{
		eck::CRefStrW rsFile{};
		eck::CRefStrW rs{};
		int idxImg{ 0 };
	};
	std::vector<ITEM> m_vItem{};
	eck::CSrwLock m_srw{};
	enum
	{
		WM_UIUIUI = 114514
	};

	LRESULT OnMsg(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) override
	{
		ID2D1Bitmap* pBitmap;
		switch (uMsg)
		{
		case eck::Dui::WM_DRAGENTER:
		case eck::Dui::WM_DRAGOVER:
			return TRUE; 
		{
			auto p = (eck::Dui::DRAGDROPINFO*)wParam;
			*(p->pdwEffect) = DROPEFFECT_COPY;
		}
		case eck::Dui::WM_DRAGLEAVE:
			return TRUE;
		case eck::Dui::WM_DROP:
			return TRUE;
		{
			auto p = (eck::Dui::DRAGDROPINFO*)wParam;
			FORMATETC fe = { CF_HDROP, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
			STGMEDIUM sm{ TYMED_HGLOBAL };
			IWICBitmapDecoder* pDecoder{};
			IWICBitmap* pWicBmp{};
			ID2D1Bitmap* pBitmap{};
			if (p->pDataObj->GetData(&fe, &sm) == S_OK)
			{
				HDROP hDrop = (HDROP)sm.hGlobal;
				UINT uFileCount = DragQueryFileW(hDrop, 0xFFFFFFFF, NULL, 0);
				for (UINT i = 0; i < uFileCount; ++i)
				{
					WCHAR szFile[MAX_PATH];
					DragQueryFileW(hDrop, i, szFile, ARRAYSIZE(szFile));
					int idxImg = -1;
					if (SUCCEEDED(eck::CreateWicBitmapDecoder(szFile, pDecoder)))
						if (SUCCEEDED(eck::CreateWicBitmap(pWicBmp, pDecoder, 160, 160)))
							if (SUCCEEDED(GetD2D().GetDC()->CreateBitmapFromWicBitmap(pWicBmp, &pBitmap)))
								idxImg = m_il.AddImage(pBitmap);
					//m_vItem.emplace_back(PathFindFileNameW(szFile), idxImg);

					eck::SafeRelease(pBitmap);
					eck::SafeRelease(pDecoder);
					eck::SafeRelease(pWicBmp);
					
					//m_Label2.SetText(mi.rsTitle);
					//m_Label2.SetPic(mi.pCoverData);
				}
				DragFinish(hDrop);
				m_List.SetItemCount((int)m_vItem.size());
			}
		}
		return TRUE;

		case WM_CREATE:
		{
			auto lResult = CDuiWnd::OnMsg(hWnd, uMsg, wParam, lParam);

			eck::LoadD2dBitmap(LR"(E:\Desktop\Temp\壁纸.bmp)", GetD2D().GetDC(), pBitmap);

			//auto sizez = pBitmap->GetSize();
			m_il.BindRenderTarget(GetD2D().GetDC());
			//SetBkgBitmap(pBitmap);

			//m_Label3.Create(L"测试标签😍😍", eck::Dui::DES_VISIBLE | eck::Dui::DES_BLURBKG, 0,
			//	0, 0, 800, 900, NULL, this, NULL);
			WIN32_FIND_DATAW wfd;
			HANDLE hFind = FindFirstFileW(LR"(H:\@重要文件\@其他\Pic\may_ena_\*.jpg)", &wfd);
			int c = 0;
			do
			{
				m_vItem.emplace_back(LR"(H:\@重要文件\@其他\Pic\may_ena_\)"_rs + wfd.cFileName, wfd.cFileName);
				++c;
				if (c == -100)
					break;
			} while (FindNextFileW(hFind, &wfd));
			FindClose(hFind);

			eck::LoadD2dBitmap(LR"(D:\@重要文件\@我的工程\PlayerNew\Res\DefCover.png)", GetD2D().GetDC(),
				pBitmap, cximg, cyimg);
			m_il.AddImage(pBitmap);

			std::thread t([this]
				{
					return;
					for (int i{}; auto & e : m_vItem)
					{
						ID2D1Bitmap* p;
						eck::LoadD2dBitmap(e.rsFile.Data(), GetD2D().GetDC(),
							p, cximg, cyimg);
						const int idx = m_il.AddImage(p);
						p->Release();
						m_srw.EnterWrite();
						e.idxImg = idx;
						m_srw.LeaveWrite();
						PostMsg(WM_UIUIUI, i, 0);
						++i;
					}
				});
			t.detach();

			eck::Dui::CElem* pElem = NULL; &m_Label3;
			pElem = 0;
			//m_List.Create(NULL, eck::Dui::DES_VISIBLE, 0,
			//	10, 10, ClientWidth - 20, ClientHeight - 20, pElem, this, NULL);// 50  70  650  670
			//m_List.SetTextFormat(GetDefTextFormat());
			//m_List.SetItemHeight(240);
			//m_List.SetItemWidth(200);
			////m_List.SetImageSize(-1);
			//m_List.SetItemPadding(5);
			//m_List.SetItemPaddingH(5);
			//m_List.SetItemCount((int)m_vItem.size());
			///*m_vItem.resize(100);
			//EckCounter(m_vItem.size(), i)
			//{
			//	m_il.AddImage(pBitmap);
			//	m_vItem[i].rs = eck::ToStr(i);
			//	m_vItem[i].pBitmap = pBitmap;
			//}
			//m_List.SetItemCount((int)m_vItem.size());*/
			//m_List.SetImageList(&m_il);
			///*m_List.SetInsertMark(5);
			//m_List.SetTopExtraSpace(100);
			//m_List.SetBottomExtraSpace(100);*/

			/*m_Label2.Create(L"测试标签😍😍", eck::Dui::DES_VISIBLE | eck::Dui::DES_BLURBKG | 0, 0,
				0, 0, 600, 100, &m_List, this, NULL);
			m_Label2.SetTextFormat(GetDefTextFormat());

			m_Label.Create(L"我是标签",
				eck::Dui::DES_VISIBLE | eck::Dui::DES_BLURBKG, 0,
				0, 500, 600, 100, &m_List, this, NULL);
			m_Label.SetTextFormat(GetDefTextFormat());

			m_Btn.Create(L"按钮", eck::Dui::DES_VISIBLE | eck::Dui::DES_TRANSPARENT, 0,
				100, 300, 300, 70, NULL, this, NULL);
			m_Btn.SetTextFormat(GetDefTextFormat());
			ID2D1Bitmap* pBmp;
			eck::LoadD2dBitmap(LR"(D:\@重要文件\@我的工程\PlayerNew\Res\Tempo.png)",
				GetD2D().GetDC(), pBmp, 54, 54);
			m_Btn.SetImage(pBmp);
			pBmp->Release();

			m_CBtn.Create(NULL, eck::Dui::DES_VISIBLE | eck::Dui::DES_TRANSPARENT, 0,
				150, 200, 60, 60, NULL, this, NULL);

			eck::LoadD2dBitmap(LR"(D:\@重要文件\@我的工程\PlayerNew\Res\Speed.png)",
				GetD2D().GetDC(), pBmp, 40, 40);
			m_CBtn.SetImage(pBmp);
			pBmp->Release();

			m_TB.Create(NULL, eck::Dui::DES_VISIBLE | eck::Dui::DES_TRANSPARENT, 0,
				100, 0, 700, 300, NULL, this, NULL);
			m_TB.SetRange(0, 100);
			m_TB.SetPos(50);
			m_TB.SetTrackSize(20);*/
			//m_TB.SetVertical(true);

			//if (pBitmap)
			//	pBitmap->Release();

			//m_SB.Create(NULL, eck::Dui::DES_VISIBLE | eck::Dui::DES_TRANSPARENT, 0,
			//	400, 300, GetDs().CommSBCxy, 300, NULL, this);
			//const auto psv = m_SB.GetScrollView();
			//psv->SetRange(-100, 100);
			//psv->SetPage(40);

			auto psz =
				L"应用程序😱😱😭😅😡😎调用此函数以获取一组对应于一系列文本位置的命中测试指标。主要用法之一是实现文本字符串的突出显示选择。\n"
				L"当 hitTestMetrics 的缓冲区大小太小，无法容纳函数计算的所有区域时，函数返回E_NOT_SUFFICIENT_BUFFER，这等效于 HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER) 。"
				L"在这种情况下，函数将输出值 * actualHitTestMetricsCount 设置为计算的几何图形数。\n"
				L"应用程序负责分配更大的新缓冲区，并再次调用函数。"
				L"应用程序😱😱😭😅😡😎调用此函数以获取一组对应于一系列文本位置的命中测试指标。主要用法之一是实现文本字符串的突出显示选择。\n"
				L"当 hitTestMetrics 的缓冲区大小太小，无法容纳函数计算的所有区域时，函数返回E_NOT_SUFFICIENT_BUFFER，这等效于 HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER) 。"
				L"在这种情况下，函数将输出值 * actualHitTestMetricsCount 设置为计算的几何图形数。\n"
				L"应用程序负责分配更大的新缓冲区，并再次调用函数。";
			auto ped = new eck::Dui::CEdit{};
			ped->Create(psz, eck::Dui::DES_VISIBLE | eck::Dui::DES_TRANSPARENT, 0,
				100, 0, 600, 600, NULL, this);
			IDWriteTextFormat* ptf;
			eck::g_pDwFactory->CreateTextFormat(
				L"微软雅黑",
				NULL,
				DWRITE_FONT_WEIGHT_NORMAL,
				DWRITE_FONT_STYLE_NORMAL,
				DWRITE_FONT_STRETCH_NORMAL,
				30.f,
				L"zh-cn",
				& ptf);
			ped->SetTextFormat(ptf);

			EnableDragDrop(TRUE);

			/*m_ec.SetWnd(HWnd);
			m_ec.SetParam((LPARAM)this);
			m_ec.SetCallBack([](float fCurrValue, float fOldValue, LPARAM lParam)
				{
					auto p = (CTestDui*)lParam;
					p->m_List.SetPos((int)fCurrValue, 70);
					p->Redraw();
				});*/
			Redraw();
			return lResult;
		}
		break;

		case WM_UIUIUI:
		{
			m_List.RedrawItem((int)wParam);
		}
		return 0;
		}

		return CDuiWnd::OnMsg(hWnd, uMsg, wParam, lParam);
	}

	LRESULT OnElemEvent(eck::Dui::CElem* pElem, UINT uMsg, WPARAM wParam, LPARAM lParam) override
	{
		using namespace eck::Dui;
		switch (uMsg)
		{
		case LEE_GETDISPINFO:
		{
			auto p = (LEEDISPINFO*)lParam;
			m_srw.EnterRead();
			const auto& e = m_vItem[p->idx];
			p->pszText = e.rs.Data();
			p->cchText = e.rs.Size();
			p->idxImg = e.idxImg;
			m_srw.LeaveRead();
			//p->pImage = e.pBitmap;
			//auto s = e.pBitmap->GetSize();
			//p->cxImage = s.width;
			//p->cyImage = s.height;
		}
		return 0;
		case EE_COMMAND:
		{
			if (pElem == &m_Btn)
			{
				static bool b{ 1 };
				b = !b;
				//m_ec.
				//if (b)
				//	m_ec.Begin(m_List.GetRect().left, 50 - m_List.GetRect().left, 400, 20);
				//else
				//	m_ec.Begin(m_List.GetRect().left, 400.f - m_List.GetRect().left, 400, 20);
			}
		}
		return 0;
		}
		return CDuiWnd::OnElemEvent(pElem, uMsg, wParam, lParam);
	}

	ECK_CWND_CREATE;
	HWND Create(PCWSTR pszText, DWORD dwStyle, DWORD dwExStyle,
		int x, int y, int cx, int cy, HWND hParent, HMENU hMenu, PCVOID pData = NULL) override
	{
		IntCreate(dwExStyle, WCN_TEST, pszText, dwStyle,
			x, y, cx, cy, hParent, hMenu, eck::g_hInstance, this);
		return m_hWnd;
	}
};

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
	eck::CListViewExt m_lve{};
	eck::CHeader m_HD{};
	eck::CTreeListExt m_tle{};
	eck::CComboBoxNew m_cbn{};
	eck::CPushButton m_btn[10]{};
	eck::CPushButton m_btn2[6]{};
	eck::CLinearLayoutH m_llh{};
	eck::CLinearLayoutV m_llv{};
	eck::CEditNcComp m_enc{};

	CTestDui m_Dui{};

	eck::CFlowLayout m_lot{};

	int m_iDpi = 96;

	HFONT m_hFont;// = eck::EzFont(L"微软雅黑", 9);
	HBITMAP m_hbm = NULL;

	HIMAGELIST m_il = NULL;
public:
	__declspec(noinline) void Test()
	{
		using namespace eck;
		using namespace eck::Literals;

		auto p = L"洛天依&乐正绫/Soda纯白";
		std::vector<CRefStrW> v{};
		SplitStrWithMultiChar(p, L"&/、", v);

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

		//Gdiplus::GpImage* pbmp;
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

		//CInputBox ib;
		//INPUTBOXOPT opt
		//{
		//	L"输入框测试",
		//	L"此 API 不参与 DPI 虚拟化",
		//	L"BeginPaint 函数准备用于绘制的指定窗口，并使用有关绘制的信息填充 PAINTSTRUCT 结构。",
		//	L"BeginPaint 函数会自动设置设备上下文的剪辑区域，以排除更新区域之外的任何区域。 更新区域由 InvalidateRect 或 InvalidateRgn"
		//	" 函数以及系统在调整大小、移动、创建、滚动或影响工作区的任何其他操作后设置。"
		//	" 如果更新区域标记为要擦除， BeginPaint 会将 WM_ERASEBKGND 消息发送到窗口。"
		//	"\n应用程序不应调用 BeginPaint ，除非响应 WM_PAINT 消息。 对 BeginPaint 的每个调用都必须具有对 EndPaint 函数的相应调用。"
		//	"\n如果插入点位于要绘制的区域， BeginPaint 会自动隐藏插入点以防止擦除它。"
		//	"\n如果窗口的 类具有背景画笔， 则 BeginPaint 使用该画笔在返回之前擦除更新区域的背景。",
		//	{},
		//	IPBF_CENTERSCREEN | IPBF_FIXWIDTH | IPBF_MULTILINE
		//	,0,
		//	0,
		//	0,
		//	0,
		//};

		//ib.DlgBox(HWnd, &opt);
		//MsgBox(opt.rsInput.Data());
		//constexpr PCWSTR psz =
		//L"; 注释测试\n"
		//"[节名]\n"
		//"值1=123456\n"
		//R"(值\=2=字符串字符\;串转义测试\n换行;这是注释)""\n"
		//;

		//CIni ini{};
		//ini.SetText(psz);
		//ini.SetParseEscapeChar(1);
		//ini.SetParseComment(1);
		//ini.SetLineBreak(L"\n");

		//ini.WriteString(L"节名", L"值2", L"还是赤石大佬");
		//ini.WriteString(L"节名1", L"值2", L"还是赤石大佬1111111111");
		//auto str = ini.ReadString(L"节名", L"值=2");

		//ini.DeleteKey(L"节名", L"值=2");
		//str = ini.GetText();

		//ini.DeleteSection(L"节名");
		//str = ini.GetText();

		//ini.Save(LR"(E:\1.ini)");
	}

	BOOL PreTranslateMessage(const MSG& Msg) override
	{
		return CForm::PreTranslateMessage(Msg);
	}
	struct WNDDATA
	{
		eck::TLNODE Node{};
		HWND hWnd{};
		eck::CRefStrW rs[3];
		std::vector<WNDDATA*> Children{};
	};
	eck::CFixedBlockCollection<WNDDATA> wdbuf{};
	void EnumWnd(HWND hWnd, WNDDATA* data, std::vector<WNDDATA*>& fvec)
	{
		auto h = GetWindow(hWnd, GW_CHILD);
		while (h)
		{
			//if (IsWindowVisible(h))
			{
				BOOL b;
				auto hicon = eck::GetWindowIcon(h, b, TRUE);
				int idx = -1;
				if (hicon)
					idx = ImageList_AddIcon(m_il, hicon);
				if (b)
					DestroyIcon(hicon);
				//EnumWnd(h, data->Children.emplace_back(new WNDDATA{ {},h }));
				auto p = wdbuf.Alloc(1, eck::TLNODE{ 0,0,0,idx,-1 }, h);
				p->rs[0].Format(L"0x%08X", h);
				p->rs[1] = eck::CWnd(h).GetClsName();
				p->rs[2] = eck::CWnd(h).GetText();
				fvec.emplace_back(p);
				EnumWnd(h, data->Children.emplace_back(p), fvec);
			}
			h = GetWindow(h, GW_HWNDNEXT);
		}
	}

	LRESULT OnMsg(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) override
	{
		//LRESULT lr;
		//DwmDefWindowProc(hWnd, uMsg, wParam, lParam, &lr);
		using namespace eck::Literals;
		static std::vector<WNDDATA*> data{};
		static std::vector<WNDDATA*> flatdata{};
		static int isortorder = 0;
		static HDC hCDCBK;
		static int cx, cy;
		static std::vector<eck::CRefStrW> vlb{};
		switch (uMsg)
		{
		case WM_CREATE:
		{
			eck::GetThreadCtx()->UpdateDefColor();
			//auto mar = eck::MakeMargin(-1);
			//DwmExtendFrameIntoClientArea(hWnd, &mar);
			WIN32_FIND_DATAW wfd;
			HANDLE hFind = FindFirstFileW(LR"(D:\@重要文件\@音乐\*.mp3)", &wfd);
			do
			{
				vlb.emplace_back(wfd.cFileName);
			} while (FindNextFileW(hFind, &wfd));
			FindClose(hFind);
			//BkColor = 0;
			//eck::EnableWindowNcDarkMode(hWnd, TRUE);
			m_iDpi = eck::GetDpi(hWnd);

			auto ptab = new eck::CTab{};
			ptab->Create(NULL, WS_CHILD | WS_VISIBLE, 0,
				100, 130, 400, 300, hWnd, 0);
			ptab->InsertItem(L"Tab 0");
			ptab->InsertItem(L"子夹 1");
			ptab->InsertItem(L"子夹 2");
			ptab->InsertItem(L"哈哈哈哈哈哈哈哈");
			ptab->InsertItem(L"测试测试");
			ptab->InsertItem(L"Tabbbbb");


			//auto hil = ImageList_Create(80, 80, ILC_COLOR32 | ILC_ORIGINALSIZE, 0, 20);
			//hFind = FindFirstFileW(LR"(H:\@存档的文件\@其他\图片素材库\64px\Kitchen (Food Beverage)\*.png)", &wfd);
			//do
			//{
			//	IWICBitmapDecoder* pd;
			//	eck::CreateWicBitmapDecoder(
			//		(LR"(H:\@存档的文件\@其他\图片素材库\64px\Kitchen (Food Beverage)\)"_rs +
			//			wfd.cFileName).Data(), pd);
			//	IWICBitmap* pb;
			//	eck::CreateWicBitmap(pb, pd, 80, 80);
			//	const auto h = eck::CreateHICON(pb);
			//	ImageList_AddIcon(hil, h);
			//} while (FindNextFileW(hFind, &wfd));
			//FindClose(hFind);
			//
			//m_lve.Create(0, WS_CHILD | WS_VISIBLE, WS_EX_CLIENTEDGE,
			//	0, 0, 800, 700, hWnd, 10002);
			//m_lve.SetView(LV_VIEW_ICON);
			//m_lve.SetImageList(hil, LVSIL_NORMAL);
			//m_lve.SetLVExtendStyle(LVS_EX_DOUBLEBUFFER|LVS_EX_FULLROWSELECT| LVS_EX_SUBITEMIMAGES|LVS_EX_BORDERSELECT);
			//
			//m_lve.InsertColumn(L"Col. 1", -1, 210);
			//m_lve.InsertColumn(L"Col. 2", -1, 210);
			//m_lve.InsertColumn(L"Col. 3", -1, 210);
			//m_lve.InsertColumn(L"Col. 4", -1, 210);

			//LVITEMW li;
			//li.mask = LVIF_IMAGE;
			//EckCounter(30, i)
			//{
			//	const int idx = m_lve.InsertItem((std::to_wstring(i) + L" 项目测试").c_str(), -1, 0, i);
			//	m_lve.SetItemText(idx, 1, L"测试子项111");
			//	m_lve.SetItemText(idx, 2, L"测试子项 二");
			//	if (i % 2)
			//	{
			//		li.iItem = idx;
			//		li.iSubItem = 2;
			//		li.iImage = 4;
			//		m_lve.SetItem(&li);
			//	}
			//}

			//RECT rc;
			//m_lve.GetItemRect(10, &rc, LVIR_ICON);

			//rc = {};

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

			//m_cbn.Create(NULL, WS_CHILD | WS_VISIBLE, 0,
			//	0, 0, 300, 60, hWnd, 2203);
			//m_cbn.SetItemCount((int)vlb.size());
			//m_lot.Add(&m_cbn, eck::FLF_FIXWIDTH | eck::FLF_FIXHEIGHT);

			//m_LBN.Create(NULL, WS_CHILD | WS_VISIBLE | WS_BORDER | WS_VSCROLL, 0,
			//	0, 0, 700, 600, hWnd, 105);
			//m_LBN.SetItemCount((int)vlb.size());
			//m_lot.Add(&m_LBN, eck::FLF_FIXWIDTH | eck::FLF_FIXHEIGHT);
			//

			//hCDCBK = CreateCompatibleDC(NULL);
			//auto hbm = eck::CreateHBITMAP(LR"(E:\Desktop\Temp\DC802FE9979A460BBA8E757382343EB4.jpg)");
			//SelectObject(hCDCBK, hbm);
			//BITMAP bb;
			//GetObjectW(hbm, sizeof(bb), &bb);
			//cx = bb.bmWidth;
			//cy = bb.bmHeight;

			//m_il = ImageList_Create(eck::DpiScale(16, m_iDpi), eck::DpiScale(16, m_iDpi),
			//	ILC_COLOR32 | ILC_ORIGINALSIZE, 0, 40);

			//HWND h = GetDesktopWindow();
			//BOOL b;
			//auto hicon = eck::GetWindowIcon(h, b, TRUE);
			//int idx = -1;
			//if (hicon)
			//	idx = ImageList_AddIcon(m_il, hicon);
			//if (b)
			//	DestroyIcon(hicon);

			//data.push_back(wdbuf.Alloc(1, eck::TLNODE{ 0,0,0,idx,-1 }, h));
			//auto p = data.back();
			//p->rs[0].Format(L"0x%08X", h);
			//p->rs[1] = eck::CWnd(h).GetClsName();
			//p->rs[2] = eck::CWnd(h).GetText();
			//EnumWnd(GetDesktopWindow(), data[0], flatdata);

			//h = HWND_MESSAGE;
			//data.push_back(wdbuf.Alloc(1, eck::TLNODE{ 0,0,0,-1,-1 }, h));
			//p = data.back();
			//p->rs[0].Format(L"0x%08X", h);
			//p->rs[1] = L"HWND_MESSAGE";
			//p->rs[2] = L"HWND_MESSAGE";

			//HWND hMo{};
			//while ((hMo = FindWindowExW(HWND_MESSAGE, hMo, 0, 0)))
			//{
			//	BOOL b;
			//	auto hicon = eck::GetWindowIcon(hMo, b, TRUE);
			//	int idx = -1;
			//	if (hicon)
			//		idx = ImageList_AddIcon(m_il, hicon);
			//	if (b)
			//		DestroyIcon(hicon);
			//	//EnumWnd(h, data->Children.emplace_back(new WNDDATA{ {},h }));
			//	auto p0 = wdbuf.Alloc(1, eck::TLNODE{ 0,0,0,idx,-1 }, hMo);
			//	p0->rs[0].Format(L"0x%08X", hMo);
			//	p0->rs[1] = eck::CWnd(hMo).GetClsName();
			//	p0->rs[2] = eck::CWnd(hMo).GetText();
			//	flatdata.emplace_back(p0);
			//	//p->Children.emplace_back(p0);
			//	EnumWnd(hMo, p->Children.emplace_back(p0), flatdata);
			//}

			//m_TL.Create(NULL, WS_CHILD | WS_VISIBLE | WS_BORDER, 0,
			//	0, 0, 1200, 1000, hWnd, 106);
			//auto& he = m_TL.GetHeader();
			//he.InsertItem(L"HWND", -1, 360);
			//he.InsertItem(L"szClsName", -1, 360);
			//he.InsertItem(L"szText", -1, 400);
			//m_TL.SetEditLabel(TRUE);
			////m_TL.SetTextClr(0xFFFFFF);
			////m_TL.SetBkClr(RGB(25, 25, 25));
			//EckDbgPrint(flatdata.size());
			////auto& tl = m_TL.GetToolTip();
			////tl.ModifyStyle(0, TTS_NOANIMATE);
			//m_TL.SetHasCheckBox(TRUE);
			//m_TL.SetHasLines(TRUE);
			//m_TL.SetImageList(m_il);
			//m_TL.SetWatermarkString(L"水印测试。\n我是第二行水印。");
			//m_TL.BuildTree();
			////m_TL.SetBackgroundNotSolid(TRUE);
			////m_TL.SetSingleSelect(TRUE);
			////m_lve.Create(NULL, WS_CHILD | WS_VISIBLE | WS_BORDER, 0,
			////	0, 0, 1200, 1000, hWnd, 107);
			//m_lot.Add(&m_TL, eck::FLF_FIXWIDTH | eck::FLF_FIXHEIGHT);

			//m_LV.Create(NULL, WS_VISIBLE | WS_CHILD | WS_CLIPSIBLINGS | LVS_REPORT, 0,
			//	0, 0, 500, 500, hWnd, 1111);
			//m_LV.InsertColumn(L"Col1", -1, 200);
			//m_LV.InsertColumn(L"Col2", -1, 200);
			//auto p1 = new eck::CHeader{};
			//p1->AttachNew(m_LV.GetHeaderCtrl().HWnd);
			//m_LV.InsertItem(L"测试");
			//m_lot.Add(&m_LV, eck::FLF_FIXWIDTH | eck::FLF_FIXHEIGHT);

			//m_HD.Create(NULL, WS_VISIBLE | WS_CHILD | WS_CLIPSIBLINGS, 0,
			//	0, 0, 300, 40, hWnd, 1122);
			//m_HD.InsertItem(L"项目",-1,100);
			//m_HD.InsertItem(L"项目2", -1, 100);
			//m_lot.Add(&m_HD, eck::FLF_FIXWIDTH | eck::FLF_FIXHEIGHT);

			//SetText((eck::ToStr(flatdata.size()) + L" 个窗口").Data());

			//m_Edit.Create(L"", WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL|WS_CLIPSIBLINGS, 0,
			//	0, 0, 200, 100, hWnd, 102);
			//m_Edit.SetFrameType(5);
			//m_lot.Add(&m_Edit, eck::FLF_FIXWIDTH | eck::FLF_FIXHEIGHT);

			//m_Btn.Create(L"筛选", WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS, 0, 900, 0, 300, 70, hWnd, 101);
			//m_lot.Add(&m_Btn, eck::FLF_FIXWIDTH | eck::FLF_FIXHEIGHT);
			
			//RECT rcDui{ 0,0,1800,1000 };
			//m_Dui.Create(L"我是 Dui 窗口", WS_CHILD | WS_VISIBLE, 0, 0, 0, rcDui.right, rcDui.bottom, hWnd, 108);
			//m_Dui.Redraw();
			//m_lot.Add(&m_Dui, eck::FLF_FIXWIDTH | eck::FLF_FIXHEIGHT);

			//m_enc.Create(L"示例编辑框", WS_VISIBLE | WS_CHILD, WS_EX_CLIENTEDGE,
			//	0, 0, 200, 40, hWnd, 0);
			//m_lot.Add(&m_enc, eck::FLF_FIXWIDTH | eck::FLF_FIXHEIGHT);

			//auto pfp = new eck::CFontPicker{};
			//pfp->Create(NULL, WS_CHILD | WS_VISIBLE, WS_EX_CLIENTEDGE,
			//	0, 0, 260, 40, hWnd, 0);
			//m_lot.Add(pfp, eck::FLF_FIXWIDTH | eck::FLF_FIXHEIGHT);

			//auto pcpb = new eck::CColorPickBlock{};
			//pcpb->Create(NULL, WS_CHILD | WS_VISIBLE|SS_NOTIFY, WS_EX_CLIENTEDGE,
			//	0, 0, 40, 40, hWnd, 0);
			//m_lot.Add(pcpb, eck::FLF_FIXWIDTH | eck::FLF_FIXHEIGHT);

			/*m_tle.Create(0, WS_CHILD | WS_VISIBLE | WS_BORDER, 0,
				0, 0, 800, 800, hWnd, 1010);
			m_tle.GetHeader().InsertItem(L"第一栏", -1, 400);
			m_tle.GetHeader().InsertItem(L"Col. 2", -1, 350);
			auto hNode = m_tle.InsertItem(L"父项1");
			eck::TLEXTITEM tlei{ eck::TLEIF_TEXT };
			tlei.idxSubItem = 1;
			EckCounter(10, i)
			{
				auto h =m_tle.InsertItem((eck::ToStr(i) + L" 子项测试").Data(), 0, hNode);
				auto rs = eck::Format(L"你好我是第二列的第 %d 项", i);
				tlei.pszText = rs.Data();
				tlei.cchText = rs.Size();
				m_tle.SetItem(h, tlei);
			}
			hNode = m_tle.InsertItem(L"父项2");
			hNode = m_tle.InsertItem(L"父项3");
			m_tle.BuildTree();
			m_lot.Add(&m_tle, eck::FLF_FIXWIDTH | eck::FLF_FIXHEIGHT);*/

			//for (int i{}; auto& e : m_btn)
			//{
			//	e.Create((L"按钮"_rs + eck::ToStr(i)).Data(), WS_VISIBLE | WS_CHILD, 0,
			//		0, 0, 500, 50, hWnd, NULL);
			//	m_llv.Add(&e, { 10,5 }, eck::LLF_FIXHEIGHT | eck::LLF_FIXWIDTH);
			//	if (i == 4)
			//	{
			//		for (int j{}; auto & f : m_btn2)
			//		{
			//			f.Create((L"按钮"_rs + eck::ToStr(j)).Data(), WS_VISIBLE | WS_CHILD, 0,
			//				0, 0, 50, 60, hWnd, NULL);
			//			m_llh.Add(&f, {}, eck::LLF_FIXHEIGHT | eck::LLF_FIXWIDTH);
			//			++j;
			//		}
			//		m_llv.Add(&m_llh, { 20 });
			//	}
			//	++i;
			//}

			m_hFont = eck::CreateDefFont(m_iDpi);
			eck::SetFontForWndAndCtrl(hWnd, m_hFont);
			m_cbn.GetListBox().SetFont(m_hFont);

			Test();

			COLORREF dummy, crBkg;
			eck::GetItemsViewForeBackColor(dummy, crBkg);
			BkColor = crBkg;
			eck::EnableWindowNcDarkMode(hWnd, eck::ShouldAppUseDarkMode());
			Redraw();
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
					if (p->bQueryRoot)
					{
						p->cChildren = (int)data.size();
						p->pChildren = (eck::TLNODE**)data.data();
						/*p->cChildren = (int)data[0]->Children.size();
						p->pChildren = (eck::TLNODE**)data[0]->Children.data();*/
					}
					else
					{
						auto pd = (WNDDATA*)p->pParent;
						p->pChildren = (eck::TLNODE**)pd->Children.data();
						p->cChildren = (int)pd->Children.size();
					}
				}
				return 0;
				case eck::NM_TL_FILLALLFLATITEM:
				{
					auto p = (eck::NMTLFILLALLFLATITEM*)lParam;
					p->cItem = (int)flatdata.size();
					p->pItems = (eck::TLNODE**)flatdata.data();
				}
				return 0;
				case eck::NM_TL_GETDISPINFO:
				{
					auto p = (eck::NMTLGETDISPINFO*)lParam;
					auto pd = (WNDDATA*)p->Item.pNode;
					switch (p->Item.idxSubItem)
					{
					case 0:
						p->Item.pszText = pd->rs[0].Data();
						p->Item.cchText = pd->rs[0].Size();
						break;
					case 1:
						p->Item.pszText = pd->rs[1].Data();
						p->Item.cchText = pd->rs[1].Size();
						break;
					case 2:
						p->Item.pszText = pd->rs[2].Data();
						p->Item.cchText = pd->rs[2].Size();
						break;
					}
				}
				return 0;
				case eck::NM_TL_HD_CLICK:
				{
					auto p = (NMHEADERW*)lParam;

					const int idxCol = p->iItem;
					auto& H = m_TL.GetHeader();

					int fmt;
					if (isortorder == 0)
					{
						isortorder = 1;
						fmt = HDF_SORTUP;
					}
					else if (isortorder == 1)
					{
						isortorder = -1;
						fmt = HDF_SORTDOWN;
					}
					else if (isortorder == -1)
					{
						isortorder = 0;
						fmt = 0;
					}
					
					H.RadioSetSortMark(idxCol, fmt);

					if (isortorder == 1)
					{
						std::sort(flatdata.begin(), flatdata.end(), [idxCol](const WNDDATA* p1, const WNDDATA* p2)
							{
								return p1->rs[idxCol] < p2->rs[idxCol];
							});
						m_TL.SetFlatMode(TRUE);
					}
					else if (isortorder == -1)
					{
						std::sort(flatdata.begin(), flatdata.end(), [idxCol](const WNDDATA* p1, const WNDDATA* p2)
							{
								return  p2->rs[idxCol] < p1->rs[idxCol];
							});
						m_TL.SetFlatMode(TRUE);
					}
					else
						m_TL.SetFlatMode(FALSE);
					m_TL.BuildTree();
					m_TL.Redraw();
				}
				return 0;
				case eck::NM_TL_CUSTOMDRAW:
				{
					auto p = (eck::NMTLCUSTOMDRAW*)lParam;
					auto pcol = m_TL.GetColumnPosInfo();
					if (p->iDrawStage == eck::TLCDD_PREFILLBK)
					{
						//RECT rc;
						//GetClientRect(m_TL.HWnd, &rc);
						//rc.top += m_TL.GetHeader().GetHeight();
						//StretchBlt(p->hDC, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top,
						//	hCDCBK, 0, 0,cx,cy, SRCCOPY);
						//return eck::TLCDRF_SKIPDEFAULT;
					}
					else if (p->iDrawStage == eck::TLCDD_PREPAINTITEM)
					{
						auto pd = (WNDDATA*)p->pNode;
						UINT u = 0;
						/*if (!pd->rs[1].IsEmpty())
						{
							if (eck::FindStr(pd->rs[1].Data(), L"Chrome") >= 0)
							{
								SetDCBrushColor(p->hDC, eck::Colorref::NeutralGray);
								FillRect(p->hDC, p->prcItem, GetStockBrush(DC_BRUSH));
								u |= eck::TLCDRF_BKGNDCHANGED;
							}
							else if (eck::FindStr(pd->rs[1].Data(), L"TX") >= 0)
							{
								SetDCBrushColor(p->hDC, eck::Colorref::MoneyGreen);
								FillRect(p->hDC, p->prcItem, GetStockBrush(DC_BRUSH));
								u |= eck::TLCDRF_BKGNDCHANGED;
							}
						}*/

						if (!IsWindow(pd->hWnd))
						{
							p->crText = eck::Colorref::Red;
							u |= eck::TLCDRF_TEXTCLRCHANGED;
						}

						if (!IsWindowVisible(pd->hWnd))
						{
							p->crText = eck::Colorref::Gray;
							u |= eck::TLCDRF_TEXTCLRCHANGED;
						}
						return u;
					}
				}
				return eck::TLCDRF_NONE;
				case eck::NM_TL_PREEDIT:
				{
					EckDbgPrint(L"Pre Edit");
				}
				return 0;
				case eck::NM_TL_POSTEDIT:
				{
					auto p = (eck::NMTLEDIT*)lParam;
					auto& Edit = m_TL.GetEdit();
					EckDbgPrintFmt(L"Post Edit, Text = %s，Changed = %d",
						Edit.GetText().Data(), !!(p->uFlags & eck::TLEDF_SHOULDSAVETEXT));
				}
				return 0;

				case eck::NM_TL_BEGINDRAG:
				{
					auto p = (eck::NMTLDRAG*)lParam;
					EckDbgPrintFmt(L"Pre Drag, bRBtn = %d", p->bRBtn);
				}
				return 0;
				case eck::NM_TL_ENDDRAG:
				{
					auto p = (eck::NMTLDRAG*)lParam;
					EckDbgPrintFmt(L"Post Drag, bRBtn = %d", p->bRBtn);
				}
				return 0;
				}
			}
			else if (pnm->hwndFrom == m_LBN.HWnd)
			{
				switch (pnm->code)
				{
				case eck::NM_LBN_GETDISPINFO:
				{
					auto p = (eck::NMLBNGETDISPINFO*)lParam;
					p->Item.pszText = vlb[p->Item.idxItem].Data();
					p->Item.cchText = vlb[p->Item.idxItem].Size();
				}
				return 0;
				}
			}
			else if (pnm->hwndFrom == m_cbn.HWnd)
			{
				switch (pnm->code)
				{
				case eck::NM_LBN_GETDISPINFO:
				{
					auto p = (eck::NMLBNGETDISPINFO*)lParam;
					p->Item.pszText = vlb[p->Item.idxItem].Data();
					p->Item.cchText = vlb[p->Item.idxItem].Size();
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
			//m_llv.Arrange(cx, cy);
		}
		return 0;
		case WM_COMMAND:
		{
			if ((HWND)lParam == m_Btn.GetHWND() && HIWORD(wParam) == BN_CLICKED)
			{
				auto rc = m_Dui.m_Label2.GetRect();
				m_Dui.m_Label2.SetRect({ rc.left + 10,rc.top ,rc.right + 10,rc.bottom });
				m_Dui.CWnd::Redraw();
				break;

				auto rs = m_Edit.GetText();
				if (rs.IsEmpty())
					for (auto e : flatdata)
						e->Node.uFlags &= ~eck::TLIF_INVISIBLE;
				else
					for (auto e : flatdata)
						if (!e->rs[1].IsEmpty() && eck::FindStr(e->rs[1].Data(), rs.Data()) < 0)
							e->Node.uFlags |= eck::TLIF_INVISIBLE;
						else
							e->Node.uFlags &= ~eck::TLIF_INVISIBLE;

				m_TL.BuildTree();
				m_TL.Redraw();

				//TrayPopBalloon(101, L"气球测试", L"我是标题");
				//m_Edit.SetClr(2, eck::Colorref::Azure);
				//BkColor = eck::Colorref::DeepGray;
				//EscClose = TRUE;
				//TotalMove = TRUE;
				//Redraw();
			}
		}
		break;
		case WM_DPICHANGED:
		{
			const int iPrevDpi = m_iDpi;
			m_iDpi = LOWORD(wParam);
			const auto prc = (RECT*)lParam;

			auto hFont = eck::ReCreateFontForDpiChanged(m_hFont, m_iDpi, iPrevDpi);
			eck::SetFontForWndAndCtrl(hWnd, hFont);
			std::swap(hFont, m_hFont);
			DeleteObject(hFont);

			SetWindowPos(hWnd, NULL, prc->left, prc->top, prc->right - prc->left, prc->bottom - prc->top,
				SWP_NOZORDER | SWP_NOACTIVATE);
		}
		break;
		case WM_DESTROY:
			PostQuitMessage(0);
			break;
		case WM_SETTINGCHANGE:
			eck::GetThreadCtx()->UpdateDefColor();

			if (eck::IsColorSchemeChangeMessage(lParam))
			{
				eck::RefreshImmersiveColorStuff();
				eck::FlushMenuTheme();
				eck::BroadcastChildrenMessage(hWnd, uMsg, wParam, lParam);
				eck::BroadcastChildrenMessage(hWnd, WM_THEMECHANGED, 0, 0);
				COLORREF dummy, crBkg;
				eck::GetItemsViewForeBackColor(dummy, crBkg);
				BkColor = crBkg;
				eck::EnableWindowNcDarkMode(hWnd, eck::ShouldAppUseDarkMode());
				Redraw();
			}
			break;
		case WM_SYSCOLORCHANGE:
			eck::GetThreadCtx()->UpdateDefColor();
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