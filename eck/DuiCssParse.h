#pragma once
#include "DuiBase.h"

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
    EckInlineNdCe BOOL CsspIsSpace(char ch)
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

inline CssResult CssParse(std::wstring_view svCss, std::vector<CSS_SELECTOR>& vSel)
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
                auto& Sel = vSel.emplace_back();
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
                auto& Style = vSel.back().vStyle.emplace_back();
                Style.svKey = { pTemp,pCurrEnd };
                pTemp = nullptr;
                eState = State::DeclValue;
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
                auto& Style = vSel.back().vStyle.back();
                Style.svValue = { pTemp,pCurrEnd };
                pTemp = nullptr;
                eState = State::Declaration;
                break;
            }
            else if (!pTemp)
                pTemp = p;
        }
        break;
        default:
        }
    }
    if (eState != State::Declaration || pTemp)
        return CssResult::UnexceptedEnd;
    return CssResult::Ok;
}

inline Part CssTagToPart(std::wstring_view svTag)
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
        return Part::LabelText;
    return Part::Invalid;
}
ECK_DUI_NAMESPACE_END
ECK_NAMESPACE_END