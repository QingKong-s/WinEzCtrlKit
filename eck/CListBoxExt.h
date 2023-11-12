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
#include "CSubclassMgr.h"

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
	SUBCLASS_MGR_DECL(CListBoxExt)
	SUBCLASS_REF_MGR_DECL(CListBoxExt, ObjRecorderRefPlaceholder)
public:
	EXELISTBOXDATA m_InfoEx{};
	//////////图像列表相关
	HIMAGELIST m_hImageList       = NULL;   // 图像列表句柄
	int m_cxImage                 = 0,
		m_cyImage                 = 0;      // 图像列表尺寸
	//////////文件框相关
	CRefStrW m_rsDir{};
	CRefStrW m_rsFilePattern{};
	//////////通用信息
	HBRUSH m_hbrBK                = NULL;   // 通用表项背景画刷
	HBRUSH m_hbrSelBK             = NULL;   // 通用选择表项背景画刷
	int m_idxChecked              = -1;     // 选中的项目，仅单选模式有效
	HTHEME m_hTheme               = NULL;   // 主题句柄，绘制选择框时用
	int m_cxCheckBox              = 0;      // 选择框尺寸
	std::vector<LBITEMINFO> m_ItemsInfo{};	// 所有项目
	HWND m_hToolTip               = NULL;   // 工具提示窗口句柄
	TTTOOLINFOW m_ti
	{
		sizeof(TTTOOLINFOW),
		TTF_ABSOLUTE | TTF_TRACK,
		NULL,
		TTID_LBITEM
	};										// 工具提示信息
	HWND m_hParent				= NULL;
private:
	void UpdateThemeInfo();

	void AddFile();

	int HitTestCheckBox(POINT pt, int* pidxItem = NULL);

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
	static LRESULT CALLBACK SubclassProc_Parent(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
		UINT_PTR uIdSubclass, DWORD_PTR dwRefData);

	static LRESULT CALLBACK SubclassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
		UINT_PTR uIdSubclass, DWORD_PTR dwRefData);
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

	HWND Create(PCWSTR pszText, DWORD dwStyle, DWORD dwExStyle,
		int x, int y, int cx, int cy, HWND hParent, int nID, PCVOID pData = NULL)
	{
		dwStyle |= WS_CHILD;

		dwStyle |= (LBS_OWNERDRAWFIXED | LBS_NODATA | LBS_NOTIFY);
		if (m_Info.bMultiSel)
			dwStyle |= LBS_MULTIPLESEL;
		if (m_Info.bExtSel)
			dwStyle |= LBS_EXTENDEDSEL;
		//if (!m_Info.bIntegralHeight)
		dwStyle |= LBS_NOINTEGRALHEIGHT;
		if (m_Info.bDisableNoScroll)
			dwStyle |= LBS_DISABLENOSCROLL;

		
		m_hWnd = CreateWindowExW(dwExStyle, WC_LISTBOXW, NULL, dwStyle,
			x, y, cx, cy, hParent, i32ToP<HMENU>(nID), NULL, NULL);
		m_SM.AddSubclass(m_hWnd, this);
		m_hParent = hParent;
		m_SMRef.AddRef(hParent, ObjRecorderRefPlaceholderVal);

		m_hToolTip = CreateWindowExW(0, TOOLTIPS_CLASSW, NULL,
			WS_POPUP | TTS_NOPREFIX | TTS_ALWAYSTIP | (m_InfoEx.bBalloonToolTip ? TTS_BALLOON : 0),
			0, 0, 0, 0, NULL, NULL, NULL, NULL);

		if (m_Info.bDragList)
			SetDragList(m_Info.bDragList);// 必须在子类化之前
		
		UpdateThemeInfo();

		m_ti.hwnd = m_hWnd;
		SendMessageW(m_hToolTip, TTM_ADDTOOL, 0, (LPARAM)&m_ti);
		return m_hWnd;
	}

	int InsertString(PCWSTR pszString, PCWSTR pszTip, const LBITEMCOMMINFO& CommInfo, int iPos = -1);

	int AddString(PCWSTR pszString, PCWSTR pszTip, const LBITEMCOMMINFO& CommInfo);

	void SetItem(int idx, PCWSTR pszString, PCWSTR pszTip, const LBITEMCOMMINFO& CommInfo);

	BOOL DeleteString(int iPos);

	BOOL InitStorage(int cItems);

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
ECK_NAMESPACE_END