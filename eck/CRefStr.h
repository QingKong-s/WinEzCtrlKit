/*
* WinEzCtrlKit Library
*
* CRefStr.h �� �ַ���
*
* Copyright(C) 2023 QingKong
*/
#pragma once
#pragma warning (disable:4996)
#include "ECK.h"
#include "CAllocator.h"

#include <vector>
#include <string>
#include <algorithm>

ECK_NAMESPACE_BEGIN
static constexpr int INVALID_STR_POS = -1;

// ���ַ��������ԶԸ�����е��ı�ִ�г��ı䳤������κβ���
class CRefStrW
{
public:
	using TAlloc = CAllocator<WCHAR, int>;

	PWSTR m_pszText = NULL;
	int m_cchText = 0;
	int m_cchCapacity = 0;

	CRefStrW() = default;

	/// <summary>
	/// �����Գ���
	/// </summary>
	/// <param name="cchInit">�ַ�������</param>
	explicit CRefStrW(int cchInit)
	{
		int cchCapacity = TAlloc::MakeCapacity(cchInit + 1);
		m_pszText = TAlloc::Alloc(cchCapacity);
		if (m_pszText)
			m_cchCapacity = cchCapacity;
	}

	/// <summary>
	/// �������ַ���
	/// </summary>
	/// <param name="psz">�ַ���ָ��</param>
	/// <param name="cchText">�ַ�������</param>
	CRefStrW(PCWSTR psz, int cchText = -1)
	{
		if (!psz || !cchText)
			return;
		if (cchText < 0)
			cchText = (int)wcslen(psz);
		if (!cchText)
			return;

		int cchCapacity = TAlloc::MakeCapacity(cchText + 1);
		m_pszText = TAlloc::Alloc(cchCapacity);
		if (m_pszText)
		{
			m_cchText = cchText;
			m_cchCapacity = cchCapacity;
			wcscpy(m_pszText, psz);
		}
	}

	CRefStrW(const CRefStrW& x)
	{
		m_cchText = x.Size();
		m_cchCapacity = TAlloc::MakeCapacity(m_cchText + 1);
		m_pszText = TAlloc::Alloc(m_cchCapacity);
		if (x.Data())
			wcscpy(m_pszText, x.Data());
	}

	CRefStrW(CRefStrW&& x) noexcept
	{
		m_cchText = x.Size();
		m_cchCapacity = x.m_cchCapacity;
		m_pszText = x.Data();
		x.Reset();
	}

	~CRefStrW()
	{
		TAlloc::Free(m_pszText);
	}

	EckInline CRefStrW& operator=(PCWSTR pszSrc)
	{
		DupString(pszSrc);
		return *this;
	}

	EckInline CRefStrW& operator=(const CRefStrW& x)
	{
		DupString(x.Data(), x.Size());
		return *this;
	}

	EckInline CRefStrW& operator=(CRefStrW&& x) noexcept
	{
		TAlloc::Free(m_pszText);
		m_cchText = x.Size();
		m_cchCapacity = x.m_cchCapacity;
		m_pszText = x.Data();
		x.Reset();
		return *this;
	}

	EckInline BOOL operator==(PCWSTR psz)
	{
		if (!m_pszText && !psz)
			return TRUE;
		else if (!m_pszText || !psz)
			return FALSE;
		else
			return wcscmp(m_pszText, psz) == 0;
	}

	EckInline CRefStrW& operator+(const CRefStrW& x)
	{
		PushBack(x, x.Size());
		return *this;
	}

	EckInline CRefStrW& operator+(PCWSTR x)
	{
		PushBack(x);
		return *this;
	}

	WCHAR& operator[](int x)
	{
		return *(m_pszText + x);
	}

	EckInline operator PWSTR() const
	{
		return m_pszText;
	}

	EckInline int Size() const
	{
		return m_cchText;
	}

	EckInline SIZE_T ByteSize() const
	{
		if (m_pszText)
			return (m_cchText + 1) * sizeof(WCHAR);
		else
			return 0u;
	}

	EckInline PWSTR Data() const
	{
		return m_pszText;
	}

	/// <summary>
	/// ��¡�ַ�����
	/// ��ָ���ַ������Ƶ�����
	/// </summary>
	/// <param name="pszSrc">�ַ���ָ��</param>
	/// <param name="cchSrc">�ַ�������</param>
	/// <returns>ʵ�ʸ��Ƶ��ַ���</returns>
	int DupString(PCWSTR pszSrc, int cchSrc = -1);

