#pragma once
class CWndSelectOverlay : public eck::CWnd
{
private:
    HPEN m_hPen{};
    std::vector<RECT> m_vRect{};

    void ReCreatePen(COLORREF cr = 0x000000) noexcept;
public:
    ECK_CWND_CREATE_CLS_HINST(eck::WCN_DUMMY, eck::g_hInstance);

    LRESULT OnMsg(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) override;

    void SetRect(HWND hWndRef, const RECT* prc, size_t cRc) noexcept;
    void ClearRect() noexcept;
};