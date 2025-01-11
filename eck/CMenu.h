#pragma once
#include "ECK.h"
#include "CRefStr.h"
#include "CRefBin.h"

ECK_NAMESPACE_BEGIN
class CMenu
{
private:
	HMENU m_hMenu = nullptr;

	EckInline static constexpr UINT PosBool2UINT(BOOL bPosition) { return bPosition ? MF_BYPOSITION : MF_BYCOMMAND; }

	static void ForMenuItemSave(CRefBin& rb, HMENU hMenu)
	{
		MENUITEMINFOW mii;
		mii.cbSize = sizeof(mii);
		PWSTR pszBuf;
		ITEM* pItem;

		const int cItem = GetMenuItemCount(hMenu);
		EckCounter(cItem, i)
		{
			mii.fMask = MIIM_DATA | MIIM_FTYPE | MIIM_ID | MIIM_STATE | MIIM_STRING | MIIM_SUBMENU;
			mii.cch = 0;
			mii.dwTypeData = nullptr;
			GetMenuItemInfoW(hMenu, i, TRUE, &mii);
			++mii.cch;
			pItem = (ITEM*)rb.PushBack(sizeof(ITEM) + mii.cch * sizeof(WCHAR));
			pszBuf = (PWSTR)(pItem + 1);
			pItem->uState = mii.fState;
			pItem->uType = mii.fType;
			pItem->uID = mii.wID;
			pItem->uData = (ULONGLONG)mii.dwItemData;
			pItem->cchText = mii.cch - 1;
			if (mii.hSubMenu)
				pItem->cSubItem = GetMenuItemCount(mii.hSubMenu);
			else
				pItem->cSubItem = 0;

			mii.dwTypeData = pszBuf;
			mii.fMask = MIIM_STRING;
			GetMenuItemInfoW(hMenu, i, TRUE, &mii);
			*(pszBuf + mii.cch) = L'\0';
			if (mii.hSubMenu)
				ForMenuItemSave(rb, mii.hSubMenu);
		}
	}

	static void ForMenuItemRead(CMemReader& r, HMENU hMenu, int cItem)
	{
		MENUITEMINFOW mii{};
		mii.cbSize = sizeof(mii);
		mii.fMask = MIIM_DATA | MIIM_FTYPE | MIIM_ID | MIIM_STATE | MIIM_STRING | MIIM_SUBMENU;
		const ITEM* pItem;
		EckCounter(cItem, i)
		{
			r.SkipPointer(pItem);
			mii.fState = pItem->uState;
			mii.fType = pItem->uType;
			mii.wID = pItem->uID;
			mii.dwItemData = (ULONG_PTR)pItem->uData;
			mii.dwTypeData = (PWSTR)r.Data();
			r += Cch2CbW(pItem->cchText);
			if (pItem->cSubItem)
			{
				mii.hSubMenu = CreatePopupMenu();
				ForMenuItemRead(r, mii.hSubMenu, pItem->cSubItem);
			}
			else
				mii.hSubMenu = nullptr;
			InsertMenuItemW(hMenu, i, TRUE, &mii);
		}
	}
public:
	constexpr static int c_DataVer1 = 0;
	struct DATAHEADER
	{
		int iVer;
		int cItem;
	};

	struct ITEM
	{
		UINT uState;
		UINT uType;
		UINT uID;
		ULONGLONG uData;
		int cchText;
		int cSubItem;
		// WCHAR szText[];
	};

	struct INITITEM
	{
		PCWSTR pszText = nullptr;
		UINT uID = 0u;
		UINT uFlags = 0u;
	};

	EckInline static constexpr BOOL StateHasEnabled(UINT u) { return !(u & (MF_DISABLED | MF_GRAYED)); }

	EckInline static constexpr BOOL StateHasString(UINT u) { return !(u & (MF_BITMAP | MF_OWNERDRAW)); }

