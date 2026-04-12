#pragma once
#include "CWindow.h"
#include "CLayoutBase.h"

#define ECK_UIBUILDER_NAMESPACE_BEGIN   namespace UiBuilder {
#define ECK_UIBUILDER_NAMESPACE_END     }

ECK_NAMESPACE_BEGIN
ECK_UIBUILDER_NAMESPACE_BEGIN
struct Rect { int x, y, cx, cy; };
struct Margin { int l, t, r, b; };
struct Extra { int cx, cy; };

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
struct Table { USHORT r, c, rSpan, cSpan; };

struct Window;
struct VBox;
struct HBox;
struct FrameBox;
struct VFlowBox;
struct HFlowBox;
struct TableBox;
struct Default;
struct Local;

namespace Detail
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
        VFlowBox,
        HFlowBox,
        FrameBox,
        TableBox,

        Default,
        Local,

        Rect,
        Margin,
        Extra,
        Table,

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
            const VFlowBox*,
            const HFlowBox*,
            const FrameBox*,
            const TableBox*,

            const Default*,
            const Local*,

            const Rect*,
            const Margin*,
            const Extra*,
            const Table*,

            Style,
            ExStyle,
            Id,
            Weight,
            Flags
        > v{};

        constexpr Proxy(const std::initializer_list<Proxy>& x) noexcept : v{ &x } {}
        constexpr Proxy(CWindow& x) noexcept : v{ &x } {}
        constexpr Proxy(CLayoutBase& x) noexcept : v{ &x } {}
        constexpr Proxy(PCWSTR x) noexcept : v{ x } {}
        constexpr Proxy(Font x) noexcept : v{ x } {}

        constexpr Proxy(const Window& x) noexcept : v{ &x } {}
        constexpr Proxy(const VBox& x) noexcept : v{ &x } {}
        constexpr Proxy(const HBox& x) noexcept : v{ &x } {}
        constexpr Proxy(const VFlowBox& x) noexcept : v{ &x } {}
        constexpr Proxy(const HFlowBox& x) noexcept : v{ &x } {}
        constexpr Proxy(const FrameBox& x) noexcept : v{ &x } {}
        constexpr Proxy(const TableBox& x) noexcept : v{ &x } {}
        constexpr Proxy(const Default& x) noexcept : v{ &x } {}
        constexpr Proxy(const Local& x) noexcept : v{ &x } {}

        constexpr Proxy(const Rect& x) noexcept : v{ &x } {}
        constexpr Proxy(const Margin& x) noexcept : v{ &x } {}
        constexpr Proxy(const Extra& x) noexcept : v{ &x } {}
        constexpr Proxy(const Table& x) noexcept : v{ &x } {}

        constexpr Proxy(Style x) noexcept : v{ x } {}
        constexpr Proxy(ExStyle x) noexcept : v{ x } {}
        constexpr Proxy(Id x) noexcept : v{ x } {}
        constexpr Proxy(Weight x) noexcept : v{ x } {}
        constexpr Proxy(Flags x) noexcept : v{ x } {}

        EckInlineNdCe Type GetType() const noexcept { return (Type)v.index(); }

        template<Type E>
        EckInlineNdCe auto& Get() const noexcept { return std::get<(size_t)E>(v); }
    };

    struct Container
    {
        const std::initializer_list<Detail::Proxy>& il;

        Container(const std::initializer_list<Detail::Proxy>& il_) noexcept : il{ il_ } {}
    };
}

#define ECK_UI_DECL_CONTAINER(Name) \
    struct Name : Detail::Container \
    { \
        using Detail::Container::Container; \
    }

ECK_UI_DECL_CONTAINER(Window);
ECK_UI_DECL_CONTAINER(Default);
ECK_UI_DECL_CONTAINER(Local);
ECK_UI_DECL_CONTAINER(VBox);
ECK_UI_DECL_CONTAINER(HBox);
ECK_UI_DECL_CONTAINER(FrameBox);
ECK_UI_DECL_CONTAINER(VFlowBox);
ECK_UI_DECL_CONTAINER(HFlowBox);
ECK_UI_DECL_CONTAINER(TableBox);

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

    NoCWindow,          // 未指定CWindow
    NoCLayoutBase,      // 未指定CLayoutBase

    DefaultAfterLocal,  // Default出现在Local之后
};

