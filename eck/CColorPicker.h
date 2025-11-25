#pragma once
#include "CWnd.h"
#include "CComboBoxNew.h"

ECK_NAMESPACE_BEGIN
// 颜色选择器 颜色被改变通知结构
struct NMCLPCLRCHANGED
{
    NMHDR nmhdr;
    COLORREF cr;
};

class CColorPicker : public CComboBoxNew
{
public:
    ECK_RTTI(CColorPicker);

    constexpr static int IdxCustom = 1;
private:
    COLORREF m_crCustom{ CLR_INVALID };
    COLORREF m_crCcDlgCustom[16]{};

    constexpr static struct CPPRESETCOLOR
    {
        COLORREF cr;
        std::wstring_view svName;
    }
    PresetColor[]
    {
        { CLR_DEFAULT,L"默认" },
        { CLR_INVALID,L"自定义..." },
        { 0x0000FF,L"红色" },
        { 0x00FF00,L"绿色" },
        { 0xFF0000,L"蓝色" },
        { 0x00FFFF,L"黄色" },
        { 0xFF00FF,L"品红" },
        { 0xFFFF00,L"艳青" },
        { 0x000080,L"红褐" },
        { 0x008000,L"墨绿" },
        { 0x008080,L"褐绿" },
        { 0x800000,L"藏青" },
        { 0x800080,L"紫红" },
        { 0x808000,L"深青" },
        { 0xC0C0C0,L"浅灰" },
        { 0xC0DCC0,L"美元绿" },
        { 0xF0CAA6,L"浅蓝" },
        { 0x808080,L"灰色" },
        { 0xA4A0A0,L"中性灰" },
        { 0xF0FBFF,L"乳白" },
        { 0x000000,L"黑色" },
        { 0xFFFFFF,L"白色" },
        { 0xFF8080,L"蓝灰" },
        { 0xE03058,L"藏蓝" },
        { 0x00E080,L"嫩绿" },
        { 0x80E000,L"青绿" },
        { 0x0060C0,L"黄褐" },
        { 0xFFA8FF,L"粉红" },
        { 0x00D8D8,L"嫩黄" },
        { 0xECECEC,L"银白" },
        { 0xFF0090,L"紫色" },
        { 0xFF8800,L"天蓝" },
        { 0x80A080,L"灰绿" },
        { 0xC06000,L"青蓝" },
        { 0x0080FF,L"橙黄" },
        { 0x8050FF,L"桃红" },
        { 0xC080FF,L"芙红" },
        { 0x606060,L"深灰" }
    };
    constexpr static int ItemPadding = 2;
    constexpr static int ClrBlockWidth = 20;
public:
    LRESULT OnNotifyMsg(HWND hParent, UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bProcessed) override
    {
        switch (uMsg)
        {
        case WM_NOTIFY:
        {
            switch (((NMHDR*)lParam)->code)
            {
            case NM_CUSTOMDRAW:
            case NM_CBN_LBCUSTOMDRAW:
            {
                bProcessed = TRUE;
                const auto p = (NMCUSTOMDRAWEXT*)lParam;
                switch (p->dwDrawStage)
                {
                case CDDS_PREPAINT:
                case CDDS_ITEMPREPAINT:
                    return CDRF_NOTIFYPOSTPAINT | CDRF_NOTIFYITEMDRAW;
                case CDDS_POSTPAINT:
                case CDDS_ITEMPOSTPAINT:
                {
                    if (p->hdr.code == NM_CBN_LBCUSTOMDRAW &&
                        p->dwDrawStage == CDDS_POSTPAINT)
                        break;
                    int idx;
                    if (p->hdr.code == NM_CBN_LBCUSTOMDRAW)
                        idx = (int)p->dwItemSpec;
                    else
                        idx = GetListBox().GetCurrSel();
                    if (idx < 0)
                        return CDRF_DODEFAULT;
                    COLORREF cr = PresetColor[idx].cr;
                    HBRUSH hbr;
                    HDC hDC = p->hdc;

                    const auto* const ptc = GetThreadCtx();
                    const auto crText = (p->uItemState & CDIS_SELECTED) ?
                        ptc->crHiLightText : ptc->crDefText;

                    if (cr == CLR_DEFAULT)
                        hbr = CreateHatchBrush(HS_BDIAGONAL, crText);
                    else if (cr == CLR_INVALID)
                        hbr = CreateSolidBrush(m_crCustom);
                    else
                        hbr = CreateSolidBrush(cr);

                    int iItemPadding = DpiScale(ItemPadding, m_iDpi);
                    int cxClrBlock = DpiScale(ClrBlockWidth, m_iDpi);

                    HGDIOBJ hOldBr = SelectObject(hDC, hbr);
                    SetDCPenColor(hDC, crText);
                    HGDIOBJ hOldPen = SelectObject(hDC, GetStockBrush(DC_PEN));
                    int xClrBlock = p->rc.left + iItemPadding;
                    Rectangle(hDC,
                        xClrBlock,
                        p->rc.top + iItemPadding,
                        xClrBlock + cxClrBlock,
                        p->rc.bottom - iItemPadding);
                    SelectObject(hDC, hOldPen);
                    DeleteObject(SelectObject(hDC, hOldBr));

                    SetTextColor(hDC, p->crText);

                    RECT rcText = p->rc;
                    rcText.left += (xClrBlock + cxClrBlock + iItemPadding);
                    const auto sv = PresetColor[idx].svName;
                    DrawTextW(hDC, sv.data(), (int)sv.size(), &rcText,
                        DT_NOCLIP | DT_SINGLELINE | DT_VCENTER | DT_NOPREFIX);
                }
                break;
                }
            }
            return CDRF_DODEFAULT;
            case NM_LBN_ITEMCHANGED:
            {
                bProcessed = TRUE;
                const auto p = (NMLBNITEMCHANGED*)lParam;
                if ((p->uFlagsNew & LBN_IF_SEL) && !(p->uFlagsOld & LBN_IF_SEL))
                {
                    COLORREF cr;
                    if (p->idx == IdxCustom)
                    {
                        CHOOSECOLORW cc{ sizeof(CHOOSECOLORW) };
                        cc.hwndOwner = hParent;
                        cc.lpCustColors = m_crCcDlgCustom;
                        cc.Flags = CC_ANYCOLOR | CC_FULLOPEN;
                        m_crCcDlgCustom[0] = m_crCustom;
                        if (ChooseColorW(&cc))
                        {
                            m_crCustom = cc.rgbResult;
                            InvalidateRect((HWND)lParam, nullptr, FALSE);
                        }
                        cr = m_crCustom;
                        Redraw();
                    }
                    else
                        cr = PresetColor[p->idx].cr;
                    NMCLPCLRCHANGED nm;
                    nm.cr = cr;
                    FillNmhdrAndSendNotify(nm, m_hParent, NM_CLP_CLRCHANGED);
                }
            }
            return 0;
            }
        }
        return 0;
        }
        return __super::OnNotifyMsg(hParent, uMsg, wParam, lParam, bProcessed);
    }

