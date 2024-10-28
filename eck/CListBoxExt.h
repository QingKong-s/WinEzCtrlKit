/*
* WinEzCtrlKit Library
*
* CListBoxExt.h ： 列表框扩展
* 封装了无数据列表框并对其功能做了扩展
*
* Copyright(C) 2023 QingKong
*/
#pragma once
#include "CListBox.h"
#include "Utility2.h"

#include <algorithm>

#include <Uxtheme.h>
#include <vsstyle.h>
#include <windowsx.h>

ECK_NAMESPACE_BEGIN

struct LBITEMCOMMINFO
{
	int idxImage;			   // 图像索引
	int iReserved;			   // 未用
	COLORREF crText;		   // 文本颜色
	COLORREF crBK;			   // 背景颜色
	COLORREF crSelText;		   // 选中文本颜色
	COLORREF crSelBK;		   // 选中背景颜色
	LPARAM lParam;			   // 表项数值
	BITBOOL bChecked : 1;	   // 是否检查
	BITBOOL bDisabled : 1;	   // 是否禁止

	LBITEMCOMMINFO()
	{
		ZeroMemory(this, sizeof(*this));
		crText = crBK = crSelText = crSelBK = CLR_DEFAULT;
	}
};

#pragma warning (push)
#pragma warning (disable:26495)// “未初始化变量”
// 只用于运行时保存信息
struct LBITEMINFO
{
	CRefStrW rsCaption;	       // 标题
	CRefStrW rsTip;		       // 提示文本
	HBRUSH hbrBK;		       // 背景画刷
	HBRUSH hbrSelBK;	       // 选中背景画刷
	LBITEMCOMMINFO Info;       // 通用信息
};
#pragma warning (pop)

// 列表框
/*
* 版本1数据布局：
* EXELISTBOXDATA结构
* 项目数据
* 图片组
* 目录（不带结尾NULL）
* 文件过滤器（不带结尾NULL）
*/
#define DATA_VER_LISTBOX_1	1
struct EXELISTBOXDATA
{
	int iVer;				// 版本号
	DWORD dwReserved;		// 保留

	int idxCurrSel;			// 现行选中
	int cyItem;				// 行高
	COLORREF crText;		// 文本颜色
	COLORREF crBK;			// 背景颜色
	COLORREF crSelText;		// 选中文本颜色
	COLORREF crSelBK;		// 选择背景颜色
	int iAlignH;			// 横向对齐
	int iAlignV;			// 纵向对齐
	UINT uFileAttr;			// 文件过滤属性
	FILETIME ftMinTime;		// 文件最小时间，基于协调世界时
	FILETIME ftMaxTime;		// 文件最大时间，基于协调世界时
	int iCheckBoxMode;		// 选择列表框模式
	BITBOOL bToolTip : 1;			// 工具提示
	BITBOOL bEllipsis : 1;			// 省略号
	BITBOOL bBalloonToolTip : 1;	// 气球工具提示
	BITBOOL bAutoSort : 1;			// 自动排序
	BITBOOL bIgnoreDisableItem : 1;	// 忽略禁止的项目 
};

constexpr int c_LBPadding = 3;
#define TTID_LBITEM		20230526'01u

class CListBoxExt :public CListBox
{
public:
	ECK_RTTI(CListBoxExt);

	EXELISTBOXDATA m_InfoEx{};
	//////////图像列表相关
	HIMAGELIST m_hImageList       = nullptr;   // 图像列表句柄
	int m_cxImage                 = 0,
		m_cyImage                 = 0;      // 图像列表尺寸
	//////////文件框相关
	CRefStrW m_rsDir{};
	CRefStrW m_rsFilePattern{};
	//////////通用信息
	HBRUSH m_hbrBK                = nullptr;   // 通用表项背景画刷
	HBRUSH m_hbrSelBK             = nullptr;   // 通用选择表项背景画刷
	int m_idxChecked              = -1;     // 选中的项目，仅单选模式有效
	HTHEME m_hTheme               = nullptr;   // 主题句柄，绘制选择框时用
	int m_cxCheckBox              = 0;      // 选择框尺寸
	std::vector<LBITEMINFO> m_ItemsInfo{};	// 所有项目
	HWND m_hToolTip               = nullptr;   // 工具提示窗口句柄
	TTTOOLINFOW m_ti
	{
		sizeof(TTTOOLINFOW),
		TTF_ABSOLUTE | TTF_TRACK,
		nullptr,
		TTID_LBITEM
	};										// 工具提示信息
	HWND m_hParent				= nullptr;
private:
	void UpdateThemeInfo()
	{
		CloseThemeData(m_hTheme);
		m_hTheme = OpenThemeData(m_hWnd, L"Button");
		HDC hDC = GetDC(m_hWnd);
		SIZE size;
		GetThemePartSize(m_hTheme, hDC, BP_CHECKBOX, CBS_CHECKEDNORMAL, nullptr, TS_DRAW, &size);
		m_cxCheckBox = size.cx;
		ReleaseDC(m_hWnd, hDC);
	}

