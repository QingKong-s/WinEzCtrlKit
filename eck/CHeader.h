/*
* WinEzCtrlKit Library
*
* CHeader.h ： 标准表头
*
* Copyright(C) 2023-2024 QingKong
*/
#pragma once
#include "CWnd.h"

ECK_NAMESPACE_BEGIN
struct HEADER_FILTER
{
	int iType;
	union
	{
		int i;
		SYSTEMTIME st;
		HD_TEXTFILTERW str;
	};
};

class CHeader :public CWnd
{
public:
	ECK_RTTI(CHeader);

	ECK_CWND_NOSINGLEOWNER(CHeader);

	class ItemsProxy;
	class ItemProxy
	{
		friend class CHeader::ItemsProxy;
	protected:
		CHeader& Hdr;
		int idx;
	private:
		ItemProxy(CHeader& Hdr, int idx) :Hdr{ Hdr }, idx{ idx } {}
	public:
		ECKPROP(GetWidth, SetWidth) int Width;
		ECKPROP(GetText, SetText) CRefStrW Text;
		ECKPROP_W(SetText) PCWSTR TextP;
		ECKPROP(GetBitmap, SetBitmap) HBITMAP Bitmap;
		ECKPROP(GetFormat, SetFormat) int Format;
		ECKPROP(GetLParam, SetLParam) LPARAM LParam;
		ECKPROP(GetImage, SetImage) int Image;
		ECKPROP(GetOrder, SetOrder) int Order;
		ECKPROP(GetFilterText, SetFilter) CRefStrW FilterText;
		ECKPROP_W(SetFilter) PCWSTR FilterTextP;
		ECKPROP(GetFilterNumber, SetFilter) int FilterNumber;
		ECKPROP(GetFilterDate, SetFilter) SYSTEMTIME FilterDate;
		ECKPROP(GetState, SetState) UINT State;
		ECKPROP_R(GetRect) RECT Rect;

		void SetWidth(int cx) const
		{
			HDITEMW hdi;
			hdi.mask = HDI_WIDTH;
			hdi.cxy = cx;
			Hdr.SetItem(idx, &hdi);
		}

		int GetWidth() const
		{
			HDITEMW hdi;
			hdi.mask = HDI_WIDTH;
			Hdr.GetItem(idx, &hdi);
			return hdi.cxy;
		}

		void SetText(PCWSTR pszText) const
		{
			HDITEMW hdi;
			hdi.mask = HDI_TEXT;
			hdi.pszText = (PWSTR)pszText;
			Hdr.SetItem(idx, &hdi);
		}

		void SetText(const CRefStrW& rsText) const { SetText(rsText.Data()); }

		CRefStrW GetText() const
		{
			CRefStrW rs{ MAX_PATH };
			HDITEMW hdi;
			hdi.mask = HDI_TEXT;
			hdi.pszText = rs.Data();
			hdi.cchTextMax = rs.Size();
			Hdr.GetItem(idx, &hdi);
			rs.ReCalcLen();
			return rs;
		}

		void SetBitmap(HBITMAP hbm) const
		{
			HDITEMW hdi;
			hdi.mask = HDI_BITMAP;
			hdi.hbm = hbm;
			Hdr.SetItem(idx, &hdi);
		}

		HBITMAP GetBitmap() const
		{
			HDITEMW hdi;
			hdi.mask = HDI_BITMAP;
			Hdr.GetItem(idx, &hdi);
			return hdi.hbm;
		}

		void SetFormat(int iFmt) const
		{
			HDITEMW hdi;
			hdi.mask = HDI_FORMAT;
			hdi.fmt = iFmt;
			Hdr.SetItem(idx, &hdi);
		}

		int GetFormat() const
		{
			HDITEMW hdi;
			hdi.mask = HDI_FORMAT;
			Hdr.GetItem(idx, &hdi);
			return hdi.fmt;
		}

		void SetLParam(LPARAM lParam) const
		{
			HDITEMW hdi;
			hdi.mask = HDI_LPARAM;
			hdi.lParam = lParam;
			Hdr.SetItem(idx, &hdi);
		}

