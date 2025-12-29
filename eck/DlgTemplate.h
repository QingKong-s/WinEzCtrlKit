#pragma once
#include "CMemWalker.h"

ECK_NAMESPACE_BEGIN

#pragma pack(push, 2)
struct DLGTHEADER
{
    WORD      wVer;// 必须为1;
    WORD      wSignature;// 必须为0xFFFF;
    DWORD     idHelp;
    DWORD     dwExStyle;
    DWORD     dwStyle;
    WORD      cDlgItems;
    short     x;
    short     y;
    short     cx;
    short     cy;
};

/*
DLGTHEADER后跟以下结构
struct DLGTMENU
{
    WCHAR ch;
    // ch == 0 - 无菜单，结构终止
    union
    {
        WORD wID;		// ch == 0xFFFF - 菜单资源ID
        WCHAR szMenu[n];// 否则			- 以NULL结尾的菜单名称
    };
};

struct DLGTWCLS
{
    WCHAR ch;
    // ch == 0 - 标准对话框类，结构终止
    union
    {
        WORD atom;		// ch == 0xFFFF - 预定义类原子
        WCHAR szCls[n];	// 否则			- 以NULL结尾的窗口类名
    };
};

struct DLGTCAPTION
{
    WCHAR ch;
    // ch == 0 - 无标题，结构终止
    WCHAR szCap[n];		// 否则			- 以NULL结尾的标题
};
上述三个数组必须在WORD边界上对齐

struct DLGTFONT// 仅当指定了DS_SETFONT或DS_SHELLFONT时存在
{
    WORD      wPoint;
    WORD      wWeight;
    BYTE      bItalic;
    BYTE      byCharSet;
    WCHAR     szName[n];// 以NULL结尾的字体名称
};
*/


// 每个DLGITEMTHEADER都必须在DWORD边界上对齐

struct DLGITEMTHEADER
{
    DWORD     idHelp;
    DWORD     dwExStyle;
    DWORD     dwStyle;
    short     x;
    short     y;
    short     cx;
    short     cy;
    DWORD     id;
};

/*
DLGITEMTHEADER后跟以下结构
struct DLGITEMTWCLS
{
    WCHAR ch;
    union
    {
        WORD atom;		// ch == 0xFFFF - 预定义类原子
        WCHAR szCls[n];	// 否则			- 以NULL结尾的窗口类名
    };
};

struct DLGITEMTCAPTION
{
    WCHAR ch;
    union
    {
        WORD atom;		// ch == 0xFFFF - 资源ID
        WCHAR szCap[n];	// 否则			- 以NULL结尾的标题
    };
};

struct DLGITEMTEXTRA
{
    WORD cbExtra;
    BYTE bYExtra[cbExtra];
};
*/
#pragma pack(pop)

struct DLGTDLG : DLGTHEADER
{
    std::optional<std::variant<WORD, CRefStrW>> Menu;
    std::optional<std::variant<ATOM, CRefStrW>> Class;
    std::optional<CRefStrW> Caption;
    struct
    {
        WORD      wPoint;
        WORD      wWeight;
        BYTE      bItalic;
        BYTE      byCharSet;
    } Font;
    CRefStrW rsFontName;

    PCWSTR GetMenu() const
    {
        if (Menu)
        {
            auto& Val = *Menu;
            if (Val.index() == 0)// 资源ID
                return ECKMAKEINTATOMW(std::get<0>(Val));
            else// 菜单名字符串
                return std::get<1>(Val).Data();
        }
        else
            return nullptr;
    }

    PCWSTR GetClass() const
    {
        if (Class)
        {
            auto& Val = *Class;
            if (Val.index() == 0)// 类原子
                return ECKMAKEINTATOMW(std::get<0>(Val));
            else// 类名字符串
                return std::get<1>(Val).Data();
        }
        else
            return ECKMAKEINTATOMW(32770);
    }

    PCWSTR GetCaption() const
    {
        if (Caption)
            return Caption->Data();
        else
            return nullptr;
    }
};

struct DLGTITEM : DLGITEMTHEADER
{
    std::variant<ATOM, CRefStrW> Class;
    std::variant<WORD, CRefStrW> Caption;
    CRefBin rbExtra;

    PCWSTR GetClass() const
    {
        if (Class.index() == 0)// 类原子
            return ECKMAKEINTATOMW(std::get<0>(Class));
        else// 类名字符串
            return std::get<1>(Class).Data();
    }
};