	void AddFile()
	{
		if (!m_rsDir.Size())
			return;
		DeleteString(-1);

		PWSTR pszPath = (PWSTR)_malloca((
			m_rsDir.Size() +
			m_rsFilePattern.Size() +
			5/*一个反斜杠，三个*.*（通配符为空时用），一个结尾NULL*/) * sizeof(WCHAR));
		assert(pszPath);// 消除警告
		wcscpy(pszPath, m_rsDir.Data());
		PWSTR pszTemp = pszPath + m_rsDir.Size();// 指针指到目录的后面，方便替换通配符

		PWSTR pszFilePattern;
		if (m_rsFilePattern.Size() && m_rsFilePattern.Data())
			pszFilePattern = m_rsFilePattern.Data();
		else
		{
#pragma warning(push)
#pragma warning(disable:6255)// 禁用警告：考虑改用_malloca
			pszFilePattern = (PWSTR)_alloca(4 * sizeof(WCHAR));
#pragma warning(pop)
			assert(pszFilePattern);// 消除警告
			wcscpy(pszFilePattern, L"*.*");
		}

		WIN32_FIND_DATAW wfd;
		HANDLE hFind;
		PWSTR pszDivPos, pszOld = pszFilePattern;
		while (TRUE)
		{
			pszDivPos = wcsstr(pszFilePattern, L"|");
			if (pszDivPos != pszOld && pszDivPos)// 常规情况
			{
				int cch = (int)(pszDivPos - pszOld - 1);
				wcscpy(pszTemp, L"\\");
				wcsncat(pszTemp, pszOld, cch);
				*(pszTemp + cch + 1) = L'\0';
			}
			else if (!pszDivPos)// 找不到下一个分隔符
			{
				wcscpy(pszTemp, L"\\");
				wcscat(pszTemp, pszOld);
			}
			else// 尾部（pszDivPos==pszOld）
				break;

			LBITEMCOMMINFO CommInfo{};
			pszOld = pszDivPos + 1;
			hFind = FindFirstFileW(pszPath, &wfd);
			if (hFind == INVALID_HANDLE_VALUE)
				if (!pszDivPos)
					break;
				else
					continue;
			do
			{
				if (memcmp(wfd.cFileName, L".", 2 * sizeof(WCHAR)) == 0 ||
					memcmp(wfd.cFileName, L"..", 3 * sizeof(WCHAR)) == 0)
					continue;
				if (!m_InfoEx.uFileAttr || wfd.dwFileAttributes & m_InfoEx.uFileAttr)
				{
					if (m_InfoEx.ftMaxTime == m_InfoEx.ftMinTime || IsFILETIMEZero(m_InfoEx.ftMaxTime))
						AddString(wfd.cFileName, nullptr, CommInfo);
					else if (wfd.ftCreationTime > m_InfoEx.ftMinTime && wfd.ftCreationTime < m_InfoEx.ftMaxTime)
						AddString(wfd.cFileName, nullptr, CommInfo);
				}
			} while (FindNextFileW(hFind, &wfd));
			FindClose(hFind);
			if (!pszDivPos)
				break;
		}

		_freea(pszPath);
	}

	int HitTestCheckBox(POINT pt, int* pidxItem = nullptr)
	{
		POINT ptScr = pt;
		ClientToScreen(m_hWnd, &ptScr);
		int idx = LBItemFromPt(m_hWnd, ptScr, FALSE);
		if (pidxItem)
			*pidxItem = idx;
		if (idx < 0)
			return -1;

		RECT rcItem;
		if (SendMessageW(m_hWnd, LB_GETITEMRECT, idx, (LPARAM)&rcItem) == LB_ERR)
			return -1;
		rcItem.left += c_LBPadding;
		rcItem.right = rcItem.left + m_cxCheckBox;
		if (PtInRect(&rcItem, pt))
			return idx;
		else
			return -1;
	}

