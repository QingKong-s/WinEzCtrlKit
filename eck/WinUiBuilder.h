#pragma once
#include "CWindow.h"
#include "CLinearLayout.h"

#define ECK_UIBUILDER_NAMESPACE_BEGIN   namespace UiBuilder {
#define ECK_UIBUILDER_NAMESPACE_END     }

ECK_NAMESPACE_BEGIN
ECK_UIBUILDER_NAMESPACE_BEGIN
struct Rect
{
    int x;
    int y;
    int cx;
    int cy;
};

struct Margin
{
    int l;
    int t;
    int r;
    int b;
};

constexpr inline BOOL Replace = TRUE;
constexpr inline BOOL Merge = FALSE;

struct Style
{
    DWORD dwStyle;
    BOOL bReplace;
};
struct ExStyle
{
    DWORD dwExStyle;
    BOOL bReplace;
};
struct Id { int iId; };
struct Weight { UINT uWeight; };
struct Flags
{
    UINT uFlags;
    BOOL bReplace;
};
struct Font { HFONT hFont; };

struct Window;
struct VBox;
struct HBox;
struct Default;
struct Local;

namespace Priv
{
    // WARNING 必须与Proxy完全对齐
    enum class Type : BYTE
    {
        Invalid,
        List,

        CWnd,
        CLayoutBase,
        String,
        Font,

        Window,
        VBox,
        HBox,

        Default,
        Local,

        Rect,
        Margin,

        Style,
        ExStyle,
        Id,
        Weight,
        Flags,
    };

    struct Proxy
    {
        std::variant <
            std::monostate,
            const std::initializer_list<Proxy>*,
            CWindow*,
            CLayoutBase*,
            PCWSTR,
            Font,

            const Window*,
            const VBox*,
            const HBox*,

            const Default*,
            const Local*,

            const Rect*,
            const Margin*,

            Style,
            ExStyle,
            Id,
            Weight,
            Flags
        > v{};

        Proxy(const std::initializer_list<Proxy>& x) noexcept : v{ &x } {}
        Proxy(CWindow& x) noexcept : v{ &x } {}
        Proxy(CLayoutBase& x) noexcept : v{ &x } {}
        Proxy(PCWSTR x) noexcept : v{ x } {}
        Proxy(Font x) noexcept : v{ x } {}

        Proxy(const Window& x) noexcept : v{ &x } {}
        Proxy(const VBox& x) noexcept : v{ &x } {}
        Proxy(const HBox& x) noexcept : v{ &x } {}
        Proxy(const Default& x) noexcept : v{ &x } {}
        Proxy(const Local& x) noexcept : v{ &x } {}

        Proxy(const Rect& x) noexcept : v{ &x } {}
        Proxy(const Margin& x) noexcept : v{ &x } {}

        Proxy(Style x) noexcept : v{ x } {}
        Proxy(ExStyle x) noexcept : v{ x } {}
        Proxy(Id x) noexcept : v{ x } {}
        Proxy(Weight x) noexcept : v{ x } {}
        Proxy(Flags x) noexcept : v{ x } {}

        EckInlineNdCe Type GetType() const noexcept { return (Type)v.index(); }

        template<Type E>
        EckInlineNdCe auto& Get() const noexcept { return std::get<(size_t)E>(v); }
    };

    struct Container
    {
        const std::initializer_list<Priv::Proxy>& il;

        Container(const std::initializer_list<Priv::Proxy>& il_) noexcept : il{ il_ } {}
    };
}

#define ECK_UI_DECL_CONTAINER(Name) \
    struct Name : Priv::Container \
    { \
        using Priv::Container::Container; \
    }

ECK_UI_DECL_CONTAINER(Window);
ECK_UI_DECL_CONTAINER(Default);
ECK_UI_DECL_CONTAINER(Local);
ECK_UI_DECL_CONTAINER(VBox);
ECK_UI_DECL_CONTAINER(HBox);

