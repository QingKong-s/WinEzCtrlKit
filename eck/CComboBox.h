/*
* WinEzCtrlKit Library
*
* CComboBox.h ： 标准组合框
*
* Copyright(C) 2023-2024 QingKong
*/
#pragma once
#include "CWnd.h"

ECK_NAMESPACE_BEGIN
inline constexpr int CDV_COMBOBOX_1 = 0;

#pragma pack(push, ECK_CTRLDATA_ALIGN)
struct CTRLDATA_COMBOBOX
{
	struct ITEM
	{
		int cchText;
		int cy;// 仅设置CBS_OWNERDRAWVARIABLE时有效
		LPARAM lParam;
		// WCHAR szText[];// 长度cchText + 1
	};

	int iVer;
	DWORD cbSize;// 整块数据的大小
	int idxCurrSel;
	int cchCueBanner;
	int cxDropped;
	int cItem;
	int cyItem;
	int cyCombo;
	LCID lcid;
	DWORD dwEditSelStart;
	DWORD dwEditSelEnd;
	int cMinVisible;
	int cxHorzExtent;
	int idxTop;
	BITBOOL bExtendedUI : 1;
	// WCHAR szCurBanner[];// 长度cchCueBanner + 1
	// ITEM Items[];// 长度cItem
};
#pragma pack(pop)

inline constexpr DWORD ComboTypeMask = CBS_SIMPLE | CBS_DROPDOWN | CBS_DROPDOWNLIST;
class CComboBox :public CWnd
{
public:
	ECK_RTTI(CComboBox);

	ECK_CWNDPROP_STYLE(AutoHScroll, CBS_AUTOHSCROLL);
	ECK_CWNDPROP_STYLE(DisableNoScroll, CBS_DISABLENOSCROLL);
	ECK_CWNDPROP_STYLE_MASK(DropDown, CBS_DROPDOWN, ComboTypeMask);
	ECK_CWNDPROP_STYLE_MASK(DropDownList, CBS_DROPDOWNLIST, ComboTypeMask);
	ECK_CWNDPROP_STYLE(HasString, CBS_HASSTRINGS);
	ECK_CWNDPROP_STYLE(LowerCase, CBS_LOWERCASE);
	ECK_CWNDPROP_STYLE(NoIntegralHeight, CBS_NOINTEGRALHEIGHT);
	ECK_CWNDPROP_STYLE(OemConvert, CBS_OEMCONVERT);
	ECK_CWNDPROP_STYLE(OwnerDrawFixed, CBS_OWNERDRAWFIXED);
	ECK_CWNDPROP_STYLE(OwnerDrawVariable, CBS_OWNERDRAWVARIABLE);
	ECK_CWNDPROP_STYLE_MASK(Simple, CBS_SIMPLE, ComboTypeMask);
	ECK_CWNDPROP_STYLE(Sort, CBS_SORT);
	ECK_CWNDPROP_STYLE(UpperCase, CBS_UPPERCASE);

	[[nodiscard]] EckInline constexpr static PCVOID SkipBaseData(PCVOID p)
	{
		const auto* const p2 = (CTRLDATA_COMBOBOX*)CWnd::SkipBaseData(p);
		return PtrStepCb(p2, p2->cbSize);
	}

	ECK_CWND_CREATE;
	HWND Create(PCWSTR pszText, DWORD dwStyle, DWORD dwExStyle,
		int x, int y, int cx, int cy, HWND hParent, HMENU hMenu, PCVOID pData = nullptr) override
	{
		if (pData)
		{
			const auto* const pBase = (CTRLDATA_WND*)pData;
			PreDeserialize(pData);
			IntCreate(pBase->dwExStyle, WC_COMBOBOXW, pBase->Text(), pBase->dwStyle,
				x, y, cx, cy, hParent, hMenu, nullptr, nullptr);
			PostDeserialize(pData);
		}
		else
		{
			IntCreate(0, WC_COMBOBOXW, pszText, dwStyle,
				x, y, cx, cy, hParent, hMenu, nullptr, nullptr);
		}
		return m_hWnd;
	}