	/// <summary>
	/// ����ָ�롣
	/// ������������ͬ
	/// </summary>
	/// <param name="psz">ָ��</param>
	/// <param name="cchCapacity">����</param>
	/// <param name="cchText">�ַ���</param>
	/// <returns>��ǰ���е�ָ��</returns>
	PWSTR Attach(PWSTR psz, int cchCapacity, int cchText = -1);

	/// <summary>
	/// ����ָ��
	/// </summary>
	/// <returns>ָ��</returns>
	EckInline PWSTR Detach()
	{
		auto pTemp = m_pszText;
		Reset();
		return pTemp;
	}

	/// <summary>
	/// �����
	/// ���е��ڴ治�ᱻ�ͷ�
	/// </summary>
	/// <returns>���е�ָ��</returns>
	EckInline PWSTR Reset()
	{
		auto p = m_pszText;
		m_cchCapacity = 0;
		m_cchText = 0;
		m_pszText = NULL;
		return p;
	}

	/// <summary>
	/// β��
	/// </summary>
	/// <param name="pszSrc">�ַ���ָ��</param>
	/// <param name="cchSrc">�ַ�������</param>
	/// <returns>ʵ�ʸ��Ƶ��ַ���</returns>
	int PushBack(PCWSTR pszSrc, int cchSrc = -1);

	/// <summary>
	/// βɾ
	/// </summary>
	/// <param name="cch">ɾ������</param>
	EckInline void PopBack(int cch)
	{
		ReSize(m_cchText - cch);
		*(m_pszText + m_cchText) = L'\0';
	}

	/// <summary>
	/// ���Ƶ�
	/// </summary>
	/// <param name="pszDst">Ŀ���ַ���ָ��</param>
	/// <param name="cch">�ַ���</param>
	/// <returns>ʵ�ʸ��Ƶ��ַ���</returns>
	EckInline int CopyTo(PWSTR pszDst, int cch = -1) const
	{
		if (cch < 0 || cch > m_cchText)
			cch = m_cchText;
		if (!cch || !m_pszText || !pszDst)
			return 0;
		wcsncpy(pszDst, m_pszText, cch);
		*(pszDst + cch) = L'\0';
		return cch;
	}

	/// <summary>
	/// �����ڴ�
	/// </summary>
	/// <param name="cch">�ַ���</param>
	void Reserve(int cch);

	/// <summary>
	/// ���óߴ�
	/// </summary>
	/// <param name="cch">�ַ���</param>
	EckInline void ReSize(int cch)
	{
		Reserve(TAlloc::MakeCapacity(cch + 1));
		m_cchText = cch;
		*(m_pszText + cch) = L'\0';
	}

	/// <summary>
	/// ���¼����ַ�������
	/// </summary>
	/// <returns>����</returns>
	EckInline int ReCalcLen()
	{
		m_cchText = (int)wcslen(m_pszText);
		return m_cchText;
	}

	/// <summary>
	/// �滻
	/// </summary>
	/// <param name="posStart">�滻λ��</param>
	/// <param name="cchReplacing">�滻����</param>
	/// <param name="pszNew">�����滻���ַ���ָ��</param>
	/// <param name="cchNew">�����滻���ַ�������</param>
	void Replace(int posStart, int cchReplacing, PCWSTR pszNew, int cchNew);

	/// <summary>
	/// �滻
	/// </summary>
	/// <param name="posStart">�滻λ��</param>
	/// <param name="cchReplacing">�滻����</param>
	/// <param name="rbNew">�����滻���ַ���</param>
	EckInline void Replace(int posStart, int cchReplacing, const CRefStrW& rbNew)
	{
		Replace(posStart, cchReplacing, rbNew, rbNew.Size());
	}

	/// <summary>
	/// ���ı��滻
	/// </summary>
	/// <param name="pszReplaced">���滻���ַ���ָ��</param>
	/// <param name="cchReplaced">���滻���ַ�������</param>
	/// <param name="pszSrc">�����滻���ַ���ָ��</param>
	/// <param name="cchSrc">�����滻���ַ�������</param>
	/// <param name="posStart">��ʼλ��</param>
	/// <param name="cReplacing">�滻���еĴ�����0Ϊִ�������滻</param>
	void ReplaceSubStr(PCWSTR pszReplaced, int cchReplaced, PCWSTR pszSrc, int cchSrc, int posStart = 0, int cReplacing = 0);