		LPARAM GetLParam() const
		{
			HDITEMW hdi;
			hdi.mask = HDI_LPARAM;
			Hdr.GetItem(idx, &hdi);
			return hdi.lParam;
		}

		void SetImage(int iImage) const
		{
			HDITEMW hdi;
			hdi.mask = HDI_IMAGE;
			hdi.iImage = iImage;
			Hdr.SetItem(idx, &hdi);
		}

		int GetImage() const
		{
			HDITEMW hdi;
			hdi.mask = HDI_IMAGE;
			Hdr.GetItem(idx, &hdi);
			return hdi.iImage;
		}

		void SetOrder(int iOrder) const
		{
			HDITEMW hdi;
			hdi.mask = HDI_ORDER;
			hdi.iOrder = iOrder;
			Hdr.SetItem(idx, &hdi);
		}

		int GetOrder() const
		{
			HDITEMW hdi;
			hdi.mask = HDI_ORDER;
			Hdr.GetItem(idx, &hdi);
			return hdi.iOrder;
		}

		void SetFilter(PCWSTR pszFilter) const
		{
			HD_TEXTFILTERW hdtf;
			hdtf.pszText = (PWSTR)pszFilter;
			hdtf.cchTextMax = 0;
			HDITEMW hdi;
			hdi.mask = HDI_FILTER;
			hdi.type = HDFT_ISSTRING;
			hdi.pvFilter = &hdtf;
			Hdr.SetItem(idx, &hdi);
		}

		void SetFilter(const CRefStrW& rsFilter) const { SetFilter(rsFilter.Data()); }

		CRefStrW GetFilterText() const
		{
			CRefStrW rs{ MAX_PATH };
			HD_TEXTFILTERW hdtf;
			hdtf.pszText = rs.Data();
			hdtf.cchTextMax = rs.Size();
			HDITEMW hdi;
			hdi.mask = HDI_FILTER;
			hdi.type = HDFT_ISSTRING;
			hdi.pvFilter = &hdtf;
			if (!Hdr.GetItem(idx, &hdi))
				return {};
			rs.ReCalcLen();
			return rs;
		}

		void SetFilter(int i) const
		{
			HDITEMW hdi;
			hdi.mask = HDI_FILTER;
			hdi.type = HDFT_ISNUMBER;
			hdi.pvFilter = &i;
			Hdr.SetItem(idx, &hdi);
		}

		int GetFilterNumber() const
		{
			int i{};
			HDITEMW hdi;
			hdi.mask = HDI_FILTER;
			hdi.type = HDFT_ISNUMBER;
			hdi.pvFilter = &i;
			Hdr.GetItem(idx, &hdi);
			return i;
		}

		void SetFilter(SYSTEMTIME st) const
		{
			HDITEMW hdi;
			hdi.mask = HDI_FILTER;
			hdi.type = HDFT_ISDATE;
			hdi.pvFilter = &st;
			Hdr.SetItem(idx, &hdi);
		}

		SYSTEMTIME GetFilterDate() const
		{
			SYSTEMTIME st{};
			HDITEMW hdi;
			hdi.mask = HDI_FILTER;
			hdi.type = HDFT_ISDATE;
			hdi.pvFilter = &st;
			Hdr.GetItem(idx, &hdi);
			return st;
		}

		void SetState(UINT uState) const
		{
			HDITEMW hdi;
			hdi.mask = HDI_STATE;
			hdi.state = uState;
			Hdr.SetItem(idx, &hdi);
		}

		UINT GetState() const
		{
			HDITEMW hdi;
			hdi.mask = HDI_STATE;
			Hdr.GetItem(idx, &hdi);
			return hdi.state;
		}

		RECT GetRect() const
		{
			RECT rc{};
			Hdr.GetItemRect(idx, &rc);
			return rc;
		}

