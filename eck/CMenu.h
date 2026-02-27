#pragma once
#include "CString.h"
#include "CByteBuffer.h"
#include "MemWalker.h"

ECK_NAMESPACE_BEGIN
class CMenu
{
private:
    HMENU m_hMenu{};

    EckInlineNdCe static UINT PosBool2UINT(BOOL bPos) noexcept { return bPos ? MF_BYPOSITION : MF_BYCOMMAND; }
public:
    struct INITITEM
    {
        PCWSTR pszText = nullptr;
        UINT uId = 0u;
        UINT uFlags = 0u;
    };

    CMenu() = default;
    CMenu(std::initializer_list<INITITEM> Items) noexcept
        : m_hMenu{ CreatePopupMenu() }
    {
        for (auto x : Items)
            AppendString(x.pszText, x.uId, x.uFlags);
    }

    CMenu(HMENU hMenu) noexcept : m_hMenu{ hMenu } {}

    CMenu(const CMenu&) = delete;
    CMenu& operator=(const CMenu&) = delete;

    CMenu(CMenu&& x) noexcept { std::swap(m_hMenu, x.m_hMenu); }
    CMenu& operator=(CMenu&& x) noexcept
    {
        std::swap(m_hMenu, x.m_hMenu);
        return *this;
    }

    ~CMenu() { Clear(); }

    EckInline HMENU Attach(HMENU hMenu) noexcept
    {
        std::swap(m_hMenu, hMenu);
        return hMenu;
    }
    EckInlineNd HMENU Detach() noexcept { return Attach(nullptr); }

    EckInlineNd HMENU GetHMenu() const noexcept { return m_hMenu; }

    EckInline BOOL AppendString(PCWSTR pszText, UINT uId, UINT uFlags = 0u) const noexcept
    {
        return AppendMenuW(m_hMenu, uFlags | MF_STRING, uId, pszText);
    }
    EckInline BOOL AppendString(PCWSTR pszText, HMENU hSubMenu, UINT uFlags = 0u) const noexcept
    {
        return AppendMenuW(m_hMenu, uFlags | MF_STRING | MF_POPUP, (UINT_PTR)hSubMenu, pszText);
    }

    EckInline BOOL AppendBitmap(HBITMAP hBitmap, UINT uId, UINT uFlags = 0u) const noexcept
    {
        return AppendMenuW(m_hMenu, uFlags | MF_BITMAP, uId, (PCWSTR)hBitmap);
    }
    EckInline BOOL AppendBitmap(HBITMAP hBitmap, HMENU hSubMenu, UINT uFlags = 0u) const noexcept
    {
        return AppendMenuW(m_hMenu, uFlags | MF_BITMAP | MF_POPUP, (UINT_PTR)hSubMenu, (PCWSTR)hBitmap);
    }

    EckInline BOOL AppendOwnerDraw(LPARAM lParam, UINT uId, UINT uFlags = 0u) const noexcept
    {
        return AppendMenuW(m_hMenu, uFlags | MF_OWNERDRAW, uId, (PCWSTR)lParam);
    }
    EckInline BOOL AppendOwnerDraw(LPARAM lParam, HMENU hSubMenu, UINT uFlags = 0u) const noexcept
    {
        return AppendMenuW(m_hMenu, uFlags | MF_OWNERDRAW | MF_POPUP, (UINT_PTR)hSubMenu, (PCWSTR)lParam);
    }

    EckInline BOOL AppendItem(UINT uFlags, UINT_PTR uIDNewItem, PCWSTR pNewItem) const noexcept
    {
        return AppendMenuW(m_hMenu, uFlags, uIDNewItem, pNewItem);
    }
    EckInline BOOL AppendSeparator() const noexcept
    {
        return AppendMenuW(m_hMenu, MF_SEPARATOR, 0, nullptr);
    }