	/*
	EckInline void OnSelChange()
	{
		EVENT_NOTIFY2 evt(m_dwWinFormID, m_dwUnitID, 0);
		elibstl::NotifySys(NRS_EVENT_NOTIFY2, (DWORD)&evt, 0);
	}

	EckInline BOOL OnBeginDrag(POINT pt)
	{
		m_idxDraggingBegin = LBItemFromPt(m_hWnd, pt, FALSE);
		if (m_idxDraggingBegin >= 0)
		{
			if (m_ItemsInfo[m_idxDraggingBegin].Info.bDisabled)
				return FALSE;
		}

		EVENT_NOTIFY2 evt(m_dwWinFormID, m_dwUnitID, 1);
		evt.m_nArgCount = 3;
		ScreenToClient(m_hWnd, &pt);
		evt.m_arg[0].m_inf.m_int = pt.x;
		evt.m_arg[1].m_inf.m_int = pt.y;

		evt.m_arg[2].m_inf.m_int = m_idxDraggingBegin;
		elibstl::NotifySys(NRS_EVENT_NOTIFY2, (DWORD)&evt, 0);
		if (evt.m_blHasRetVal)
			return evt.m_infRetData.m_bool;
		else
			return TRUE;
	}

	EckInline void OnCancelDrag(POINT pt)
	{
		EVENT_NOTIFY2 evt(m_dwWinFormID, m_dwUnitID, 2);
		evt.m_nArgCount = 2;
		ScreenToClient(m_hWnd, &pt);
		evt.m_arg[0].m_inf.m_int = pt.x;
		evt.m_arg[1].m_inf.m_int = pt.y;
		elibstl::NotifySys(NRS_EVENT_NOTIFY2, (DWORD)&evt, 0);
	}

	EckInline int OnDragging(POINT pt)
	{
		int idx = LBItemFromPt(m_hWnd, pt, TRUE);

		EVENT_NOTIFY2 evt(m_dwWinFormID, m_dwUnitID, 3);
		evt.m_nArgCount = 4;
		ScreenToClient(m_hWnd, &pt);
		evt.m_arg[0].m_inf.m_int = pt.x;
		evt.m_arg[1].m_inf.m_int = pt.y;

		DrawInsert(m_hParent, m_hWnd, idx);
		evt.m_arg[2].m_inf.m_int = idx;

		evt.m_arg[3].m_inf.m_int = m_idxDraggingBegin;
		elibstl::NotifySys(NRS_EVENT_NOTIFY2, (DWORD)&evt, 0);
		if (evt.m_blHasRetVal)
			return evt.m_infRetData.m_int;
		else
			return 0;
	}

	EckInline void OnDropped(POINT pt)
	{
		int idx = LBItemFromPt(m_hWnd, pt, FALSE);
		DrawInsert(m_hParent, m_hWnd, -1);

		EVENT_NOTIFY2 evt(m_dwWinFormID, m_dwUnitID, 4);
		evt.m_nArgCount = 4;
		ScreenToClient(m_hWnd, &pt);
		evt.m_arg[0].m_inf.m_int = pt.x;
		evt.m_arg[1].m_inf.m_int = pt.y;

		evt.m_arg[2].m_inf.m_int = idx;
		evt.m_arg[3].m_inf.m_int = m_idxDraggingBegin;
		elibstl::NotifySys(NRS_EVENT_NOTIFY2, (DWORD)&evt, 0);

		BOOL bSwapItem = (evt.m_blHasRetVal ? evt.m_infRetData.m_bool : TRUE);
		if (!bSwapItem || m_idxDraggingBegin < 0 || idx < 0 || m_idxDraggingBegin == idx)
			return;

		LBMoveItem(m_idxDraggingBegin, idx);
	}
	*/
public:
	CListBoxExt()
	{
		m_InfoEx.idxCurrSel = -1;
		m_InfoEx.iAlignV = 1;

		SetSelClr(0, CLR_DEFAULT);
		SetSelClr(1, CLR_DEFAULT);
		SetClr(0, CLR_DEFAULT);
		SetClr(1, CLR_DEFAULT);
	}

