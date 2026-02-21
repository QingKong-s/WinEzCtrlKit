#pragma once
#include "CWnd.h"

ECK_NAMESPACE_BEGIN
class CHotKey : public CWindow
{
public:
    ECK_RTTI(CHotKey, CWindow);
    ECK_CWND_NOSINGLEOWNER(CHotKey);
    ECK_CWND_CREATE_CLS(HOTKEY_CLASSW);

    /// <summary>
    /// 取热键
    /// </summary>
    /// <param name="vkCom">功能键组合，HOTKEYF_常量</param>
    /// <returns>热键的虚拟键代码</returns>
    UINT GetHotKey(_Out_ UINT& vkCom) const noexcept
    {
        WORD wRet = (WORD)SendMsg(HKM_GETHOTKEY, 0, 0);
        vkCom = HIBYTE(wRet);
        return LOBYTE(wRet);
    }

    void SetHotKey(UINT vk, UINT vkCom) const noexcept
    {
        SendMsg(HKM_SETHOTKEY, MAKEWORD(vk, vkCom), 0);
    }

    /// <summary>
    /// 置无效规则
    /// </summary>
    /// <param name="uInvalid">无效组合键标志，HKCOMB_常量</param>
    /// <param name="uReplace">输入无效时替换为哪种组合键，HOTKEYF_常量</param>
    void SetRules(UINT uInvalid, UINT uReplace) const noexcept
    {
        SendMsg(HKM_SETRULES, uInvalid, uReplace);
    }
};
ECK_NAMESPACE_END