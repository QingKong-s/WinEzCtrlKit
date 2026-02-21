#pragma once
#include "CWnd.h"

ECK_NAMESPACE_BEGIN
class CDummyWindow : public CWindow
{
public:
    ECK_RTTI(CDummyWindow, CWindow);
    ECK_CWND_SINGLEOWNER(CDummyWindow);
    ECK_CWND_CREATE_CLS_HINST(WCN_BK, g_hInstance);
};
ECK_NAMESPACE_END