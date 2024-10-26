/*
* WinEzCtrlKit Library
*
* CHotKey.h ： 标准热键框
*
* Copyright(C) 2023-2024 QingKong
*/
#pragma once
#include "CWnd.h"

ECK_NAMESPACE_BEGIN
class CHotKey :public CWnd
{
public:
	ECK_RTTI(CHotKey);
	ECK_CWND_NOSINGLEOWNER(CHotKey);
	ECK_CWND_CREATE_CLS(HOTKEY_CLASSW);

	UINT GetHotKey(UINT& vkCom) const
	{
		WORD wRet = (WORD)SendMsg(HKM_GETHOTKEY, 0, 0);
		vkCom = HIBYTE(wRet);
		return LOBYTE(wRet);
	}

	void SetHotKey(UINT vk, UINT vkCom) const
	{
		SendMsg(HKM_SETHOTKEY, MAKEWORD(vk, vkCom), 0);
	}

	/// <summary>
	/// 置无效规则
	/// </summary>
	/// <param name="uInvalid">无效组合键标志，HKCOMB_常量</param>
	/// <param name="uReplace">输入无效时替换为哪种组合键，HOTKEYF_常量</param>
	void SetRules(UINT uInvalid, UINT uReplace) const
	{
		SendMsg(HKM_SETRULES, uInvalid, uReplace);
	}
};
ECK_RTTI_IMPL_BASE_INLINE(CHotKey, CWnd);
ECK_NAMESPACE_END