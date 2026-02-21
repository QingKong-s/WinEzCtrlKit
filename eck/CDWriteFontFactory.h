#pragma once
#include "CString.h"
#include "DpiApi.h"
#include "ComPtr.h"

ECK_NAMESPACE_BEGIN
struct DwFontFactoryNoDef_T {};

class CDWriteFontFactory
{
private:
    struct TEMPLATE
    {
        CStringW rsFace;
        float cy;
        DWRITE_FONT_WEIGHT eWeight;
        BOOL bItalic;
    };
    IDWriteFontCollection* m_pFontCollection{};
    TEMPLATE m_Template{};
    CStringW m_rsLocalName{};
public:
    CDWriteFontFactory(DwFontFactoryNoDef_T) noexcept {}

    CDWriteFontFactory(int iDpi = USER_DEFAULT_SCREEN_DPI) noexcept
    {
        SetLocaleNameToDefault();
        SetTemplateToDefault(iDpi);
    }

    ~CDWriteFontFactory() noexcept
    {
        SafeRelease(m_pFontCollection);
    }

    void SetLocaleNameToDefault() noexcept
    {
        auto cch = GetUserDefaultLocaleName(
            m_rsLocalName.Data(), m_rsLocalName.Capacity());
        if (cch)
            m_rsLocalName.ReSize(cch);
        else
        {
            m_rsLocalName.Reserve(LOCALE_NAME_MAX_LENGTH);
            cch = GetUserDefaultLocaleName(
                m_rsLocalName.Data(), m_rsLocalName.Capacity());
            m_rsLocalName.ReSize(cch);
            m_rsLocalName.ShrinkToFit();
        }
    }

    void SetTemplateToDefault(int iDpi = USER_DEFAULT_SCREEN_DPI) noexcept
    {
        LOGFONTW lf;
        DaSystemParametersInfo(SPI_GETICONTITLELOGFONT,
            sizeof(lf), &lf, 0, iDpi);
        m_Template.rsFace = lf.lfFaceName;
        m_Template.cy = fabsf((float)lf.lfHeight);
        m_Template.eWeight = (DWRITE_FONT_WEIGHT)lf.lfWeight;
        m_Template.bItalic = lf.lfItalic ? TRUE : FALSE;
    }

    void SetFontCollection(IDWriteFontCollection* pFontCollection) noexcept
    {
        std::swap(m_pFontCollection, pFontCollection);
        if (m_pFontCollection)
            m_pFontCollection->AddRef();
        if (pFontCollection)
            pFontCollection->Release();
    }

    EckInline HRESULT NewFont(_Out_ IDWriteTextFormat*& pTf, PCWSTR pszFace,
        float fSize, BOOL bItalic = FALSE,
        DWRITE_FONT_WEIGHT eWeight = DWRITE_FONT_WEIGHT_NORMAL) const noexcept
    {
        return g_pDwFactory->CreateTextFormat(
            pszFace, m_pFontCollection, eWeight,
            bItalic ? DWRITE_FONT_STYLE_ITALIC : DWRITE_FONT_STYLE_NORMAL,
            DWRITE_FONT_STRETCH_NORMAL,
            fSize, m_rsLocalName.Data(), &pTf);
    }

    EckInline HRESULT NewFont(_Out_ IDWriteTextFormat*& pTf, PCWSTR pszFace,
        float fSize, BOOL bItalic = FALSE, int iWeight = 400) const noexcept
    {
        return NewFont(pTf, pszFace, fSize, bItalic, (DWRITE_FONT_WEIGHT)iWeight);
    }

    EckInline HRESULT NewFontWithTemplate(_Out_ IDWriteTextFormat*& pTf) const noexcept
    {
        return NewFont(pTf, m_Template.rsFace.Data(), m_Template.cy,
            m_Template.bItalic, m_Template.eWeight);
    }

    EckInline HRESULT NewFont(_Out_ IDWriteTextFormat*& pTf,
        Align eAlignText = Align::Near, Align eAlignPara = Align::Near,
        float fSize = -1.f, int iWeight = 400, BOOL bEllipsis = FALSE) const noexcept
    {
        const auto hr = NewFont(pTf, m_Template.rsFace.Data(),
            fSize < 0.f ? m_Template.cy : fSize,
            m_Template.bItalic, (DWRITE_FONT_WEIGHT)iWeight);
        if (SUCCEEDED(hr))
        {
            DWRITE_PARAGRAPH_ALIGNMENT e1;
            switch (eAlignPara)
            {
            default:			e1 = DWRITE_PARAGRAPH_ALIGNMENT_NEAR; break;
            case Align::Far:	e1 = DWRITE_PARAGRAPH_ALIGNMENT_FAR; break;
            case Align::Center: e1 = DWRITE_PARAGRAPH_ALIGNMENT_CENTER; break;
            }
            pTf->SetParagraphAlignment(e1);
            DWRITE_TEXT_ALIGNMENT e2;
            switch (eAlignText)
            {
            default:			e2 = DWRITE_TEXT_ALIGNMENT_LEADING; break;
            case Align::Far:	e2 = DWRITE_TEXT_ALIGNMENT_TRAILING; break;
            case Align::Center: e2 = DWRITE_TEXT_ALIGNMENT_CENTER; break;
            }
            pTf->SetTextAlignment(e2);
            if (bEllipsis)
            {
                ComPtr<IDWriteInlineObject> pEllipsis;
                g_pDwFactory->CreateEllipsisTrimmingSign(pTf, &pEllipsis);
                constexpr DWRITE_TRIMMING Opt{ DWRITE_TRIMMING_GRANULARITY_CHARACTER };
                pTf->SetTrimming(&Opt, pEllipsis.Get());
            }
        }
        return hr;
    }

    EckInlineNdCe auto& GetTemplate() noexcept { return m_Template; }
    EckInlineNdCe auto& GetLocaleName() noexcept { return m_rsLocalName; }
};
ECK_NAMESPACE_END