		void operator=(const ItemProxy& x)
		{
			HDITEMW hdi;
			hdi.mask = HDI_BITMAP | HDI_FORMAT | HDI_IMAGE | HDI_LPARAM |
				HDI_TEXT | HDI_WIDTH | HDI_FILTER;
			WCHAR szBuf[MAX_PATH];
			hdi.pszText = szBuf;
			hdi.cchTextMax = MAX_PATH;
			hdi.pvFilter = nullptr;
			if (!x.Hdr.GetItem(x.idx, &hdi))
				return;
			if (hdi.type != HDFT_HASNOVALUE)
			{
				hdi.mask &= ~HDI_FILTER;
				Hdr.SetItem(idx, &hdi);
				HEADER_FILTER hf;
				hf.str.pszText = szBuf;
				hf.str.cchTextMax = MAX_PATH;
				hdi.mask = HDI_FILTER;
				hdi.pvFilter = &hf.str;
				x.Hdr.GetItem(idx, &hdi);
			}
			Hdr.SetItem(idx, &hdi);
		}
	};

	class ItemsProxy
	{
		friend class CHeader::ItemProxy;
		friend class CHeader;
	protected:
		CHeader& Hdr;

		ItemsProxy(CHeader& Hdr) :Hdr{ Hdr } {}
	public:
		ItemProxy operator[](int idx) const { return ItemProxy{ Hdr, idx }; }
	};
public:
	ECKPROP(GetItemsProxy, SetItemsProxy)	ItemsProxy Items;
	ECKPROP_R(GetItemCount)					int ItemsCount;
	ECKPROP(GetImageList, SetImageList)		HIMAGELIST ImageList;
	ECKPROP(GetOrderArray, SetOrderArray)	std::vector<int> OrderArray;

	ECK_CWND_CREATE;
	HWND Create(PCWSTR pszText, DWORD dwStyle, DWORD dwExStyle,
		int x, int y, int cx, int cy, HWND hParent, HMENU hMenu, PCVOID pData = nullptr) override
	{
		return IntCreate(dwExStyle, WC_HEADERW, pszText, dwStyle,
			x, y, cx, cy, hParent, hMenu, nullptr, nullptr);
	}

	/// <summary>
	/// 清除筛选器
	/// </summary>
	/// <param name="idx">列索引，若为-1则清除所有筛选器</param>
	/// <returns></returns>
	EckInline BOOL ClearFilter(int idx) const
	{
		return (BOOL)SendMsg(HDM_CLEARFILTER, idx, 0);
	}

	EckInline HIMAGELIST CreateDragImage(int idx) const
	{
		return (HIMAGELIST)SendMsg(HDM_CREATEDRAGIMAGE, idx, 0);
	}

	EckInline BOOL DeleteItem(int idx) const
	{
		return (BOOL)SendMsg(HDM_DELETEITEM, idx, 0);
	}

	EckInline BOOL EditFilter(int idx, BOOL bDiscardUserInput) const
	{
		return (BOOL)SendMsg(HDM_CLEARFILTER, idx, bDiscardUserInput);
	}

	EckInline int GetBitmapMargin() const
	{
		return (int)SendMsg(HDM_GETBITMAPMARGIN, 0, 0);
	}

	EckInline int GetFocusItem() const
	{
		return (int)SendMsg(HDM_GETFOCUSEDITEM, 0, 0);
	}

	EckInline HIMAGELIST GetImageList(UINT uType) const
	{
		return (HIMAGELIST)SendMsg(HDM_GETIMAGELIST, uType, 0);
	}

	EckInline HIMAGELIST GetImageList() const { return GetImageList(HDSIL_NORMAL); }

	EckInline BOOL GetItem(int idx, HDITEMW* phdi) const
	{
		return (BOOL)SendMsg(HDM_GETITEMW, idx, (LPARAM)phdi);
	}

	EckInline int GetItemCount() const
	{
		return (int)SendMsg(HDM_GETITEMCOUNT, 0, 0);
	}

	/// <summary>
	/// 取项目拆分按钮矩形
	/// </summary>
	/// <param name="idx">项目索引</param>
	/// <param name="prc">矩形指针，相对控件父窗口</param>
	/// <returns>成功返回TRUE，失败返回FALSE</returns>
	EckInline BOOL GetItemDropDownRect(int idx, RECT* prc) const
	{
		return (BOOL)SendMsg(HDM_GETITEMDROPDOWNRECT, idx, (LPARAM)prc);
	}