	void SerializeData(CRefBin& rb)
	{
		CWnd::SerializeData(rb);
		CRefStrW rs;
		GetCurBanner(rs);
		rb.Reserve(rb.Size() + sizeof(CTRLDATA_COMBOBOX) + rs.ByteSize() + 512);
		const auto pTemp = (CTRLDATA_COMBOBOX*)rb.PushBack(sizeof(CTRLDATA_COMBOBOX));
		pTemp->cchCueBanner = rs.Size();
		const auto ocbHeader = (PCBYTE)pTemp - rb.Data();
		if (rs.IsEmpty())
			*(PWSTR)rb.PushBack(sizeof(WCHAR)) = L'\0';
		else
			wmemcpy((PWSTR)rb.PushBack(rs.ByteSize()), rs.Data(), rs.ByteSize());
		const auto cItem = GetCount();
		const BOOL bOdVar = OwnerDrawVariable;
		EckCounter(cItem, i)
		{
			const auto cch = GetItemTextLength(i);
			const auto pItem = (CTRLDATA_COMBOBOX::ITEM*)
				rb.PushBack(sizeof(CTRLDATA_COMBOBOX::ITEM) + (cch + 1) * sizeof(WCHAR));
			pItem->cchText = cch;
			pItem->lParam = GetItemData(i);
			if (bOdVar)
				pItem->cy = GetItemHeight(i);
			else
				pItem->cy = INT_MIN;
			GetItemText(i, (PWSTR)PtrSkipType(pItem));
		}
		const auto p = (CTRLDATA_COMBOBOX*)(rb.Data() + ocbHeader);
		p->iVer = CDV_COMBOBOX_1;
		p->cbSize = rb.Size() - ocbHeader;
		p->idxCurrSel = GetCurSel();
		p->cxDropped = GetDroppedWidth();
		p->cItem = cItem;
		p->cyItem = GetItemHeight(0);
		p->cyCombo = GetItemHeight(-1);
		p->lcid = GetLocale();
		GetEditSel(&p->dwEditSelStart, &p->dwEditSelEnd);
		p->cMinVisible = GetMinVisible();
		p->cxHorzExtent = GetHorizontalExtent();
		p->idxTop = GetTopIndex();
		p->bExtendedUI = GetExtendUI();
	}

	void PostDeserialize(PCVOID pData) override
	{
		CWnd::PostDeserialize(pData);
		const auto* const p = (CTRLDATA_COMBOBOX*)CWnd::SkipBaseData(pData);
		if (p->iVer != CDV_COMBOBOX_1)
			return;
		SetRedraw(FALSE);
		ResetContent();
		InitStorage(p->cItem, p->cbSize - sizeof(CTRLDATA_COMBOBOX) -
			(p->cchCueBanner + 1) * sizeof(WCHAR) - p->cItem * sizeof(CTRLDATA_COMBOBOX::ITEM));
		if (p->cchCueBanner)
			SetCueBanner((PWSTR)PtrSkipType(p));
		const auto* pItem = (CTRLDATA_COMBOBOX::ITEM*)
			((PWSTR)PtrSkipType(p) + p->cchCueBanner + 1);
		const auto dwStyle = GetStyle();
		const BOOL bOdVar = IsBitSet(dwStyle, CBS_OWNERDRAWVARIABLE);
		const BOOL bShouldInsertStr = ((dwStyle & (CBS_OWNERDRAWFIXED | CBS_OWNERDRAWVARIABLE)) ?
			(dwStyle & CBS_HASSTRINGS) : TRUE);
		EckCounter(p->cItem, i)
		{
			if (bShouldInsertStr)
			{
				InsertString((PCWSTR)PtrSkipType(pItem));
				if (pItem->lParam)
					SetItemData(i, pItem->lParam);
			}
			else
				InsertString(pItem->lParam);
			if (bOdVar)
				SetItemHeight(i, pItem->cy);
			pItem = PtrStepCb(pItem, sizeof(CTRLDATA_COMBOBOX::ITEM) +
				(pItem->cchText + 1) * sizeof(WCHAR));
		}
		SetCurSel(p->idxCurrSel);
		SetDroppedWidth(p->cxDropped);
		SetItemHeight(-1, p->cyCombo);
		if (!bOdVar)
			SetItemHeight(0, p->cyItem);
		SetLocale(p->lcid);
		SetEditSel(p->dwEditSelStart, p->dwEditSelEnd);
		SetMinVisible(p->cMinVisible);
		SetHorizontalExtent(p->cxHorzExtent);
		SetTopIndex(p->idxTop);
		SetExtendUI(p->bExtendedUI);
		SetRedraw(TRUE);
	}

