#pragma once
#include "CWnd.h"

ECK_NAMESPACE_BEGIN
inline constexpr int CDV_BUTTON_1 = 1;

#pragma pack(push, ECK_CTRLDATA_ALIGN)
struct CTRLDATA_BUTTON
{
    int iVer;
    BYTE eCheckState;
    DWORD cchNote;
    // WCHAR szNote[];// 长度为cchNote + 1

    EckInline constexpr PCWSTR Note() const
    {
        return (PCWSTR)PtrSkipType(this);
    }
};
#pragma pack(pop)

inline constexpr DWORD ButtonTypeMask = (BS_PUSHBUTTON | BS_DEFPUSHBUTTON |
    BS_SPLITBUTTON | BS_DEFSPLITBUTTON | BS_COMMANDLINK | BS_DEFCOMMANDLINK |
    BS_RADIOBUTTON | BS_AUTORADIOBUTTON | BS_CHECKBOX | BS_AUTOCHECKBOX |
    BS_3STATE | BS_AUTO3STATE | BS_GROUPBOX);

// 建议直接使用此类代替其他细分类
class CButton :public CWnd
{
public:
    ECK_RTTI(CButton);
    ECK_CWND_NOSINGLEOWNER(CButton);
    ECK_CWND_CREATE_CLS(WC_BUTTONW);

    enum class Type
    {
        PushButton,
        DefPushButton,
        CheckBox,
        AutoCheckButton,
        RadioButton,
        TripleState,
        AutoTripleState,
        GroupBox,
        UserButton,// 已弃用
        AutoRadioButton,
        PushBox,// 与PushButton类似，但仅显示文本
        OwnerDraw,
        SplitButton,
        DefSplitButton,
        CommandLink,
        DefCommandLink,

        Unknown = -1
    };
public:
    ECK_CWNDPROP_STYLE_MASK(TripleState, BS_3STATE, ButtonTypeMask);
    ECK_CWNDPROP_STYLE_MASK(AutoTripleState, BS_AUTO3STATE, ButtonTypeMask);
    ECK_CWNDPROP_STYLE_MASK(AutoCheckButton, BS_AUTOCHECKBOX, ButtonTypeMask);
    ECK_CWNDPROP_STYLE_MASK(AutoRadioButton, BS_AUTORADIOBUTTON, ButtonTypeMask);
    ECK_CWNDPROP_STYLE(ShowBitmap, BS_BITMAP);
    ECK_CWNDPROP_STYLE(AlignBottom, BS_BOTTOM);
    ECK_CWNDPROP_STYLE(AlignCenter, BS_CENTER);
    ECK_CWNDPROP_STYLE_MASK(CheckBox, BS_CHECKBOX, ButtonTypeMask);
    ECK_CWNDPROP_STYLE(CommandLink, BS_COMMANDLINK);
    ECK_CWNDPROP_STYLE(DefCommandLink, BS_DEFCOMMANDLINK);
    ECK_CWNDPROP_STYLE_MASK(DefPushButton, BS_DEFPUSHBUTTON, ButtonTypeMask);
    ECK_CWNDPROP_STYLE(DefSplitButton, BS_DEFSPLITBUTTON);
    ECK_CWNDPROP_STYLE_MASK(GroupBox, BS_GROUPBOX, ButtonTypeMask);
    ECK_CWNDPROP_STYLE(ShowIcon, BS_ICON);
    ECK_CWNDPROP_STYLE(Flat, BS_FLAT);
    ECK_CWNDPROP_STYLE(AlignLeft, BS_LEFT);
    ECK_CWNDPROP_STYLE(MultiLine, BS_MULTILINE);
    ECK_CWNDPROP_STYLE(Notify, BS_NOTIFY);
    ECK_CWNDPROP_STYLE_MASK(OwnerDraw, BS_OWNERDRAW, ButtonTypeMask);
    ECK_CWNDPROP_STYLE_MASK(PushBox, BS_PUSHBOX, ButtonTypeMask);
    ECK_CWNDPROP_STYLE_MASK(PushButton, BS_PUSHBUTTON, ButtonTypeMask);
    ECK_CWNDPROP_STYLE(PushLike, BS_PUSHLIKE);
    ECK_CWNDPROP_STYLE_MASK(RadioButton, BS_RADIOBUTTON, ButtonTypeMask);
    ECK_CWNDPROP_STYLE(AlignRight, BS_RIGHT);
    ECK_CWNDPROP_STYLE(RightButton, BS_RIGHTBUTTON);
    ECK_CWNDPROP_STYLE(SplitButton, BS_SPLITBUTTON);
    ECK_CWNDPROP_STYLE(ShowText, BS_TEXT);
    ECK_CWNDPROP_STYLE(AlignTop, BS_TOP);
    ECK_CWNDPROP_STYLE(AlignVCenter, BS_VCENTER);