	EckInline static constexpr BOOL StateHasUnchecked(UINT u) { return !(u & MF_CHECKED); }

	EckInline static constexpr BOOL StateHasUnHiLite(UINT u) { return !(u & MF_HILITE); }

	CMenu() = default;
	CMenu(std::initializer_list<INITITEM> Items)
		:m_hMenu{ CreatePopupMenu() }
	{
		for (auto x : Items)
			AppendItem(x.pszText, x.uID, x.uFlags);
	}

	CMenu(HMENU hMenu) :m_hMenu{ hMenu } {}

	CMenu(PCVOID pData, SIZE_T cbData)
		:m_hMenu{ CreatePopupMenu() }
	{
		AppendItems(pData, cbData);
	}

	CMenu(const CMenu&) = delete;
	CMenu(CMenu&& x) noexcept
		:m_hMenu{ x.m_hMenu }
	{
		x.m_hMenu = nullptr;
	}

	CMenu& operator=(const CMenu&) = delete;
	CMenu& operator=(CMenu&& x) noexcept
	{
		std::swap(m_hMenu, x.m_hMenu);
		return *this;
	}

	~CMenu() { Destroy(); }

	/// <summary>
	/// 从数据附加项目。
	/// 反序列化给定数据并将其中的项目附加到当前菜单
	/// </summary>
	/// <param name="pData">指针</param>
	/// <param name="cb">长度</param>
	void AppendItems(PCVOID pData, SIZE_T cb)
	{
		CMemReader r(pData, cb);
		const DATAHEADER* pHeader;
		r.SkipPointer(pHeader);
		if (pHeader->iVer != c_DataVer1)
		{
			EckDbgBreak();
			return;
		}
		ForMenuItemRead(r, m_hMenu, pHeader->cItem);
	}

	/// <summary>
	/// 序列化数据
	/// </summary>
	/// <param name="rb">字节集</param>
	EckInline void SerializeData(CRefBin& rb)
	{
		const auto pHeader = rb.PushBack<DATAHEADER>();
		pHeader->iVer = c_DataVer1;
		pHeader->cItem = GetItemCount();
		ForMenuItemSave(rb, m_hMenu);
	}

	EckInline HMENU Attach(HMENU hMenu)
	{
		std::swap(m_hMenu, hMenu);
		return hMenu;
	}

	[[nodiscard]] EckInline HMENU Detach()
	{
		return Attach(nullptr);
	}

	[[nodiscard]] EckInline HMENU GetHMenu() const { return m_hMenu; }

	EckInline BOOL AppendItem(PCWSTR pszText, UINT uID, UINT uFlags = 0u)
	{
		return AppendMenuW(m_hMenu, uFlags | MF_STRING, uID, pszText);
	}

	EckInline BOOL AppendItem(HBITMAP hBitmap, UINT uID, UINT uFlags = 0u)
	{
		return AppendMenuW(m_hMenu, uFlags | MF_BITMAP, uID, (PCWSTR)hBitmap);
	}

	EckInline BOOL AppendItem(LPARAM lParam, UINT uID, UINT uFlags = 0u)
	{
		return AppendMenuW(m_hMenu, uFlags | MF_OWNERDRAW, uID, (PCWSTR)lParam);
	}

	EckInline BOOL AppendItem(PCWSTR pszText, HMENU hSubMenu, UINT uFlags = 0u)
	{
		return AppendMenuW(m_hMenu, uFlags | MF_STRING | MF_POPUP, (UINT_PTR)hSubMenu, pszText);
	}

	EckInline BOOL AppendItem(HBITMAP hBitmap, HMENU hSubMenu, UINT uFlags = 0u)
	{
		return AppendMenuW(m_hMenu, uFlags | MF_BITMAP | MF_POPUP, (UINT_PTR)hSubMenu, (PCWSTR)hBitmap);
	}

