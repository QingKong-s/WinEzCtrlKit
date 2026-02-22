#pragma once
#include "CWindow.h"

ECK_NAMESPACE_BEGIN
class CToolTip : public CWindow
{
public:
    ECK_RTTI(CToolTip, CWindow);
    ECK_CWND_NOSINGLEOWNER(CToolTip);
    ECK_CWND_CREATE_CLS(TOOLTIPS_CLASS);

    ECK_CWNDPROP_STYLE(AlwaysTip, TTS_ALWAYSTIP);
    ECK_CWNDPROP_STYLE(Balloon, TTS_BALLOON);
    ECK_CWNDPROP_STYLE(CloseButton, TTS_CLOSE);
    ECK_CWNDPROP_STYLE(NoAnimate, TTS_NOANIMATE);
    ECK_CWNDPROP_STYLE(NoFade, TTS_NOFADE);
    ECK_CWNDPROP_STYLE(NoPrefix, TTS_NOPREFIX);
    ECK_CWNDPROP_STYLE(UseVisualStyle, TTS_USEVISUALSTYLE);

    EckInline void Active(BOOL bActive) const noexcept
    {
        SendMessage(TTM_ACTIVATE, bActive, 0);
    }

    EckInline BOOL AddTool(_In_ const TTTOOLINFOW* pti) const noexcept
    {
        return (BOOL)SendMessage(TTM_ADDTOOLW, 0, (LPARAM)pti);
    }

    EckInline BOOL AdjustRect(BOOL bTextToWnd, _Inout_ RECT* prc) const noexcept
    {
        return (BOOL)SendMessage(TTM_ADJUSTRECT, bTextToWnd, (LPARAM)prc);
    }

    EckInline void DeleteTool(_In_ const TTTOOLINFOW* pti) const noexcept
    {
        SendMessage(TTM_DELTOOLW, 0, (LPARAM)pti);
    }

    EckInline void EnumerateTools(int idxTool, _Inout_ TTTOOLINFOW* pti) const noexcept
    {
        SendMessage(TTM_ENUMTOOLSW, idxTool, (LPARAM)pti);
    }

    EckInline BOOL GetBubbleSize(_In_ const TTTOOLINFOW* pti,
        _Out_opt_ int* pcx, _Out_opt_ int* pcy) const noexcept
    {
        const auto uRet = (UINT)SendMessage(TTM_GETBUBBLESIZE, 0, (LPARAM)pti);
        if (uRet == 0)
            return FALSE;
        if (pcx)
            *pcx = LOWORD(uRet);
        if (pcy)
            *pcy = HIWORD(uRet);
        return TRUE;
    };

    EckInline BOOL GetCurrentTool(_Inout_ TTTOOLINFOW* pti) const noexcept
    {
        return (BOOL)SendMessage(TTM_GETCURRENTTOOL, 0, (LPARAM)pti);
    }

    /// <summary>
    /// 取延迟时间
    /// </summary>
    /// <param name="iType">TTDT_常量</param>
    /// <returns>以毫秒为单位的时间</returns>
    EckInline int GetDelayTime(int iType) const noexcept
    {
        return (int)SendMessage(TTM_GETDELAYTIME, iType, 0);
    }

    EckInline void GetMargin(_Out_ RECT* prc) const noexcept
    {
        SendMessage(TTM_GETMARGIN, 0, (LPARAM)prc);
    }

    EckInline int GetMaximumTipWidth() const noexcept
    {
        return (int)SendMessage(TTM_GETMAXTIPWIDTH, 0, 0);
    }

    EckInline void GetText(_In_ const TTTOOLINFOW* pti, int cchBuf) const noexcept
    {
        SendMessage(TTM_GETTEXTW, cchBuf, (LPARAM)pti);
    }

    EckInline COLORREF GetTipBackgroundColor() const noexcept
    {
        return (COLORREF)SendMessage(TTM_GETTIPBKCOLOR, 0, 0);
    }

    EckInline COLORREF GetTipTextColor() const noexcept
    {
        return (COLORREF)SendMessage(TTM_GETTIPTEXTCOLOR, 0, 0);
    }