	/// <summary>
	/// 取项目矩形
	/// </summary>
	/// <param name="idx">项目索引</param>
	/// <param name="prc">矩形指针，相对控件父窗口</param>
	/// <returns>成功返回TRUE，失败返回FALSE</returns>
	EckInline BOOL GetItemRect(int idx, RECT* prc) const
	{
		return (BOOL)SendMsg(HDM_GETITEMRECT, idx, (LPARAM)prc);
	}

	EckInline BOOL GetOrderArray(int* piOrder, size_t cBuf) const
	{
		return (BOOL)SendMsg(HDM_GETORDERARRAY, cBuf, (LPARAM)piOrder);
	}

	EckInline BOOL GetOrderArray(std::vector<int>& vOrder) const
	{
		vOrder.resize(GetItemCount());
		return GetOrderArray(vOrder.data(), vOrder.size());
	}

	// For getter
	std::vector<int> GetOrderArray() const
	{
		std::vector<int> vOrder;
		GetOrderArray(vOrder);
		return vOrder;
	}

	/// <summary>
	/// 取溢出按钮矩形
	/// </summary>
	/// <param name="prc">矩形指针，相对屏幕</param>
	/// <returns>成功返回TRUE，失败返回FALSE</returns>
	EckInline BOOL GetOverFlowRect(RECT* prc) const
	{
		return (BOOL)SendMsg(HDM_GETOVERFLOWRECT, 0, (LPARAM)prc);
	}

	EckInline int HitTest(HDHITTESTINFO* phdhti) const
	{
		return (int)SendMsg(HDM_HITTEST, 0, (LPARAM)phdhti);
	}

	EckInline int InsertItem(int idx, HDITEMW* phdi) const
	{
		return (int)SendMsg(HDM_INSERTITEMW, idx, (LPARAM)phdi);
	}

	EckInline int InsertItem(PCWSTR pszText, int idx = -1, int cxItem = -1,
		int idxImage = -1, int iFmt = HDF_LEFT, LPARAM lParam = 0) const
	{
		if (idx < 0)
			idx = INT_MAX;
		HDITEMW hdi;
		hdi.mask = HDI_TEXT | HDI_FORMAT | HDI_LPARAM;
		hdi.fmt = iFmt;
		hdi.lParam = lParam;
		hdi.pszText = (PWSTR)pszText;
		if (cxItem >= 0)
		{
			hdi.mask |= HDI_WIDTH;
			hdi.cxy = cxItem;
		}

		if (idxImage >= 0)
		{
			hdi.mask |= HDI_IMAGE;
			hdi.iImage = idxImage;
		}

		return InsertItem(idx, &hdi);
	}

	EckInline BOOL Layout(HDLAYOUT* phdl) const
	{
		return (BOOL)SendMsg(HDM_LAYOUT, 0, (LPARAM)phdl);
	}

	EckInline int OrderToIndex(int iOrder) const
	{
		return (int)SendMsg(HDM_ORDERTOINDEX, iOrder, 0);
	}

	EckInline int SetBitmapMargin(int iMargin) const
	{
		return (int)SendMsg(HDM_SETBITMAPMARGIN, iMargin, 0);
	}

	EckInline int SetFilterChangeTimeout(int iTimeout) const
	{
		return (int)SendMsg(HDM_SETFILTERCHANGETIMEOUT, 0, iTimeout);
	}

	EckInline BOOL SetFocusedItem(int idx) const
	{
		return (BOOL)SendMsg(HDM_SETFOCUSEDITEM, 0, idx);
	}

	EckInline int SetHotDivider(int idxDivider) const
	{
		return (int)SendMsg(HDM_SETHOTDIVIDER, FALSE, idxDivider);
	}

	EckInline int SetHotDivider(POINT ptCursor) const
	{
		return (int)SendMsg(HDM_SETHOTDIVIDER, TRUE, MAKELPARAM(ptCursor.x, ptCursor.y));
	}

	EckInline HIMAGELIST SetImageList(HIMAGELIST hImageList, UINT uType) const
	{
		return (HIMAGELIST)SendMsg(HDM_SETIMAGELIST, uType, (LPARAM)hImageList);
	}

	EckInline HIMAGELIST SetImageList(HIMAGELIST hImageList) const { return SetImageList(hImageList, HDSIL_NORMAL); }