	/// <summary>
	/// ���ı��滻
	/// </summary>
	/// <param name="rbReplaced">���滻���ַ���</param>
	/// <param name="rbSrc">�����滻���ַ���</param>
	/// <param name="posStart">��ʼλ��</param>
	/// <param name="cReplacing">�滻���еĴ�����0Ϊִ�������滻</param>
	EckInline void ReplaceSubStr(const CRefStrW& rbReplaced, const CRefStrW& rbSrc, int posStart = 0, int cReplacing = 0)
	{
		ReplaceSubStr(rbReplaced, rbReplaced.Size(), rbSrc, rbSrc.Size(), posStart, cReplacing);
	}

	/// <summary>
	/// ȡ�հ��ı�
	/// </summary>
	/// <param name="cch">����</param>
	/// <param name="posStart">��ʼλ��</param>
	EckInline void MakeEmpty(int cch, int posStart)
	{
		ReSize(posStart + cch);
		for (PWSTR psz = m_pszText + posStart; psz < m_pszText + m_cchText; ++psz)
			*psz = L' ';
	}

	/// <summary>
	/// ȡ�ظ��ı�
	/// </summary>
	/// <param name="pszText">�ַ���ָ��</param>
	/// <param name="cchText">�ַ�������</param>
	/// <param name="cCount">�ظ�����</param>
	/// <param name="posStart">��ʼλ��</param>
	void MakeRepeatedStrSequence(PCWSTR pszText, int cchText, int cCount, int posStart = 0);

	/// <summary>
	/// ȡ�ظ��ı�
	/// </summary>
	/// <param name="pszText">�ַ���</param>
	/// <param name="cCount">�ظ�����</param>
	/// <param name="posStart">��ʼλ��</param>
	EckInline void MakeRepeatedStrSequence(const CRefStrW& rbText, int cCount, int posStart = 0)
	{
		MakeRepeatedStrSequence(rbText, rbText.Size(), cCount, posStart);
	}
};

CRefStrW ToStr(int x, int iRadix = 10);

CRefStrW ToStr(UINT x, int iRadix = 10);

CRefStrW ToStr(LONGLONG x, int iRadix = 10);

CRefStrW ToStr(ULONGLONG x, int iRadix = 10);

CRefStrW ToStr(double x, int iPrecision = 6);

namespace Literals
{
	EckInline CRefStrW operator""_rs(PCWSTR psz, size_t cch)
	{
		return CRefStrW(psz, (int)cch);
	}
}

/// <summary>
/// ��Сд
/// </summary>
/// <param name="pszText">�ַ���ָ��</param>
/// <param name="cchText">�ַ�������</param>
/// <returns></returns>
EckInline CRefStrW ToLowerCase(PCWSTR pszText, int cchText = -1)
{
	CRefStrW rs;
	int cchResult = LCMapStringEx(LOCALE_NAME_USER_DEFAULT, LCMAP_LOWERCASE, pszText, cchText, NULL, 0, NULL, NULL, 0);
	rs.ReSize(cchResult);
	LCMapStringEx(LOCALE_NAME_USER_DEFAULT, LCMAP_LOWERCASE, pszText, cchText, rs, cchResult, NULL, NULL, 0);
	return rs;
}

/// <summary>
/// ����д
/// </summary>
/// <param name="pszText">�ַ���ָ��</param>
/// <param name="cchText">�ַ�������</param>
/// <returns></returns>
EckInline CRefStrW ToUpperCase(PCWSTR pszText, int cchText = -1)
{
	CRefStrW rs;
	int cchResult = LCMapStringEx(LOCALE_NAME_USER_DEFAULT, LCMAP_UPPERCASE, pszText, cchText, NULL, 0, NULL, NULL, 0);
	rs.ReSize(cchResult);
	LCMapStringEx(LOCALE_NAME_USER_DEFAULT, LCMAP_UPPERCASE, pszText, cchText, rs, cchResult, NULL, NULL, 0);
	return rs;
}