    EckInline void GetTitle(_Inout_ TTGETTITLE* pttgt) const noexcept
    {
        SendMessage(TTM_GETTITLE, 0, (LPARAM)pttgt);
    }

    EckInline int GetToolCount() const noexcept
    {
        return (int)SendMessage(TTM_GETTOOLCOUNT, 0, 0);
    }

    EckInline BOOL GetToolInfomation(_Inout_ TTTOOLINFOW* pti) const noexcept
    {
        return (BOOL)SendMessage(TTM_GETTOOLINFOW, 0, (LPARAM)pti);
    }

    EckInline BOOL HitTest(_Inout_ TTHITTESTINFOW* pti) const noexcept
    {
        return (BOOL)SendMessage(TTM_HITTEST, 0, (LPARAM)pti);
    }

    /// <summary>
    /// 
    /// </summary>
    /// <param name="pti">hwnd、uId、rect </param>
    EckInline void NewToolRect(_In_ const TTTOOLINFOW* pti) const noexcept
    {
        SendMessage(TTM_NEWTOOLRECTW, 0, (LPARAM)pti);
    }

    EckInline void Pop() const noexcept
    {
        SendMessage(TTM_POP, 0, 0);
    }

    EckInline void PopUp() const noexcept
    {
        SendMessage(TTM_POPUP, 0, 0);
    }

    EckInline void RelayEvent(_In_ const MSG* pMsg, LPARAM lExtraInfo = 0) const noexcept
    {
        SendMessage(TTM_RELAYEVENT, lExtraInfo, (LPARAM)pMsg);
    }

    EckInline void SetDelayTime(int iType, int iDelay) const noexcept
    {
        SendMessage(TTM_SETDELAYTIME, iType, LOWORD(iDelay));
    }

    EckInline void SetMargin(_In_ const RECT* prc) const noexcept
    {
        SendMessage(TTM_SETMARGIN, 0, (LPARAM)prc);
    }

    EckInline int SetMaximumTipWidth(int iWidth) const noexcept
    {
        return (int)SendMessage(TTM_SETMAXTIPWIDTH, 0, iWidth);
    }

    EckInline void SetTipBackgroundColor(COLORREF cr) const noexcept
    {
        SendMessage(TTM_SETTIPBKCOLOR, cr, 0);
    }

    EckInline void SetTipTextColor(COLORREF cr) const noexcept
    {
        SendMessage(TTM_SETTIPTEXTCOLOR, cr, 0);
    }

    EckInline BOOL SetTitle(int iIcon, _In_z_ PCWSTR pszTitle) const noexcept
    {
        return (BOOL)SendMessage(TTM_SETTITLEW, iIcon, (LPARAM)pszTitle);
    }
    EckInline BOOL SetTitleHIcon(HICON hIcon, _In_z_ PCWSTR pszTitle) const noexcept
    {
        return (BOOL)SendMessage(TTM_SETTITLEW, (WPARAM)hIcon, (LPARAM)pszTitle);
    }

    EckInline void SetToolInfomation(_In_ const TTTOOLINFOW* pti) const noexcept
    {
        SendMessage(TTM_SETTOOLINFOW, 0, (LPARAM)pti);
    }

    /// <summary>
    /// 
    /// </summary>
    /// <param name="pti">hwnd、uId</param>
    /// <param name="bActivate"></param>
    /// <returns></returns>
    EckInline void TrackActivate(_In_ const TTTOOLINFOW* pti, BOOL bActivate) const noexcept
    {
        SendMessage(TTM_TRACKACTIVATE, bActivate, (LPARAM)pti);
    }

    EckInline void TrackPosition(int x, int y) const noexcept
    {
        SendMessage(TTM_TRACKPOSITION, 0, MAKELPARAM(x, y));
    }

    EckInline void Update() const noexcept
    {
        SendMessage(TTM_UPDATE, 0, 0);
    }

    /// <summary>
    /// 
    /// </summary>
    /// <param name="pti">hwnd、uId、hinst、lpszText</param>
    /// <returns></returns>
    EckInline void UpdateTipText(_In_ const TTTOOLINFOW* pti) const noexcept
    {
        SendMessage(TTM_UPDATETIPTEXTW, 0, (LPARAM)pti);
    }
};
ECK_NAMESPACE_END