	~CListBoxExt()
	{
		for (auto& x : m_ItemsInfo)
		{
			DeleteObject(x.hbrBK);
			DeleteObject(x.hbrSelBK);
		}

		DeleteObject(m_hbrBK);
		DeleteObject(m_hbrSelBK);

		CloseThemeData(m_hTheme);

		DestroyWindow(m_hToolTip);
	}

	LRESULT OnNotifyMsg(HWND hParent, UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bProcessed) override
	{
		switch (uMsg)
		{
		case WM_MEASUREITEM:
		{
			if (m_InfoEx.cyItem <= 0)
				break;
			bProcessed = TRUE;
			auto pmis = (MEASUREITEMSTRUCT*)lParam;
			pmis->itemHeight = m_InfoEx.cyItem;
		}
		return TRUE;

		case WM_DRAWITEM:
		{
			auto pdis = (DRAWITEMSTRUCT*)lParam;
			if (pdis->itemID == -1)
				break;
			bProcessed = TRUE;
			auto& Item = m_ItemsInfo[pdis->itemID];

			HDC hDC = pdis->hDC;
			// 画背景
			if (IsBitSet(pdis->itemState, ODS_SELECTED)/* && !Item.Info.bDisabled*/)
			{
				if (Item.hbrSelBK)
					FillRect(hDC, &pdis->rcItem, Item.hbrSelBK);
				else
					FillRect(hDC, &pdis->rcItem, m_hbrSelBK);

				if (Item.Info.crSelText != CLR_DEFAULT)
					SetTextColor(hDC, Item.Info.crSelText);
				else if (m_InfoEx.crSelText != CLR_DEFAULT)
					SetTextColor(hDC, m_InfoEx.crSelText);
				else
					SetTextColor(hDC, GetSysColor(COLOR_HIGHLIGHTTEXT));
			}
			else
			{
				if (Item.hbrBK)
					FillRect(hDC, &pdis->rcItem, Item.hbrBK);
				else
					FillRect(hDC, &pdis->rcItem, m_hbrBK);

				if (Item.Info.bDisabled)
					SetTextColor(hDC, GetSysColor(COLOR_GRAYTEXT));
				else if (Item.Info.crText != CLR_DEFAULT)
					SetTextColor(hDC, Item.Info.crText);
				else if (m_InfoEx.crText != CLR_DEFAULT)
					SetTextColor(hDC, m_InfoEx.crText);
				else
					SetTextColor(hDC, GetSysColor(COLOR_WINDOWTEXT));
			}
			// 画选择框
			RECT rc = pdis->rcItem;
			if (m_InfoEx.iCheckBoxMode)
			{
				rc.left += c_LBPadding;
				rc.right = rc.left + m_cxCheckBox;

				int iPartID;
				int iStateID;
				if (m_InfoEx.iCheckBoxMode == 1)
				{
					iPartID = BP_RADIOBUTTON;
					if (m_idxChecked == pdis->itemID)
						iStateID = Item.Info.bDisabled ? RBS_CHECKEDDISABLED : RBS_CHECKEDNORMAL;
					else
						iStateID = Item.Info.bDisabled ? RBS_UNCHECKEDDISABLED : RBS_UNCHECKEDNORMAL;
				}
				else
				{
					iPartID = BP_CHECKBOX;
					if (Item.Info.bChecked)
						iStateID = Item.Info.bDisabled ? CBS_CHECKEDDISABLED : CBS_CHECKEDNORMAL;
					else
						iStateID = Item.Info.bDisabled ? CBS_UNCHECKEDDISABLED : CBS_UNCHECKEDNORMAL;
				}
				DrawThemeBackground(m_hTheme, hDC, iPartID, iStateID, &rc, nullptr);
				rc.right += c_LBPadding;
				rc.left = rc.right;
			}
			else
			{
				rc.left += c_LBPadding;
				rc.right = rc.left;
			}
			// 画图片
			if (m_cxImage)
			{
				if (m_hImageList && Item.Info.idxImage >= 0)
				{
					ImageList_Draw(m_hImageList, Item.Info.idxImage, hDC,
						rc.right,
						rc.top + ((rc.bottom - rc.top) - m_cyImage) / 2,
						ILD_NORMAL | (IsBitSet(pdis->itemState, ODS_SELECTED) ? ILD_SELECTED : 0));
				}
				rc.left += (m_cxImage + c_LBPadding);
			}
			rc.right = pdis->rcItem.right;
			// 画文本
			UINT uDTFlags = DT_NOCLIP | DT_SINGLELINE | (m_InfoEx.bEllipsis ? DT_END_ELLIPSIS : 0);
			switch (m_InfoEx.iAlignH)
			{
			case 0:uDTFlags |= DT_LEFT; break;
			case 1:uDTFlags |= DT_CENTER; break;
			case 2:uDTFlags |= DT_VCENTER; break;
			default:EckDbgBreak(); break;
			}

			switch (m_InfoEx.iAlignV)
			{
			case 0:uDTFlags |= DT_TOP; break;
			case 1:uDTFlags |= DT_VCENTER; break;
			case 2:uDTFlags |= DT_BOTTOM; break;
			default:EckDbgBreak(); break;
			}

			SetBkMode(hDC, TRANSPARENT);
			DrawTextW(hDC, Item.rsCaption.Data(), -1, &rc, uDTFlags);
		}
		return TRUE;
		}

		return CListBox::OnNotifyMsg(hParent, uMsg, wParam, lParam, bProcessed);
	}

