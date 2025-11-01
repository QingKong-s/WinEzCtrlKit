#pragma once
#include "DuiBase.h"

ECK_NAMESPACE_BEGIN
ECK_DUI_NAMESPACE_BEGIN
class CSizeBox
{
private:

    CElem* m_pElem{};
    CElem::HSlot m_hSlot{};
    HCURSOR m_hCursor{};
    int m_htTracking{};
    float cxBorder{ 3.f };
    BOOLEAN m_bAllowMove{ TRUE };
    BOOLEAN m_bAllowSize{ TRUE };
    D2D1_POINT_2F m_ptStart{};
    D2D1_RECT_F m_rcStart{};
public:
    ECK_DISABLE_COPY_MOVE_DEF_CONS(CSizeBox);
    ~CSizeBox() { Detach(); }

    static HCURSOR LoadCursorFromHtCode(int ht)
    {
        PCWSTR pszCursor;
        switch (ht)
        {
        case HTTOP:
        case HTBOTTOM:
            pszCursor = IDC_SIZENS;
            break;
        case HTLEFT:
        case HTRIGHT:
            pszCursor = IDC_SIZEWE;
            break;
        case HTTOPLEFT:
        case HTBOTTOMRIGHT:
            pszCursor = IDC_SIZENWSE;
            break;
        case HTTOPRIGHT:
        case HTBOTTOMLEFT:
            pszCursor = IDC_SIZENESW;
            break;
        default:
            pszCursor = IDC_ARROW;
            break;
        }
        return LoadCursorW(nullptr, pszCursor);
    }

    int HitTestBorder(D2D1_POINT_2F pt) const
    {
        if (m_bAllowSize)
            if (pt.x < cxBorder)
            {
                if (pt.y < cxBorder)
                    return HTTOPLEFT;
                else if (pt.y >= m_pElem->GetHeightF() - cxBorder)
                    return HTBOTTOMLEFT;
                else
                    return HTLEFT;
            }
            else if (pt.x >= m_pElem->GetWidthF() - cxBorder)
            {
                if (pt.y < cxBorder)
                    return HTTOPRIGHT;
                else if (pt.y >= m_pElem->GetHeightF() - cxBorder)
                    return HTBOTTOMRIGHT;
                else
                    return HTRIGHT;
            }
            else if (pt.y < cxBorder)
                return HTTOP;
            else if (pt.y >= m_pElem->GetHeightF() - cxBorder)
                return HTBOTTOM;
        if (m_bAllowMove)
            return HTCLIENT;
        return HTNOWHERE;
    }

    LRESULT OnEvent(UINT uMsg, WPARAM wParam, LPARAM lParam, SlotCtx& Ctx)
    {
        switch (uMsg)
        {
        case WM_NCHITTEST:
        {
            if (!m_pElem->GetCapture())
            {
                D2D1_POINT_2F pt ECK_GET_PT_LPARAM_F(lParam);
                m_pElem->ClientToElem(pt);
                const int ht = HitTestBorder(pt);
                if (ht != HTNOWHERE)
                {
                    const auto hCursor = LoadCursorFromHtCode(ht);
                    if (hCursor != m_hCursor)
                    {
                        m_hCursor = hCursor;
                        SetCursor(m_hCursor);
                    }
                }
            }
        }
        break;
        case WM_SETCURSOR:
            Ctx.Processed();
            SetCursor(m_hCursor);
            return TRUE;
        case WM_MOUSEMOVE:
        {
            if (m_htTracking == HTNOWHERE)
                break;
            D2D1_POINT_2F pt ECK_GET_PT_LPARAM_F(lParam);
            m_pElem->ElemToClient(pt);
            const auto dx = pt.x - m_ptStart.x;
            const auto dy = pt.y - m_ptStart.y;
            if (m_htTracking == HTCLIENT)
            {
                m_pElem->SetPos(m_rcStart.left + dx, m_rcStart.top + dy);
                break;
            }
            auto rcCurr{ m_rcStart };
            BOOL bChangeR{}, bChangeB{};
            switch (m_htTracking)
            {
            case HTTOP:
                rcCurr.top += dy;
                break;
            case HTBOTTOM:
                rcCurr.bottom += dy;
                bChangeB = TRUE;
                break;
            case HTLEFT:
                rcCurr.left += dx;
                break;
            case HTRIGHT:
                rcCurr.right += dx;
                bChangeR = TRUE;
                break;
            case HTTOPLEFT:
                rcCurr.top += dy;
                rcCurr.left += dx;
                break;
            case HTTOPRIGHT:
                rcCurr.top += dy;
                rcCurr.right += dx;
                bChangeR = TRUE;
                break;
            case HTBOTTOMLEFT:
                rcCurr.bottom += dy;
                rcCurr.left += dx;
                bChangeB = TRUE;
                break;
            case HTBOTTOMRIGHT:
                rcCurr.bottom += dy;
                rcCurr.right += dx;
                bChangeR = bChangeB = TRUE;
                break;
            }
            if (rcCurr.right - rcCurr.left < cxBorder * 2)
                if (bChangeR)
                    rcCurr.right = rcCurr.left + cxBorder * 2;
                else
                    rcCurr.left = rcCurr.right - cxBorder * 2;
            if (rcCurr.bottom - rcCurr.top < cxBorder * 2)
                if (bChangeB)
                    rcCurr.bottom = rcCurr.top + cxBorder * 2;
                else
                    rcCurr.top = rcCurr.bottom - cxBorder * 2;
            m_pElem->SetRect(rcCurr);
        }
        break;
        case WM_LBUTTONDOWN:
        {
            const D2D1_POINT_2F pt ECK_GET_PT_LPARAM_F(lParam);
            m_htTracking = HitTestBorder(pt);
            if (m_htTracking != HTNOWHERE)
            {
                m_ptStart = pt;
                m_pElem->ElemToClient(m_ptStart);
                m_rcStart = m_pElem->GetRectF();
                m_pElem->SetCapture();
            }
        }
        break;
        case WM_LBUTTONUP:
        {
            if (m_htTracking != HTNOWHERE)
            {
                m_htTracking = HTNOWHERE;
                m_pElem->ReleaseCapture();
            }
        }
        break;
        case WM_CAPTURECHANGED:
            m_htTracking = HTNOWHERE;
            break;
        }
        return 0;
    }

    void Attach(CElem* pElem)
    {
        if (m_pElem)
            Detach();
        m_pElem = pElem;
        m_hSlot = m_pElem->GetSignal().Connect(this, &CSizeBox::OnEvent);
    }
    void Detach()
    {
        if (m_pElem)
        {
            m_pElem->GetSignal().Disconnect(m_hSlot);
            m_hSlot = {};
            m_pElem = nullptr;
            m_htTracking = HTNOWHERE;
            m_hCursor = nullptr;
        }
    }

    EckInlineCe void SetAllowMove(BOOLEAN bAllow) { m_bAllowMove = bAllow; }
    EckInlineNdCe BOOLEAN GetAllowMove() const { return m_bAllowMove; }
    EckInlineCe void SetAllowSize(BOOLEAN bAllow) { m_bAllowSize = bAllow; }
    EckInlineNdCe BOOLEAN GetAllowSize() const { return m_bAllowSize; }
    EckInlineCe void SetBorderWidth(float cx) { cxBorder = cx; }
    EckInlineNdCe float GetBorderWidth() const { return cxBorder; }
};
ECK_DUI_NAMESPACE_END
ECK_NAMESPACE_END