    EckInline BOOL CheckItem(BOOL bCheck, UINT uPos, BOOL bPosition = FALSE) const noexcept
    {
        MENUITEMINFOW mii;
        mii.cbSize = sizeof(mii);
        mii.fMask = MIIM_STATE;
        if (!GetItemInfomation(&mii, uPos, bPosition))
            return FALSE;
        mii.fState &= (MF_CHECKED | MF_UNCHECKED);
        mii.fState |= (bCheck ? MF_CHECKED : MF_UNCHECKED);
        return SetItemInfomation(&mii, uPos, bPosition);
    }

    EckInline BOOL CheckRadioItem(UINT uBegin, UINT uEnd, UINT uCheck, BOOL bPosition = FALSE) const noexcept
    {
        return CheckMenuRadioItem(m_hMenu, uBegin, uEnd, uCheck, PosBool2UINT(bPosition));
    }

    EckInline HMENU Create() noexcept
    {
        DestroyMenu(Attach(CreateMenu()));
        return m_hMenu;
    }
    EckInline HMENU CreatePopup() noexcept
    {
        DestroyMenu(Attach(CreatePopupMenu()));
        return m_hMenu;
    }

    EckInline BOOL DeleteItem(UINT uPos, BOOL bPosition = FALSE) const noexcept
    {
        return DeleteMenu(m_hMenu, uPos, PosBool2UINT(bPosition));
    }

    EckInline BOOL Clear() noexcept
    {
        return DestroyMenu(Attach(nullptr));
    }

    EckInline int EnableItem(UINT uPos, UINT uFlags, BOOL bPosition = FALSE) const noexcept
    {
        return EnableMenuItem(m_hMenu, uPos, PosBool2UINT(bPosition) | uFlags);
    }

    EckInline UINT GetDefaultItem(UINT uFlags, BOOL bPosition = FALSE) const noexcept
    {
        return GetMenuDefaultItem(m_hMenu, PosBool2UINT(bPosition), uFlags);
    }

    EckInline BOOL GetInfomation(MENUINFO* pmi) const noexcept
    {
        return GetMenuInfo(m_hMenu, pmi);
    }

    EckInlineNd int GetItemCount() const noexcept
    {
        return GetMenuItemCount(m_hMenu);
    }

    EckInlineNd UINT GetItemId(int idx) const noexcept
    {
        return GetMenuItemID(m_hMenu, idx);
    }

    EckInline BOOL GetItemInfomation(_Inout_ MENUITEMINFOW* pmii,
        UINT uPos, BOOL bPosition = FALSE) const noexcept
    {
        return GetMenuItemInfoW(m_hMenu, uPos, bPosition, pmii);
    }

    EckInline BOOL GetItemRect(HWND hWnd, int idx, RECT* prc) const noexcept
    {
        return GetMenuItemRect(hWnd, m_hMenu, idx, prc);
    }

    EckInlineNd UINT GetItemState(UINT uPos, BOOL bPosition = FALSE) const noexcept
    {
        return GetMenuState(m_hMenu, uPos, PosBool2UINT(bPosition));
    }

    EckInlineNd BOOL GetItemString(CStringW& rs, UINT uPos, BOOL bPosition = FALSE) const noexcept
    {
        MENUITEMINFOW mii;
        mii.cbSize = sizeof(mii);
        mii.fMask = MIIM_TYPE;
        mii.cch = 0;
        if (!GetItemInfomation(&mii, uPos, bPosition))
            return FALSE;
        if (mii.cch)
        {
            ++mii.cch;
            mii.dwTypeData = rs.PushBackNoExtra(mii.cch);
            mii.fMask = MIIM_STRING;
            return GetItemInfomation(&mii, uPos, bPosition);
        }
        return TRUE;
    }

    EckInline BOOL GetItemString(PWSTR pszBuf, int cchBuf,
        UINT uPos, BOOL bPosition = FALSE) const noexcept
    {
        MENUITEMINFOW mii;
        mii.cbSize = sizeof(mii);
        mii.fMask = MIIM_STRING;
        mii.cch = cchBuf;
        mii.dwTypeData = pszBuf;
        return GetItemInfomation(&mii, uPos, bPosition);
    }

