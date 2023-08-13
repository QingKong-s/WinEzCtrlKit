#pragma once
#include "CWnd.h"

ECK_NAMESPACE_BEGIN
class CHotKey :public CWnd
{
public:
	EckInline HWND Create(PCWSTR pszText, DWORD dwStyle, DWORD dwExStyle,
		int x, int y, int cx, int cy, HWND hParent, UINT nID)
	{
		dwStyle |= WS_CHILD;
		m_hWnd = CreateWindowExW(dwExStyle, HOTKEY_CLASSW, pszText, dwStyle,
			x, y, cx, cy, hParent, i32ToP<HMENU>(nID), NULL, NULL);
		return m_hWnd;
	}

	UINT GetHotKey(UINT& vkCom)
	{
		WORD wRet = (WORD)SendMsg(HKM_GETHOTKEY, 0, 0);
		vkCom = HIBYTE(wRet);
		return LOBYTE(wRet);
	}

	void SetHotKey(UINT vk, UINT vkCom)
	{
		SendMsg(HKM_SETHOTKEY, MAKEWORD(vk, vkCom), 0);
	}

	/// <summary>
	/// ����Ч����
	/// </summary>
	/// <param name="uInvalid">��Ч��ϼ���־��HKCOMB_����</param>
	/// <param name="uReplace">������Чʱ�滻Ϊ������ϼ���HOTKEYF_����</param>
	void SetRules(UINT uInvalid, UINT uReplace)
	{
		SendMsg(HKM_SETRULES, uInvalid, uReplace);
	}
};
ECK_NAMESPACE_END