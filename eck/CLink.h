/*
* WinEzCtrlKit Library
*
* CLink.h ： 标准链接标签
*
* Copyright(C) 2024 QingKong
*/
#pragma once
#include "CWnd.h"

ECK_NAMESPACE_BEGIN
/*
* 标准链接标签产生WM_CTLCOLORSTATIC消息，
* 若要适配暗色则父窗口应给予处理
*/
class CLink :public CWnd
{
public:
	ECK_RTTI(CLink);
	ECK_CWND_NOSINGLEOWNER(CLink);
	ECK_CWND_CREATE_CLS(WC_LINK);

	ECK_CWNDPROP_STYLE(Transparent, LWS_TRANSPARENT);
	ECK_CWNDPROP_STYLE(IgnoreReturn, LWS_IGNORERETURN);
	ECK_CWNDPROP_STYLE(NoPrefix, LWS_NOPREFIX);
	ECK_CWNDPROP_STYLE(UseVisualStyle, LWS_USEVISUALSTYLE);
	ECK_CWNDPROP_STYLE(UseCustomText, LWS_USECUSTOMTEXT);
	ECK_CWNDPROP_STYLE(AlignRight, LWS_RIGHT);

	EckInline int GetIdealHeight() const
	{
		return (int)SendMsg(LM_GETIDEALHEIGHT, 0, 0);
	}

	EckInline int GetIdealSize(int cxMax, SIZE* psize) const
	{
		return (int)SendMsg(LM_GETIDEALSIZE, cxMax, (LPARAM)psize);
	}

	EckInline BOOL GetItem(LITEM* pli) const
	{
		return (BOOL)SendMsg(LM_GETITEM, 0, (LPARAM)pli);
	}

	EckInline BOOL HitTest(LHITTESTINFO* plht) const
	{
		return (BOOL)SendMsg(LM_HITTEST, 0, (LPARAM)plht);
	}

	EckInline BOOL SetItem(LITEM* pli) const
	{
		return (BOOL)SendMsg(LM_SETITEM, 0, (LPARAM)pli);
	}
};
ECK_RTTI_IMPL_BASE_INLINE(CLink, CWnd);
ECK_NAMESPACE_END