	LRESULT OnMsg(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) override
	{
		switch (uMsg)
		{
		case WM_LBUTTONDOWN:// 更新检查框
		{
			if (!m_InfoEx.iCheckBoxMode)
				break;

			POINT pt{ GET_X_LPARAM(lParam),GET_Y_LPARAM(lParam) };
			int idxItem;
			int idx = HitTestCheckBox(pt, &idxItem);
			// 过滤禁止的项目
			if (idxItem < 0)
				break;
			if (m_ItemsInfo[idxItem].Info.bDisabled)
				return 0;
			// 更新检查状态
			if (idx < 0)
				break;
			if (idx >= 0)
				if (m_InfoEx.iCheckBoxMode == 1)
				{
					if (m_idxChecked != idx)
					{
						if (m_idxChecked >= 0)
							RedrawItem(m_idxChecked);
						m_idxChecked = idx;
						RedrawItem(idx);
					}
				}
				else
				{
					ECKBOOLNOT(m_ItemsInfo[idx].Info.bChecked);
					RedrawItem(idx);
				}
		}
		break;

		case WM_KEYDOWN:
		{

		}
		break;

		case WM_LBUTTONDBLCLK:// 连击修复
		{
			POINT pt{ GET_X_LPARAM(lParam),GET_Y_LPARAM(lParam) };
			int idx = HitTestCheckBox(pt);
			if (idx >= 0)
				PostMessageW(hWnd, WM_LBUTTONDOWN, wParam, lParam);
		}
		break;

		case WM_MOUSEMOVE:
		{
			if (!m_InfoEx.bToolTip)
				break;
			TRACKMOUSEEVENT tme;
			tme.cbSize = sizeof(TRACKMOUSEEVENT);
			tme.dwFlags = TME_HOVER | TME_LEAVE;
			tme.dwHoverTime = 400;
			tme.hwndTrack = hWnd;
			TrackMouseEvent(&tme);
		}
		break;

		case WM_MOUSEHOVER:
		{
			if (!m_InfoEx.bToolTip)
				break;
			SendMessageW(m_hToolTip, TTM_TRACKACTIVATE, FALSE, (LPARAM)&m_ti);
			POINT ptScr{ GET_X_LPARAM(lParam),GET_Y_LPARAM(lParam) };
			ClientToScreen(hWnd, &ptScr);
			int idx = LBItemFromPt(hWnd, ptScr, FALSE);
			if (idx < 0)
				break;
			SendMessageW(m_hToolTip, TTM_GETTOOLINFOW, 0, (LPARAM)&m_ti);
			m_ti.lpszText = m_ItemsInfo[idx].rsTip.Data();
			SendMessageW(m_hToolTip, TTM_SETTOOLINFOW, 0, (LPARAM)&m_ti);
			SendMessageW(m_hToolTip, TTM_TRACKPOSITION, 0, MAKELPARAM(ptScr.x, ptScr.y));
			SendMessageW(m_hToolTip, TTM_TRACKACTIVATE, TRUE, (LPARAM)&m_ti);
		}
		break;

		case WM_MOUSELEAVE:
			if (!m_InfoEx.bToolTip)
				break;
			SendMessageW(m_hToolTip, TTM_TRACKACTIVATE, FALSE, (LPARAM)&m_ti);
			break;

		case WM_THEMECHANGED:
			UpdateThemeInfo();
			break;
		}

		return CListBox::OnMsg(hWnd, uMsg, wParam, lParam);
	}