	EckInline BOOL AppendItem(LPARAM lParam, HMENU hSubMenu, UINT uFlags = 0u)
	{
		return AppendMenuW(m_hMenu, uFlags | MF_OWNERDRAW | MF_POPUP, (UINT_PTR)hSubMenu, (PCWSTR)lParam);
	}

	EckInline BOOL AppendItem(UINT uFlags, UINT_PTR uIDNewItem, PCWSTR pNewItem)
	{
		return AppendMenuW(m_hMenu, uFlags, uIDNewItem, pNewItem);
	}

	EckInline BOOL CheckItem(BOOL bCheck, UINT uPos, BOOL bPosition = FALSE)
	{
		MENUITEMINFOW mii;
		mii.cbSize = sizeof(mii);
		mii.fMask = MIIM_STATE;
		if (!GetItemInfo(&mii, uPos, bPosition))
			return FALSE;
		mii.fState &= (MF_CHECKED | MF_UNCHECKED);
		mii.fState |= (bCheck ? MF_CHECKED : MF_UNCHECKED);
		return SetItemInfo(&mii, uPos, bPosition);
	}

	EckInline BOOL CheckRadioItem(UINT uBegin, UINT uEnd, UINT uCheck, BOOL bPosition = FALSE)
	{
		return CheckMenuRadioItem(m_hMenu, uBegin, uEnd, uCheck, PosBool2UINT(bPosition));
	}

	/// <summary>
	/// 创建菜单。
	/// 旧的菜单将被销毁
	/// </summary>
	/// <returns></returns>
	EckInline HMENU Create()
	{
		DestroyMenu(Attach(CreateMenu()));
		return m_hMenu;
	}

	/// <summary>
	/// 创建弹出式菜单。
	/// 旧的菜单将被销毁
	/// </summary>
	/// <returns></returns>
	EckInline HMENU CreatePopup()
	{
		DestroyMenu(Attach(CreatePopupMenu()));
		return m_hMenu;
	}

	EckInline BOOL DeleteItem(UINT uPos, BOOL bPosition = FALSE)
	{
		return DeleteMenu(m_hMenu, uPos, PosBool2UINT(bPosition));
	}

	EckInline BOOL Destroy()
	{
		return DestroyMenu(Attach(nullptr));
	}

	EckInline int EnableItem(UINT uPos, UINT uFlags, BOOL bPosition = FALSE)
	{
		return EnableMenuItem(m_hMenu, uPos, PosBool2UINT(bPosition) | uFlags);
	}

	EckInline UINT GetDefaultItem(UINT uFlags, BOOL bPosition = FALSE)
	{
		return GetMenuDefaultItem(m_hMenu, PosBool2UINT(bPosition), uFlags);
	}

	EckInline BOOL GetInfo(MENUINFO* pmi)
	{
		return GetMenuInfo(m_hMenu, pmi);
	}

	[[nodiscard]] EckInline int GetItemCount()
	{
		return GetMenuItemCount(m_hMenu);
	}

	[[nodiscard]] EckInline UINT GetItemID(int idx)
	{
		return GetMenuItemID(m_hMenu, idx);
	}

	EckInline BOOL GetItemInfo(MENUITEMINFOW* pmii, UINT uPos, BOOL bPosition = FALSE)
	{
		return GetMenuItemInfoW(m_hMenu, uPos, bPosition, pmii);
	}

	EckInline BOOL GetItemRect(HWND hWnd, int idx, RECT* prc)
	{
		return GetMenuItemRect(hWnd, m_hMenu, idx, prc);
	}

	[[nodiscard]] EckInline UINT GetItemState(UINT uPos, BOOL bPosition = FALSE)
	{
		return GetMenuState(m_hMenu, uPos, PosBool2UINT(bPosition));
	}

