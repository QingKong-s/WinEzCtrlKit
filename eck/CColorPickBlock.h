#pragma once
#include "CStatic.h"

ECK_NAMESPACE_BEGIN
inline constexpr int CDV_COLOR_PICK_BLOCK_1 = 1;

#pragma pack(push, ECK_CTRLDATA_ALIGN)
struct CTRLDATA_COLOR_PICK_BLOCK
{
    int iVer;
    COLORREF crCust[16];
    COLORREF cr;
    UINT uCCFlags;
};
#pragma pack(pop)

class CColorPickBlock : public CStatic
{
public:
    ECK_RTTI(CColorPickBlock, CStatic);
private:
    COLORREF m_crCust[16]{};
    COLORREF m_cr{ CLR_INVALID };
    UINT m_uCCFlags{ CC_FULLOPEN };

    void CleanupForDestroyWindow() noexcept
    {
        ZeroMemory(m_crCust, sizeof(m_crCust));
        m_cr = CLR_INVALID;
        m_uCCFlags = CC_FULLOPEN;
    }
public:
    EckInlineNdCe static PCVOID SkipBaseData(PCVOID p) noexcept
    {
        return PointerStepBytes(CStatic::SkipBaseData(p), sizeof(CTRLDATA_COLOR_PICK_BLOCK));
    }

    void SerializeData(CByteBuffer& rb, const SERIALIZE_OPT* pOpt = nullptr) noexcept override
    {
        CStatic::SerializeData(rb, pOpt);
        constexpr auto cbSize = sizeof(CTRLDATA_COLOR_PICK_BLOCK);
        CMemoryWalker w(rb.PushBack(cbSize), cbSize);
        CTRLDATA_COLOR_PICK_BLOCK* p;
        w.SkipPointer(p);
        p->iVer = CDV_COLOR_PICK_BLOCK_1;
        memcpy(p->crCust, m_crCust, sizeof(m_crCust));
        p->cr = m_cr;
        p->uCCFlags = m_uCCFlags;
    }

    void PostDeserialize(PCVOID pData) noexcept override
    {
        __super::PostDeserialize(pData);
        const auto* const p = (CTRLDATA_COLOR_PICK_BLOCK*)__super::SkipBaseData(pData);

        memcpy(m_crCust, p->crCust, sizeof(m_crCust));
        m_cr = p->cr;
        m_uCCFlags = p->uCCFlags;
    }

    void AttachNew(HWND hWnd) noexcept override
    {
        __super::AttachNew(hWnd);
        SetText(nullptr);
        Redraw();
    }

    void DetachNew() noexcept override
    {
        __super::DetachNew();
        CleanupForDestroyWindow();
    }

    LRESULT OnMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) noexcept override
    {
        switch (uMsg)
        {
        case WM_LBUTTONDBLCLK:
        {
            CHOOSECOLORW cc{ sizeof(CHOOSECOLORW) };
            cc.hwndOwner = hWnd;
            cc.Flags = m_uCCFlags;
            cc.lpCustColors = m_crCust;
            if (ChooseColorW(&cc))
            {
                m_cr = cc.rgbResult;
                Redraw();
            }
        }
        return 0;

        case WM_SETTEXT:
            return TRUE;

        case WM_CREATE:
            ((CREATESTRUCTW*)lParam)->lpszName = nullptr;
            break;
        case WM_DESTROY:
            CleanupForDestroyWindow();
            break;
        }
        return CStatic::OnMessage(hWnd, uMsg, wParam, lParam);
    }

    LRESULT OnNotifyMessage(HWND hParent, UINT uMsg,
        WPARAM wParam, LPARAM lParam, BOOL& bProcessed) noexcept override
    {
        switch (uMsg)
        {
        case WM_CTLCOLORSTATIC:
        {
            bProcessed = TRUE;
            SetDCBrushColor((HDC)wParam, m_cr);
            return (LRESULT)GetStockBrush(DC_BRUSH);
        }
        break;
        }
        return CStatic::OnNotifyMessage(hParent, uMsg, wParam, lParam, bProcessed);
    }

    EckInlineCe void SetColor(COLORREF cr) noexcept { m_cr = cr; }
    EckInlineNdCe COLORREF GetColor() const noexcept { return m_cr; }

    EckInlineNdCe auto GetCustomColor() const noexcept { return m_crCust; }
    EckInlineNdCe auto GetCustomColor() noexcept { return m_crCust; }

    EckInlineNdCe UINT GetChooseColorFlags() const noexcept { return m_uCCFlags; }
    EckInlineCe void SetChooseColorFlags(UINT u) noexcept { m_uCCFlags = u; }
};
ECK_NAMESPACE_END