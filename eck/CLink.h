#pragma once
#include "CWindow.h"

ECK_NAMESPACE_BEGIN
// 标准链接标签产生WM_CTLCOLORSTATIC消息，
// 若要适配暗色则父窗口应给予处理
class CLink : public CWindow
{
public:
    ECK_RTTI(CLink, CWindow);
    ECK_CWND_NOSINGLEOWNER(CLink);
    ECK_CWND_CREATE_CLS(WC_LINK);

    ECK_CWNDPROP_STYLE(Transparent, LWS_TRANSPARENT);
    ECK_CWNDPROP_STYLE(IgnoreReturn, LWS_IGNORERETURN);
    ECK_CWNDPROP_STYLE(NoPrefix, LWS_NOPREFIX);
    ECK_CWNDPROP_STYLE(UseVisualStyle, LWS_USEVISUALSTYLE);
    ECK_CWNDPROP_STYLE(UseCustomText, LWS_USECUSTOMTEXT);
    ECK_CWNDPROP_STYLE(AlignRight, LWS_RIGHT);

    EckInline int GetIdealHeight() const noexcept
    {
        return (int)SendMsg(LM_GETIDEALHEIGHT, 0, 0);
    }

    EckInline int GetIdealSize(int cxMax, _Out_ SIZE* psize) const noexcept
    {
        return (int)SendMsg(LM_GETIDEALSIZE, cxMax, (LPARAM)psize);
    }

    EckInline BOOL GetItem(_Inout_ LITEM* pli) const noexcept
    {
        return (BOOL)SendMsg(LM_GETITEM, 0, (LPARAM)pli);
    }

    EckInline BOOL HitTest(_Inout_ LHITTESTINFO* plht) const noexcept
    {
        return (BOOL)SendMsg(LM_HITTEST, 0, (LPARAM)plht);
    }

    EckInline BOOL SetItem(_In_ const LITEM* pli) const noexcept
    {
        return (BOOL)SendMsg(LM_SETITEM, 0, (LPARAM)pli);
    }
};
ECK_NAMESPACE_END