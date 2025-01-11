#pragma once
#include "CEditNcComp.h"
#include "CCommDlg.h"

ECK_NAMESPACE_BEGIN
struct FONTPICKERINFO
{
	PCWSTR pszFontName;
	int iPointSize;
	int iWeight;
};

class CFontPicker :public CEditNcComp
{
public:
	ECK_RTTI(CFontPicker);
private:
	DWORD m_dwCfFlags = CF_SCREENFONTS | CF_FORCEFONTEXIST | CF_NOVERTFONTS;

	static int FindFontName(const CRefStrW& rs)
	{
		if (rs.IsEmpty())
			return -1;
		int iPos = FindStrRev(rs.Data(), rs.Size(), L",", 1);
		if (iPos <= 1)
			return -1;// 找不到逗号
		iPos = FindStrRev(rs.Data(), rs.Size(), L",", 1, iPos - 1);
		if (iPos <= 1)
			return -1;// 找不到逗号
		return iPos;
	}
public:
	BOOL ToLogFont(LOGFONTW& lf)
	{
		const auto rs = GetText();
		const int iPos = FindFontName(rs);
		if (iPos < 0)
			return FALSE;

		wcsncpy_s(lf.lfFaceName, rs.Data(), iPos);

		int iPt{ 9 }, iWeight{ 400 };
		(void)swscanf(rs.Data() + iPos, L",%d,%d", &iPt, &iWeight);

		const HDC hDC = GetDC(HWnd);
		lf.lfHeight = -MulDiv(iPt, GetDeviceCaps(hDC, LOGPIXELSY), 72);
		ReleaseDC(HWnd, hDC);

		lf.lfWeight = iWeight;
		return TRUE;
	}

	BOOL FromInfo(const FONTPICKERINFO& fpi)
	{
		WCHAR szBuf[LF_FACESIZE + CchI32ToStrBufNoRadix2 * 2 + 5];
		swprintf_s(szBuf, L"%s,%d,%d", fpi.pszFontName, fpi.iPointSize, fpi.iWeight);
		SetText(szBuf);
		return TRUE;
	}

	CRefStrW ToInfo(FONTPICKERINFO& fpi)
	{
		fpi.pszFontName = nullptr;
		auto rs = GetText();
		const int iPos = FindFontName(rs);

		(void)swscanf(rs.Data() + iPos, L",%d,%d", &fpi.iPointSize, &fpi.iWeight);
		rs.ReSize(iPos);
		return rs;
	}

	void OnBtnClick()
	{
		LOGFONTW lf{};
		const BOOL bHasInfo = ToLogFont(lf);

		CHOOSEFONTW cf{ sizeof(CHOOSEFONTW) };
		cf.hwndOwner = HWnd;
		cf.lpLogFont = &lf;
		cf.Flags = m_dwCfFlags | (bHasInfo ? CF_INITTOLOGFONTSTRUCT : 0u);
		CFontDialog dlg{};
		if (dlg.DlgBox(nullptr,&cf))
		{
			WCHAR szBuf[LF_FACESIZE + CchI32ToStrBufNoRadix2 * 2 + 5];
			swprintf_s(szBuf, L"%s,%d,%d", lf.lfFaceName, cf.iPointSize / 10, lf.lfWeight);
			SetText(szBuf);
		}
	}
};
ECK_RTTI_IMPL_BASE_INLINE(CFontPicker, CEditNcComp);
ECK_NAMESPACE_END