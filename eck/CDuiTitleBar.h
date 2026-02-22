#pragma once
#include "DuiBase.h"
#include "ImageHelper.h"

ECK_NAMESPACE_BEGIN
ECK_DUI_NAMESPACE_BEGIN
class CTitleBar : public CElem
{
protected:
    CDwmWndPartMgr m_DwmPartMgr{};
    ID2D1Bitmap1* m_pBmpDwmWndAtlas{};
    float m_cxClose{};
    float m_cxMax{};
    float m_cxMin{};
    float m_cyBtn{};
    DwmWndPart m_idxHot{ DwmWndPart::Invalid };
    DwmWndPart m_idxPressed{ DwmWndPart::Invalid };
    BOOLEAN m_bMaximized{};
    BYTE m_eInterMode{ (BYTE)D2D1_INTERPOLATION_MODE_NEAREST_NEIGHBOR };

    DwmWPartState GetPartState(DwmWndPart idx) const
    {
        if (m_idxPressed == idx)
            return DwmWPartState::Pressed;
        else if (m_idxHot == idx)
            return DwmWPartState::Hot;
        return DwmWPartState::Normal;
    }

    LRESULT OnWindowMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, SlotCtx&)
    {
        if (uMsg == WM_WINDOWPOSCHANGED)
            m_bMaximized = IsZoomed(hWnd);
        return 0;
    }

    static constexpr BOOL IsNeedRedraw(DwmWndPart ePart)
    {
        return ePart != DwmWndPart::Invalid && ePart != DwmWndPart::Extra;
    }
    static constexpr BOOL IsNeedRedraw(DwmWndPart ePartOle, DwmWndPart ePartNew)
    {
        return (ePartOle != ePartNew) && (IsNeedRedraw(ePartOle) || IsNeedRedraw(ePartNew));
    }