	ECK_CWND_CREATE;
	HWND Create(PCWSTR pszText, DWORD dwStyle, DWORD dwExStyle,
		int x, int y, int cx, int cy, HWND hParent, HMENU hMenu, PCVOID pData = nullptr) override
	{
		dwStyle |= WS_CHILD;

		dwStyle |= (LBS_OWNERDRAWFIXED | LBS_NODATA | LBS_NOTIFY);
		//if (m_Info.bMultiSel)
		//	dwStyle |= LBS_MULTIPLESEL;
		//if (m_Info.bExtSel)
		//	dwStyle |= LBS_EXTENDEDSEL;
		////if (!m_Info.bIntegralHeight)
		//dwStyle |= LBS_NOINTEGRALHEIGHT;
		//if (m_Info.bDisableNoScroll)
		//	dwStyle |= LBS_DISABLENOSCROLL;

		
		m_hWnd = CreateWindowExW(dwExStyle, WC_LISTBOXW, nullptr, dwStyle,
			x, y, cx, cy, hParent, hMenu, nullptr, nullptr);
		m_hParent = hParent;

		m_hToolTip = CreateWindowExW(0, TOOLTIPS_CLASSW, nullptr,
			WS_POPUP | TTS_NOPREFIX | TTS_ALWAYSTIP | (m_InfoEx.bBalloonToolTip ? TTS_BALLOON : 0),
			0, 0, 0, 0, nullptr, nullptr, nullptr, nullptr);

		//if (m_Info.bDragList)
			;// SetDragList(m_Info.bDragList);
		
		UpdateThemeInfo();

		m_ti.hwnd = m_hWnd;
		SendMessageW(m_hToolTip, TTM_ADDTOOL, 0, (LPARAM)&m_ti);
		return m_hWnd;
	}

	int InsertString(PCWSTR pszString, PCWSTR pszTip, const LBITEMCOMMINFO& CommInfo, int iPos = -1)
	{
		int idx = (int)SendMessageW(m_hWnd, LB_INSERTSTRING, iPos, (LPARAM)L"");
		if (idx == LB_ERR)
			return -1;
		LBITEMINFO Item{ pszString,pszTip,nullptr,nullptr,CommInfo };
		if (CommInfo.crBK != CLR_DEFAULT)
			Item.hbrBK = CreateSolidBrush(CommInfo.crBK);
		if (CommInfo.crSelBK != CLR_DEFAULT)
			Item.hbrSelBK = CreateSolidBrush(CommInfo.crSelBK);
		if (!m_InfoEx.bAutoSort)
			if (iPos < 0)
				m_ItemsInfo.push_back(std::move(Item));
			else
				m_ItemsInfo.insert(m_ItemsInfo.begin() + idx, std::move(Item));
		else
		{
			for (int i = 0; i < (int)m_ItemsInfo.size(); ++i)
			{
				if (wcscmp(m_ItemsInfo[i].rsCaption.Data(), pszString) > 0)
				{
					m_ItemsInfo.insert(m_ItemsInfo.begin() + i, std::move(Item));
					goto Ret;
				}
			}
			m_ItemsInfo.push_back(std::move(Item));
		}
	Ret:
		return idx;
	}

	int AddString(PCWSTR pszString, PCWSTR pszTip, const LBITEMCOMMINFO& CommInfo)
	{
		int idx = (int)SendMessageW(m_hWnd, LB_ADDSTRING, 0, (LPARAM)L"");
		if (idx == LB_ERR)
			return -1;
		LBITEMINFO Item{ pszString,pszTip,nullptr,nullptr,CommInfo };
		if (CommInfo.crBK != CLR_DEFAULT)
			Item.hbrBK = CreateSolidBrush(CommInfo.crBK);
		if (CommInfo.crSelBK != CLR_DEFAULT)
			Item.hbrSelBK = CreateSolidBrush(CommInfo.crSelBK);
		if (!m_InfoEx.bAutoSort)
			m_ItemsInfo.push_back(std::move(Item));
		else
		{
			for (int i = 0; i < (int)m_ItemsInfo.size(); ++i)
			{
				if (wcscmp(m_ItemsInfo[i].rsCaption.Data(), pszString) > 0)
				{
					m_ItemsInfo.insert(m_ItemsInfo.begin() + i, std::move(Item));
					goto Ret;
				}
			}
			m_ItemsInfo.push_back(std::move(Item));
		}
	Ret:
		return idx;
	}