	EckInline int AddString(PCWSTR psz) const
	{
		return (int)SendMsg(CB_ADDSTRING, 0, (LPARAM)psz);
	}

	EckInline int AddString(LPARAM lParam) const
	{
		return (int)SendMsg(CB_ADDSTRING, 0, lParam);
	}

	/// <summary>
	/// 删除项目
	/// </summary>
	/// <param name="idx"></param>
	/// <returns>返回剩余项目数</returns>
	EckInline int DeleteString(int idx) const
	{
		return (int)SendMsg(CB_DELETESTRING, idx, 0);
	}

	/// <summary>
	/// 加入路径
	/// </summary>
	/// <param name="pszPath">路径</param>
	/// <param name="uFlags">DDL_常量</param>
	/// <returns>索引</returns>
	EckInline int Dir(PCWSTR pszPath, UINT uFlags) const
	{
		return (int)SendMsg(CB_DIR, uFlags, (LPARAM)pszPath);
	}

	/// <summary>
	/// 查找项目。
	/// 不区分大小写
	/// </summary>
	/// <param name="pszText">文本，将匹配以该文本开头的项目</param>
	/// <param name="idxStart">起始索引，-1 = 从头搜索整个列表</param>
	/// <returns>索引</returns>
	EckInline int FindString(PCWSTR pszText, int idxStart = -1) const
	{
		return (int)SendMsg(CB_FINDSTRING, idxStart, (LPARAM)pszText);
	}

	/// <summary>
	/// 查找完全匹配项目。
	/// 不区分大小写
	/// </summary>
	/// <param name="pszText">文本，将匹配与该文本完全相同的项目</param>
	/// <param name="idxStart">起始索引，-1 = 从头搜索整个列表</param>
	/// <returns>索引</returns>
	EckInline int FindStringExact(PCWSTR pszText, int idxStart = -1) const
	{
		return (int)SendMsg(CB_FINDSTRINGEXACT, idxStart, (LPARAM)pszText);
	}

	EckInline BOOL GetComboBoxInfo(COMBOBOXINFO* pcbi) const
	{
		return (BOOL)SendMsg(CB_GETCOMBOBOXINFO, 0, (LPARAM)pcbi);
	}

	EckInline int GetCount() const
	{
		return (int)SendMsg(CB_GETCOUNT, 0, 0);
	}

	/// <summary>
	/// 取提示横幅文本
	/// </summary>
	/// <param name="pszBuf">缓冲区</param>
	/// <param name="cchBuf">pszBuf指示的缓冲区大小，以WCHAR计，包含结尾NULL</param>
	/// <returns>成功返回1，失败返回错误代码</returns>
	EckInline int GetCueBanner(PWSTR pszBuf, int cchBuf) const
	{
		return (int)SendMsg(CB_GETCUEBANNER, (WPARAM)pszBuf, cchBuf);
	}

	// 取提示横幅文本
	EckInline int GetCurBanner(CRefStrW& rs)
	{
		rs.ReSize(MAX_PATH);
		const auto cch = GetCueBanner(rs.Data(), MAX_PATH);
		rs.ReSize(cch);
		return cch;
	}

	/// <summary>
	/// 取现行选中项。
	/// 对单选列表框调用返回现行选中项，对多选列表框调用返回焦点项目
	/// </summary>
	/// <returns>索引</returns>
	EckInline int GetCurSel() const
	{
		return (int)SendMsg(CB_GETCURSEL, 0, 0);
	}

	/// <summary>
	/// 取下拉列表框矩形
	/// </summary>
	/// <param name="prc">接收矩形，相对屏幕</param>
	/// <returns>成功返回TRUE，失败返回FALSE</returns>
	EckInline BOOL GetDroppedCtrlRect(RECT* prc) const
	{
		return (BOOL)SendMsg(CB_GETDROPPEDCONTROLRECT, 0, (LPARAM)prc);
	}

	EckInline BOOL GetDroppedState() const
	{
		return (BOOL)SendMsg(CB_GETDROPPEDSTATE, 0, 0);
	}