#undef ECK_UI_DECL_CONTAINER

enum class Result
{
    Ok,

    InvalidConfig,          // 不支持某种子配置
    InvalidConfigDefault,   // Default或Local内不支持某种子配置
    InvalidConfigWindow,    // Window内不支持某种子配置
    InvalidConfigLayout,    // 布局配置内不支持某种子配置

    InvalidLayout,      // 不支持的布局类型
    Win32,              // Win32错误，已设置LastError

    NoCWnd,             // 未指定CWnd
    NoCLayoutBase,      // 未指定CLayoutBase

    DefaultAfterLocal,  // Default出现在Local之后
};

struct ERR_CTX
{
    CStringW rsPath;
};

namespace Priv
{
    // 压入当前节点名称
    struct ScopedPath
    {
        ERR_CTX& ErrCtx;
        int cchOld;

        ScopedPath(ERR_CTX& ErrCtx_, std::wstring_view sv, int idxNode) noexcept
            : ErrCtx{ ErrCtx_ }, cchOld{ ErrCtx.rsPath.Size() }
        {
            constexpr size_t cchBuf = TcsCvtCalcBufferSize<int>();

            auto p = ErrCtx.rsPath.PushBack(int(sv.size() + cchBuf));
            TcsCopyLength(p, sv.data(), sv.size());
            p += sv.size();

            PWCH pEnd;
            TcsFromInt(p, cchBuf, idxNode, 10, TRUE, &pEnd);
            ErrCtx.rsPath.ReSize(int(pEnd - ErrCtx.rsPath.Data()));
        }
        void Pop() noexcept
        {
            ErrCtx.rsPath.ReSize(cchOld);
        }
    };

    struct PARENT_NODE
    {
        CWindow* pWndParent;
        CLayoutBase* pLytParent;
    };

    struct DEF_PARAM
    {
        HFONT hDefFont{};

        Rect DefRect{};
        DWORD dwDefStyle{ WS_CHILD | WS_VISIBLE };
        DWORD dwDefExStyle{};

        Margin DefMargin{};
        UINT uDefFlags{};
        UINT uDefWeight{};
    };

    struct LYT_PARAM
    {
        Margin Margin;
        UINT uFlags;
        union
        {
            UINT uWeight;
        };
    };

    inline Result CfgCreateWindow(
        const Priv::Proxy& pr,
        const PARENT_NODE& Parent,
        const DEF_PARAM& Default,
        const DEF_PARAM& Local,
        _Out_ LYT_PARAM& LytParam,
        _Out_ ILayout*& pNewObject,
        ERR_CTX& ErrCtx) noexcept;
    inline Result CfgCreateLinearLayout(
        const Priv::Proxy& pr,
        const PARENT_NODE& Parent,
        const DEF_PARAM& Default,
        const DEF_PARAM& Local,
        _Out_ LYT_PARAM& LytParam,
        _Out_ ILayout*& pNewObject,
        ERR_CTX& ErrCtx) noexcept;
    // idxNode: pr在同级配置中的序号，从0开始，用于诊断
    inline Result CfgCreate(
        const Priv::Proxy& pr,
        const PARENT_NODE& Parent,
        const DEF_PARAM& Default,
        const DEF_PARAM& Local,
        ERR_CTX& ErrCtx,
        int idxNode) noexcept;

