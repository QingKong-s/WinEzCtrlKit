#pragma once
#include "DuiTheme.h"
#include "StringConvert.h"

ECK_NAMESPACE_BEGIN
ECK_DUI_NAMESPACE_BEGIN
enum class CssResult
{
    Ok,
    SelectorEmpty,
    PseudoEmpty,
    PropertyEmpty,
    PropertyValueEmpty,
    UnexceptedEnd,
};

namespace Priv
{
    EckInlineNdCe BOOL CsspIsSpace(WCHAR ch) noexcept
    {
        return ch == ' ' || ch == '\t' || ch == '\r' || ch == '\n';
    }
}

enum class CssSelType : BYTE
{
    Tag,
    Class,
    Id,
};

struct CSS_STYLE
{
    std::wstring_view svKey{};
    std::wstring_view svValue{};
};
struct CSS_SELECTOR
{
    CssSelType eType{};
    std::wstring_view svName{};
    std::wstring_view svPseudo{};
    std::vector<CSS_STYLE> vStyle{};
};
struct CSS_DOC
{
    std::vector<CSS_SELECTOR> vSel;
};

inline CssResult CssParse(std::wstring_view svCss, CSS_DOC& Doc) noexcept
{
    enum class State
    {
        Selector,
        Declaration,
        DeclValue,
    };
    State eState = State::Selector;
    const PCWCH pEnd = svCss.data() + svCss.size();
    PCWCH pTemp{};
    BOOL bInString{};
    for (auto p = svCss.data(); p < pEnd; ++p)
    {
        const auto ch = *p;
        switch (eState)
        {
        case State::Selector:
        {
            if (Priv::CsspIsSpace(ch))
                break;
            if (ch == '{')
            {
                if (!pTemp || p <= pTemp)
                    return CssResult::SelectorEmpty;
                const auto pCurrEnd = RTrimStr(pTemp, int(p - pTemp));
                if (pCurrEnd <= pTemp)
                    return CssResult::SelectorEmpty;
                auto& Sel = Doc.vSel.emplace_back();
                switch (*pTemp)
                {
                case '#':
                    Sel.eType = CssSelType::Id;
                    ++pTemp;
                    break;
                case '.':
                    Sel.eType = CssSelType::Class;
                    ++pTemp;
                    break;
                default:
                    Sel.eType = CssSelType::Tag;
                    break;
                }
                if (pCurrEnd <= pTemp)
                    return CssResult::SelectorEmpty;
                const auto pPseudo = TcsCharLen(pTemp, pCurrEnd - pTemp, L':');
                if (pPseudo)
                {
                    if (pCurrEnd <= pPseudo)
                        return CssResult::PseudoEmpty;
                    Sel.svName = { pTemp,pPseudo };
                    Sel.svPseudo = { pPseudo + 1,pCurrEnd };
                }
                else
                    Sel.svName = { pTemp,pCurrEnd };
                pTemp = nullptr;
                eState = State::Declaration;
                break;
            }
            if (!pTemp)
                pTemp = p;
        }
        break;
        case State::Declaration:
        {
            if (Priv::CsspIsSpace(ch))
                break;
            if (ch == ':')
            {
                if (!pTemp || p <= pTemp)
                    return CssResult::PropertyEmpty;
                const auto pCurrEnd = RTrimStr(pTemp, int(p - pTemp));
                if (pCurrEnd <= pTemp)
                    return CssResult::PropertyEmpty;
                auto& Style = Doc.vSel.back().vStyle.emplace_back();
                Style.svKey = { pTemp,pCurrEnd };
                pTemp = nullptr;
                eState = State::DeclValue;
                break;
            }
            else if (ch == '}')
            {
                eState = State::Selector;
                break;
            }
            if (!pTemp)
                pTemp = p;
        }
        break;
        case State::DeclValue:
        {
            if (ch == '\"' || ch == '\'')
                bInString = !bInString;
            if (!bInString && ch == ';')
            {
                if (!pTemp || p <= pTemp)
                    return CssResult::PropertyValueEmpty;
                const auto pCurrEnd = RTrimStr(pTemp, int(p - pTemp));
                if (pCurrEnd <= pTemp)
                    return CssResult::PropertyEmpty;
                auto& Style = Doc.vSel.back().vStyle.back();
                Style.svValue = { pTemp,pCurrEnd };
                pTemp = nullptr;
                eState = State::Declaration;
                break;
            }
            else if (!pTemp)
            {
                if (Priv::CsspIsSpace(ch))
                    break;
                pTemp = p;
            }
        }
        break;
        default:;
        }
    }
    if (eState != State::Selector || pTemp)
        return CssResult::UnexceptedEnd;
    return CssResult::Ok;
}