/// <summary>
/// ȡ�ı����
/// </summary>
/// <param name="pszText">�ַ���ָ��</param>
/// <param name="cchLeft">��߳���</param>
/// <returns></returns>
EckInline CRefStrW StrLeft(PCWSTR pszText, int cchLeft)
{
	return CRefStrW(pszText, cchLeft);
}

/// <summary>
/// ȡ�ı��ұ�
/// </summary>
/// <param name="pszText">�ַ���ָ��</param>
/// <param name="cchRight">�ұ߳���</param>
/// <param name="cchText">�ַ�������</param>
/// <returns></returns>
EckInline CRefStrW StrRight(PCWSTR pszText, int cchRight, int cchText = -1)
{
	if (cchText < 0)
		cchText = (int)wcslen(pszText);
	return CRefStrW(pszText + cchText - cchRight, cchRight);
}

/// <summary>
/// ȡ�ı��м�
/// </summary>
/// <param name="pszText">�ַ���ָ��</param>
/// <param name="posStart">��ʼλ��</param>
/// <param name="cchMid">�м䳤��</param>
/// <returns></returns>
EckInline CRefStrW StrMid(PCWSTR pszText, int posStart, int cchMid)
{
	return CRefStrW(pszText + posStart, cchMid);
}

/// <summary>
/// Ѱ���ı�
/// </summary>
/// <param name="pszText">Ҫ������Ѱ�ҵ��ַ���ָ��</param>
/// <param name="pszSub">ҪѰ�ҵ��ַ���ָ��</param>
/// <param name="posStart">��ʼλ��</param>
/// <returns>λ�ã���δ�ҵ�����INVALID_STR_POS</returns>
EckInline int FindStr(PCWSTR pszText, PCWSTR pszSub, int posStart = 0)
{
	PCWSTR pszFind = wcsstr(pszText + posStart, pszSub);
	if (pszFind)
		return (int)(pszFind - pszText);
	else
		return 0;
}

/// <summary>
/// Ѱ���ı���
/// �����ִ�Сд��
/// �����Ƚ������ַ���ת��Ϊ��д��Ȼ��������FindStr
/// </summary>
/// <param name="pszText">Ҫ������Ѱ�ҵ��ַ���ָ��</param>
/// <param name="cchText">Ҫ������Ѱ�ҵ��ַ�������</param>
/// <param name="pszSub">ҪѰ�ҵ��ַ���ָ��</param>
/// <param name="cchSub">ҪѰ�ҵ��ַ�������</param>
/// <param name="posStart">��ʼλ��</param>
/// <returns>λ�ã���δ�ҵ�����INVALID_STR_POS</returns>
EckInline int FindStrNcs(PCWSTR pszText, int cchText, PCWSTR pszSub, int cchSub, int posStart = 0)
{
	return FindStr(ToUpperCase(pszText, cchText), ToUpperCase(pszSub, cchSub), posStart);
}

/// <summary>
/// Ѱ���ı���
/// �����ִ�Сд��
/// �����Ƚ������ַ���ת��Ϊ��д��Ȼ��������FindStr
/// </summary>
/// <param name="rbText">Ҫ������Ѱ�ҵ��ַ���</param>
/// <param name="rbSub">ҪѰ�ҵ��ַ���</param>
/// <param name="posStart">��ʼλ��</param>
/// <returns>λ�ã���δ�ҵ�����INVALID_STR_POS</returns>
EckInline int FindStrNcs(const CRefStrW& rbText, const CRefStrW& rbSub, int posStart = 0)
{
	return FindStrNcs(rbText, rbText.Size(), rbSub, posStart, rbSub.Size());
}

/// <summary>
/// �����ı�
/// </summary>
/// <param name="pszText">Ҫ������Ѱ�ҵ��ַ���ָ��</param>
/// <param name="cchText">Ҫ������Ѱ�ҵ��ַ�������</param>
/// <param name="pszSub">ҪѰ�ҵ��ַ���ָ��</param>
/// <param name="cchSub">ҪѰ�ҵ��ַ�������</param>
/// <param name="posStart">��ʼλ��</param>
/// <returns>λ�ã���δ�ҵ�����INVALID_STR_POS</returns>
int FindStrRev(PCWSTR pszText, int cchText, PCWSTR pszSub, int cchSub, int posStart = 0);

