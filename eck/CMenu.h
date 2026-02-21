#pragma once
#include "CString.h"
#include "CByteBuffer.h"
#include "MemWalker.h"

ECK_NAMESPACE_BEGIN
class CMenu
{
private:
    HMENU m_hMenu = nullptr;

    EckInline static constexpr UINT PosBool2UINT(BOOL bPosition) noexcept { return bPosition ? MF_BYPOSITION : MF_BYCOMMAND; }

    static void SerializeData(CByteBuffer& rb, HMENU hMenu) noexcept
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
            pItem->uId = mii.wID;
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
                SerializeData(rb, mii.hSubMenu);
        }
    }

    static void DeserializeData(CMemoryReader& r, HMENU hMenu, int cItem) noexcept
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
            mii.wID = pItem->uId;
            mii.dwItemData = (ULONG_PTR)pItem->uData;
            mii.dwTypeData = (PWSTR)r.Data();
            r += Cch2CbW(pItem->cchText);
            if (pItem->cSubItem)
            {
                mii.hSubMenu = CreatePopupMenu();
                DeserializeData(r, mii.hSubMenu, pItem->cSubItem);
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
        UINT uId;
        ULONGLONG uData;
        int cchText;
        int cSubItem;
        // WCHAR szText[];
    };

    struct INITITEM
    {
        PCWSTR pszText = nullptr;
        UINT uId = 0u;
        UINT uFlags = 0u;
    };

    EckInlineNdCe static BOOL StateHasEnabled(UINT u) noexcept { return !(u & (MF_DISABLED | MF_GRAYED)); }
    EckInlineNdCe static BOOL StateHasString(UINT u) noexcept { return !(u & (MF_BITMAP | MF_OWNERDRAW)); }
    EckInlineNdCe static BOOL StateHasUnchecked(UINT u) noexcept { return !(u & MF_CHECKED); }
    EckInlineNdCe static BOOL StateHasUnHiLite(UINT u) noexcept { return !(u & MF_HILITE); }

    CMenu() = default;
    CMenu(std::initializer_list<INITITEM> Items) noexcept
        : m_hMenu{ CreatePopupMenu() }
    {
        for (auto x : Items)
            AppendItem(x.pszText, x.uId, x.uFlags);
    }

    CMenu(HMENU hMenu) noexcept : m_hMenu{ hMenu } {}

    CMenu(PCVOID pData, size_t cbData) noexcept
        : m_hMenu{ CreatePopupMenu() }
    {
        AppendSerializedItems(pData, cbData);
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
    /// <param name="cbData">长度</param>
    void AppendSerializedItems(PCVOID pData, size_t cbData) noexcept
    {
        CMemoryReader r{ pData, cbData };
        const DATAHEADER* pHeader;
        r.SkipPointer(pHeader);
        if (pHeader->iVer != c_DataVer1)
        {
            EckDbgBreak();
            return;
        }
        DeserializeData(r, m_hMenu, pHeader->cItem);
    }

    /// <summary>
    /// 序列化数据
    /// </summary>
    /// <param name="rb">字节集</param>
    EckInline void SerializeData(CByteBuffer& rb) noexcept
    {
        const auto pHeader = rb.PushBack<DATAHEADER>();
        pHeader->iVer = c_DataVer1;
        pHeader->cItem = GetItemCount();
        SerializeData(rb, m_hMenu);
    }

    EckInline HMENU Attach(HMENU hMenu) noexcept
    {
        std::swap(m_hMenu, hMenu);
        return hMenu;
    }

    EckInlineNd HMENU Detach() noexcept
    {
        return Attach(nullptr);
    }

    EckInlineNd HMENU GetHMenu() const noexcept { return m_hMenu; }

    EckInline BOOL AppendItem(PCWSTR pszText, UINT uId, UINT uFlags = 0u) const noexcept
    {
        return AppendMenuW(m_hMenu, uFlags | MF_STRING, uId, pszText);
    }

    EckInline BOOL AppendItem(HBITMAP hBitmap, UINT uId, UINT uFlags = 0u) const noexcept
    {
        return AppendMenuW(m_hMenu, uFlags | MF_BITMAP, uId, (PCWSTR)hBitmap);
    }

    EckInline BOOL AppendItem(LPARAM lParam, UINT uId, UINT uFlags = 0u) const noexcept
    {
        return AppendMenuW(m_hMenu, uFlags | MF_OWNERDRAW, uId, (PCWSTR)lParam);
    }

    EckInline BOOL AppendItem(PCWSTR pszText, HMENU hSubMenu, UINT uFlags = 0u) const noexcept
    {
        return AppendMenuW(m_hMenu, uFlags | MF_STRING | MF_POPUP, (UINT_PTR)hSubMenu, pszText);
    }

    EckInline BOOL AppendItem(HBITMAP hBitmap, HMENU hSubMenu, UINT uFlags = 0u) const noexcept
    {
        return AppendMenuW(m_hMenu, uFlags | MF_BITMAP | MF_POPUP, (UINT_PTR)hSubMenu, (PCWSTR)hBitmap);
    }

    EckInline BOOL AppendItem(LPARAM lParam, HMENU hSubMenu, UINT uFlags = 0u) const noexcept
    {
        return AppendMenuW(m_hMenu, uFlags | MF_OWNERDRAW | MF_POPUP, (UINT_PTR)hSubMenu, (PCWSTR)lParam);
    }

    EckInline BOOL AppendItem(UINT uFlags, UINT_PTR uIDNewItem, PCWSTR pNewItem) const noexcept
    {
        return AppendMenuW(m_hMenu, uFlags, uIDNewItem, pNewItem);
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

    /// <summary>
    /// 创建菜单。
    /// 旧的菜单将被销毁
    /// </summary>
    /// <returns></returns>
    EckInline HMENU Create() noexcept
    {
        DestroyMenu(Attach(CreateMenu()));
        return m_hMenu;
    }

    /// <summary>
    /// 创建弹出式菜单。
    /// 旧的菜单将被销毁
    /// </summary>
    /// <returns></returns>
    EckInline HMENU CreatePopup() noexcept
    {
        DestroyMenu(Attach(CreatePopupMenu()));
        return m_hMenu;
    }

    EckInline BOOL DeleteItem(UINT uPos, BOOL bPosition = FALSE) const noexcept
    {
        return DeleteMenu(m_hMenu, uPos, PosBool2UINT(bPosition));
    }

    EckInline BOOL Destroy() noexcept
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

    EckInlineNd CStringW GetItemString(UINT uPos, BOOL bPosition = FALSE) const noexcept
    {
        MENUITEMINFOW mii;
        mii.cbSize = sizeof(mii);
        mii.fMask = MIIM_TYPE;
        mii.cch = 0;
        GetItemInfomation(&mii, uPos, bPosition);
        CStringW rs(mii.cch);
        if (mii.cch)
        {
            ++mii.cch;
            mii.dwTypeData = rs.Data();
            mii.fMask = MIIM_STRING;
            GetItemInfomation(&mii, uPos, bPosition);
        }
        return rs;
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

    EckInline BOOL HiLiteItem(HWND hWnd, BOOL bHiLite,
        UINT uPos, BOOL bPosition = FALSE) const noexcept
    {
        return HiliteMenuItem(hWnd, m_hMenu, uPos,
            PosBool2UINT(bPosition) | (bHiLite ? MF_HILITE : MF_UNHILITE));
    }

    EckInline BOOL InsertItem(PCWSTR pszText, UINT uId, UINT uPos, UINT uFlags = 0u) const noexcept
    {
        return InsertMenuW(m_hMenu, uPos, uFlags | MF_STRING, uId, pszText);
    }

    EckInline BOOL InsertItem(HBITMAP hBitmap, UINT uId, UINT uPos, UINT uFlags = 0u) const noexcept
    {
        return InsertMenuW(m_hMenu, uPos, uFlags | MF_BITMAP, uId, (PCWSTR)hBitmap);
    }

    EckInline BOOL InsertItem(LPARAM lParam, UINT uId, UINT uPos, UINT uFlags = 0u) const noexcept
    {
        return InsertMenuW(m_hMenu, uPos, uFlags | MF_OWNERDRAW, uId, (PCWSTR)lParam);
    }

    EckInline BOOL InsertItem(PCWSTR pszText, HMENU hSubMenu, UINT uPos, UINT uFlags = 0u) const noexcept
    {
        return InsertMenuW(m_hMenu, uPos, uFlags | MF_STRING | MF_POPUP, (UINT_PTR)hSubMenu, pszText);
    }

    EckInline BOOL InsertItem(HBITMAP hBitmap, HMENU hSubMenu, UINT uPos, UINT uFlags = 0u) const noexcept
    {
        return InsertMenuW(m_hMenu, uPos, uFlags | MF_BITMAP | MF_POPUP, (UINT_PTR)hSubMenu, (PCWSTR)hBitmap);
    }

    EckInline BOOL InsertItem(LPARAM lParam, HMENU hSubMenu, UINT uPos, UINT uFlags = 0u) const noexcept
    {
        return InsertMenuW(m_hMenu, uPos, uFlags | MF_OWNERDRAW | MF_POPUP, (UINT_PTR)hSubMenu, (PCWSTR)lParam);
    }

    EckInline BOOL InsertItem(UINT uPos, UINT uFlags, UINT_PTR uIDNewItem, PCWSTR pNewItem) const noexcept
    {
        return InsertMenuW(m_hMenu, uPos, uFlags, uIDNewItem, pNewItem);
    }

    EckInline BOOL InsertItem(MENUITEMINFOW* pmii, UINT uPos, BOOL bPosition = FALSE) const noexcept
    {
        return InsertMenuItemW(m_hMenu, uPos, bPosition, pmii);
    }

    EckInlineNd BOOL IsMenu() const noexcept
    {
        return ::IsMenu(m_hMenu);
    }

    EckInlineNd int HitTest(HWND hWnd, POINT pt) const noexcept
    {
        return MenuItemFromPoint(hWnd, m_hMenu, pt);
    }

    EckInline BOOL ModifyItem(PCWSTR pszText, UINT uId, UINT uPos, UINT uFlags = 0u) const noexcept
    {
        return ModifyMenuW(m_hMenu, uPos, uFlags | MF_STRING, uId, pszText);
    }

    EckInline BOOL ModifyItem(HBITMAP hBitmap, UINT uId, UINT uPos, UINT uFlags = 0u) const noexcept
    {
        return ModifyMenuW(m_hMenu, uPos, uFlags | MF_BITMAP, uId, (PCWSTR)hBitmap);
    }

    EckInline BOOL ModifyItem(LPARAM lParam, UINT uId, UINT uPos, UINT uFlags = 0u) const noexcept
    {
        return ModifyMenuW(m_hMenu, uPos, uFlags | MF_OWNERDRAW, uId, (PCWSTR)lParam);
    }

    EckInline BOOL ModifyItem(PCWSTR pszText, HMENU hSubMenu, UINT uPos, UINT uFlags = 0u) const noexcept
    {
        return ModifyMenuW(m_hMenu, uPos, uFlags | MF_STRING | MF_POPUP, (UINT_PTR)hSubMenu, pszText);
    }

    EckInline BOOL ModifyItem(HBITMAP hBitmap, HMENU hSubMenu, UINT uPos, UINT uFlags = 0u) const noexcept
    {
        return ModifyMenuW(m_hMenu, uPos, uFlags | MF_BITMAP | MF_POPUP, (UINT_PTR)hSubMenu, (PCWSTR)hBitmap);
    }

    EckInline BOOL ModifyItem(LPARAM lParam, HMENU hSubMenu, UINT uPos, UINT uFlags = 0u) const noexcept
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
        UINT uFlags, TPMPARAMS* ptpmp) const noexcept
    {
        return ::TrackPopupMenuEx(m_hMenu, uFlags, x, y, hWnd, ptpmp);
    }
};
ECK_NAMESPACE_END