struct ERR_CTX
{
    CStringW rsPath;
};

namespace Detail
{
    // 压入当前节点名称
    struct ScopedPath
    {
        ERR_CTX& ErrCtx;
        int cchOld;

        ScopedPath(ERR_CTX& ErrCtx_, std::wstring_view sv, int idxNode) noexcept
            : ErrCtx{ ErrCtx_ }, cchOld{ ErrCtx.rsPath.Size() }
        {
            constexpr size_t cchBuf = TcvIntBufferSize<int>();

            auto p = ErrCtx.rsPath.PushBack(int(sv.size() + cchBuf));
            TcsCopyLength(p, sv.data(), sv.size());
            p += sv.size();

            PWCH pEnd;
            TcvFromInt(p, cchBuf, idxNode, 10, TRUE, &pEnd);
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
        Type eType;
        CLayoutBase* pLytParent;
    };

    struct DEF_PARAM
    {
        HFONT hDefFont{};

        Rect DefRect{};
        DWORD dwDefStyle{ WS_CHILD | WS_VISIBLE };
        DWORD dwDefExStyle{};

        Margin DefMargin{};
        Extra DefExtra{};
        UINT uDefFlags{};
        UINT uDefWeight{};
    };

    struct LYT_PARAM
    {
        Margin Margin;
        Extra Extra;
        UINT uFlags;
        UINT uWeight;
        USHORT idxRow, idxCol;
        USHORT cRowSpan, cColSpan;
    };

    inline Result CfgCreateWindow(
        const Detail::Proxy& pr,
        const PARENT_NODE& Parent,
        const DEF_PARAM& Default,
        const DEF_PARAM& Local,
        _Out_ LYT_PARAM& LytParam,
        _Out_ ILayout*& pNewObject,
        ERR_CTX& ErrCtx) noexcept;
    inline Result CfgCreateLayout(
        const Detail::Proxy& pr,
        const PARENT_NODE& Parent,
        const DEF_PARAM& Default,
        const DEF_PARAM& Local,
        _Out_ LYT_PARAM& LytParam,
        _Out_ ILayout*& pNewObject,
        ERR_CTX& ErrCtx) noexcept;
    // idxNode: pr在同级配置中的序号，从0开始，用于诊断
    inline Result CfgCreate(
        const Detail::Proxy& pr,
        const PARENT_NODE& Parent,
        const DEF_PARAM& Default,
        const DEF_PARAM& Local,
        ERR_CTX& ErrCtx,
        int idxNode) noexcept;

    inline Result CfgParseDefault(
        const Detail::Proxy& pr,
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
            case Type::Extra:
                Param.DefExtra = *e.Get<Type::Extra>();
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

    inline Result CfgParseLayoutParamters(
        const Proxy* it,
        LYT_PARAM& LytParam) noexcept
    {
        switch (it->GetType())
        {
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
        case Type::Extra:
            LytParam.Extra = *it->Get<Type::Extra>();
            break;
        case Type::Table:
        {
            const auto p = it->Get<Type::Table>();
            LytParam.idxRow = p->r;
            LytParam.idxCol= p->c;
            LytParam.cRowSpan = p->rSpan;
            LytParam.cColSpan = p->cSpan;
        }
        break;
        default:
            return Result::InvalidConfig;
        }
        return Result::Ok;
    }

    inline Result CfgCreateWindow(
        const Detail::Proxy& pr,
        const PARENT_NODE& Parent,
        const DEF_PARAM& Default,
        const DEF_PARAM& Local,
        _Out_ LYT_PARAM& LytParam,
        _Out_ ILayout*& pNewObject,
        ERR_CTX& ErrCtx) noexcept
    {
        LytParam.Margin = Local.DefMargin;
        LytParam.Extra = Local.DefExtra;
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
            case Type::VBox:     case Type::HBox:
            case Type::VFlowBox: case Type::HFlowBox:
            case Type::FrameBox: case Type::TableBox:
                goto EndLoop;

            default:
                r = CfgParseLayoutParamters(it, LytParam);
                if (r == Result::InvalidConfig)
                    return Result::InvalidConfigWindow;
                else if (r != Result::Ok)
                    return r;
            }
        }
    EndLoop:
        if (!pWnd)
            return Result::NoCWindow;
        const auto hParent = Parent.pWndParent ? Parent.pWndParent->GetHWnd() : nullptr;
        pWnd->Create(pszCaption, dwStyle, dwExStyle,
            rc.x, rc.y, rc.cx, rc.cy, hParent, iId);
        if (hFont)
            pWnd->SetFont(hFont);
        if (!pWnd->GetHWnd())
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