	/// <summary>
	/// 取下拉列表框最小宽度。
	/// 默认最小宽度为0，列表框宽度为max(最小宽度, 组合框主控件宽度)
	/// </summary>
	/// <returns>成功返回正的最小宽度，失败返回CB_ERR</returns>
	EckInline BOOL GetDroppedWidth() const
	{
		return (BOOL)SendMsg(CB_GETDROPPEDWIDTH, 0, 0);
	}

	EckInline void GetEditSel(DWORD* pdwStart = nullptr, DWORD* pdwEnd = nullptr) const
	{
		SendMsg(CB_GETEDITSEL, (WPARAM)pdwStart, (LPARAM)pdwEnd);
	}

	EckInline BOOL GetExtendUI() const
	{
		return (BOOL)SendMsg(CB_GETEXTENDEDUI, 0, 0);
	}

	EckInline int GetHorizontalExtent() const
	{
		return (int)SendMsg(CB_GETHORIZONTALEXTENT, 0, 0);
	}

	EckInline LPARAM GetItemData(int idx) const
	{
		return SendMsg(CB_GETITEMDATA, idx, 0);
	}

	EckInline int GetItemHeight(int idx) const
	{
		return (int)SendMsg(CB_GETITEMHEIGHT, idx, 0);
	}

	/// <summary>
	/// 取项目文本
	/// </summary>
	/// <param name="idx">项目索引</param>
	/// <param name="pszBuf">缓冲区</param>
	/// <returns>返回字符数（不含结尾NULL），失败返回-1</returns>
	EckInline int GetItemText(int idx, PWSTR pszBuf) const
	{
		return (int)SendMsg(CB_GETLBTEXT, idx, (LPARAM)pszBuf);
	}

	EckInline BOOL GetItemText(int idx, CRefStrW& rs) const
	{
		int cch = GetItemTextLength(idx);
		if (cch <= 0)
			return FALSE;
		rs.ReSize(cch);
		return GetItemText(idx, rs.Data()) >= 0;
	}

	EckInline CRefStrW GetItemText(int idx) const
	{
		CRefStrW rs;
		GetItemText(idx, rs);
		return rs;
	}

	/// <summary>
	/// 取项目文本长度
	/// </summary>
	/// <param name="idx"></param>
	/// <returns>返回字符数（不含结尾NULL）</returns>
	EckInline int GetItemTextLength(int idx) const
	{
		return (int)SendMsg(CB_GETLBTEXTLEN, idx, 0);
	}

	EckInline LCID GetLocale() const
	{
		return (LCID)SendMsg(CB_GETLOCALE, 0, 0);
	}

	EckInline int GetMinVisible() const
	{
		return (int)SendMsg(CB_GETMINVISIBLE, 0, 0);
	}

	EckInline int GetTopIndex() const
	{
		return (int)SendMsg(CB_GETTOPINDEX, 0, 0);
	}

	/// <summary>
	/// 保留空间
	/// </summary>
	/// <param name="cItems">保留项目数</param>
	/// <param name="cbString">保留字符串长度</param>
	/// <returns>成功返回已预分配的项目总数，失败返回CB_ERRSPACE</returns>
	EckInline int InitStorage(int cItems, SIZE_T cbString) const
	{
		return (int)SendMsg(CB_INITSTORAGE, cItems, cbString);
	}

	EckInline int InsertString(PCWSTR psz, int idxPos = -1) const
	{
		return (int)SendMsg(CB_INSERTSTRING, idxPos, (LPARAM)psz);
	}

	EckInline int InsertString(LPARAM lParam, int idxPos = -1) const
	{
		return (int)SendMsg(CB_INSERTSTRING, idxPos, lParam);
	}

	/// <summary>
	/// 置文本输入限制
	/// </summary>
	/// <param name="cch">字符串最长长度，若为0则限制为0x7FFFFFFE</param>
	EckInline void LimitText(int cch = 0) const
	{
		SendMsg(CB_LIMITTEXT, cch, 0);
	}

	EckInline void ResetContent() const
	{
		SendMsg(CB_RESETCONTENT, 0, 0);
	}

	/// <summary>
	/// 查找并选择项目。
	/// 不区分大小写
	/// </summary>
	/// <param name="pszText">文本，将匹配以该文本开头的项目</param>
	/// <param name="idxStart">起始索引，-1 = 从头搜索整个列表</param>
	/// <returns>索引，失败返回CB_ERR</returns>
	EckInline int SelectString(PCWSTR pszText, int idxStart = -1) const
	{
		return (int)SendMsg(CB_SELECTSTRING, idxStart, (LPARAM)pszText);
	}