    LRESULT OnMsg(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) override
    {
        switch (uMsg)
        {
        case WM_CREATE:
        {
            const auto lResult = __super::OnMsg(hWnd, uMsg, wParam, lParam);
            GetListBox().SetItemCount(ARRAYSIZE(PresetColor));
            return lResult;
        }
        }
        return __super::OnMsg(hWnd, uMsg, wParam, lParam);
    }

    COLORREF GetColor() noexcept
    {
        const auto idx = GetListBox().GetCurrSel();
        if (idx < 0)
            return CLR_INVALID;
        const auto cr = PresetColor[idx].cr;
        if (cr == CLR_INVALID)
            return m_crCustom;
        else
            return cr;
    }

    BOOL SetColor(COLORREF cr) noexcept
    {
        if (cr == CLR_INVALID)
            return FALSE;
        auto& LB = GetListBox();
        LB.SetGenerateItemNotify(FALSE);
        for (int i{}; const auto& e : PresetColor)
        {
            if (e.cr == cr)
            {
                LB.SetCurrSel(i);
                LB.SetGenerateItemNotify(TRUE);
                return TRUE;
            }
            ++i;
        }
        m_crCustom = cr;
        LB.SetCurrSel(1);
        LB.SetGenerateItemNotify(TRUE);
        return TRUE;
    }

    void SelectColor(int idx) noexcept
    {
        auto& LB = GetListBox();
        LB.SetGenerateItemNotify(FALSE);
        LB.SetCurrSel(idx);
        LB.SetGenerateItemNotify(TRUE);
    }
};
ECK_RTTI_IMPL_BASE_INLINE(CColorPicker, CComboBoxNew);
ECK_NAMESPACE_END