/// <summary>
/// 序列化对话框模板
/// </summary>
/// <param name="rbTemplate">序列化后的对话框模板字节集，将在原有内容后追加</param>
/// <param name="Dlg">对话框信息，调用前必须将wVer设为1，wSignature设为0xFFFF，cDlgItems设为与Items元素数相同</param>
/// <param name="vItem">对话框项目信息</param>
/// <returns>HRESULT</returns>
inline HRESULT SerializeDialogTemplate(
    CRefBin& rbTemplate,
    const DLGTDLG& Dlg,
    const std::vector<DLGTITEM>& vItem) noexcept
{
    using namespace eck::Literals;
    if (Dlg.wVer != 1_us || Dlg.wSignature != 0xFFFF_us)
        return HRESULT_FROM_WIN32(ERROR_BAD_FORMAT);
    if ((size_t)Dlg.cDlgItems != vItem.size())
        return E_BOUNDS;
    SIZE_T cbTotal =
        sizeof(DLGTHEADER) /*头*/ +
        sizeof(WORD) * 3 /*菜单、窗口类、标题三个数组的第一个元素*/;

    if (Dlg.Menu)
    {
        auto& Val = *Dlg.Menu;
        if (Val.index() == 0)// 资源ID
            cbTotal += sizeof(WORD);
        else// 名称字符串
            cbTotal += ((std::get<1>(Val).Size() + 1) * sizeof(WCHAR));
    }

    if (Dlg.Class)
    {
        auto& Val = *Dlg.Class;
        if (Val.index() == 0)// 类原子
            cbTotal += sizeof(ATOM);
        else// 类名字符串
            cbTotal += ((std::get<1>(Val).Size() + 1) * sizeof(WCHAR));
    }

    if (Dlg.Caption)
    {
        auto& Val = *Dlg.Caption;
        cbTotal += ((Val.Size() + 1) * sizeof(WCHAR));
    }

    const auto bHasFontStru = IsBitSet(Dlg.dwStyle, DS_SETFONT) ||
        IsBitSet(Dlg.dwStyle, DS_SHELLFONT);
    if (bHasFontStru)
    {
        cbTotal += sizeof(Dlg.Font);
        cbTotal += ((Dlg.rsFontName.Size() + 1) * sizeof(WCHAR));
    }

    cbTotal += CalcNextAlignBoundaryDistance(nullptr, (PCVOID)cbTotal, sizeof(DWORD));

    cbTotal += (vItem.size() * (
        sizeof(DLGITEMTHEADER) /*头*/ +
        sizeof(WORD) * 3 /*窗口类、标题两个数组的第一个元素和附加数据大小*/));
    for (auto& e : vItem)
    {
        if (e.Class.index() == 0)// 类原子
            cbTotal += sizeof(ATOM);
        else// 类名字符串
            cbTotal += ((std::get<1>(e.Class).Size() + 1) * sizeof(WCHAR));

        if (e.Caption.index() == 0)// 资源ID
            cbTotal += sizeof(WORD);
        else// 标题字符串
            cbTotal += ((std::get<1>(e.Caption).Size() + 1) * sizeof(WCHAR));

        if (e.rbExtra.Size())
            cbTotal += AlignMemSize(e.rbExtra.Size(), 2);

        cbTotal += CalcNextAlignBoundaryDistance(nullptr, (PCVOID)cbTotal, sizeof(DWORD));
    }

    rbTemplate.PushBackNoExtra(cbTotal);

    CMemWriter w(rbTemplate.Data(), rbTemplate.Size());
    w.Write(&Dlg, sizeof(DLGTHEADER));

    if (Dlg.Menu)
    {
        auto& Val = *Dlg.Menu;
        if (Val.index() == 0)// 资源ID
            w << 0xFFFF_us << std::get<0>(Val);
        else// 名称字符串
            w << std::get<1>(Val);
    }
    else
        w << 0_us;

    if (Dlg.Class)
    {
        auto& Val = *Dlg.Class;
        if (Val.index() == 0)// 类原子
            w << 0xFFFF_us << std::get<0>(Val);
        else// 类名字符串
            w << std::get<1>(Val);
    }
    else
        w << 0_us;

    if (Dlg.Caption)
        w << *Dlg.Caption;
    else
        w << 0_us;

    if (bHasFontStru)
        w << Dlg.Font << Dlg.rsFontName;

    w += CalcNextAlignBoundaryDistance(rbTemplate.Data(), w.Data(), sizeof(DWORD));

    for (const auto& x : vItem)
    {
        w.Write(&x, sizeof(DLGITEMTHEADER));

        if (x.Class.index() == 0)// 类原子
            w << 0xFFFF_us << std::get<0>(x.Class);
        else// 类名字符串
            w << std::get<1>(x.Class);

        if (x.Caption.index() == 0)// 资源ID
            w << 0xFFFF_us << std::get<0>(x.Caption);
        else// 标题字符串
            w << std::get<1>(x.Caption);

        w << (WORD)x.rbExtra.Size();
        if (x.rbExtra.Size())
        {
            memcpy(w.Data(), x.rbExtra.Data(), x.rbExtra.Size());
            w += AlignMemSize(x.rbExtra.Size(), 2);
        }

        w += CalcNextAlignBoundaryDistance(rbTemplate.Data(), w.Data(), sizeof(DWORD));
    }

    return S_OK;
}

