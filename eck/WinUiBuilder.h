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

struct Style { DWORD dwStyle; };
struct ExStyle { DWORD dwExStyle; };
struct Id { int iId; };
struct Weight { UINT uWeight; };
struct Flags { UINT uFlags; };
struct Font { HFONT hFont; };

struct Window;
struct VBox;
struct HBox;
struct Default;

namespace Priv
{
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
ECK_UI_DECL_CONTAINER(VBox);
ECK_UI_DECL_CONTAINER(HBox);

#undef ECK_UI_DECL_CONTAINER

enum class Result
{
    Ok,

    InvalidConfig,  // 当前配置不支持某种子配置
    InvalidLayout,  // 不支持的布局类型
    Win32,          // Win32错误，已设置LastError

    NoCWnd,         // 未指定CWnd
    NoCLayoutBase,  // 未指定CLayoutBase
};

namespace Priv
{
    struct PARENT_NODE
    {
        CWindow* pWndParent{};
        CLayoutBase* pLytParent{};

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
        _Out_ LYT_PARAM& LytParam,
        _Out_ ILayout*& pNewObject) noexcept;
    inline Result CfgCreateLinearLayout(
        const Priv::Proxy& pr,
        const PARENT_NODE& Parent,
        _Out_ LYT_PARAM& LytParam,
        _Out_ ILayout*& pNewObject) noexcept;
    inline Result CfgCreate(
        const Priv::Proxy& pr,
        const PARENT_NODE& Parent) noexcept;

    inline Result CfgParseDefault(
        const Priv::Proxy& pr,
        PARENT_NODE& Parent) noexcept
    {
        for (const auto& e : pr.Get<Type::Default>()->il)
        {
            switch (e.GetType())
            {
            case Type::Margin:
                Parent.DefMargin = *e.Get<Type::Margin>();
                break;
            case Type::Rect:
                Parent.DefRect = *e.Get<Type::Rect>();
                break;
            case Type::Style:
                Parent.dwDefStyle |= e.Get<Type::Style>().dwStyle;
                break;
            case Type::ExStyle:
                Parent.dwDefExStyle |= e.Get<Type::ExStyle>().dwExStyle;
                break;
            case Type::Flags:
                Parent.uDefFlags |= e.Get<Type::Flags>().uFlags;
                break;
            case Type::Weight:
                Parent.uDefWeight = e.Get<Type::Weight>().uWeight;
                break;
            case Type::Font:
                Parent.hDefFont = e.Get<Type::Font>().hFont;
                break;
            default:
                return Result::InvalidConfig;
            }
        }
        return Result::Ok;
    }

    inline Result CfgCreateWindow(
        const Priv::Proxy& pr,
        const PARENT_NODE& Parent,
        _Out_ LYT_PARAM& LytParam,
        _Out_ ILayout*& pNewObject) noexcept
    {
        LytParam.Margin = Parent.DefMargin;
        LytParam.uFlags = Parent.uDefFlags;
        LytParam.uWeight = Parent.uDefWeight;

        pNewObject = nullptr;
        CWindow* pWnd{};
        PCWSTR pszCaption{};
        Rect rc{ Parent.DefRect };
        DWORD dwStyle{ Parent.dwDefStyle };
        DWORD dwExStyle{ Parent.dwDefExStyle };
        HFONT hFont{ Parent.hDefFont };
        int iId{};

        Result r;
        PARENT_NODE NewParent{ Parent };

        const auto& il = pr.Get<Type::Window>()->il;
        auto it = il.begin();
        for (; it != il.end(); ++it)
        {
            switch (it->GetType())
            {
            case Type::Default:
                r = CfgParseDefault(*it, NewParent);
                if (r != Result::Ok)
                    return r;
                break;
            case Type::CWnd:
                pWnd = it->Get<Type::CWnd>();
                break;
            case Type::String:
                pszCaption = it->Get<Type::String>();
                break;
            case Type::Rect:
                rc = *it->Get<Type::Rect>();
                break;
            case Type::Style:
                dwStyle |= it->Get<Type::Style>().dwStyle;
                break;
            case Type::ExStyle:
                dwExStyle |= it->Get<Type::ExStyle>().dwExStyle;
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
                LytParam.uFlags = it->Get<Type::Flags>().uFlags;
                break;

            default:
                return Result::InvalidConfig;
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
            NewParent.pLytParent = nullptr;// 规定布局同一层级内有效
            NewParent.pWndParent = pWnd;
            for (; it != il.end(); ++it)
            {
                r = CfgCreate(*it, NewParent);
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
        _Out_ LYT_PARAM& LytParam,
        _Out_ ILayout*& pNewObject) noexcept
    {
        LytParam.Margin = Parent.DefMargin;
        LytParam.uFlags = Parent.uDefFlags;
        LytParam.uWeight = Parent.uDefWeight;

        pNewObject = nullptr;
        CLayoutBase* pLyt{};
        PARENT_NODE NewParent{ Parent };
        Result r;

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
            {
                r = CfgParseDefault(*it, NewParent);
                if (r != Result::Ok)
                    return r;
            }
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
                LytParam.uFlags = it->Get<Type::Flags>().uFlags;
                break;

            default:
                return Result::InvalidConfig;
            }
        }
    EndLoop:
        if (!pLyt)
            return Result::NoCLayoutBase;
        if (it != il.end())
        {
            NewParent.pLytParent = pLyt;
            for (; it != il.end(); ++it)
            {
                r = CfgCreate(*it, NewParent);
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
        const PARENT_NODE& Parent) noexcept
    {
        Result r;
        LYT_PARAM Param{};
        ILayout* pNewObject;
        switch (pr.GetType())
        {
        case Type::Window:
            r = CfgCreateWindow(pr, Parent, Param, pNewObject);
            if (r != Result::Ok)
                return r;
            r = CfgAddLayout(pNewObject, Parent, Param);
            if (r != Result::Ok)
                return r;
            break;
        case Type::VBox:
        case Type::HBox:
            r = CfgCreateLinearLayout(pr, Parent, Param, pNewObject);
            if (r != Result::Ok)
                return r;
            r = CfgAddLayout(pNewObject, Parent, Param);
            if (r != Result::Ok)
                return r;
            break;
        default:
            return Result::InvalidConfig;
        }
        return Result::Ok;
    }
}

inline Result Create(CWindow* pWndParent, Priv::Proxy pr) noexcept
{
    const Priv::PARENT_NODE Parent{ .pWndParent = pWndParent };
    if (pr.GetType() == Priv::Type::List)
    {
        for (const auto& e : *pr.Get<Priv::Type::List>())
        {
            const auto r = Priv::CfgCreate(e, Parent);
            if (r != Result::Ok)
                return r;
        }
        return Result::Ok;
    }
    else
        return Priv::CfgCreate(pr, Parent);
}
ECK_UIBUILDER_NAMESPACE_END
ECK_NAMESPACE_END