    inline Result CfgParseDefault(
        const Priv::Proxy& pr,
        DEF_PARAM& Param,
        BOOL bDefaultOrLocal) noexcept
    {
        const auto& il = (bDefaultOrLocal ?
            pr.Get<Type::Default>()->il :
            pr.Get<Type::Local>()->il);
        for (const auto& e : il)
        {
            switch (e.GetType())
            {
            case Type::Margin:
                Param.DefMargin = *e.Get<Type::Margin>();
                break;
            case Type::Rect:
                Param.DefRect = *e.Get<Type::Rect>();
                break;
            case Type::Style:
            {
                const auto& f = e.Get<Type::Style>();
                if (f.bReplace)
                    Param.dwDefStyle = f.dwStyle;
                else
                    Param.dwDefStyle |= f.dwStyle;
            }
            break;
            case Type::ExStyle:
            {
                const auto& f = e.Get<Type::ExStyle>();
                if (f.bReplace)
                    Param.dwDefExStyle = f.dwExStyle;
                else
                    Param.dwDefExStyle |= f.dwExStyle;
            }
            break;
            case Type::Flags:
            {
                const auto& f = e.Get<Type::Flags>();
                if (f.bReplace)
                    Param.uDefFlags = f.uFlags;
                else
                    Param.uDefFlags |= f.uFlags;
            }
            break;
            case Type::Weight:
                Param.uDefWeight = e.Get<Type::Weight>().uWeight;
                break;
            case Type::Font:
                Param.hDefFont = e.Get<Type::Font>().hFont;
                break;
            default:
                return Result::InvalidConfigDefault;
            }
        }
        return Result::Ok;
    }

    inline Result CfgCreateWindow(
        const Priv::Proxy& pr,
        const PARENT_NODE& Parent,
        const DEF_PARAM& Default,
        const DEF_PARAM& Local,
        _Out_ LYT_PARAM& LytParam,
        _Out_ ILayout*& pNewObject,
        ERR_CTX& ErrCtx) noexcept
    {
        LytParam.Margin = Local.DefMargin;
        LytParam.uFlags = Local.uDefFlags;
        LytParam.uWeight = Local.uDefWeight;

        pNewObject = nullptr;
        CWindow* pWnd{};
        PCWSTR pszCaption{};
        Rect rc{ Local.DefRect };
        DWORD dwStyle{ Local.dwDefStyle };
        DWORD dwExStyle{ Local.dwDefExStyle };
        HFONT hFont{ Local.hDefFont };
        int iId{};

        Result r;
        DEF_PARAM NewDefault{ Default }, NewLocal{ Default };
        BOOLEAN bLocal{};

        const auto& il = pr.Get<Type::Window>()->il;
        auto it = il.begin();
        for (; it != il.end(); ++it)
        {
            switch (it->GetType())
            {
            case Type::Default:
                if (bLocal)// Default必须在Local前
                    return Result::DefaultAfterLocal;
                r = CfgParseDefault(*it, NewDefault, TRUE);
                if (r != Result::Ok)
                    return r;
                NewLocal = NewDefault;
                break;
            case Type::Local:
                r = CfgParseDefault(*it, NewLocal, FALSE);
                if (r != Result::Ok)
                    return r;
                bLocal = TRUE;
                break;
            case Type::CWnd:
                pWnd = it->Get<Type::CWnd>();
                break;
            case Type::String:
                pszCaption = it->Get<Type::String>();
                ErrCtx.rsPath
                    .PushBackChar(L'[')
                    .PushBack(pszCaption)
                    .PushBackChar(L']');
                break;
            case Type::Rect:
                rc = *it->Get<Type::Rect>();
                break;
            case Type::Style:
            {
                const auto& e = it->Get<Type::Style>();
                if (e.bReplace)
                    dwStyle = e.dwStyle;
                else
                    dwStyle |= e.dwStyle;
            }
            break;
            case Type::ExStyle:
            {
                const auto& e = it->Get<Type::ExStyle>();
                if (e.bReplace)
                    dwExStyle = e.dwExStyle;
                else
                    dwExStyle |= e.dwExStyle;
            }
            break;
            case Type::Id:
                iId = it->Get<Type::Id>().iId;
                break;
            case Type::Font:
                hFont = it->Get<Type::Font>().hFont;
                break;

            case Type::Window:
            case Type::VBox:
            case Type::HBox:
                goto EndLoop;

            case Type::Margin:
                LytParam.Margin = *it->Get<Type::Margin>();
                break;
            case Type::Weight:
                LytParam.uWeight = it->Get<Type::Weight>().uWeight;
                break;
            case Type::Flags:
            {
                const auto& e = it->Get<Type::Flags>();
                if (e.bReplace)
                    LytParam.uFlags = e.uFlags;
                else
                    LytParam.uFlags |= e.uFlags;
            }
            break;

            default:
                return Result::InvalidConfigWindow;
            }
        }
    EndLoop:
        if (!pWnd)
            return Result::NoCWnd;
        const auto hParent = Parent.pWndParent ? Parent.pWndParent->GetHWND() : nullptr;
        pWnd->Create(pszCaption, dwStyle, dwExStyle,
            rc.x, rc.y, rc.cx, rc.cy, hParent, iId);
        if (hFont)
            pWnd->SetFont(hFont);
        if (!pWnd->GetHWND())
            return Result::Win32;

        if (it != il.end())
        {
            const PARENT_NODE Current{ pWnd };
            const auto itBegin = it;
            for (; it != il.end(); ++it)
            {
                r = CfgCreate(*it, Current,
                    NewDefault, NewLocal, ErrCtx, int(it - itBegin));
                if (r != Result::Ok)
                    return r;
            }
        }
        pNewObject = pWnd;
        return Result::Ok;
    }

