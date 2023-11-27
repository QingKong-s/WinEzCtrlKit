#include "CRefStr.h"

ECK_NAMESPACE_BEGIN




CRefStrW ToStr(int x, int iRadix)
{
	CRefStrW rs;
	rs.ReSize(_MAX_ITOSTR_BASE10_COUNT);
	*rs.Data() = L'\0';
	_itow_s(x, rs.Data(), (SIZE_T)rs.Size(), iRadix);
	rs.ReCalcLen();
	return rs;
}

CRefStrW ToStr(UINT x, int iRadix)
{
	CRefStrW rs;
	rs.ReSize(24);
	*rs.Data() = L'\0';
	_ultow_s(x, rs.Data(), (SIZE_T)rs.Size(), iRadix);
	rs.ReCalcLen();
	return rs;
}

CRefStrW ToStr(LONGLONG x, int iRadix)
{
	CRefStrW rs;
	rs.ReSize(24);
	*rs.Data() = L'\0';
	_i64tow_s(x, rs.Data(), (SIZE_T)rs.Size(), iRadix);
	rs.ReCalcLen();
	return rs;
}

CRefStrW ToStr(ULONGLONG x, int iRadix)
{
	CRefStrW rs;
	rs.ReSize(24);
	*rs.Data() = L'\0';
	_ui64tow_s(x, rs.Data(), (SIZE_T)rs.Size(), iRadix);
	rs.ReCalcLen();
	return rs;
}

CRefStrW ToStr(double x, int iPrecision)
{
	CRefStrW rs;
	rs.ReSize(24);
	*rs.Data() = L'\0';
	_snwprintf_s(rs.Data(), rs.Size(), rs.Size(), L"%.*g", iPrecision, x);
	rs.ReCalcLen();
	return rs;
}

int FindStrRev(PCWSTR pszText, int posStart, PCWSTR pszSub, int cchText, int cchSub)
{
	if (cchText < 0)
		cchText = (int)wcslen(pszText);
	if (cchSub < 0)
		cchSub = (int)wcslen(pszSub);
	if (!cchText || !cchSub || cchText < cchSub)
		return INVALID_STR_POS;

	for (PCWSTR pCurr = pszText + cchText - posStart - cchSub; pCurr >= pszText; --pCurr)
	{
		if (wcsncmp(pCurr, pszSub, cchSub) == 0)
			return (int)(pCurr - pszText);
	}
	return INVALID_STR_POS;
}

void SplitStr(PCWSTR pszText, PCWSTR pszDiv, std::vector<CRefStrW>& aResult, int cSubTextExpected, int cchText, int cchDiv)
{
	SplitStr(pszText, pszDiv, cSubTextExpected, cchText, cchDiv,
		[&](PCWSTR pszStart, int cchSub) {
			aResult.push_back(CRefStrW(pszStart, cchSub));
			return FALSE;
		});
}

void SplitStr(PCWSTR pszText, PCWSTR pszDiv, std::vector<SPLTEXTINFO>& aResult, int cSubTextExpected, int cchText, int cchDiv)
{
	SplitStr(pszText, pszDiv, cSubTextExpected, cchText, cchDiv,
		[&](PCWSTR pszStart, int cchSub) {
			aResult.push_back({ pszStart,cchSub });
			return FALSE;
		});
}

void SplitStr(PWSTR pszText, PCWSTR pszDiv, std::vector<PWSTR>& aResult, int cSubTextExpected, int cchText, int cchDiv)
{
	SplitStr(pszText, pszDiv, cSubTextExpected, cchText, cchDiv,
		[&](PCWSTR pszStart, int cchSub) {
			PWSTR psz = (PWSTR)pszStart;
			*(psz + cchSub) = L'\0';
			aResult.push_back(psz);
			return FALSE;
		});
}

int RTrimStr(PCWSTR pszText, int cchText)
{
	if (cchText < 0)
		cchText = (int)wcslen(pszText);
	if (!cchText)
		return 0;

	PCWSTR pszTemp = pszText + cchText - 1;
	WCHAR ch;
	while (pszTemp != pszText)
	{
		ch = *pszTemp;
		if (ch == L' ' || ch == L'　')
			--pszTemp;
		else
			break;
	}
	return (int)(pszTemp - pszText);
}

EckInline int PrivFindSpace(PCWSTR pszText)
{
	PCWSTR pszOrg = pszText;
	WCHAR ch = *pszText;
	while (ch != L' ' && ch != L'　' && ch != L'\0')
		ch = *++pszText;
	return (int)(pszText - pszOrg);
}

CRefStrW AllTrimStr(PCWSTR pszText, int cchText)
{
	if (cchText < 0)
		cchText = (int)wcslen(pszText);
	if (cchText <= 0)
		return {};
	PCWSTR pszOrg = pszText;
	CRefStrW rs{};
	rs.Reserve(cchText);

	int posTemp;
	while ((int)(pszText - pszOrg) < cchText)
	{
		posTemp = PrivFindSpace(pszText);
		rs.PushBack(pszText, posTemp);

		posTemp += (int)(LTrimStr(pszText) - pszText);
		pszText += posTemp;
	}

	return rs;
}
ECK_NAMESPACE_END