    EckInlineNdCe static PCVOID SkipBaseData(PCVOID p) noexcept
    {
        const auto* const p2 = (CTRLDATA_BUTTON*)CWnd::SkipBaseData(p);
        return PtrStepCb(p2, sizeof(CTRLDATA_BUTTON) + (p2->cchNote + 1) * sizeof(WCHAR));
    }

    void SerializeData(CRefBin& rb, const SERIALIZE_OPT* pOpt = nullptr) override
    {
        auto cchNote = GetNoteLength();
        const SIZE_T cbSize = sizeof(CTRLDATA_BUTTON) +
            (cchNote + 1) * sizeof(WCHAR);
        CWnd::SerializeData(rb, pOpt);
        CMemWriter w(rb.PushBack(cbSize), cbSize);

        CTRLDATA_BUTTON* p;
        w.SkipPointer(p);
        p->iVer = CDV_BUTTON_1;
        p->eCheckState = GetCheckState();
        p->cchNote = cchNote;
        if (cchNote)
        {
            ++cchNote;
            GetNote((PWSTR)w.Data(), cchNote);
        }
        else
            *(PWSTR)w.Data() = L'\0';
    }

    void PostDeserialize(PCVOID pData) override
    {
        CWnd::PostDeserialize(pData);
        const auto* const p = (const CTRLDATA_BUTTON*)CWnd::SkipBaseData(pData);
        if (p->iVer != CDV_BUTTON_1)
            return;
        SetCheckState(p->eCheckState);
        if (p->cchNote)
            SetNote(p->Note());
    }

    EckInline BOOL GetIdealSize(_Out_ SIZE* psize) const noexcept
    {
        return (int)SendMsg(BCM_GETIDEALSIZE, 0, (LPARAM)psize);
    }

    EckInline BOOL GetImageList(_Out_ BUTTON_IMAGELIST* pbil) const noexcept
    {
        return (BOOL)SendMsg(BCM_GETIMAGELIST, 0, (LPARAM)pbil);
    }

    ECK_SUPPRESS_MISSING_ZERO_TERMINATION;
    EckInline BOOL GetNote(_Out_writes_(cchBuf) PWSTR pszBuf, _Inout_ DWORD& cchBuf) const noexcept
    {
        return (BOOL)SendMsg(BCM_GETNOTE, (WPARAM)&cchBuf, (LPARAM)pszBuf);
    }

    EckInlineNd DWORD GetNoteLength() const noexcept
    {
        return (DWORD)SendMsg(BCM_GETNOTELENGTH, 0, 0);
    }

    BOOL GetNote(_Inout_ CRefStrW& rs) const noexcept
    {
        DWORD cch = GetNoteLength();
        if (!cch)
            return FALSE;
        rs.PushBack(cch);
        ++cch;
        return GetNote(rs.Data(), cch);
    }
    // For compatibility
    EckInlineNd CRefStrW GetNote() const noexcept
    {
        CRefStrW rs;
        GetNote(rs);
        return rs;
    }

    EckInline BOOL GetSplitInfo(_Inout_ BUTTON_SPLITINFO* pbsi) const noexcept
    {
        return (BOOL)SendMsg(BCM_GETSPLITINFO, 0, (LPARAM)pbsi);
    }

    EckInline BOOL GetTextMargin(_Out_ RECT* prc) const noexcept
    {
        return (BOOL)SendMsg(BCM_GETTEXTMARGIN, 0, (LPARAM)prc);
    }

    EckInline BOOL SetDropDownState(BOOL bDropDown) const noexcept
    {
        return (BOOL)SendMsg(BCM_SETDROPDOWNSTATE, bDropDown, 0);
    }

    EckInline BOOL SetImageList(_In_ BUTTON_IMAGELIST* pbil) const noexcept
    {
        return (BOOL)SendMsg(BCM_SETIMAGELIST, 0, (LPARAM)pbil);
    }

    EckInline BOOL SetNote(_In_z_ PCWSTR pszText) const noexcept
    {
        return (BOOL)SendMsg(BCM_SETNOTE, 0, (LPARAM)pszText);
    }

    EckInline void SetShieldIcon(BOOL bShieldIcon) const noexcept
    {
        SendMsg(BCM_SETSHIELD, 0, bShieldIcon);
    }

    EckInline BOOL SetSplitInfo(_In_ BUTTON_SPLITINFO* pbsi) const noexcept
    {
        return (BOOL)SendMsg(BCM_SETSPLITINFO, 0, (LPARAM)pbsi);
    }