	[[nodiscard]] EckInline CRefStrW GetItemString(UINT uPos, BOOL bPosition = FALSE)
	{
		MENUITEMINFOW mii;
		mii.cbSize = sizeof(mii);
		mii.fMask = MIIM_TYPE;
		mii.cch = 0;
		GetItemInfo(&mii, uPos, bPosition);
		CRefStrW rs(mii.cch);
		if (mii.cch)
		{
			++mii.cch;
			mii.dwTypeData = rs.Data();
			mii.fMask = MIIM_STRING;
			GetItemInfo(&mii, uPos, bPosition);
		}
		return rs;
	}

	EckInline BOOL GetItemString(PWSTR pszBuf, int cchBuf, UINT uPos, BOOL bPosition = FALSE)
	{
		MENUITEMINFOW mii;
		mii.cbSize = sizeof(mii);
		mii.fMask = MIIM_STRING;
		mii.cch = cchBuf;
		mii.dwTypeData = pszBuf;
		return GetItemInfo(&mii, uPos, bPosition);
	}

	[[nodiscard]] EckInline HMENU GetSubMenu(int idx)
	{
		return ::GetSubMenu(m_hMenu, idx);
	}

	EckInline BOOL HiLiteItem(HWND hWnd, BOOL bHiLite, UINT uPos, BOOL bPosition = FALSE)
	{
		return HiliteMenuItem(hWnd, m_hMenu, uPos,
			PosBool2UINT(bPosition) | (bHiLite ? MF_HILITE : MF_UNHILITE));
	}

	EckInline BOOL InsertItem(PCWSTR pszText, UINT uID,UINT uPos, UINT uFlags = 0u)
	{
		return InsertMenuW(m_hMenu, uPos, uFlags | MF_STRING, uID, pszText);
	}

	EckInline BOOL InsertItem(HBITMAP hBitmap, UINT uID, UINT uPos, UINT uFlags = 0u)
	{
		return InsertMenuW(m_hMenu, uPos, uFlags | MF_BITMAP, uID, (PCWSTR)hBitmap);
	}

	EckInline BOOL InsertItem(LPARAM lParam, UINT uID, UINT uPos, UINT uFlags = 0u)
	{
		return InsertMenuW(m_hMenu, uPos, uFlags | MF_OWNERDRAW, uID, (PCWSTR)lParam);
	}

	EckInline BOOL InsertItem(PCWSTR pszText, HMENU hSubMenu, UINT uPos, UINT uFlags = 0u)
	{
		return InsertMenuW(m_hMenu, uPos, uFlags | MF_STRING | MF_POPUP, (UINT_PTR)hSubMenu, pszText);
	}

	EckInline BOOL InsertItem(HBITMAP hBitmap, HMENU hSubMenu, UINT uPos, UINT uFlags = 0u)
	{
		return InsertMenuW(m_hMenu, uPos, uFlags | MF_BITMAP | MF_POPUP, (UINT_PTR)hSubMenu, (PCWSTR)hBitmap);
	}

	EckInline BOOL InsertItem(LPARAM lParam, HMENU hSubMenu, UINT uPos, UINT uFlags = 0u)
	{
		return InsertMenuW(m_hMenu, uPos, uFlags | MF_OWNERDRAW | MF_POPUP, (UINT_PTR)hSubMenu, (PCWSTR)lParam);
	}

	EckInline BOOL InsertItem(UINT uPos, UINT uFlags, UINT_PTR uIDNewItem, PCWSTR pNewItem)
	{
		return InsertMenuW(m_hMenu, uPos, uFlags, uIDNewItem, pNewItem);
	}

	EckInline BOOL InsertItem(MENUITEMINFOW* pmii, UINT uPos, BOOL bPosition = FALSE)
	{
		return InsertMenuItemW(m_hMenu, uPos, bPosition, pmii);
	}

	[[nodiscard]] EckInline BOOL IsMenu()
	{
		return ::IsMenu(m_hMenu);
	}

	[[nodiscard]] EckInline int HitTest(HWND hWnd, POINT pt)
	{
		return MenuItemFromPoint(hWnd, m_hMenu, pt);
	}