inline Part CssNcTagToPart(std::wstring_view svTag) noexcept
{
#define ECK_CSS_HIT_PART(Str) TcsEqualLen2I(svTag.data(), svTag.size(), EckStrAndLen(Str))
    if (ECK_CSS_HIT_PART(L"Button"))
        return Part::Button;
    if (ECK_CSS_HIT_PART(L"CircleButton"))
        return Part::CircleButton;
    if (ECK_CSS_HIT_PART(L"RadioButton"))
        return Part::RadioButton;
    if (ECK_CSS_HIT_PART(L"CheckButton"))
        return Part::CheckButton;
    if (ECK_CSS_HIT_PART(L"TriStateCheckButton"))
        return Part::TriStateCheckButton;
    if (ECK_CSS_HIT_PART(L"GroupBox"))
        return Part::GroupBox;

    if (ECK_CSS_HIT_PART(L"Edit"))
        return Part::Edit;
    if (ECK_CSS_HIT_PART(L"EditBottomBar"))
        return Part::EditBottomBar;

    if (ECK_CSS_HIT_PART(L"List"))
        return Part::List;
    if (ECK_CSS_HIT_PART(L"ListSelRect"))
        return Part::ListSelRect;
    if (ECK_CSS_HIT_PART(L"ListItem"))
        return Part::ListItem;

    if (ECK_CSS_HIT_PART(L"Header"))
        return Part::Header;
    if (ECK_CSS_HIT_PART(L"HeaderItem"))
        return Part::HeaderItem;

    if (ECK_CSS_HIT_PART(L"Progress"))
        return Part::Progress;

    if (ECK_CSS_HIT_PART(L"TrackBar"))
        return Part::TrackBar;
    if (ECK_CSS_HIT_PART(L"TrackBarThumb"))
        return Part::TrackBarThumb;

    if (ECK_CSS_HIT_PART(L"ScrollBar"))
        return Part::ScrollBar;
    if (ECK_CSS_HIT_PART(L"ScrollButton"))
        return Part::ScrollButton;
    if (ECK_CSS_HIT_PART(L"ScrollThumb"))
        return Part::ScrollThumb;

    if (ECK_CSS_HIT_PART(L"Label") || ECK_CSS_HIT_PART(L"LabelBk"))
        return Part::LabelBk;
    return Part::Invalid;
}

inline State CssNcPseudoToState(std::wstring_view svPseudo) noexcept
{
#define ECK_CSS_HIT_STATE(Str) TcsEqualLen2I(svPseudo.data(), svPseudo.size(), EckStrAndLen(Str))
    if (svPseudo.empty())
        return State::Normal;
    if (ECK_CSS_HIT_STATE(L"hover"))
        return State::Hot;
    if (ECK_CSS_HIT_STATE(L"active"))
        return State::Selected;
    if (ECK_CSS_HIT_STATE(L"Disabled"))
        return State::Disabled;

    if (ECK_CSS_HIT_STATE(L"Checked"))
        return State::Checked;
    if (ECK_CSS_HIT_STATE(L"Mixed"))
        return State::Mixed;
    if (ECK_CSS_HIT_STATE(L"focus"))
        return State::Focused;

    if (ECK_CSS_HIT_STATE(L"hover:active") ||
        ECK_CSS_HIT_STATE(L"HotSelected"))
        return State::HotSelected;
    if (ECK_CSS_HIT_STATE(L"Checked:hover") ||
        ECK_CSS_HIT_STATE(L"CheckedHot"))
        return State::CheckedHot;
    if (ECK_CSS_HIT_STATE(L"Checked:active") ||
        ECK_CSS_HIT_STATE(L"CheckedSelected"))
        return State::CheckedSelected;
    if (ECK_CSS_HIT_STATE(L"Checked:Disabled") ||
        ECK_CSS_HIT_STATE(L"CheckedDisabled"))
        return State::CheckedDisabled;

    if (ECK_CSS_HIT_STATE(L"Mixed:hover") ||
        ECK_CSS_HIT_STATE(L"MixedHot"))
        return State::MixedHot;
    if (ECK_CSS_HIT_STATE(L"Mixed:active") ||
        ECK_CSS_HIT_STATE(L"MixedSelected"))
        return State::MixedSelected;
    if (ECK_CSS_HIT_STATE(L"Mixed:Disabled") ||
        ECK_CSS_HIT_STATE(L"MixedDisabled"))
        return State::MixedDisabled;
    if (ECK_CSS_HIT_STATE(L"NoFocusSelected"))
        return State::NoFocusSelected;

    return State::None;
#undef ECK_CSS_HIT_STATE
}