/// <summary>
/// �����ı�
/// </summary>
/// <param name="rbText">Ҫ������Ѱ�ҵ��ַ���</param>
/// <param name="rbSub">ҪѰ�ҵ��ַ���</param>
/// <param name="posStart">��ʼλ��</param>
/// <returns>λ�ã���δ�ҵ�����INVALID_STR_POS</returns>
EckInline int FindStrRev(const CRefStrW& rbText, const CRefStrW& rbSub, int posStart = 0)
{
	return FindStrRev(rbText, rbText.Size(), rbSub, posStart,  rbSub.Size());
}

/// <summary>
/// �����ı���
/// �����ִ�Сд��
/// �����Ƚ������ַ���ת��Ϊ��д��Ȼ��������FindStrRev
/// </summary>
/// <param name="pszText">Ҫ������Ѱ�ҵ��ַ���ָ��</param>
/// <param name="cchText">Ҫ������Ѱ�ҵ��ַ�������</param>
/// <param name="pszSub">ҪѰ�ҵ��ַ���ָ��</param>
/// <param name="cchSub">ҪѰ�ҵ��ַ�������</param>
/// <param name="posStart">��ʼλ��</param>
/// <returns>λ�ã���δ�ҵ�����INVALID_STR_POS</returns>
EckInline int FindStrRevNcs(PCWSTR pszText, int cchText, PCWSTR pszSub, int cchSub, int posStart = 0)
{
	auto rbText = ToUpperCase(pszText, cchText), rbSub = ToUpperCase(pszSub, cchSub);
	return FindStrRev(rbText, rbText.Size(), rbSub, posStart,  rbSub.Size());
}

/// <summary>
/// �����ı���
/// �����ִ�Сд��
/// �����Ƚ������ַ���ת��Ϊ��д��Ȼ��������FindStrRev
/// </summary>
/// <param name="rbText">Ҫ������Ѱ�ҵ��ַ���</param>
/// <param name="rbSub">ҪѰ�ҵ��ַ���</param>
/// <param name="posStart">��ʼλ��</param>
/// <returns>λ�ã���δ�ҵ�����INVALID_STR_POS</returns>
EckInline int FindStrRevNcs(const CRefStrW& rbText, const CRefStrW& rbSub, int posStart = 0)
{
	return FindStrRevNcs(rbText, rbText.Size(), rbSub, posStart, rbSub.Size());
}

template<class TProcesser>
void SplitStrInt(PCWSTR pszText, PCWSTR pszDiv, int cSubTextExpected, int cchText, int cchDiv, TProcesser Processer)
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

/// <summary>
/// �ָ��ı���
/// ���������¡���ı�������������
/// </summary>
/// <param name="pszText">Ҫ�ָ���ı�</param>
/// <param name="pszDiv">�ָ���</param>
/// <param name="aResult">�������</param>
/// <param name="cSubTextExpected">���ص�������ı���</param>
/// <param name="cchText">Ҫ�ָ���ı�����</param>
/// <param name="cchDiv">�ָ�������</param>
void SplitStr(PCWSTR pszText, PCWSTR pszDiv, std::vector<CRefStrW>& aResult, int cSubTextExpected = 0, int cchText = -1, int cchDiv = -1);

struct SPLTEXTINFO
{
	PCWSTR pszStart;
	int cchText;
};

/// <summary>
/// �ָ��ı���
/// ������ÿ�����ı���λ����Ϣ�������������˺�����ִ���κθ���
/// </summary>
/// <param name="pszText">Ҫ�ָ���ı�</param>
/// <param name="pszDiv">�ָ���</param>
/// <param name="aResult">�������</param>
/// <param name="cSubTextExpected">���ص�������ı���</param>
/// <param name="cchText">Ҫ�ָ���ı�����</param>
/// <param name="cchDiv">�ָ�������</param>
void SplitStr(PCWSTR pszText, PCWSTR pszDiv, std::vector<SPLTEXTINFO>& aResult, int cSubTextExpected = 0, int cchText = -1, int cchDiv = -1);

