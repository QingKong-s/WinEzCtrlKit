/*
* WinEzCtrlKit Library
*
* CFontPicker.h ： 字体选择器
*
* Copyright(C) 2024 QingKong
*/
#pragma once
#include "CEditNcComp.h"

ECK_NAMESPACE_BEGIN
class CFontPicker :public CEditNcComp
{
private:
	DWORD m_dwCfFlags = CF_SCREENFONTS | CF_FORCEFONTEXIST | CF_NOVERTFONTS;
public:
	BOOL ToLogFont(LOGFONTW& lf)
	{
		const auto rs = GetText();
		if (rs.IsEmpty())
			return FALSE;
		int iPt{ 9 }, iWeight{ 400 };
		if (swscanf_s(rs.Data(), L"%31s,%d,%d", &lf.lfFaceName, LF_FACESIZE, &iPt, &iWeight) < 1)
			return FALSE;

		const HDC hDC = GetDC(HWnd);
		lf.lfHeight = -MulDiv(iPt, GetDeviceCaps(hDC, LOGPIXELSY), 72);
		ReleaseDC(HWnd, hDC);

		lf.lfWeight = iWeight;
		return TRUE;
	}

	void OnBtnClick()
	{
		LOGFONTW lf{};
		const BOOL bHasInfo = ToLogFont(lf);

		CHOOSEFONTW cf{ sizeof(CHOOSEFONTW) };
		cf.hwndOwner = HWnd;
		cf.lpLogFont = &lf;
		cf.Flags = m_dwCfFlags | (bHasInfo ? CF_INITTOLOGFONTSTRUCT : 0u);
	}
};
ECK_NAMESPACE_END