#pragma once
#include "CWnd.h"

ECK_NAMESPACE_BEGIN
class CBk : public CWnd
{
public:
    ECK_RTTI(CBk, CWnd);
    ECK_CWND_SINGLEOWNER(CBk);
    ECK_CWND_CREATE_CLS_HINST(WCN_BK, g_hInstance);
};
ECK_NAMESPACE_END