/// <summary>
/// 反序列化对话框模板
/// </summary>
/// <param name="pTemplate">对话框模板指针</param>
/// <param name="Dlg">接收对话框信息变量，若函数失败，则此参数内容无法确定</param>
/// <param name="vItem">接收对话框项目信息变量</param>
/// <returns>HRESULT</returns>
inline HRESULT DeserializeDialogTemplate(
    _In_reads_bytes_(cbTemplate) PCVOID pTemplate,
    size_t cbTemplate,
    _Inout_ DLGTDLG& Dlg,
    _Inout_ std::vector<DLGTITEM>& Items) noexcept
{
    using namespace eck::Literals;
    CMemReader r{ pTemplate, cbTemplate };
    r.Read(&Dlg, sizeof(DLGTHEADER));
    if (Dlg.wVer != 1_us || Dlg.wSignature != 0xFFFF_us)
        return HRESULT_FROM_WIN32(ERROR_BAD_FORMAT);

    WORD us;
    CRefStrW rs{};

    r >> us;
    switch (us)
    {
    case 0_us:
        Dlg.Menu.reset();
        break;
    case 0xFFFF_us:
        r >> us;
        Dlg.Menu = us;
        break;
    default:
        r -= sizeof(WORD);
        r >> rs;
        Dlg.Menu = std::move(rs);
        break;
    }

    r >> us;
    switch (us)
    {
    case 0_us:
        Dlg.Class.reset();
        break;
    case 0xFFFF_us:
        r >> us;
        Dlg.Class = us;
        break;
    default:
        r -= sizeof(WORD);
        r >> rs;
        Dlg.Class = std::move(rs);
        break;
    }

    r >> us;
    if (us)
    {
        r -= sizeof(WORD);
        r >> rs;
        Dlg.Caption = std::move(rs);
    }
    else
        Dlg.Caption.reset();

    if (IsBitSet(Dlg.dwStyle, DS_SETFONT) || IsBitSet(Dlg.dwStyle, DS_SHELLFONT))
    {
        r >> Dlg.Font;
        r >> rs;
        Dlg.rsFontName = std::move(rs);
    }
    else
        Dlg.rsFontName.Clear();

    r += CalcNextAlignBoundaryDistance(pTemplate, r.Data(), sizeof(DWORD));

    Items.resize(Dlg.cDlgItems);
    for (auto& x : Items)
    {
        r.Read(&x, sizeof(DLGITEMTHEADER));

        r >> us;
        if (us == 0xFFFF_us)
        {
            r >> us;
            x.Class = us;
        }
        else
        {
            r -= sizeof(WCHAR);
            r >> rs;
            x.Class = std::move(rs);
        }

        r >> us;
        if (us == 0xFFFF_us)
        {
            r >> us;
            x.Caption = us;
        }
        else
        {
            r -= sizeof(WCHAR);
            r >> rs;
            x.Caption = std::move(rs);
        }

        r >> us;
        if (us)
        {
            CRefBin rb;
            rb.ReSize(us);
            r.Read(rb.Data(), rb.Size());
            x.rbExtra = std::move(rb);
        }
        else
            x.rbExtra.Clear();

        r += CalcNextAlignBoundaryDistance(pTemplate, r.Data(), sizeof(DWORD));
    }

    return S_OK;
}
ECK_NAMESPACE_END