    inline Result CfgCreateLinearLayout(
        const Priv::Proxy& pr,
        const PARENT_NODE& Parent,
        const DEF_PARAM& Default,
        const DEF_PARAM& Local,
        _Out_ LYT_PARAM& LytParam,
        _Out_ ILayout*& pNewObject,
        ERR_CTX& ErrCtx) noexcept
    {
        LytParam.Margin = Local.DefMargin;
        LytParam.uFlags = Local.uDefFlags;
        LytParam.uWeight = Local.uDefWeight;

        pNewObject = nullptr;
        CLayoutBase* pLyt{};
        DEF_PARAM NewDefault{ Default }, NewLocal{ Default };
        Result r;
        BOOLEAN bLocal{};

        const auto eType = pr.GetType();
        const auto& il = (
            (eType == Type::VBox) ? pr.Get<Type::VBox>()->il :
            (eType == Type::HBox) ? pr.Get<Type::HBox>()->il :
            pr.Get<Type::HBox>()->il);

        auto it = il.begin();
        for (; it != il.end(); ++it)
        {
            switch (it->GetType())
            {
            case Type::Default:
                if (bLocal)// Default必须在Local前
                    return Result::DefaultAfterLocal;
                r = CfgParseDefault(*it, NewDefault, TRUE);
                if (r != Result::Ok)
                    return r;
                NewLocal = NewDefault;
                break;
            case Type::Local:
                r = CfgParseDefault(*it, NewLocal, FALSE);
                if (r != Result::Ok)
                    return r;
                bLocal = TRUE;
                break;
            case Type::CLayoutBase:
                pLyt = it->Get<Type::CLayoutBase>();
                break;
            case Type::Window:
            case Type::VBox:
            case Type::HBox:
                goto EndLoop;

            case Type::Margin:
                LytParam.Margin = *it->Get<Type::Margin>();
                break;
            case Type::Weight:
                LytParam.uWeight = it->Get<Type::Weight>().uWeight;
                break;
            case Type::Flags:
            {
                const auto& e = it->Get<Type::Flags>();
                if (e.bReplace)
                    LytParam.uFlags = e.uFlags;
                else
                    LytParam.uFlags |= e.uFlags;
            }
            break;

            case Type::String:// 仅用于诊断
                ErrCtx.rsPath
                    .PushBackChar(L'[')
                    .PushBack(it->Get<Type::String>())
                    .PushBackChar(L']');
                break;

            default:
                return Result::InvalidConfigLayout;
            }
        }
    EndLoop:
        if (!pLyt)
            return Result::NoCLayoutBase;
        if (it != il.end())
        {
            const PARENT_NODE NewParent{ Parent.pWndParent, pLyt };
            const auto itBegin = it;
            for (; it != il.end(); ++it)
            {
                r = CfgCreate(*it, NewParent, NewDefault, NewLocal, ErrCtx, int(it - itBegin));
                if (r != Result::Ok)
                    return r;
            }
        }
        pNewObject = pLyt;
        return Result::Ok;
    }

