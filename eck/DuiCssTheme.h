#pragma once
#include "DuiCssParse.h"

ECK_NAMESPACE_BEGIN
ECK_DUI_NAMESPACE_BEGIN
class CCssTheme : public CUnknown<CCssTheme, ITheme>
{
public:
    enum class Draw : BYTE
    {
        SolidColor,

    };
    struct DRAW
    {
        Part ePart;
        State eState;
        Draw eDraw;
        union
        {
            D2D1_COLOR_F cr;
        };
    };

    D2D1_COLOR_F m_crColorization{};
    std::vector<DRAW> m_vDraw{};
private:
    HRESULT DrawBackground(Part ePart, State eState,
        const D2D1_RECT_F& rc, _In_opt_ const DTB_OPT* pOpt) override
    {

    }
    HRESULT SetColorizationColor(const D2D1_COLOR_F& cr) override
    {
        m_crColorization = cr;
        return S_OK;
    }
    HRESULT GetColorizationColor(_Out_ D2D1_COLOR_F& cr) override
    {
        cr = m_crColorization;
        return S_OK;
    }
    HRESULT GetColor(Part ePart, State eState,
        ClrPart eClrPart, _Out_ D2D1_COLOR_F& cr) override
    {

    }
    HRESULT GetSysColor(SysColor eSysColor, _Out_ D2D1_COLOR_F& cr) override
    {

    }
    float GetMetrics(Metrics eMetrics) override
    {

    }
};


inline CssResult CssBuildTheme(std::wstring_view svCss, _Out_ ITheme*& pTheme)
{
    std::vector<CSS_SELECTOR> vSel;
    const auto r = CssParse(svCss, vSel);
    if (r != CssResult::Ok)
        return r;
    for (const auto& Sel : vSel)
    {

    }
}
ECK_DUI_NAMESPACE_END
ECK_NAMESPACE_END