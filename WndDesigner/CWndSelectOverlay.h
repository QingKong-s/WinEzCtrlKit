#pragma once
class CWndSelectOverlay : public eck::CWnd
{
private:
    HPEN m_hPen{};
    std::vector<RECT> m_vRect{};
    std::vector<RECT> m_vRectInWorkWnd{};
    int m_ox{}, m_oy{};

    void ReCreatePen(COLORREF cr = 0x000000) noexcept;

    void DrawDragBlock(HDC hDC) noexcept;
public:
    ECK_CWND_CREATE_CLS_HINST(eck::WCN_DUMMY, eck::g_hInstance);

    LRESULT OnMsg(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) override;

    void SetRect(HWND hWndRef, const RECT* prc, size_t cRc) noexcept;
    void ClearRect() noexcept;
    EckInlineNdCe BOOL IsEmpty() const noexcept { return m_vRectInWorkWnd.empty(); }
    constexpr void AddRectInWorkWindow(const RECT& rc) noexcept { m_vRectInWorkWnd.emplace_back(rc); }
    void EndAddRect(HWND hWndRef) noexcept;
    EckInlineNdCe auto& GetRectInWorkWnd() const noexcept { return m_vRectInWorkWnd.front(); }
    void SetOffset(int ox, int oy) noexcept { m_ox = ox, m_oy = oy; }
};