	void SetItem(int idx, PCWSTR pszString, PCWSTR pszTip, const LBITEMCOMMINFO& CommInfo)
	{
		LBITEMINFO& Item = m_ItemsInfo[idx];
		Item = { pszString,pszTip,nullptr,nullptr,CommInfo };
		if (CommInfo.crBK != CLR_DEFAULT)
			Item.hbrBK = CreateSolidBrush(CommInfo.crBK);
		if (CommInfo.crSelBK != CLR_DEFAULT)
			Item.hbrSelBK = CreateSolidBrush(CommInfo.crSelBK);
	}

	BOOL DeleteString(int iPos)
	{
		if (iPos < 0)
		{
			SendMessageW(m_hWnd, LB_RESETCONTENT, 0, 0);
			for (auto& x : m_ItemsInfo)
			{
				DeleteObject(x.hbrBK);
				DeleteObject(x.hbrSelBK);
			}
			m_ItemsInfo.resize(0u);
		}
		else if (iPos >= 0 && iPos < (int)m_ItemsInfo.size())
		{
			if (SendMessageW(m_hWnd, LB_DELETESTRING, iPos, 0) == LB_ERR)
				return FALSE;
			auto& Item = m_ItemsInfo[iPos];
			DeleteObject(Item.hbrBK);
			DeleteObject(Item.hbrSelBK);
			m_ItemsInfo.erase(m_ItemsInfo.begin() + iPos);
		}
		Redraw();
		return TRUE;
	}

	BOOL InitStorage(int cItems)
	{
		if (SendMessageW(m_hWnd, LB_INITSTORAGE, cItems, 0) == LB_ERRSPACE)
			return FALSE;
		m_ItemsInfo.reserve(cItems);
		return TRUE;
	}

	EckInline void SwapItem(int idx1, int idx2)
	{
		std::swap(m_ItemsInfo[idx1], m_ItemsInfo[idx2]);
		Redraw();
	}

	EckInline void MoveItem(int idxSrc, int idxDst)
	{
		LBITEMINFO Info = std::move(m_ItemsInfo[idxSrc]);
		if (idxSrc < idxDst)
		{
			m_ItemsInfo.insert(m_ItemsInfo.begin() + idxDst, std::move(Info));
			m_ItemsInfo.erase(m_ItemsInfo.begin() + idxSrc);
		}
		else
		{
			m_ItemsInfo.erase(m_ItemsInfo.begin() + idxSrc);
			m_ItemsInfo.insert(m_ItemsInfo.begin() + idxDst, std::move(Info));
		}
		Redraw();
	}

	void SetClr(int idx, COLORREF cr)
	{
		if (idx)
		{
			m_InfoEx.crBK = cr;
			DeleteObject(m_hbrBK);
			if (cr != CLR_DEFAULT)
				m_hbrBK = CreateSolidBrush(cr);
			else
				m_hbrBK = GetSysColorBrush(COLOR_WINDOW);
		}
		else
			m_InfoEx.crText = cr;
		Redraw();
	}

	EckInline int GetClr(int idx)
	{
		if (idx)
			return m_InfoEx.crBK;
		else
			return m_InfoEx.crText;
	}

	EckInline void SetAutoSort(BOOL bAutoSort)
	{
		m_InfoEx.bAutoSort = bAutoSort;
		if (bAutoSort && m_ItemsInfo.size())
		{
			std::sort(m_ItemsInfo.begin(), m_ItemsInfo.end(),
				[](LBITEMINFO& i1, LBITEMINFO& i2) -> bool
				{
					return wcscmp(i1.rsCaption.Data(), i2.rsCaption.Data()) < 0;
				});
			Redraw();
		}
	}

	EckInline void SetToolTip(BOOL bToolTip)
	{
		m_InfoEx.bToolTip = bToolTip;
	}

