#pragma once
#include "CTreeList.h"

ECK_NAMESPACE_BEGIN
struct TLEXTNODE
{
    TLNODE Node;
    std::vector<eck::CStringW> vText;
    LPARAM lParam;
    std::vector<TLEXTNODE*> vChildren;
};

using HTLENODE = TLEXTNODE*;

enum : UINT
{
    TLEIF_TEXT = 1u << 0,
    TLEIF_PARAM = 1u << 1,
};

struct TLEXTITEM
{
    UINT uMask;
    PWSTR pszText;
    int cchText;
    int idxSubItem;
    LPARAM lParam;
};

class CTreeListExt : public CTreeList
{
public:
    ECK_RTTI(CTreeListExt, CTreeList);
private:
    std::vector<TLEXTNODE*> m_vRoot{};
    std::vector<TLEXTNODE*> m_vFlat{};

    BITBOOL m_bMaintainFlatList : 1 = FALSE;

    void RemoveNodeFromFlatList(HTLENODE hNode) noexcept
    {
        EckAssert(m_bMaintainFlatList);
        const auto it = std::find(m_vFlat.begin(), m_vFlat.end(), hNode);
        EckAssert(it != m_vFlat.end());
        for (const auto e : hNode->vChildren)
            RemoveNodeFromFlatList(e);
        m_vFlat.erase(it);
        delete hNode;
    }

    void RemoveNode(HTLENODE hNode) noexcept
    {
        EckAssert(!m_bMaintainFlatList);
        for (const auto e : hNode->vChildren)
            RemoveNode(e);
        delete hNode;
    }
public:
    HTLENODE InsertItem(const TLEXTITEM& tlei,
        HTLENODE hParent = nullptr, int idxPos = -1) noexcept
    {
        const auto p = new TLEXTNODE{};
        SetItem(p, tlei);

        if (hParent)
            if (idxPos < 0)
                hParent->vChildren.push_back(p);
            else
                hParent->vChildren.insert(hParent->vChildren.begin() + idxPos, p);
        else
            if (idxPos < 0)
                m_vRoot.push_back(p);
            else
                m_vRoot.insert(m_vRoot.begin() + idxPos, p);
        if (m_bMaintainFlatList)
            m_vFlat.push_back(p);
        return p;
    }

    EckInline HTLENODE InsertItem(PCWSTR pszText,
        LPARAM lParam = 0, HTLENODE hParent = nullptr, int idxPos = -1) noexcept
    {
        const TLEXTITEM tlei
        {
            TLEIF_TEXT | TLEIF_PARAM,
            (PWSTR)pszText,
            (int)wcslen(pszText),
            0,
            lParam
        };
        return InsertItem(tlei, hParent, idxPos);
    }

    void DeleteItem(HTLENODE hParent, int idxPos) noexcept
    {
        const auto p = (hParent ? hParent->vChildren[idxPos] : m_vRoot[idxPos]);
        if (m_bMaintainFlatList)
            RemoveNodeFromFlatList(p);
        else
            RemoveNode(p);

        if (hParent)
            hParent->vChildren.erase(hParent->vChildren.begin() + idxPos);
        else
            m_vRoot.erase(m_vRoot.begin() + idxPos);
    }

    void SetItem(HTLENODE hNode, const TLEXTITEM& tlei) noexcept
    {
        if (tlei.uMask & TLEIF_TEXT)
        {
            EckAssert(tlei.idxSubItem >= 0 && tlei.idxSubItem < GetColumnCount());
            if ((int)hNode->vText.size() <= tlei.idxSubItem)
                hNode->vText.resize(tlei.idxSubItem + 1);
            hNode->vText[tlei.idxSubItem].Assign(tlei.pszText, tlei.cchText);
        }
        if (tlei.uMask & TLEIF_PARAM)
            hNode->lParam = tlei.lParam;
    }

    void GetItem(HTLENODE hNode, TLEXTITEM& tlei) noexcept
    {
        if (tlei.uMask & TLEIF_TEXT)
        {
            EckAssert(tlei.idxSubItem >= 0 && tlei.idxSubItem < GetColumnCount());
            EckAssert(tlei.cchText >= 0);
            if (tlei.cchText)
            {
                if ((int)hNode->vText.size() <= tlei.idxSubItem)
                    *tlei.pszText = L'\0';
                else
                    hNode->vText[tlei.idxSubItem].CopyTo(tlei.pszText, tlei.cchText);
            }
        }
        if (tlei.uMask & TLEIF_PARAM)
            tlei.lParam = hNode->lParam;
    }

    LRESULT OnNotifyMessage(HWND hParent, UINT uMsg,
        WPARAM wParam, LPARAM lParam, BOOL& bProcessed) noexcept override
    {
        switch (uMsg)
        {
        case WM_NOTIFY:
        {
            switch (((NMHDR*)lParam)->code)
            {
            case NM_TL_FILLCHILDREN:
            {
                bProcessed = TRUE;
                const auto p = (NMTLFILLCHILDREN*)lParam;
                if (p->bQueryRoot)
                {
                    p->pChildren = (TLNODE**)m_vRoot.data();
                    p->cChildren = (int)m_vRoot.size();
                }
                else
                {
                    const auto pParent = (TLEXTNODE*)p->pParent;
                    p->pChildren = (TLNODE**)pParent->vChildren.data();
                    p->cChildren = (int)pParent->vChildren.size();
                }
            }
            return 0;

            case NM_TL_FILLALLFLATITEM:
            {
                EckAssert(m_bMaintainFlatList);
                bProcessed = TRUE;
                const auto p = (NMTLFILLALLFLATITEM*)lParam;
                p->pItems = (TLNODE**)m_vFlat.data();
                p->cItem = (int)m_vFlat.size();
            }
            return 0;

            case NM_TL_GETDISPINFO:
            {
                bProcessed = TRUE;
                const auto p = (NMTLGETDISPINFO*)lParam;
                const auto pNode = (TLEXTNODE*)p->Item.pNode;
                // if (p->Item.uMask & TLIM_TEXT)
                {
                    if (p->Item.idxSubItem < (int)pNode->vText.size())
                    {
                        const auto& rs = pNode->vText[p->Item.idxSubItem];
                        p->Item.pszText = rs.Data();
                        p->Item.cchText = rs.Size();
                    }
                }
            }
            return 0;
            }
        }
        break;
        }
        return __super::OnNotifyMessage(hParent, uMsg, wParam, lParam, bProcessed);
    }

    EckInlineCe void SetMaintainFlatList(BOOL b) noexcept { m_bMaintainFlatList = b; }
    EckInlineNdCe BOOL GetMaintainFlatList() const noexcept { return m_bMaintainFlatList; }
};
ECK_NAMESPACE_END