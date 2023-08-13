#include "CRefStr.h"

#include <functional>

ECK_NAMESPACE_BEGIN
int CRefStrW::DupString(PCWSTR pszSrc, int cchSrc)
{
	if (!pszSrc || !cchSrc)
	{
	NullStr:
		m_cchText = 0;
		if (m_pszText)
			*m_pszText = L'\0';
		return 0;
	}

	if (cchSrc < 0)
		cchSrc = (int)wcslen(pszSrc);
	if (!cchSrc)
		goto NullStr;
	if (m_cchCapacity < cchSrc + 1)
	{
		int cch = TAlloc::MakeCapacity(cchSrc + 1);
		PWSTR pNew;
		if (m_pszText)
		{
			pNew = TAlloc::ReAlloc(m_pszText, cch);
			if (!pNew)
				return 0;
		}
		else
		{
			pNew = TAlloc::Alloc(cch);
			if (!pNew)
				return 0;
		}
		m_cchCapacity = cch;
		m_pszText = pNew;
	}
	m_cchText = cchSrc;
	wcsncpy(m_pszText, pszSrc, cchSrc);
	*(m_pszText + cchSrc) = L'\0';
	return cchSrc;
}

PWSTR CRefStrW::Attach(PWSTR psz, int cchCapacity, int cchText)
{
	auto pTemp = m_pszText;
	if (!psz)
	{
		m_cchCapacity = 0;
		m_cchText = 0;
		m_pszText = NULL;
		return pTemp;
	}
	m_cchCapacity = cchCapacity;
	if (cchText < 0)
		m_cchText = (int)wcslen(psz);
	else
		m_cchText = cchText;
	m_pszText = psz;
	return pTemp;
}

int CRefStrW::PushBack(PCWSTR pszSrc, int cchSrc)
{
	if (!pszSrc || !cchSrc)
		return 0;
	if (cchSrc < 0)
		cchSrc = (int)wcslen(pszSrc);
	if (!cchSrc)
		return 0;
	if (m_cchCapacity < m_cchText + cchSrc + 1)
		Reserve(TAlloc::MakeCapacity(m_cchText + cchSrc + 1));

	wcsncpy(m_pszText + m_cchText, pszSrc, cchSrc);
	m_cchText += cchSrc;
	*(m_pszText + m_cchText) = L'\0';
	return cchSrc;
}

void CRefStrW::Reserve(int cch)
{
	if (m_cchCapacity >= cch + 1)
		return;

	if (m_pszText)
	{
		auto pTemp = TAlloc::ReAlloc(m_pszText, cch + 1);
		if (pTemp)
		{
			m_cchCapacity = cch + 1;
			m_pszText = pTemp;
		}
	}
	else
	{
		m_pszText = TAlloc::Alloc(cch + 1);
		if (m_pszText)
			m_cchCapacity = cch + 1;
	}
}

void CRefStrW::Replace(int posStart, int cchReplacing, PCWSTR pszNew, int cchNew)
{
	if (cchNew < 0)
		cchNew = (int)wcslen(pszNew);

	int cchOrg = m_cchText;
	ReSize(m_cchText + cchNew - cchReplacing);

	memmove(m_pszText + posStart + cchNew, m_pszText + posStart + cchReplacing, (cchOrg - posStart - cchReplacing) * sizeof(WCHAR));
	if (pszNew)
		memcpy(m_pszText + posStart, pszNew, cchNew * sizeof(WCHAR));

	*(m_pszText + m_cchText) = L'\0';
}

void CRefStrW::ReplaceSubStr(PCWSTR pszReplaced, int cchReplaced, PCWSTR pszSrc, int cchSrc, int posStart, int cReplacing)
{
	if (cchReplaced < 0)
		cchReplaced = (int)wcslen(pszReplaced);
	if (cchSrc < 0)
		cchSrc = (int)wcslen(pszSrc);
	int pos = 0;
	for (int c = 1;; ++c)
	{
		pos = FindStr(m_pszText, pszReplaced, posStart + pos);
		if (pos == INVALID_STR_POS)
			break;
		Replace(pos, cchReplaced, pszSrc, cchSrc);
		pos += cchSrc;
		if (c == cReplacing)
			break;
	}
}

void CRefStrW::MakeRepeatedStrSeq(PCWSTR pszText, int cchText, int cCount, int posStart)
{
	if (cchText < 0)
		cchText = (int)wcslen(pszText);

	ReSize(posStart + cchText * cCount);
	PWSTR pszCurr = m_pszText;
	int i = 0;
	for (; i < cCount, pszCurr += cchText; ++i)
		memcpy(pszCurr, pszText, cchText * sizeof(WCHAR));
	*(m_pszText + m_cchText) = L'\0';
}