public:
    LRESULT OnEvent(UINT uMsg, WPARAM wParam, LPARAM lParam) noexcept override
    {
        switch (uMsg)
        {
        case WM_PAINT:
        {
            ELEMPAINTSTRU eps;
            BeginPaint(eps, wParam, lParam);

            D2D1_RECT_F rcDst
            {
                GetWidthF() - m_cxClose,
                0,
                GetWidthF(),
                (float)m_cyBtn
            };
            D2D1_RECT_F rcTemp;

            RECT rc, rcBkg;
            DWMW_GET_PART_EXTRA Extra;
            const auto bDarkMode = ShouldAppsUseDarkMode();
            const auto iUserDpi = GetWnd()->GetUserDpi();
            const auto dMargin = DpiScaleF(1.f, 96, iUserDpi);
            const auto eInterMode = (D2D1_INTERPOLATION_MODE)m_eInterMode;

            m_pDC->SetAntialiasMode(D2D1_ANTIALIAS_MODE_ALIASED);
            if (m_DwmPartMgr.GetPartRect(rc, rcBkg, DwmWndPart::Close,
                GetPartState(DwmWndPart::Close), bDarkMode, TRUE, iUserDpi, &Extra))
            {
                rcDst.left += dMargin;
                DrawImageFromGrid(m_pDC, m_pBmpDwmWndAtlas, rcDst,
                    MakeD2DRectF(rcBkg), MarginsToD2DRectF(Extra.pBkg->mgSizing),
                    (D2D1_INTERPOLATION_MODE)m_eInterMode);

                rcTemp = MakeD2DRectF(rc);
                GetWnd()->Phy2Log(rcTemp);
                CenterRect(rcTemp, rcDst);

                m_pDC->DrawBitmap(m_pBmpDwmWndAtlas, rcTemp, 1.f,
                    D2D1_INTERPOLATION_MODE_NEAREST_NEIGHBOR, MakeD2DRectF(rc));
                //rcDst.left -= dMargin;
            }

            rcDst.left -= m_cxMax;
            rcDst.right = rcDst.left + m_cxMax;

            if (m_DwmPartMgr.GetPartRect(rc, rcBkg,
                m_bMaximized ? DwmWndPart::Restore : DwmWndPart::Max,
                GetPartState(DwmWndPart::Max), bDarkMode, TRUE, iUserDpi, &Extra))
            {
                rcDst.left += dMargin;
                DrawImageFromGrid(m_pDC, m_pBmpDwmWndAtlas, rcDst,
                    MakeD2DRectF(rcBkg), MarginsToD2DRectF(Extra.pBkg->mgSizing),
                    (D2D1_INTERPOLATION_MODE)m_eInterMode);

                rcTemp = MakeD2DRectF(rc);
                GetWnd()->Phy2Log(rcTemp);
                CenterRect(rcTemp, rcDst);

                m_pDC->DrawBitmap(m_pBmpDwmWndAtlas, rcTemp, 1.f,
                    D2D1_INTERPOLATION_MODE_NEAREST_NEIGHBOR, MakeD2DRectF(rc));
                rcDst.left -= dMargin;
            }

            rcDst.left -= m_cxMin;
            rcDst.right = rcDst.left + m_cxMin;

            if (m_DwmPartMgr.GetPartRect(rc, rcBkg, DwmWndPart::Min,
                GetPartState(DwmWndPart::Min), bDarkMode, TRUE, iUserDpi, &Extra))
            {
                rcDst.left += (dMargin * 2);
                DrawImageFromGrid(m_pDC, m_pBmpDwmWndAtlas, rcDst,
                    MakeD2DRectF(rcBkg), MarginsToD2DRectF(Extra.pBkg->mgSizing),
                    (D2D1_INTERPOLATION_MODE)m_eInterMode);

                rcTemp = MakeD2DRectF(rc);
                GetWnd()->Phy2Log(rcTemp);
                CenterRect(rcTemp, rcDst);

                m_pDC->DrawBitmap(m_pBmpDwmWndAtlas, rcTemp, 1.0f,
                    D2D1_INTERPOLATION_MODE_NEAREST_NEIGHBOR, MakeD2DRectF(rc));
            }
            m_pDC->SetAntialiasMode(D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);

            ECK_DUI_DBG_DRAW_FRAME;
            EndPaint(eps);
        }
        return 0;

        case WM_NCHITTEST:
        {
            POINT pt ECK_GET_PT_LPARAM(lParam);
            ClientToElem(pt);
            const DwmWndPart ePart = HitTest(pt);
            switch (ePart)
            {
            case DwmWndPart::Close:
                return HTCLOSE;
            case DwmWndPart::Max:
                return HTMAXBUTTON;
            case DwmWndPart::Min:
                return HTMINBUTTON;
            case DwmWndPart::Extra:
                if (!m_bMaximized && pt.y < DaGetSystemMetrics(SM_CYFRAME, 96))
                    return HTTOP;
                else
                    return HTCAPTION;
            }
        }
        return HTTRANSPARENT;

        case WM_NCLBUTTONDOWN:
        {
            POINT pt ECK_GET_PT_LPARAM(lParam);
            ScreenToClient(GetWnd()->HWnd, &pt);
            ClientToElem(pt);
            GetWnd()->Phy2Log(pt);
            const auto idxOld = m_idxPressed;
            m_idxPressed = HitTest(pt);
            if (IsNeedRedraw(idxOld, m_idxPressed))
                InvalidateBtnRect();
        }
        return 0;

        case WM_NCLBUTTONUP:
        {
            if (m_idxPressed != DwmWndPart::Invalid)
            {
                POINT pt ECK_GET_PT_LPARAM(lParam);
                ScreenToClient(GetWnd()->HWnd, &pt);
                ClientToElem(pt);
                GetWnd()->Phy2Log(pt);
                const auto idx = HitTest(pt);
                if (m_idxPressed == idx)
                    switch (idx)
                    {
                    case DwmWndPart::Close:
                        GetWnd()->PostMessage(WM_SYSCOMMAND, SC_CLOSE, 0);
                        break;
                    case DwmWndPart::Max:
                        if (m_bMaximized)
                            GetWnd()->PostMessage(WM_SYSCOMMAND, SC_RESTORE, 0);
                        else
                            GetWnd()->PostMessage(WM_SYSCOMMAND, SC_MAXIMIZE, 0);
                        break;
                    case DwmWndPart::Min:
                        GetWnd()->PostMessage(WM_SYSCOMMAND, SC_MINIMIZE, 0);
                        break;
                    }
                const auto idxOld = m_idxPressed;
                m_idxPressed = DwmWndPart::Invalid;
                if (IsNeedRedraw(idxOld, DwmWndPart::Invalid))
                    InvalidateBtnRect();
            }
        }
        return 0;

        case WM_NCMOUSEMOVE:
        {
            POINT pt ECK_GET_PT_LPARAM(lParam);
            ScreenToClient(GetWnd()->HWnd, &pt);
            ClientToElem(pt);
            GetWnd()->Phy2Log(pt);
            const auto idxOld = m_idxHot;
            m_idxHot = HitTest(pt);
            if (IsNeedRedraw(m_idxHot, idxOld))
                InvalidateBtnRect();
        }
        return 0;

        case WM_MOUSELEAVE:
        {
            if (m_idxHot != DwmWndPart::Invalid)
            {
                const auto idxOld = m_idxHot;
                m_idxHot = DwmWndPart::Invalid;
                if (IsNeedRedraw(idxOld, DwmWndPart::Invalid))
                    InvalidateBtnRect();
            }
        }
        return 0;

        case WM_CAPTURECHANGED:
        {
            if (m_idxPressed != DwmWndPart::Invalid)
            {
                const auto idxOld = m_idxPressed;
                m_idxPressed = DwmWndPart::Invalid;
                if (IsNeedRedraw(idxOld, DwmWndPart::Invalid))
                    InvalidateBtnRect();
            }
        }
        return 0;

        case WM_CREATE:
        {
            GetWnd()->GetSignal().Connect(this, &CTitleBar::OnWindowMessage, MHI_DUI_TITLEBAR);
            m_bMaximized = IsZoomed(GetWnd()->HWnd);
            UpdateTitleBarInfo(TRUE);
            UpdateMetrics();
        }
        break;

        case WM_DESTROY:
            GetWnd()->GetSignal().Disconnect(MHI_DUI_TITLEBAR);
            SafeRelease(m_pBmpDwmWndAtlas);
            break;
        }
        return CElem::OnEvent(uMsg, wParam, lParam);
    }

    void UpdateTitleBarInfo(BOOL bForceUpdate = FALSE)
    {
        if (bForceUpdate || !m_DwmPartMgr.GetHTheme())
        {
            m_DwmPartMgr.AnalyzeDefaultTheme();
            PCVOID pData;
            UINT cbData;
            m_DwmPartMgr.GetData(&pData, &cbData);
            const auto pStream = new CStreamView(pData, cbData);
            IWICBitmapDecoder* pDecoder;
            IWICBitmap* pBitmap{};
            CreateWicBitmapDecoder(pStream, pDecoder);
            CreateWicBitmap(pBitmap, pDecoder);
            m_pDC->CreateBitmapFromWicBitmap(pBitmap, &m_pBmpDwmWndAtlas);
            pDecoder->Release();
            pBitmap->Release();
            pStream->LeaveRelease();
        }
    }

    void UpdateMetrics()
    {
        if (g_NtVer.uBuild >= WINVER_11_21H2)
        {
            const auto iWndDpi = GetWnd()->GetDpiValue();
            //----计算高度
            m_cyBtn = (float)DaGetSystemMetrics(SM_CYSIZE, iWndDpi);
            m_cyBtn += float(DaGetSystemMetrics(SM_CXPADDEDBORDER, iWndDpi) +
                DaGetSystemMetrics(SM_CYFRAME, iWndDpi) +
                DaGetSystemMetrics(SM_CYBORDER, iWndDpi));
            m_cyBtn = m_cyBtn * 96.f / iWndDpi;
            // 高度为SM_CYSIZE + 通常模式下的客户区上边距
            //----计算宽度
            int nSys = DaGetSystemMetrics(SM_CYSIZE, iWndDpi);
            nSys = (int)floorf(nSys * 0.95454544f + 0.5f);
            // 对于标准的4个按钮（关闭、最大化、最小化、帮助）
            // 在两边的使用2.2272727，中间的使用2.1818182
            // 若只有关闭按钮，使用1.6363636
            m_cxClose = m_cxMin = floorf(nSys * 2.2272727f + 0.5f)
                * 96.f / iWndDpi;
            m_cxMax = floorf(nSys * 2.1818182f + 0.5f)
                * 96.f / iWndDpi;
            return;
        }

        if (g_NtVer.uMajor == 6 && (g_NtVer.uMinor == 2 || g_NtVer.uMinor == 3))
        {
            m_cxClose = 46;
            m_cxMax = m_cxMin = m_cxClose * 80 / 150;
            m_cyBtn = 21;
        }
        else
        {
            m_cxClose = 46;
            m_cxMax = m_cxMin = m_cxClose;
            m_cyBtn = 31;
        }
    }

    DwmWndPart HitTest(POINT ptClient) const
    {
        const auto cx = GetWidthF();
        const auto cy = GetHeightF();
        if (ptClient.x < 0 || ptClient.x > cx || ptClient.y < 0 || ptClient.y > cy ||
            ptClient.y > m_cyBtn)
            return DwmWndPart::Invalid;
        if (ptClient.x > cx - m_cxClose)
            return DwmWndPart::Close;
        else if (ptClient.x > cx - m_cxMax - m_cxClose)
            return DwmWndPart::Max;
        else if (ptClient.x > cx - m_cxMin * 2 - m_cxClose)
            return DwmWndPart::Min;
        else
            return DwmWndPart::Extra;
    }

    void InvalidateBtnRect()
    {
        const auto cxBtn = m_cxClose + m_cxMax + m_cxMin;
        D2D1_RECT_F rc{ GetWidthF() - cxBtn,0,GetWidthF(),m_cyBtn };
        InvalidateRect(rc);
    }

    EckInlineCe void SetInterpolationMode(D2D1_INTERPOLATION_MODE eInterMode)
    {
        m_eInterMode = eInterMode;
    }
    EckInlineNdCe D2D1_INTERPOLATION_MODE GetInterpolationMode() const
    {
        return (D2D1_INTERPOLATION_MODE)m_eInterMode;
    }
};
ECK_DUI_NAMESPACE_END
ECK_NAMESPACE_END