	EckInline BOOL GetToolTip()
	{
		return m_InfoEx.bToolTip;
	}

	EckInline void SetDir(PCWSTR pszDir)
	{
		m_rsDir = pszDir;
		AddFile();
	}

	EckInline const CRefStrW& GetDir()
	{
		return m_rsDir;
	}

	EckInline void SetFilePattern(PCWSTR pszFilePattern)
	{
		m_rsFilePattern = pszFilePattern;
		AddFile();
	}

	EckInline const CRefStrW& GetFilePattern()
	{
		return m_rsFilePattern;
	}

	void SetFileAttr(UINT uAttr)
	{
		m_InfoEx.uFileAttr = uAttr;
		AddFile();
	}

	UINT GetFileAttr()
	{
		return m_InfoEx.uFileAttr;
	}

	void SetFileTime(BOOL bMaxTime, DATE date)
	{
		SYSTEMTIME st;
		FILETIME ft;
		VariantTimeToSystemTime(date, &st);
		SystemTimeToFileTime(&st, &ft);
		if (bMaxTime)
			LocalFileTimeToFileTime(&ft, &m_InfoEx.ftMaxTime);
		else
			LocalFileTimeToFileTime(&ft, &m_InfoEx.ftMinTime);
	}

	DATE GetFileTime(BOOL bMaxTime)
	{
		SYSTEMTIME st;
		FILETIME ft;
		DATE date;
		if (bMaxTime)
			FileTimeToLocalFileTime(&m_InfoEx.ftMaxTime, &ft);
		else
			FileTimeToLocalFileTime(&m_InfoEx.ftMinTime, &ft);
		FileTimeToSystemTime(&ft, &st);
		SystemTimeToVariantTime(&st, &date);
		return date;
	}

	HIMAGELIST SetImageList(HIMAGELIST hImageList)
	{
		auto hOld = m_hImageList;
		m_hImageList = hImageList;
		Redraw();
		return hOld;
	}

	EckInline HIMAGELIST GetImageList()
	{
		return m_hImageList;
	}

	void SetSelClr(int idx, COLORREF cr)
	{
		if (idx)
		{
			m_InfoEx.crSelBK = cr;
			DeleteObject(m_hbrSelBK);
			if (cr != CLR_DEFAULT)
				m_hbrSelBK = CreateSolidBrush(cr);
			else
				m_hbrSelBK = GetSysColorBrush(COLOR_HIGHLIGHT);// 可以删除系统颜色画刷
		}
		else
			m_InfoEx.crSelText = cr;
		Redraw();
	}

	EckInline COLORREF GetSelClr(int idx)
	{
		if (idx)
			return m_InfoEx.crSelBK;
		else
			return m_InfoEx.crSelText;
	}

	EckInline void SetEllipsis(BOOL bEllipsis)
	{
		m_InfoEx.bEllipsis = bEllipsis;
	}

	EckInline BOOL GetEllipsis()
	{
		return m_InfoEx.bEllipsis;
	}

	EckInline void SetAlign(BOOL bHAlign, int iAlign)
	{
		if (bHAlign)
			m_InfoEx.iAlignH = iAlign;
		else
			m_InfoEx.iAlignV = iAlign;
	}

	EckInline int GetAlign(BOOL bHAlign)
	{
		if (bHAlign)
			return m_InfoEx.iAlignH;
		else
			return m_InfoEx.iAlignV;
	}

	EckInline void SetCheckBoxMode(int iCheckBoxMode)
	{
		m_InfoEx.iCheckBoxMode = iCheckBoxMode;
		Redraw();
	}

	EckInline int GetCheckBoxMode()
	{
		return m_InfoEx.iCheckBoxMode;
	}

	EckInline void SetBalloonToolTip(BOOL bBalloonToolTip)
	{
		m_InfoEx.bBalloonToolTip = bBalloonToolTip;
		ModifyWindowStyle(m_hToolTip, bBalloonToolTip ? TTS_BALLOON : 0, TTS_BALLOON);
	}

	EckInline BOOL GetBalloonToolTip()
	{
		return m_InfoEx.bBalloonToolTip;
	}
};
ECK_RTTI_IMPL_BASE_INLINE(CListBoxExt, CListBox);
ECK_NAMESPACE_END