CRefStrW ToStr(int x, int iRadix)
{
	CRefStrW rs;
	rs.ReSize(24);
	*rs = L'\0';
	_itow_s(x, rs, (SIZE_T)rs.m_cchText, iRadix);
	rs.ReCalcLen();
	return rs;
}

CRefStrW ToStr(UINT x, int iRadix)
{
	CRefStrW rs;
	rs.ReSize(24);
	*rs = L'\0';
	_ultow_s(x, rs, (SIZE_T)rs.m_cchText, iRadix);
	rs.ReCalcLen();
	return rs;
}

CRefStrW ToStr(LONGLONG x, int iRadix)
{
	CRefStrW rs;
	rs.ReSize(24);
	*rs = L'\0';
	_i64tow_s(x, rs, (SIZE_T)rs.m_cchText, iRadix);
	rs.ReCalcLen();
	return rs;
}

CRefStrW ToStr(ULONGLONG x, int iRadix)
{
	CRefStrW rs;
	rs.ReSize(24);
	*rs = L'\0';
	_ui64tow_s(x, rs, (SIZE_T)rs.m_cchText, iRadix);
	rs.ReCalcLen();
	return rs;
}

CRefStrW ToStr(double x, int iPrecision)
{
	CRefStrW rs;
	rs.ReSize(24);
	*rs = L'\0';
	_snwprintf_s(rs, rs.m_cchText, rs.m_cchText, L"%*f", iPrecision, x);
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

template<class TProcesser>
void SplitTextInt(PCWSTR pszText, PCWSTR pszDiv, int cSubTextExpected, int cchText, int cchDiv, TProcesser Processer)
{
	if (cchText < 0)
		cchText = (int)wcslen(pszText);
	if (cchDiv < 0)
		cchDiv = (int)wcslen(pszDiv);
	if (cSubTextExpected <= 0)
		cSubTextExpected = INT_MAX;

	PCWSTR pszFind = wcsstr(pszText, pszDiv);
	PCWSTR pszPrevFirst = pszText;
	int c = 0;
	while (pszFind)
	{
		Processer(pszPrevFirst, (int)(pszFind - pszPrevFirst));
		++c;
		if (c == cSubTextExpected)
			return;
		pszPrevFirst = pszFind + cchDiv;
		pszFind = wcsstr(pszPrevFirst, pszDiv);
	}

	Processer(pszPrevFirst, (int)(pszText + cchText - pszPrevFirst));
}

void SplitStr(PCWSTR pszText, PCWSTR pszDiv, std::vector<CRefStrW>& aResult, int cSubTextExpected, int cchText, int cchDiv)
{
	SplitTextInt(pszText, pszDiv, cSubTextExpected, cchText, cchDiv,
		[&](PCWSTR pszStart, int cchSub) {
			aResult.push_back(CRefStrW(pszStart, cchSub));
		});
}

void SplitStr(PCWSTR pszText, PCWSTR pszDiv, std::vector<SPLTEXTINFO>& aResult, int cSubTextExpected, int cchText, int cchDiv)
{
	SplitTextInt(pszText, pszDiv, cSubTextExpected, cchText, cchDiv,
		[&](PCWSTR pszStart, int cchSub) {
			aResult.push_back({ pszStart,cchSub });
		});
}

void SplitStr(PWSTR pszText, PCWSTR pszDiv, std::vector<PWSTR>& aResult, int cSubTextExpected, int cchText, int cchDiv)
{
	SplitTextInt(pszText, pszDiv, cSubTextExpected, cchText, cchDiv,
		[&](PCWSTR pszStart, int cchSub) {
			PWSTR psz = (PWSTR)pszStart;
			*(psz + cchSub) = L'\0';
			aResult.push_back(psz);
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
		if (ch == L' ' || ch == L'��')
			--pszTemp;
		else
			break;
	}
	return (int)(pszText + cchText - pszTemp);
}

EckInline int PrivFindSpace(PCWSTR pszText)
{
	PCWSTR pszOrg = pszText;
	WCHAR ch = *pszText;
	while (ch != L' ' && ch != L'��')
		ch = *++pszText;
	return (int)(pszText - pszOrg);
}

CRefStrW AllTrimStr(PCWSTR pszText, int cchText)
{
	PCWSTR pszOrg = pszText;
	CRefStrW rs;
	if (cchText > 0)
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