	EckInline BOOL ModifyItem(PCWSTR pszText, UINT uID, UINT uPos, UINT uFlags = 0u)
	{
		return ModifyMenuW(m_hMenu, uPos, uFlags | MF_STRING, uID, pszText);
	}

	EckInline BOOL ModifyItem(HBITMAP hBitmap, UINT uID, UINT uPos, UINT uFlags = 0u)
	{
		return ModifyMenuW(m_hMenu, uPos, uFlags | MF_BITMAP, uID, (PCWSTR)hBitmap);
	}

	EckInline BOOL ModifyItem(LPARAM lParam, UINT uID, UINT uPos, UINT uFlags = 0u)
	{
		return ModifyMenuW(m_hMenu, uPos, uFlags | MF_OWNERDRAW, uID, (PCWSTR)lParam);
	}

	EckInline BOOL ModifyItem(PCWSTR pszText, HMENU hSubMenu, UINT uPos, UINT uFlags = 0u)
	{
		return ModifyMenuW(m_hMenu, uPos, uFlags | MF_STRING | MF_POPUP, (UINT_PTR)hSubMenu, pszText);
	}

	EckInline BOOL ModifyItem(HBITMAP hBitmap, HMENU hSubMenu, UINT uPos, UINT uFlags = 0u)
	{
		return ModifyMenuW(m_hMenu, uPos, uFlags | MF_BITMAP | MF_POPUP, (UINT_PTR)hSubMenu, (PCWSTR)hBitmap);
	}

	EckInline BOOL ModifyItem(LPARAM lParam, HMENU hSubMenu, UINT uPos, UINT uFlags = 0u)
	{
		return ModifyMenuW(m_hMenu, uPos, uFlags | MF_OWNERDRAW | MF_POPUP, (UINT_PTR)hSubMenu, (PCWSTR)lParam);
	}

	EckInline BOOL ModifyItem(UINT uPos, UINT uFlags, UINT_PTR uIDNewItem, PCWSTR pNewItem)
	{
		return ModifyMenuW(m_hMenu, uPos, uFlags, uIDNewItem, pNewItem);
	}

	EckInline BOOL RemoveItem(UINT uPos, BOOL bPosition = FALSE)
	{
		return RemoveMenu(m_hMenu, uPos, PosBool2UINT(bPosition));
	}

	EckInline BOOL SetMenu(HWND hWnd)
	{
		return ::SetMenu(hWnd, m_hMenu);
	}

	EckInline BOOL SetDefaultItem(UINT uPos, BOOL bPosition = FALSE)
	{
		return SetMenuDefaultItem(m_hMenu, uPos, bPosition);
	}

	EckInline BOOL SetInfo(const MENUINFO* pmi)
	{
		return SetMenuInfo(m_hMenu, pmi);
	}

	EckInline BOOL SetItemBitmaps(HBITMAP hbmUnchecked, HBITMAP hbmChecked, UINT uPos, BOOL bPosition = FALSE)
	{
		return SetMenuItemBitmaps(m_hMenu, uPos, PosBool2UINT(bPosition), hbmUnchecked, hbmChecked);
	}

	EckInline BOOL SetItemInfo(const MENUITEMINFOW* pmii, UINT uPos, BOOL bPosition = FALSE)
	{
		return SetMenuItemInfoW(m_hMenu, uPos, bPosition, pmii);
	}

	EckInline BOOL TrackPopupMenu(HWND hWnd, int x, int y, UINT uFlags = 0u)
	{
		return ::TrackPopupMenu(m_hMenu, uFlags, x, y, 0, hWnd, nullptr);
	}

	EckInline BOOL TrackPopupMenuEx(HWND hWnd, int x, int y, UINT uFlags, TPMPARAMS* ptpmp)
	{
		return ::TrackPopupMenuEx(m_hMenu, uFlags, x, y, hWnd, ptpmp);
	}
};
ECK_NAMESPACE_END