    EckInline BOOL SetTextMargin(_In_ RECT* prc) const noexcept
    {
        return (BOOL)SendMsg(BCM_SETTEXTMARGIN, 0, (LPARAM)prc);
    }

    EckInline void Click() const noexcept { SendMsg(BM_CLICK, 0, 0); }

    EckInline void SetCheckState(int iState) const noexcept { SendMsg(BM_SETCHECK, iState, 0); }

    EckInline int GetCheckState() const noexcept { return (int)SendMsg(BM_GETCHECK, 0, 0); }

    EckInline HANDLE GetImage(UINT uType) const noexcept
    {
        return (HANDLE)SendMsg(BM_GETIMAGE, uType, 0);
    }

    EckInline UINT GetState() const noexcept { return (UINT)SendMsg(BM_GETSTATE, 0, 0); }

    EckInline void SetDontClick(BOOL bDontClick) const noexcept { SendMsg(BM_SETDONTCLICK, bDontClick, 0); }

    EckInline HANDLE SetImage(HANDLE hImage, UINT uType) const noexcept
    {
        return (HANDLE)SendMsg(BM_SETIMAGE, uType, (LPARAM)hImage);
    }

    // 该消息仅能设置按下状态
    EckInline void SetState(BOOL bPressed) const noexcept { SendMsg(BM_SETSTATE, bPressed, 0); }

    EckInline void SetButtonStyle(DWORD dwStyle, BOOL bRedraw = TRUE) const noexcept
    {
        SendMsg(BM_SETSTYLE, dwStyle, bRedraw);
    }

    EckInline Type GetButtonType() const noexcept { return Type(Style & ButtonTypeMask); }

    void SetButtonType(Type eType) const noexcept
    {
        Style = (Style & ~ButtonTypeMask) | (DWORD)eType;
    }
    BOOL GetButtonDefault() const noexcept
    {
        return !!(Style &
            (BS_DEFPUSHBUTTON | BS_DEFSPLITBUTTON | BS_DEFCOMMANDLINK));
    }

    void SetButtonDefault(BOOL bDef) const noexcept
    {
        auto dwStyle = Style;
        auto iType = dwStyle & ButtonTypeMask;
        if (iType == BS_PUSHBUTTON || iType == BS_DEFPUSHBUTTON)
            iType = bDef ? BS_DEFPUSHBUTTON : BS_PUSHBUTTON;
        else if (iType == BS_SPLITBUTTON || iType == BS_DEFSPLITBUTTON)
            iType = bDef ? BS_DEFSPLITBUTTON : BS_SPLITBUTTON;
        else if (iType == BS_COMMANDLINK || iType == BS_DEFCOMMANDLINK)
            iType = bDef ? BS_DEFCOMMANDLINK : BS_COMMANDLINK;
        else
            return;
        dwStyle = (dwStyle & ~ButtonTypeMask) | iType;
        Style = dwStyle;
    }

    /// <summary>
    /// 置图片文本同时显示
    /// </summary>
    /// <param name="bShowTextAndImage">是否同时显示</param>
    void SetTextImageShowing(BOOL bShowTextAndImage) const noexcept
    {
        DWORD dwStyle = GetStyle();
        if (bShowTextAndImage)
            dwStyle &= ~(BS_BITMAP);
        else if (SendMsg(BM_GETIMAGE, IMAGE_BITMAP, 0) || SendMsg(BM_GETIMAGE, IMAGE_ICON, 0))
            dwStyle |= BS_BITMAP;
        SetStyle(dwStyle);
    }

    void SetAlign(BOOL bHAlign, Align iAlign) const noexcept
    {
        DWORD dwStyle = GetStyle();
        if (bHAlign)
        {
            dwStyle &= (~(BS_LEFT | BS_CENTER | BS_RIGHT));
            switch (iAlign)
            {
            case Align::Near: dwStyle |= BS_LEFT; break;
            case Align::Center: dwStyle |= BS_CENTER; break;
            case Align::Far: dwStyle |= BS_RIGHT; break;
            }
        }
        else
        {
            dwStyle &= (~(BS_TOP | BS_VCENTER | BS_BOTTOM));
            switch (iAlign)
            {
            case Align::Near: dwStyle |= BS_TOP; break;
            case Align::Center: dwStyle |= BS_VCENTER; break;
            case Align::Far: dwStyle |= BS_BOTTOM; break;
            }
        }
        SetStyle(dwStyle);
    }

    Align GetAlign(BOOL bHAlign) const noexcept
    {
        DWORD dwStyle = GetStyle();
        if (bHAlign)
        {
            if (IsBitSet(dwStyle, BS_CENTER))
                return Align::Center;
            else if (IsBitSet(dwStyle, BS_RIGHT))
                return Align::Far;
            else
                return Align::Near;
        }
        else
        {
            if (IsBitSet(dwStyle, BS_VCENTER))
                return Align::Center;
            else if (IsBitSet(dwStyle, BS_BOTTOM))
                return Align::Far;
            else
                return Align::Near;
        }
    }
};
ECK_RTTI_IMPL_BASE_INLINE(CButton, CWnd);

