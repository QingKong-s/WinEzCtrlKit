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

#define WCN_TEST L"CTestWindow"

using eck::PCVOID;
using eck::PCBYTE;

struct MUSICINFO
{
	eck::CRefStrW rsTitle{};
	eck::CRefStrW rsArtist{};
	eck::CRefStrW rsAlbum{};
	eck::CRefStrW rsComment{};
	eck::CRefStrW rsLrc{};
	IStream* pCoverData = NULL;
};

#pragma pack (push)
#pragma pack (1)
struct ID3v2_Header		// ID3v2标签头
{
	CHAR Header[3];		// "ID3"
	BYTE Ver;			// 版本号
	BYTE Revision;		// 副版本号
	BYTE Flags;			// 标志
	BYTE Size[4];		// 标签大小，28位数据，每个字节最高位不使用，包括标签头的10个字节和所有的标签帧
};

struct ID3v2_ExtHeader  // ID3v2扩展头
{
	BYTE ExtHeaderSize[4];  // 扩展头大小
	BYTE Flags[2];          // 标志
	BYTE PaddingSize[4];    // 空白大小
};

struct ID3v2_FrameHeader// ID3v2帧头
{
	CHAR ID[4];			// 帧标识
	BYTE Size[4];		// 帧内容的大小，32位数据，不包括帧头
	BYTE Flags[2];		// 存放标志
};

struct FLAC_Header      // Flac头
{
	BYTE by;
	BYTE bySize[3];
};
#pragma pack (pop)

inline DWORD SynchSafeIntToDWORD(PCBYTE p)
{
	return ((p[0] & 0x7F) << 21) | ((p[1] & 0x7F) << 14) | ((p[2] & 0x7F) << 7) | (p[3] & 0x7F);
}

inline eck::CRefStrW GetMP3ID3v2_ProcString(PCBYTE pStream, int cb, int iTextEncoding = -1)
{
	int iType = 0, cchBuf;
	if (iTextEncoding == -1)
	{
		memcpy(&iType, pStream, 1);
		++pStream;// 跳过文本编码标志
		--cb;
	}
	else
		iType = iTextEncoding;

	eck::CRefStrW rsResult{};

	switch (iType)
	{
	case 0:// ISO-8859-1，即Latin-1（拉丁语-1）
		cchBuf = MultiByteToWideChar(CP_ACP, 0, (PCCH)pStream, cb, NULL, 0);
		if (cchBuf == 0)
			return {};
		rsResult.ReSize(cchBuf);
		MultiByteToWideChar(CP_ACP, 0, (PCCH)pStream, cb, rsResult.Data(), cchBuf);
		break;
	case 1:// UTF-16LE
		if (*(PWSTR)pStream == L'\xFEFF')// 跳BOM（要不是算出来哈希值不一样我可能还真发现不了这个BOM的问题.....）
		{
			pStream += sizeof(WCHAR);
			cb -= sizeof(WCHAR);
		}
		cchBuf = cb / sizeof(WCHAR);
		rsResult.ReSize(cchBuf);
		wcsncpy(rsResult.Data(), (PWSTR)pStream, cchBuf);
		break;
	case 2:// UTF-16BE
		if (*(PWSTR)pStream == L'\xFFFE')// 跳BOM
		{
			pStream += sizeof(WCHAR);
			cb -= sizeof(WCHAR);
		}
		cchBuf = cb / sizeof(WCHAR);
		rsResult.ReSize(cchBuf);
		LCMapStringEx(LOCALE_NAME_USER_DEFAULT, LCMAP_BYTEREV,
			(PCWSTR)pStream, cchBuf, rsResult.Data(), cchBuf, NULL, NULL, 0);// 反转字节序
		break;
	case 3:// UTF-8
		cchBuf = MultiByteToWideChar(CP_UTF8, 0, (PCCH)pStream, cb, NULL, 0);
		if (cchBuf == 0)
			return {};
		rsResult.ReSize(cchBuf);
		MultiByteToWideChar(CP_UTF8, 0, (PCCH)pStream, cb, rsResult.Data(), cchBuf);
		break;
	default:
		EckDbgBreak();
		break;
	}

	return rsResult;
}