	EckInline BOOL SetItem(int idx, HDITEMW* phdi) const
	{
		return (BOOL)SendMsg(HDM_SETITEMW, idx, (LPARAM)phdi);
	}

	EckInline BOOL SetOrderArray(const int* piOrder) const
	{
		return (BOOL)SendMsg(HDM_SETORDERARRAY, GetItemCount(), (LPARAM)piOrder);
	}

	// For setter
	EckInline BOOL SetOrderArray(const std::vector<int>& vOrder) const
	{
		return SetOrderArray(vOrder.data());
	}

	void RadioSetSortMark(int idx, int iFmt) const
	{
		HDITEMW hdi;
		hdi.mask = HDI_FORMAT;
		int cItems = GetItemCount();
		EckCounter(cItems, i)
		{
			GetItem(i, &hdi);
			hdi.fmt &= ~(HDF_SORTDOWN | HDF_SORTUP);
			if (i == idx)
				hdi.fmt |= iFmt;
			SetItem(i, &hdi);
		}
	}

	BOOL SetItemFilter(int idx, HEADER_FILTER& hf) const
	{
		HDITEMW hdi;
		hdi.mask = HDI_FILTER;
		hdi.type = hf.iType;
		switch (hf.iType)
		{
		case HDFT_ISSTRING:
			hdi.pvFilter = &hf.str;
			break;
		case HDFT_ISNUMBER:
			hdi.pvFilter = &hf.i;
			break;
		case HDFT_ISDATE:
			hdi.pvFilter = &hf.st;
			break;
		}
		return SetItem(idx, &hdi);
	}

	BOOL GetItemFilter(int idx, HEADER_FILTER& hf) const
	{
		hf.iType = HDFT_HASNOVALUE;
		HDITEMW hdi;
		hdi.mask = HDI_FILTER;
		hdi.type = HDFT_HASNOVALUE;
		hdi.pvFilter = nullptr;
		if (!GetItem(idx, &hdi))
			return FALSE;
		if (hdi.type == HDFT_HASNOVALUE)
			return TRUE;
		hf.iType = hdi.type;
		switch (hdi.type)
		{
		case HDFT_ISSTRING:
			hdi.pvFilter = &hf.str;
			break;
		case HDFT_ISNUMBER:
			hdi.pvFilter = &hf.i;
			break;
		case HDFT_ISDATE:
			hdi.pvFilter = &hf.st;
			break;
		}
		return GetItem(idx, &hdi);
	}

	void SetItemsProxy(ItemsProxy is)
	{
		if (&is.Hdr == this)
			return;
		const int cItem = is.Hdr.ItemsCount;
		int cItemCur = ItemsCount;
		if (cItem < cItemCur)
		{
			for (int i = cItemCur - 1; i >= cItem; --i)
				DeleteItem(i);
			cItemCur = cItem;
		}
		WCHAR szBuf[MAX_PATH];
		HDITEMW hdi{};
		hdi.mask = HDI_BITMAP | HDI_FORMAT | HDI_IMAGE | HDI_LPARAM |
			HDI_TEXT | HDI_WIDTH;
		int i{};
		for (; i < std::min(cItem, cItemCur); ++i)
		{
			hdi.pszText = szBuf;
			hdi.cchTextMax = MAX_PATH;
			is.Hdr.GetItem(i, &hdi);
			SetItem(i, &hdi);
		}
		for (; i < cItem; ++i)
		{
			hdi.pszText = szBuf;
			hdi.cchTextMax = MAX_PATH;
			is.Hdr.GetItem(i, &hdi);
			InsertItem(i, &hdi);
		}
		const auto pOrder = (int*)_malloca(cItem * sizeof(int));
		EckCheckMem(pOrder);
		is.Hdr.GetOrderArray(pOrder, cItem);
		SetOrderArray(pOrder);
		_freea(pOrder);
	}

	EckInline ItemsProxy GetItemsProxy() { return ItemsProxy{ *this }; }

	void DeleteAllItems() const
	{
		const int cItem = GetItemCount();
		for (int i = cItem - 1; i >= 0; --i)
			DeleteItem(i);
	}
};
ECK_RTTI_IMPL_BASE_INLINE(CHeader, CWnd);
ECK_NAMESPACE_END