    EckInlineNd HMENU GetSubMenu(int idx) const noexcept
    {
        return ::GetSubMenu(m_hMenu, idx);
    }

    EckInline BOOL HilightItem(HWND hWnd, BOOL bHiLite,
        UINT uPos, BOOL bPosition = FALSE) const noexcept
    {
        return HiliteMenuItem(hWnd, m_hMenu, uPos,
            PosBool2UINT(bPosition) | (bHiLite ? MF_HILITE : MF_UNHILITE));
    }

    EckInline BOOL InsertString(PCWSTR pszText, UINT uId, UINT uPos, UINT uFlags = 0u) const noexcept
    {
        return InsertMenuW(m_hMenu, uPos, uFlags | MF_STRING, uId, pszText);
    }
    EckInline BOOL InsertString(PCWSTR pszText, HMENU hSubMenu, UINT uPos, UINT uFlags = 0u) const noexcept
    {
        return InsertMenuW(m_hMenu, uPos, uFlags | MF_STRING | MF_POPUP, (UINT_PTR)hSubMenu, pszText);
    }

    EckInline BOOL InsertBitmap(HBITMAP hBitmap, UINT uId, UINT uPos, UINT uFlags = 0u) const noexcept
    {
        return InsertMenuW(m_hMenu, uPos, uFlags | MF_BITMAP, uId, (PCWSTR)hBitmap);
    }
    EckInline BOOL InsertBitmap(HBITMAP hBitmap, HMENU hSubMenu, UINT uPos, UINT uFlags = 0u) const noexcept
    {
        return InsertMenuW(m_hMenu, uPos, uFlags | MF_BITMAP | MF_POPUP, (UINT_PTR)hSubMenu, (PCWSTR)hBitmap);
    }

    EckInline BOOL InsertOwnerDraw(LPARAM lParam, UINT uId, UINT uPos, UINT uFlags = 0u) const noexcept
    {
        return InsertMenuW(m_hMenu, uPos, uFlags | MF_OWNERDRAW, uId, (PCWSTR)lParam);
    }
    EckInline BOOL InsertOwnerDraw(LPARAM lParam, HMENU hSubMenu, UINT uPos, UINT uFlags = 0u) const noexcept
    {
        return InsertMenuW(m_hMenu, uPos, uFlags | MF_OWNERDRAW | MF_POPUP, (UINT_PTR)hSubMenu, (PCWSTR)lParam);
    }

    EckInline BOOL InsertItem(UINT uPos, UINT uFlags, UINT_PTR uIDNewItem, PCWSTR pNewItem) const noexcept
    {
        return InsertMenuW(m_hMenu, uPos, uFlags, uIDNewItem, pNewItem);
    }
    EckInline BOOL InsertItem(const MENUITEMINFOW* pmii, UINT uPos, BOOL bPosition = FALSE) const noexcept
    {
        return InsertMenuItemW(m_hMenu, uPos, bPosition, pmii);
    }
    EckInline BOOL InsertSeparator(UINT uPos, UINT uFlags) const noexcept
    {
        return InsertMenuW(m_hMenu, uPos, MF_SEPARATOR | uFlags, 0, nullptr);
    }

    EckInlineNd BOOL IsValid() const noexcept
    {
        return m_hMenu && ::IsMenu(m_hMenu);
    }

    EckInlineNd int HitTest(HWND hWnd, POINT pt) const noexcept
    {
        return MenuItemFromPoint(hWnd, m_hMenu, pt);
    }