/// <summary>
/// �ָ��ı���
/// ������ÿ���ָ����ĵ�һ���ַ�����ΪL'\0'��ͬʱ��ÿ�����ı���λ����Ϣ��������
/// </summary>
/// <param name="pszText">Ҫ�ָ���ı��������д</param>
/// <param name="pszDiv">�ָ���</param>
/// <param name="aResult">�������</param>
/// <param name="cSubTextExpected">���ص�������ı���</param>
/// <param name="cchText">Ҫ�ָ���ı�����</param>
/// <param name="cchDiv">�ָ�������</param>
void SplitStr(PWSTR pszText, PCWSTR pszDiv, std::vector<PWSTR>& aResult, int cSubTextExpected = 0, int cchText = -1, int cchDiv = -1);

/// <summary>
/// ��ȫ��
/// </summary>
/// <param name="pszText">ԭʼ�ı�</param>
/// <param name="cchText">�ַ���</param>
/// <returns>ת�����</returns>
EckInline CRefStrW ToFullWidth(PCWSTR pszText, int cchText = -1)
{
	CRefStrW rs;
	int cchResult = LCMapStringEx(LOCALE_NAME_USER_DEFAULT, LCMAP_FULLWIDTH, pszText, cchText, NULL, 0, NULL, NULL, 0);
	rs.ReSize(cchResult);
	LCMapStringEx(LOCALE_NAME_USER_DEFAULT, LCMAP_FULLWIDTH, pszText, cchText, rs, cchResult, NULL, NULL, 0);
	return rs;
}

/// <summary>
/// �����
/// </summary>
/// <param name="pszText">ԭʼ�ı�</param>
/// <param name="cchText">�ַ���</param>
/// <returns>ת�����</returns>
EckInline CRefStrW ToHalfWidth(PCWSTR pszText, int cchText = -1)
{
	CRefStrW rs;
	int cchResult = LCMapStringEx(LOCALE_NAME_USER_DEFAULT, LCMAP_HALFWIDTH, pszText, cchText, NULL, 0, NULL, NULL, 0);
	rs.ReSize(cchResult);
	LCMapStringEx(LOCALE_NAME_USER_DEFAULT, LCMAP_HALFWIDTH, pszText, cchText, rs, cchResult, NULL, NULL, 0);
	return rs;
}

/// <summary>
/// ɾ�׿ա�
/// ������pszText��ʼ��󲽽�����һ���ǿո��ַ���Ȼ�󷵻�ָ������ַ���ָ�롣
/// �˺�����ִ���κ��޸��ַ����Ĳ���
/// </summary>
/// <param name="pszText">ԭʼ�ı�</param>
/// <returns>��һ���ǿո��ַ���ָ��</returns>
EckInline PCWSTR LTrimStr(PCWSTR pszText)
{
	WCHAR ch = *pszText;
	while ((ch == L' ' || ch == L'��') && ch != L'\0')
		ch = *++pszText;
	return pszText;
}

/// <summary>
/// ɾβ�ա�
/// ������pszText��β����ʼ��ǰ��������һ���ǿո��ַ���Ȼ�󷵻�����ַ���λ�á�
/// �˺�����ִ���κ��޸��ַ����Ĳ���
/// </summary>
/// <param name="pszText">ԭʼ�ı�</param>
/// <param name="cchText">�ı�����</param>
/// <returns>���ַ�����ͷ�����һ���ǿո��ַ��ĳ���</returns>
int RTrimStr(PCWSTR pszText, int cchText = -1);

/// <summary>
/// ɾ��β�ա�
/// �����ڲ��򵥵ص���LTrimStr��RTrimStr��ȡ��β����Ϣ��
/// �˺�����ִ���κ��޸��ַ����Ĳ���
/// </summary>
/// <param name="pszText">ԭʼ�ı�</param>
/// <param name="piEndPos">����RTrimStr����ֵ</param>
/// <param name="cchText">�ı�����</param>
/// <returns>LTrimStr����ֵ</returns>
EckInline PCWSTR RLTrimStr(PCWSTR pszText, int* piEndPos, int cchText = -1)
{
	auto pszLeft = LTrimStr(pszText);
	auto posRight = RTrimStr(pszText, cchText);
	*piEndPos = posRight;
	return pszLeft;
}

/// <summary>
/// ɾȫ����
/// </summary>
/// <param name="pszText">ԭʼ�ı�</param>
/// <param name="cchText">�ı�����</param>
/// <returns>������</returns>
CRefStrW AllTrimStr(PCWSTR pszText, int cchText = -1);
ECK_NAMESPACE_END