inline GeoType CssNcToGeometryType(std::wstring_view svName) noexcept
{
#define ECK_CSS_HIT_TYPE(Str) TcsEqualLen2I(svName.data(), svName.size(), EckStrAndLen(Str))
    if (ECK_CSS_HIT_TYPE(L"FillRect"))
        return GeoType::FillRect;
    if (ECK_CSS_HIT_TYPE(L"FillRoundRect"))
        return GeoType::FillRoundRect;
    if (ECK_CSS_HIT_TYPE(L"FillEllipse"))
        return GeoType::FillEllipse;

    if (ECK_CSS_HIT_TYPE(L"FrameRect"))
        return GeoType::FrameRect;
    if (ECK_CSS_HIT_TYPE(L"FrameRoundRect"))
        return GeoType::FrameRoundRect;
    if (ECK_CSS_HIT_TYPE(L"FrameEllipse"))
        return GeoType::FrameEllipse;

    if (ECK_CSS_HIT_TYPE(L"FillFrameRect"))
        return GeoType::FillFrameRect;
    if (ECK_CSS_HIT_TYPE(L"FillFrameRoundRect"))
        return GeoType::FillFrameRoundRect;
    if (ECK_CSS_HIT_TYPE(L"FillFrameEllipse"))
        return GeoType::FillFrameEllipse;

    if (ECK_CSS_HIT_TYPE(L"Line"))
        return GeoType::Line;

    return GeoType::None;
#undef ECK_CSS_HIT_TYPE
}

inline BOOL CssParseColor(std::wstring_view svColor, _Out_ D2D1_COLOR_F& cr) noexcept
{
    cr = {};
    if (svColor.empty())
        return FALSE;
    ARGB argb;
    if (svColor[0] == L'#')
    {
        if (TcsToInt(svColor.data() + 1, svColor.size() - 1, argb, 16) != TcsCvtErr::Ok)
            return FALSE;
        if (!(argb & 0xFF00'0000))
            argb |= 0xFF00'0000;
    }
    else if (TcsIsStartWithLen2I(svColor.data(), svColor.size(), EckStrAndLen(L"rgb(")) ||
        TcsIsStartWithLen2I(svColor.data(), svColor.size(), EckStrAndLen(L"argb(")))
    {
        const auto pEnd = RTrimStr(svColor.data(), (int)svColor.size());
        if (pEnd == svColor.data() || *(pEnd - 1) != L')')
            return FALSE;
        auto p = svColor.data() + 3;
        p += ((*p != L'(') ? 2 : 1);
        if (p >= pEnd)
            return FALSE;
        int cPart{};
        BOOL bInvalid{};
        BYTE by[4]{};
        SplitStr(p, int(pEnd - p), EckStrAndLen(L","), 0, [&](PCWCH psz, int cch)
            {
                if (++cPart > 4)
                {
                    bInvalid = TRUE;
                    return FALSE;
                }
                const auto p = LTrimStr(psz, cch);
                if (p >= psz + cch)
                {
                    bInvalid = TRUE;
                    return FALSE;
                }
                if (TcsToInt(p, psz + cch - p, by[cPart - 1]) != TcsCvtErr::Ok)
                {
                    bInvalid = TRUE;
                    return FALSE;
                }
                return TRUE;
            });
        if (bInvalid)
            return FALSE;
        if (cPart == 4)
            argb = by[0] << 24 | by[1] << 16 | by[2] << 8 | by[3];
        else if (cPart == 3)
            argb = 0xFF00'0000 | by[0] << 16 | by[1] << 8 | by[2];
        else
            return FALSE;
    }
    else
    {
        if (TcsToInt(svColor.data(), svColor.size(), argb, 0) != TcsCvtErr::Ok)
            return FALSE;
        if (!(argb & 0xFF00'0000))
            argb |= 0xFF00'0000;
    }
    cr = ArgbToD2DColorF(argb);
    return TRUE;
}

inline BOOL CssIsColorization(std::wstring_view sv) noexcept
{
    return TcsEqualLen2I(sv.data(), sv.size(), EckStrAndLen(L"colorization")) ||
        TcsEqualLen2I(sv.data(), sv.size(), EckStrAndLen(L"theme"));
}
ECK_DUI_NAMESPACE_END
ECK_NAMESPACE_END