    EckInline BOOL ModifyString(PCWSTR pszText, UINT uId, UINT uPos, UINT uFlags = 0u) const noexcept
    {
        return ModifyMenuW(m_hMenu, uPos, uFlags | MF_STRING, uId, pszText);
    }
    EckInline BOOL ModifyString(PCWSTR pszText, HMENU hSubMenu, UINT uPos, UINT uFlags = 0u) const noexcept
    {
        return ModifyMenuW(m_hMenu, uPos, uFlags | MF_STRING | MF_POPUP, (UINT_PTR)hSubMenu, pszText);
    }

    EckInline BOOL ModifyBitmap(HBITMAP hBitmap, UINT uId, UINT uPos, UINT uFlags = 0u) const noexcept
    {
        return ModifyMenuW(m_hMenu, uPos, uFlags | MF_BITMAP, uId, (PCWSTR)hBitmap);
    }
    EckInline BOOL ModifyBitmap(HBITMAP hBitmap, HMENU hSubMenu, UINT uPos, UINT uFlags = 0u) const noexcept
    {
        return ModifyMenuW(m_hMenu, uPos, uFlags | MF_BITMAP | MF_POPUP, (UINT_PTR)hSubMenu, (PCWSTR)hBitmap);
    }

    EckInline BOOL ModifyOwnerDraw(LPARAM lParam, UINT uId, UINT uPos, UINT uFlags = 0u) const noexcept
    {
        return ModifyMenuW(m_hMenu, uPos, uFlags | MF_OWNERDRAW, uId, (PCWSTR)lParam);
    }
    EckInline BOOL ModifyOwnerDraw(LPARAM lParam, HMENU hSubMenu, UINT uPos, UINT uFlags = 0u) const noexcept
    {
        return ModifyMenuW(m_hMenu, uPos, uFlags | MF_OWNERDRAW | MF_POPUP, (UINT_PTR)hSubMenu, (PCWSTR)lParam);
    }

    EckInline BOOL ModifyItem(UINT uPos, UINT uFlags, UINT_PTR uIDNewItem, PCWSTR pNewItem) const noexcept
    {
        return ModifyMenuW(m_hMenu, uPos, uFlags, uIDNewItem, pNewItem);
    }

    EckInline BOOL RemoveItem(UINT uPos, BOOL bPosition = FALSE) const noexcept
    {
        return RemoveMenu(m_hMenu, uPos, PosBool2UINT(bPosition));
    }

    EckInline BOOL SetMenu(HWND hWnd) const noexcept
    {
        return ::SetMenu(hWnd, m_hMenu);
    }

    EckInline BOOL SetDefaultItem(UINT uPos, BOOL bPosition = FALSE) const noexcept
    {
        return SetMenuDefaultItem(m_hMenu, uPos, bPosition);
    }

    EckInline BOOL SetInfomation(const MENUINFO* pmi) const noexcept
    {
        return SetMenuInfo(m_hMenu, pmi);
    }

    EckInline BOOL SetItemBitmaps(HBITMAP hbmUnchecked,
        HBITMAP hbmChecked, UINT uPos, BOOL bPosition = FALSE) const noexcept
    {
        return SetMenuItemBitmaps(m_hMenu, uPos, PosBool2UINT(bPosition), hbmUnchecked, hbmChecked);
    }

    EckInline BOOL SetItemInfomation(const MENUITEMINFOW* pmii,
        UINT uPos, BOOL bPosition = FALSE) const noexcept
    {
        return SetMenuItemInfoW(m_hMenu, uPos, bPosition, pmii);
    }

    EckInline BOOL TrackPopupMenu(HWND hWnd, int x, int y, UINT uFlags = 0u) const noexcept
    {
        return ::TrackPopupMenu(m_hMenu, uFlags, x, y, 0, hWnd, nullptr);
    }
    EckInline BOOL TrackPopupMenuEx(HWND hWnd, int x, int y,
        UINT uFlags, const TPMPARAMS* ptpmp) const noexcept
    {
        return ::TrackPopupMenuEx(m_hMenu, uFlags, x, y, hWnd, (TPMPARAMS*)ptpmp);
    }
};
ECK_NAMESPACE_END