inline BOOL GetMusicInfo(PCWSTR pszFile, MUSICINFO& mi)
{
	eck::CFile File;
	if (File.Open(pszFile, eck::FCD_ONLYEXISTING, GENERIC_READ, FILE_SHARE_READ) == INVALID_HANDLE_VALUE)
	{
		EckDbgPrintFormatMessage(GetLastError());
		return FALSE;
	}
	DWORD cbFile = File.GetSize32();

	BYTE by[4];
	File >> by;// 读文件头
	if (memcmp(by, "ID3", 3) == 0)// ID3v2
	{
		if (cbFile < sizeof(ID3v2_Header))
			return FALSE;

		eck::CMappingFile2 mf(File);
		eck::CMemReader r(mf.Create(), cbFile);

		ID3v2_Header* pHeader;
		r.SkipPointer(pHeader);
		DWORD cbTotal = SynchSafeIntToDWORD(pHeader->Size);// 28位数据，包括标签头和扩展头
		if (cbTotal > cbFile)
			return FALSE;

		PCVOID pEnd = r.Data() + cbTotal;

		auto pExtHeader = (const ID3v2_ExtHeader*)r.Data();

		if (pHeader->Ver == 3)// 2.3
		{
			if (pHeader->Flags & 0x20)// 有扩展头
				r += (4 + eck::ReverseInteger(*(DWORD*)pExtHeader->ExtHeaderSize));
		}
		else if (pHeader->Ver == 4)// 2.4
		{
			if (pHeader->Flags & 0x20)// 有扩展头
				r += SynchSafeIntToDWORD(pExtHeader->ExtHeaderSize);
			// 2.4里变成了同步安全整数，而且这个尺寸包含了记录尺寸的四个字节
		}

		DWORD cbUnit;
		ID3v2_FrameHeader* pFrame;
		while (r < pEnd)
		{
			r.SkipPointer(pFrame);

			if (pHeader->Ver == 3)
				cbUnit = eck::ReverseInteger(*(DWORD*)pFrame->Size);// 2.3：32位数据，不包括帧头（偏4字节）
			else if (pHeader->Ver == 4)
				cbUnit = SynchSafeIntToDWORD(pFrame->Size);// 2.4：28位数据（同步安全整数）

			if (memcmp(pFrame->ID, "TIT2", 4) == 0)// 标题
			{
				mi.rsTitle = GetMP3ID3v2_ProcString(r, cbUnit);
				r += cbUnit;
			}
			else if (memcmp(pFrame->ID, "TPE1", 4) == 0)// 作者
			{
				mi.rsArtist = GetMP3ID3v2_ProcString(r, cbUnit);
				r += cbUnit;
			}
			else if (memcmp(pFrame->ID, "TALB", 4) == 0)// 专辑
			{
				mi.rsAlbum = GetMP3ID3v2_ProcString(r, cbUnit);
				r += cbUnit;
			}
			else if (memcmp(pFrame->ID, "USLT", 4) == 0)// 不同步歌词
			{
				/*
				<帧头>（帧标识为USLT）
				文本编码						$xx
				自然语言代码					$xx xx xx
				内容描述						<字符串> $00 (00)
				歌词							<字符串>
				*/
				DWORD cb = cbUnit;

				BYTE byEncodeingType;
				r >> byEncodeingType;// 读文本编码

				CHAR byLangCode[3];
				r >> byLangCode;// 读自然语言代码

				int t;
				if (byEncodeingType == 0 || byEncodeingType == 3)// ISO-8859-1或UTF-8
					t = (int)strlen((PCSTR)r.m_pMem) + 1;
				else// UTF-16LE或UTF-16BE
					t = ((int)wcslen((PCWSTR)r.m_pMem) + 1) * sizeof(WCHAR);
				r += t;// 跳过内容描述

				cb -= (t + 4);

				mi.rsLrc = GetMP3ID3v2_ProcString(r, cb, byEncodeingType);
				r += cb;
			}
			else if (memcmp(pFrame->ID, "COMM", 4) == 0)// 备注
			{
				/*
				<帧头>（帧标识为COMM）
				文本编码						$xx
				自然语言代码					$xx xx xx
				备注摘要						<字符串> $00 (00)
				备注							<字符串>
				*/
				DWORD cb = cbUnit;

				BYTE byEncodeingType;
				r >> byEncodeingType;// 读文本编码

				CHAR byLangCode[3];
				r >> byLangCode;// 读自然语言代码

				int t;
				if (byEncodeingType == 0 || byEncodeingType == 3)// ISO-8859-1或UTF-8
					t = (int)strlen((PCSTR)pFrame) + 1;
				else// UTF-16LE或UTF-16BE
					t = ((int)wcslen((PCWSTR)pFrame) + 1) * sizeof(WCHAR);
				r += t;// 跳过备注摘要

				cb -= (t + 4);
				// 此时pFrame指向备注字符串
				mi.rsComment = GetMP3ID3v2_ProcString(r, cb, byEncodeingType);
				r += cb;
			}
			else if (memcmp(pFrame->ID, "APIC", 4) == 0)// 图片
			{
				/*
				<帧头>（帧标识为APIC）
				文本编码                        $xx
				MIME 类型                       <ASCII字符串>$00（如'image/bmp'）
				图片类型                        $xx
				描述                            <字符串>$00(00)
				<图片数据>
				*/
				DWORD cb = cbUnit;

				BYTE byEncodeingType;
				r >> byEncodeingType;// 读文本编码

				int t;
				t = (int)strlen((PCSTR)r.m_pMem);
				r += (t + 2);// 跳过MIME类型字符串和图片类型

				cb -= (t + 3);

				if (byEncodeingType == 0 || byEncodeingType == 3)// ISO-8859-1或UTF-8
					t = (int)strlen((PCSTR)r.m_pMem) + 1;
				else// UTF-16LE或UTF-16BE
					t = ((int)wcslen((PCWSTR)r.m_pMem) + 1) * sizeof(WCHAR);

				r += t;
				cb -= t;// 跳过描述字符串和结尾NULL

				mi.pCoverData = SHCreateMemStream(r, cb);// 创建流对象
				r += cb;
			}
			else
				r += cbUnit;
		}
	}
	else if (memcmp(by, "fLaC", 4) == 0)// Flac
	{
		FLAC_Header Header;
		DWORD cbBlock;
		UINT t;
		char* pBuffer;
		do
		{
			File >> Header;
			cbBlock = Header.bySize[2] | Header.bySize[1] << 8 | Header.bySize[0] << 16;
			switch (Header.by & 0x7F)
			{
			case 4:// 标签信息，注意：这一部分是小端序
			{
				File >> t;// 编码器信息大小
				File += t;// 跳过编码器信息

				UINT uCount;
				File >> uCount;// 标签数量

				for (UINT i = 0; i < uCount; ++i)
				{
					File >> t;// 标签大小

					pBuffer = new char[t + 1];
					File.Read(pBuffer, t);// 读标签
					*(pBuffer + t) = '\0';

					t = MultiByteToWideChar(CP_UTF8, 0, pBuffer, -1, NULL, 0);
					PWSTR pszLabel = new WCHAR[t];
					MultiByteToWideChar(CP_UTF8, 0, pBuffer, -1, pszLabel, t);// 转换编码，UTF-8到UTF-16LE
					delete[] pBuffer;

					int iPos = eck::FindStr(pszLabel, L"=");// 找等号
					if (iPos != eck::INVALID_STR_POS)
					{
						int cch = t - iPos;
						if (eck::FindStr(pszLabel, L"TITLE"))
						{
							mi.rsTitle.ReSize(cch);
							wcscpy(mi.rsTitle.Data(), pszLabel + iPos);
						}
						else if (eck::FindStr(pszLabel, L"ALBUM"))
						{
							mi.rsAlbum.ReSize(cch);
							wcscpy(mi.rsAlbum.Data(), pszLabel + iPos);
						}
						else if (eck::FindStr(pszLabel, L"ARTIST"))
						{
							mi.rsArtist.ReSize(cch);
							wcscpy(mi.rsArtist.Data(), pszLabel + iPos);
						}
						else if (eck::FindStr(pszLabel, L"DESCRIPTION"))
						{
							mi.rsComment.ReSize(cch);
							wcscpy(mi.rsComment.Data(), pszLabel + iPos);
						}
						else if (eck::FindStr(pszLabel, L"LYRICS"))
						{
							mi.rsLrc.ReSize(cch);
							wcscpy(mi.rsLrc.Data(), pszLabel + iPos);
						}
					}

					delete[] pszLabel;
				}
			}
			break;
			case 6:// 图片（大端序）
			{
				File += 4;// 跳过图片类型

				File >> t;// MIME类型字符串长度
				t = eck::ReverseInteger(t);// 大端序字节到整数，下同
				File += t;// 跳过MIME类型字符串

				File >> t;// 描述字符串长度
				t = eck::ReverseInteger(t);
				File += (t + 16);// 跳过描述字符串、宽度、高度、色深、索引图颜色数

				File >> t;// 图片数据长度
				t = eck::ReverseInteger(t);// 图片数据长度

				pBuffer = new char[t];
				File.Read(pBuffer, t);
				mi.pCoverData = SHCreateMemStream((const BYTE*)pBuffer, t);// 创建流对象
				delete[] pBuffer;
			}
			break;
			default:
				File += cbBlock;// 跳过块
			}

		} while (!(Header.by & 0x80));// 检查最高位，判断是不是最后一个块
	}
	return TRUE;
}

