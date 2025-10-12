#pragma once
#include "DuiBase.h"
#include "CD2DImageList.h"

ECK_NAMESPACE_BEGIN
ECK_DUI_NAMESPACE_BEGIN
struct NMTTHDISPINFO : DUINMHDR
{
    DispInfoMask uMask;
    PCWSTR pszText;
    int cchText;
};

class CTabHeader : public CElem
{
private:
    struct ITEM
    {
        ComPtr<IDWriteTextLayout> pTextLayout;
        int x;
        int cx;
    };

    std::vector<ITEM> m_vItem{};
    CD2DImageList* m_pImgList{};
public:
    LRESULT OnEvent(UINT uMsg, WPARAM wParam, LPARAM lParam) override
    {
        switch (uMsg)
        {
        case WM_PAINT:
        {
            ELEMPAINTSTRU ps;
            BeginPaint(ps, wParam, lParam);

            EndPaint(ps);
        }
        break;
        }
        return __super::OnEvent(uMsg, wParam, lParam);
    }
};
ECK_DUI_NAMESPACE_END
ECK_NAMESPACE_END