// 普通按钮
class CPushButton :public CButton
{
public:
    /// <summary>
    /// 置类型
    /// </summary>
    /// <param name="iType">类型，0 - 普通  1 - 拆分</param>
    void SetType(int iType) noexcept
    {
        const BOOL bDef = GetDef();
        DWORD dwStyle = GetStyle() &
            ~(BS_PUSHBUTTON | BS_SPLITBUTTON | BS_DEFPUSHBUTTON | BS_DEFSPLITBUTTON);

        switch (iType)
        {
        case 0:
            dwStyle |= (bDef ? BS_DEFPUSHBUTTON : BS_PUSHBUTTON);
            break;
        case 1:
            dwStyle |= (bDef ? BS_DEFSPLITBUTTON : BS_SPLITBUTTON);
            break;
        }

        SetStyle(dwStyle);
    }

    /// <summary>
    /// 取类型
    /// </summary>
    /// <returns>类型，0 - 普通  1 - 拆分  -1 - 未知</returns>
    int GetType() noexcept
    {
        const auto dwStyle = GetStyle();
        if (IsBitSet(dwStyle, BS_SPLITBUTTON) || IsBitSet(dwStyle, BS_DEFSPLITBUTTON))
            return 1;
        else if (IsBitSet(dwStyle, BS_PUSHBUTTON) || IsBitSet(dwStyle, BS_DEFPUSHBUTTON))
            return 0;
        return -1;
    }

    void SetDef(BOOL bDef) noexcept
    {
        const int iType = GetType();
        DWORD dwStyle = GetStyle() &
            ~(BS_DEFPUSHBUTTON | BS_DEFSPLITBUTTON | BS_PUSHBUTTON | BS_SPLITBUTTON);
        if (bDef)
            if (iType)
                dwStyle |= BS_DEFSPLITBUTTON;
            else
                dwStyle |= BS_DEFPUSHBUTTON;
        else
            if (iType)
                dwStyle |= BS_SPLITBUTTON;
            else
                dwStyle |= BS_PUSHBUTTON;

        SetStyle(dwStyle);
    }

    EckInline BOOL GetDef() noexcept
    {
        const auto dwStyle = GetStyle();
        return (IsBitSet(dwStyle, BS_DEFSPLITBUTTON) || IsBitSet(dwStyle, BS_DEFPUSHBUTTON));
    }
};

// 选择框
class CCheckButton :public CButton
{
public:
    /// <summary>
    /// 置类型
    /// </summary>
    /// <param name="iType">类型，0 - 单选框  1 - 复选框  2 - 三态复选框</param>
    void SetType(int iType) noexcept
    {
        DWORD dwStyle = GetStyle() &
            ~(BS_AUTORADIOBUTTON | BS_AUTOCHECKBOX | BS_AUTO3STATE);
        switch (iType)
        {
        case 0: dwStyle |= BS_AUTORADIOBUTTON; break;
        case 1: dwStyle |= BS_AUTOCHECKBOX; break;
        case 2: dwStyle |= BS_AUTO3STATE; break;
        default: EckDbgBreak(); return;
        }
        SetStyle(dwStyle);
    }

    /// <summary>
    /// 取类型
    /// </summary>
    /// <returns>类型，0 - 单选框  1 - 复选框  2 - 三态复选框  -1 - 未知</returns>
    int GetType() noexcept
    {
        DWORD dwStyle = GetStyle();
        if (IsBitSet(dwStyle, BS_AUTORADIOBUTTON))
            return 0;
        else if (IsBitSet(dwStyle, BS_AUTOCHECKBOX))
            return 1;
        else if (IsBitSet(dwStyle, BS_AUTO3STATE))
            return 2;
        else
            return -1;
    }
};

// 命令链接
class CCommandLink :public CButton
{
public:
    void SetDef(BOOL bDef) noexcept
    {
        DWORD dwStyle = GetStyle() &
            ~(BS_DEFPUSHBUTTON | BS_PUSHBUTTON | BS_DEFCOMMANDLINK | BS_COMMANDLINK);
        if (bDef)
            dwStyle |= BS_DEFCOMMANDLINK;
        else
            dwStyle |= BS_COMMANDLINK;

        SetStyle(dwStyle);
    }

    EckInline BOOL GetDef() noexcept
    {
        return IsBitSet(Style, BS_DEFCOMMANDLINK);
    }
};
ECK_NAMESPACE_END