    inline Result CfgAddLayout(
        ILayout* pNewObject,
        const PARENT_NODE& Parent,
        const LYT_PARAM& Param) noexcept
    {
        if (!Parent.pLytParent)
            return Result::Ok;

        const LYTMARGINS m
        {
            (TLytCoord)Param.Margin.l,
            (TLytCoord)Param.Margin.t,
            (TLytCoord)Param.Margin.r,
            (TLytCoord)Param.Margin.b
        };

        if (Parent.pLytParent->RttiIsKindOf<CLinearLayoutV>())
        {
            const auto pLyt = DbgDynamicCast<CLinearLayoutV*>(Parent.pLytParent);
            pLyt->Add(pNewObject, m, Param.uFlags, Param.uWeight);
        }
        else if (Parent.pLytParent->RttiIsKindOf<CLinearLayoutH>())
        {
            const auto pLyt = DbgDynamicCast<CLinearLayoutH*>(Parent.pLytParent);
            pLyt->Add(pNewObject, m, Param.uFlags, Param.uWeight);
        }
        else
            return Result::InvalidLayout;
        return Result::Ok;
    }

    inline Result CfgCreate(
        const Priv::Proxy& pr,
        const PARENT_NODE& Parent,
        const DEF_PARAM& Default,
        const DEF_PARAM& Local,
        ERR_CTX& ErrCtx,
        int idxNode) noexcept
    {
        Result r;
        LYT_PARAM Param{};
        ILayout* pNewObject;

        const auto eType = pr.GetType();
        switch (eType)
        {
        case Type::Window:
        {
            ScopedPath Path{ ErrCtx, L"/Window"sv, idxNode };
            r = CfgCreateWindow(pr, Parent, Default, Local, Param, pNewObject, ErrCtx);
            if (r != Result::Ok)
                return r;
            r = CfgAddLayout(pNewObject, Parent, Param);
            if (r != Result::Ok)
                return r;
            Path.Pop();
        }
        break;
        case Type::VBox:
        case Type::HBox:
        {
            ScopedPath Path{ ErrCtx, (eType == Type::VBox) ? L"/VBox"sv : L"/HBox"sv, idxNode };
            r = CfgCreateLinearLayout(pr, Parent, Default, Local, Param, pNewObject, ErrCtx);
            if (r != Result::Ok)
                return r;
            r = CfgAddLayout(pNewObject, Parent, Param);
            if (r != Result::Ok)
                return r;
            Path.Pop();
        }
        break;
        default:
            return Result::InvalidConfig;
        }
        return Result::Ok;
    }
}

inline Result Create(
    CWindow* pWndParent,
    ERR_CTX* pErrCtx,
    Priv::Proxy pr) noexcept
{
    ERR_CTX ErrCtx{};
    if (!pErrCtx)
        pErrCtx = &ErrCtx;

    const Priv::PARENT_NODE Parent{ .pWndParent = pWndParent };
    constexpr Priv::DEF_PARAM Default{};
    constexpr Priv::DEF_PARAM Local{};

    if (pr.GetType() == Priv::Type::List)
    {
        for (int i{}; const auto& e : *pr.Get<Priv::Type::List>())
        {
            const auto r = Priv::CfgCreate(e, Parent, Default, Local, *pErrCtx, i);
            if (r != Result::Ok)
                return r;
            ++i;
        }
        return Result::Ok;
    }
    else
        return Priv::CfgCreate(pr, Parent, Default, Local, *pErrCtx, 0);
}
ECK_UIBUILDER_NAMESPACE_END
ECK_NAMESPACE_END