class CMyPage :public eck::Dui::CElem
{

};

class CTestDui :public eck::Dui::CDuiWnd
{
public:
	eck::Dui::CLabel m_Label{};
	eck::Dui::CLabel m_Label2{};
	eck::Dui::CLabel m_Label3{};
	eck::Dui::CButton m_Btn{};
	eck::Dui::CList m_List{};

	eck::CD2dImageList m_il{ 80, 80 };

	struct ITEM
	{
		eck::CRefStrW rs;
		int idxImg;
	};
	std::vector<ITEM> m_vItem{};

	LRESULT OnMsg(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) override
	{
		IWICBitmapDecoder* pDecoder;
		IWICBitmap* pWicBmp;
		ID2D1Bitmap* pBitmap;
		switch (uMsg)
		{
		case eck::Dui::WM_DRAGENTER:
		case eck::Dui::WM_DRAGOVER:
		{
			auto p = (eck::Dui::DRAGDROPINFO*)wParam;
			*(p->pdwEffect) = DROPEFFECT_COPY;
		}
		case eck::Dui::WM_DRAGLEAVE:
			return TRUE;
		case eck::Dui::WM_DROP:
		{
			auto p = (eck::Dui::DRAGDROPINFO*)wParam;
			FORMATETC fe = { CF_HDROP, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
			STGMEDIUM sm{ TYMED_HGLOBAL };

			if (p->pDataObj->GetData(&fe, &sm) == S_OK)
			{
				HDROP hDrop = (HDROP)sm.hGlobal;
				UINT uFileCount = DragQueryFileW(hDrop, 0xFFFFFFFF, NULL, 0);
				for (UINT i = 0; i < uFileCount; ++i)
				{
					WCHAR szFile[MAX_PATH];
					DragQueryFileW(hDrop, i, szFile, ARRAYSIZE(szFile));
					MUSICINFO mi;
					GetMusicInfo(szFile, mi);
					int idxImg = -1;
					if(SUCCEEDED( eck::CreateWicBitmapDecoder(mi.pCoverData, pDecoder)))
						if (SUCCEEDED(eck::CreateWicBitmap(pWicBmp, pDecoder, 80, 80)))
							if (SUCCEEDED(GetD2D().GetDC()->CreateBitmapFromWicBitmap(pWicBmp, &pBitmap)))
								idxImg = m_il.AddImage(pBitmap);
					m_vItem.emplace_back(PathFindFileNameW(szFile), idxImg);
					
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

			auto sizez = pBitmap->GetSize();
			m_il.BindRenderTarget(GetD2D().GetDC());
			SetBkgBitmap(pBitmap);

			m_Label3.Create(L"测试标签😍😍", eck::Dui::DES_VISIBLE | eck::Dui::DES_BLURBKG, 0,
				100, 0, 800, 900, NULL, this, NULL);
			eck::Dui::CElem* pElem = &m_Label3;
			m_List.Create(NULL, eck::Dui::DES_VISIBLE, 0,
				50, 70, 600, 600, pElem, this, NULL);
			m_List.SetTextFormat(GetDefTextFormat());
			m_List.SetItemHeight(70.f);
			m_List.SetImageSize(-1.f);
			m_List.SetItemPadding(5.f);
			/*m_vItem.resize(100);
			EckCounter(m_vItem.size(), i)
			{
				m_il.AddImage(pBitmap);
				m_vItem[i].rs = eck::ToStr(i);
				m_vItem[i].pBitmap = pBitmap;
			}
			m_List.SetItemCount((int)m_vItem.size());*/
			m_List.SetImageList(&m_il);
			//m_List.SetInsertMark(5);
			m_List.SetTopExtraSpace(100);
			m_List.SetBottomExtraSpace(100);

			m_Label2.Create(L"测试标签😍😍", eck::Dui::DES_VISIBLE | eck::Dui::DES_BLURBKG | 0, 0,
				0, 0, 600, 100, &m_List, this, NULL);
			m_Label2.SetTextFormat(GetDefTextFormat());

			m_Label.Create(L"我是标签",
				eck::Dui::DES_VISIBLE | eck::Dui::DES_BLURBKG, 0,
				0, 500, 600, 100, &m_List, this, NULL);
			m_Label.SetTextFormat(GetDefTextFormat());

			m_Btn.Create(L"按钮测试按钮测试按钮测试", eck::Dui::DES_VISIBLE, 0,
				100, 300, 300, 70, NULL, this, NULL);
			m_Btn.SetTextFormat(GetDefTextFormat());
			ID2D1Bitmap* pBmp;
			eck::LoadD2dBitmap(LR"(D:\@重要文件\@我的工程\PlayerNew\Res\Tempo.png)",
				GetD2D().GetDC(), pBmp, 54, 54);
			m_Btn.SetImage(pBmp);
			pBmp->Release();

			if (pBitmap)
				pBitmap->Release();

			EnableDragDrop(TRUE);
			return lResult;
		}
		break;
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
			const auto& e = m_vItem[p->idx];
			p->pszText = e.rs.Data();
			p->cchText = e.rs.Size();
			p->idxImg = e.idxImg;
			//p->pImage = e.pBitmap;
			//auto s = e.pBitmap->GetSize();
			//p->cxImage = s.width;
			//p->cyImage = s.height;
		}
		return 0;
		}
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
			IPBF_CENTERSCREEN | IPBF_FIXWIDTH //| IPBF_MULTILINE
			,0,
			0,
			0,
			0,
		};

		//ib.DlgBox(HWnd, &opt);
		//MsgBox(opt.rsInput.Data());
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
		static std::vector<WNDDATA*> data{};
		static std::vector<WNDDATA*> flatdata{};
		static int isortorder = 0;
		static HDC hCDCBK;
		static int cx, cy;
		switch (uMsg)
		{
		case WM_CREATE:
		{
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
			m_iDpi = eck::GetDpi(hWnd);

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
			//while (hMo = FindWindowExW(HWND_MESSAGE, hMo, 0, 0))
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

			//m_Edit.Create(L"", WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL, 0,
			//	0, 0, 200, 100, hWnd, 102);
			//m_Edit.SetFrameType(1);
			//m_lot.Add(&m_Edit, eck::FLF_FIXWIDTH | eck::FLF_FIXHEIGHT);

			m_Btn.Create(L"筛选", WS_CHILD | WS_VISIBLE, 0, 900, 0, 300, 70, hWnd, 101);
			//m_lot.Add(&m_Btn, eck::FLF_FIXWIDTH | eck::FLF_FIXHEIGHT);
			
			RECT rcDui{ 0,0,900,700 };
			m_Dui.Create(L"我是 Dui 窗口", WS_CHILD | WS_VISIBLE, 0, 0, 0, rcDui.right, rcDui.bottom, hWnd, 108);
			m_Dui.Redraw();

			m_hFont = eck::CreateDefFont();
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
						if (!pd->rs[1].IsEmpty())
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
						}

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
			SetWindowPos(hWnd, NULL, prc->left, prc->top, prc->right - prc->left, prc->bottom - prc->top,
				SWP_NOZORDER | SWP_NOACTIVATE);

		}
		break;
		case WM_DESTROY:
			PostQuitMessage(0);
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