	/// <summary>
	/// 置提示横幅文本
	/// </summary>
	/// <param name="pszText">文本</param>
	/// <returns>成功返回1，失败返回错误码</returns>
	EckInline int SetCueBanner(PWSTR pszText) const
	{
		return (int)SendMsg(CB_SETCUEBANNER, 0, (LPARAM)pszText);
	}

	EckInline BOOL SetCurSel(int idxSel = -1) const
	{
		int iRet = (int)SendMsg(CB_SETCURSEL, idxSel, 0);
		if (idxSel < 0)
			return TRUE;
		else
			return (iRet != CB_ERR);
	}

	EckInline BOOL SetDroppedWidth(int cx = 0) const
	{
		return (SendMsg(CB_SETDROPPEDWIDTH, cx, 0) != CB_ERR);
	}

	EckInline void SetEditSel(WORD wStart, WORD wEnd) const
	{
		SendMsg(CB_SETEDITSEL, 0, MAKELPARAM(wStart, wEnd));
	}

	EckInline BOOL SetExtendUI(BOOL bExtUI) const
	{
		return (SendMsg(CB_SETEXTENDEDUI, bExtUI, 0) != CB_ERR);
	}

	EckInline void SetHorizontalExtent(int iHorizontalExtent) const
	{
		SendMsg(CB_SETHORIZONTALEXTENT, iHorizontalExtent, 0);
	}

	EckInline BOOL SetItemData(int idx, LPARAM lParam) const
	{
		return (SendMsg(CB_SETITEMDATA, idx, lParam) != CB_ERR);
	}

	/// <summary>
	/// 置项目高度
	/// </summary>
	/// <param name="idx">为0时设置列表项目高度，为-1时设置主控件高度。
	/// 若组合框具有CBS_OWNERDRAWVARIABLE，则该参数指示项目索引</param>
	/// <param name="cy">高度</param>
	/// <returns>成功返回TRUE，失败返回FALSE</returns>
	EckInline BOOL SetItemHeight(int idx, int cy) const
	{
		return (SendMsg(CB_SETITEMHEIGHT, idx, cy) != CB_ERR);
	}

	/// <summary>
	/// 置LCID
	/// </summary>
	/// <param name="lcid"></param>
	/// <returns>成功返回先前的LCID，失败返回CB_ERR</returns>
	EckInline LCID SetLocale(LCID lcid) const
	{
		return (LCID)SendMsg(CB_SETLOCALE, lcid, 0);
	}

	EckInline BOOL SetMinVisible(int cItems) const
	{
		return (BOOL)SendMsg(CB_SETMINVISIBLE, cItems, 0);
	}

	EckInline BOOL SetTopIndex(int idx) const
	{
		return (SendMsg(CB_SETTOPINDEX, idx, 0) != CB_ERR);
	}

	EckInline void ShowDropDown(BOOL bShow) const
	{
		SendMsg(CB_SHOWDROPDOWN, bShow, 0);
	}

	/// <summary>
	/// 置项目高度扩展。
	/// 修复设置高度时有偏差的问题，仅用于设置主控件高度
	/// </summary>
	/// <param name="cy">高度</param>
	/// <returns>成功返回TRUE，失败返回FALSE</returns>
	EckInline BOOL SetItemHeight(int cy) const
	{
		RECT rc;
		GetClientRect(m_hWnd, &rc);
		int iOffset = rc.bottom - (int)SendMsg(CB_GETITEMHEIGHT, -1, 0);
		return (SendMsg(CB_SETITEMHEIGHT, -1, cy - iOffset) != CB_ERR);
	}

	void SetItemString(int idx, PCWSTR pszText) const
	{
		LPARAM lParam = GetItemData(idx);
		int idxNew = InsertString(pszText, idx);
		SetItemData(idxNew, lParam);
		if (idxNew <= idx)
			DeleteString(idx + 1);
		else
			DeleteString(idx);
		SetCurSel(idxNew);
	}
};
ECK_RTTI_IMPL_BASE_INLINE(CComboBox, CWnd);
ECK_NAMESPACE_END