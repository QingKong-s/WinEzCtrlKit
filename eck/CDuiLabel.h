#pragma once
#include "DuiBase.h"

ECK_NAMESPACE_BEGIN
ECK_DUI_NAMESPACE_BEGIN
enum : int
{
    CxLabelFade = 40,
};
class CLabel : public CElem
{
private:
    IDWriteTextLayout* m_pLayout{};
    union
    {
        ID2D1SolidColorBrush* m_pBrSolid{};
        ID2D1LinearGradientBrush* m_pBrFade;
    };
    ID2D1Bitmap* m_pBmp{};
    D2D1_COLOR_F m_crText{};

    BOOLEAN m_bTransparent{ TRUE };
    BOOLEAN m_bOnlyBitmap{};
    BOOLEAN m_bFullElem{};
    BOOLEAN m_bUserColor{};
    BOOLEAN m_bFade{};
    BkImgMode m_eBkImgMode{ BkImgMode::TopLeft };
    BYTE m_eInterMode{ D2D1_INTERPOLATION_MODE_LINEAR };

    void UpdateTextLayout(PCWSTR pszText, int cchText)
    {
        SafeRelease(m_pLayout);
        if (!pszText || !cchText || m_bOnlyBitmap)
            return;
        float cx;
        if (m_pBmp)
            cx = GetWidthF() - m_pBmp->GetSize().width -
            GetTheme()->GetMetrics(Metrics::SmallPadding);
        else
            cx = GetWidthF();
        g_pDwFactory->CreateTextLayout(pszText, cchText,
            GetTextFormat(), cx, GetHeightF(), &m_pLayout);
    }

    EckInline void UpdateTextLayout()
    {
        UpdateTextLayout(GetText().Data(), GetText().Size());
    }

    void UpdateFadeBrush()
    {
        const auto cx = GetWidthF();
        const D2D1_GRADIENT_STOP Stop[]
        {
            { (cx - (float)CxLabelFade) / cx,m_crText },
            { 1.f }
        };
        ComPtr<ID2D1GradientStopCollection> pStopColl;
        m_pDC->CreateGradientStopCollection(Stop, 2, &pStopColl);
        SafeRelease(m_pBrFade);
        const D2D1_LINEAR_GRADIENT_BRUSH_PROPERTIES Prop{ {}, { cx,0.f } };
        m_pDC->CreateLinearGradientBrush(Prop, pStopColl.Get(), &m_pBrFade);
    }
public:
    LRESULT OnEvent(UINT uMsg, WPARAM wParam, LPARAM lParam) noexcept override
    {
        switch (uMsg)
        {
        case WM_PAINT:
        {
            ELEMPAINTSTRU ps;
            BeginPaint(ps, wParam, lParam);

            if (m_bOnlyBitmap)
            {
                DrawBackgroundImage(m_pDC, m_pBmp, GetViewRectF(), -1, -1,
                    m_eBkImgMode, m_bFullElem, (D2D1_INTERPOLATION_MODE)m_eInterMode);
            }
            else
            {
                float x{};
                if (m_pBmp)
                {
                    const auto sz = m_pBmp->GetSize();
                    D2D1_RECT_F rc;
                    rc.left = x;
                    rc.top = (GetHeightF() - sz.height) / 2.f;
                    rc.right = x + sz.width;
                    rc.bottom = rc.top + sz.height;
                    m_pDC->DrawBitmap(m_pBmp, rc);
                    x += (sz.width + GetTheme()->GetMetrics(Metrics::SmallPadding));
                }

                if (m_pLayout)
                {
                    ID2D1Brush* pBr;
                    if (m_bFade)
                        pBr = m_pBrFade;
                    else
                    {
                        if (m_bUserColor)
                            m_pBrSolid->SetColor(m_crText);
                        else
                        {
                            D2D1_COLOR_F cr;
                            GetTheme()->GetSysColor(SysColor::Text, cr);
                            m_pBrSolid->SetColor(cr);
                        }
                        pBr = m_pBrSolid;
                    }
                    m_pDC->DrawTextLayout({ x }, m_pLayout, pBr,
                        DrawTextLayoutFlags);
                }
            }
            ECK_DUI_DBG_DRAW_FRAME;
            EndPaint(ps);
        }
        return 0;

        case WM_SETTEXT:
            UpdateTextLayout((PCWSTR)lParam, (int)wParam);
            return 0;
        case WM_SIZE:
            UpdateTextLayout();
            if (m_bFade)
                UpdateFadeBrush();
            return 0;
        case WM_SETFONT:
            UpdateTextLayout();
            if (lParam)
                InvalidateRect();
            break;

        case WM_ERASEBKGND:
        {
            const auto* const pps = (ELEMPAINTSTRU*)lParam;
            if (!m_bTransparent)
                GetTheme()->DrawBackground(Part::LabelBk, State::Normal,
                    pps->rcfClipInElem, nullptr);
        }
        return 0;

        case WM_CREATE:
            m_pDC->CreateSolidColorBrush({}, &m_pBrSolid);
            UpdateTextLayout();
            break;

        case WM_DESTROY:
            SafeRelease(m_pLayout);
            SafeRelease(m_pBrSolid);
            SafeRelease(m_pBmp);
            break;
        }
        return CElem::OnEvent(uMsg, wParam, lParam);
    }

    void SetBitmap(ID2D1Bitmap* pBmp)
    {
        ECK_DUILOCK;
        std::swap(m_pBmp, pBmp);
        if (m_pBmp)
            m_pBmp->AddRef();
        if (pBmp)
            pBmp->Release();
        UpdateTextLayout();
    }
    EckInlineNdCe auto GetBitmap() const { return m_pBmp; }

    void SetFade(BOOL b)
    {
        ECK_DUILOCK;
        m_bFade = b;
        if (m_bFade)
            UpdateFadeBrush();
        else
        {
            SafeRelease(m_pBrFade);
            m_pDC->CreateSolidColorBrush({}, &m_pBrSolid);
        }
    }
    EckInlineNdCe BOOL GetFade() const { return m_bFade; }

    void UpdateFadeColor()
    {
        ECK_DUILOCK;
        if (m_bFade)
            UpdateFadeBrush();
    }

    EckInlineCe void SetTransparent(BOOL b) { m_bTransparent = b; }
    EckInlineNdCe BOOL GetTransparent() const { return m_bTransparent; }

    EckInlineCe void SetOnlyBitmap(BOOL b) { m_bOnlyBitmap = b; }
    EckInlineNdCe BOOL GetOnlyBitmap() const { return m_bOnlyBitmap; }

    EckInlineCe void SetFullElem(BOOL b) { m_bFullElem = b; }
    EckInlineNdCe BOOL GetFullElem() const { return m_bFullElem; }

    EckInlineCe void SetBackgroundMode(BkImgMode e) { m_eBkImgMode = e; }
    EckInlineNdCe BkImgMode GetBackgroundMode() const { return m_eBkImgMode; }

    EckInlineCe void SetInterMode(D2D1_INTERPOLATION_MODE e) { m_eInterMode = e; }
    EckInlineNdCe D2D1_INTERPOLATION_MODE GetInterMode() const { return (D2D1_INTERPOLATION_MODE)m_eInterMode; }

    EckInlineCe void SetUserColor(BOOL b) { m_bUserColor = b; }
    EckInlineNdCe BOOL GetUserColor() const { return m_bUserColor; }

    EckInlineCe void SetColor(const D2D1_COLOR_F& cr) { m_crText = cr; }
    EckInlineNdCe const auto& GetColor() const { return m_crText; }
};
ECK_DUI_NAMESPACE_END
ECK_NAMESPACE_END