    inline Result CfgCreateLayout(
        const Detail::Proxy& pr,
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
            (eType == Type::FrameBox) ? pr.Get<Type::FrameBox>()->il :
            (eType == Type::VFlowBox) ? pr.Get<Type::VFlowBox>()->il :
            (eType == Type::HFlowBox) ? pr.Get<Type::HFlowBox>()->il :
            (eType == Type::TableBox) ? pr.Get<Type::TableBox>()->il :
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
            case Type::VBox:     case Type::HBox:
            case Type::VFlowBox: case Type::HFlowBox:
            case Type::FrameBox: case Type::TableBox:
                goto EndLoop;

            case Type::String:// 仅用于诊断
                ErrCtx.rsPath
                    .PushBackChar(L'[')
                    .PushBack(it->Get<Type::String>())
                    .PushBackChar(L']');
                break;

            default:
                r = CfgParseLayoutParamters(it, LytParam);
                if (r == Result::InvalidConfig)
                    return Result::InvalidConfigWindow;
                else if (r != Result::Ok)
                    return r;
            }
        }
    EndLoop:
        if (!pLyt)
            return Result::NoCLayoutBase;
        if (it != il.end())
        {
            const PARENT_NODE NewParent{ Parent.pWndParent, eType, pLyt };
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

        LOB_PARAM LobParam
        {
            .pObject = pNewObject,
            .Margins = m,
            .uFlags = Param.uFlags,
            .cxExtra = (TLytCoord)Param.Extra.cx,
            .cyExtra = (TLytCoord)Param.Extra.cy,
        };
        switch (Parent.eType)
        {
        case Type::VBox: case Type::HBox:
            LobParam.uWeight = Param.uWeight;
            break;
        case Type::Table:
            LobParam.idxRow = Param.idxRow;
            LobParam.idxCol = Param.idxCol;
            LobParam.cRowSpan = Param.cRowSpan;
            LobParam.cColSpan = Param.cColSpan;
            break;
        }
        Parent.pLytParent->LobAddObject(LobParam);
        return Result::Ok;
    }

    inline Result CfgCreate(
        const Detail::Proxy& pr,
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
        case Type::FrameBox:
        case Type::VFlowBox:
        case Type::HFlowBox:
        case Type::TableBox:
        {
            const auto FnGetNodeName = [](Type eType) -> std::wstring_view
                {
                    switch (eType)
                    {
                    case Type::VBox:     return L"/VBox"sv;
                    case Type::HBox:     return L"/HBox"sv;
                    case Type::FrameBox: return L"/FrameBox"sv;
                    case Type::VFlowBox: return L"/VFlowBox"sv;
                    case Type::HFlowBox: return L"/GFlowBox"sv;
                    case Type::TableBox: return L"/TableBox"sv;
                    default:             ECK_UNREACHABLE;
                    }
                };

            ScopedPath Path{ ErrCtx, FnGetNodeName(eType), idxNode };
            r = CfgCreateLayout(pr, Parent, Default, Local, Param, pNewObject, ErrCtx);
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
    ERR_CTX& ErrCtx,
    Detail::Proxy pr) noexcept
{
    const Detail::PARENT_NODE Parent{ .pWndParent = pWndParent };
    constexpr Detail::DEF_PARAM Default{};
    constexpr Detail::DEF_PARAM Local{};

    if (pr.GetType() == Detail::Type::List)
    {
        for (int i{}; const auto& e : *pr.Get<Detail::Type::List>())
        {
            const auto r = Detail::CfgCreate(e, Parent, Default, Local, ErrCtx, i);
            if (r != Result::Ok)
                return r;
            ++i;
        }
        return Result::Ok;
    }
    else
        return Detail::CfgCreate(pr, Parent, Default, Local, ErrCtx, 0);
}
ECK_UIBUILDER_NAMESPACE_END
ECK_NAMESPACE_END