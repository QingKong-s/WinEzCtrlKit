/*
* WinEzCtrlKit Library
*
* CRefStr.h ： 字符串
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

// 简单字符串，可以对该类持有的文本执行除改变长度外的任何操作
class CRefStrW
{
public:
	using TAlloc = CAllocator<WCHAR, int>;

	PWSTR m_pszText = NULL;
	int m_cchText = 0;
	int m_cchCapacity = 0;

	CRefStrW() = default;

	/// <summary>
	/// 创建自长度
	/// </summary>
	/// <param name="cchInit">字符串长度</param>
	explicit CRefStrW(int cchInit)
	{
		int cchCapacity = TAlloc::MakeCapacity(cchInit + 1);
		m_pszText = TAlloc::Alloc(cchCapacity);
		if (m_pszText)
			m_cchCapacity = cchCapacity;
	}

	/// <summary>
	/// 创建自字符串
	/// </summary>
	/// <param name="psz">字符串指针</param>
	/// <param name="cchText">字符串长度</param>
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
	/// 克隆字符串。
	/// 将指定字符串复制到自身
	/// </summary>
	/// <param name="pszSrc">字符串指针</param>
	/// <param name="cchSrc">字符串长度</param>
	/// <returns>实际复制的字符数</returns>
	int DupString(PCWSTR pszSrc, int cchSrc = -1);

	/// <summary>
	/// 依附指针。
	/// 分配器必须相同
	/// </summary>
	/// <param name="psz">指针</param>
	/// <param name="cchCapacity">容量</param>
	/// <param name="cchText">字符数</param>
	/// <returns>先前持有的指针</returns>
	PWSTR Attach(PWSTR psz, int cchCapacity, int cchText = -1);

	/// <summary>
	/// 拆离指针
	/// </summary>
	/// <returns>指针</returns>
	EckInline PWSTR Detach()
	{
		auto pTemp = m_pszText;
		Reset();
		return pTemp;
	}

	/// <summary>
	/// 清除。
	/// 持有的内存不会被释放
	/// </summary>
	/// <returns>持有的指针</returns>
	EckInline PWSTR Reset()
	{
		auto p = m_pszText;
		m_cchCapacity = 0;
		m_cchText = 0;
		m_pszText = NULL;
		return p;
	}

	/// <summary>
	/// 尾插
	/// </summary>
	/// <param name="pszSrc">字符串指针</param>
	/// <param name="cchSrc">字符串长度</param>
	/// <returns>实际复制的字符数</returns>
	int PushBack(PCWSTR pszSrc, int cchSrc = -1);

	/// <summary>
	/// 尾删
	/// </summary>
	/// <param name="cch">删除长度</param>
	EckInline void PopBack(int cch)
	{
		ReSize(m_cchText - cch);
		*(m_pszText + m_cchText) = L'\0';
	}

	/// <summary>
	/// 复制到
	/// </summary>
	/// <param name="pszDst">目的字符串指针</param>
	/// <param name="cch">字符数</param>
	/// <returns>实际复制的字符数</returns>
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
	/// 保留内存
	/// </summary>
	/// <param name="cch">字符数</param>
	void Reserve(int cch);

	/// <summary>
	/// 重置尺寸
	/// </summary>
	/// <param name="cch">字符数</param>
	EckInline void ReSize(int cch)
	{
		Reserve(TAlloc::MakeCapacity(cch + 1));
		m_cchText = cch;
		*(m_pszText + cch) = L'\0';
	}

	/// <summary>
	/// 重新计算字符串长度
	/// </summary>
	/// <returns>长度</returns>
	EckInline int ReCalcLen()
	{
		m_cchText = (int)wcslen(m_pszText);
		return m_cchText;
	}

	/// <summary>
	/// 替换
	/// </summary>
	/// <param name="posStart">替换位置</param>
	/// <param name="cchReplacing">替换长度</param>
	/// <param name="pszNew">用作替换的字符串指针</param>
	/// <param name="cchNew">用作替换的字符串长度</param>
	void Replace(int posStart, int cchReplacing, PCWSTR pszNew, int cchNew);

	/// <summary>
	/// 替换
	/// </summary>
	/// <param name="posStart">替换位置</param>
	/// <param name="cchReplacing">替换长度</param>
	/// <param name="rbNew">用作替换的字符串</param>
	EckInline void Replace(int posStart, int cchReplacing, const CRefStrW& rbNew)
	{
		Replace(posStart, cchReplacing, rbNew, rbNew.Size());
	}

	/// <summary>
	/// 子文本替换
	/// </summary>
	/// <param name="pszReplaced">被替换的字符串指针</param>
	/// <param name="cchReplaced">被替换的字符串长度</param>
	/// <param name="pszSrc">用作替换的字符串指针</param>
	/// <param name="cchSrc">用作替换的字符串长度</param>
	/// <param name="posStart">起始位置</param>
	/// <param name="cReplacing">替换进行的次数，0为执行所有替换</param>
	void ReplaceSubStr(PCWSTR pszReplaced, int cchReplaced, PCWSTR pszSrc, int cchSrc, int posStart = 0, int cReplacing = 0);

	/// <summary>
	/// 子文本替换
	/// </summary>
	/// <param name="rbReplaced">被替换的字符串</param>
	/// <param name="rbSrc">用作替换的字符串</param>
	/// <param name="posStart">起始位置</param>
	/// <param name="cReplacing">替换进行的次数，0为执行所有替换</param>
	EckInline void ReplaceSubStr(const CRefStrW& rbReplaced, const CRefStrW& rbSrc, int posStart = 0, int cReplacing = 0)
	{
		ReplaceSubStr(rbReplaced, rbReplaced.Size(), rbSrc, rbSrc.Size(), posStart, cReplacing);
	}

	/// <summary>
	/// 取空白文本
	/// </summary>
	/// <param name="cch">长度</param>
	/// <param name="posStart">起始位置</param>
	EckInline void MakeEmpty(int cch, int posStart)
	{
		ReSize(posStart + cch);
		for (PWSTR psz = m_pszText + posStart; psz < m_pszText + m_cchText; ++psz)
			*psz = L' ';
	}

	/// <summary>
	/// 取重复文本
	/// </summary>
	/// <param name="pszText">字符串指针</param>
	/// <param name="cchText">字符串长度</param>
	/// <param name="cCount">重复次数</param>
	/// <param name="posStart">起始位置</param>
	void MakeRepeatedStrSequence(PCWSTR pszText, int cchText, int cCount, int posStart = 0);

	/// <summary>
	/// 取重复文本
	/// </summary>
	/// <param name="pszText">字符串</param>
	/// <param name="cCount">重复次数</param>
	/// <param name="posStart">起始位置</param>
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
/// 到小写
/// </summary>
/// <param name="pszText">字符串指针</param>
/// <param name="cchText">字符串长度</param>
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
/// 到大写
/// </summary>
/// <param name="pszText">字符串指针</param>
/// <param name="cchText">字符串长度</param>
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
/// 取文本左边
/// </summary>
/// <param name="pszText">字符串指针</param>
/// <param name="cchLeft">左边长度</param>
/// <returns></returns>
EckInline CRefStrW StrLeft(PCWSTR pszText, int cchLeft)
{
	return CRefStrW(pszText, cchLeft);
}

/// <summary>
/// 取文本右边
/// </summary>
/// <param name="pszText">字符串指针</param>
/// <param name="cchRight">右边长度</param>
/// <param name="cchText">字符串长度</param>
/// <returns></returns>
EckInline CRefStrW StrRight(PCWSTR pszText, int cchRight, int cchText = -1)
{
	if (cchText < 0)
		cchText = (int)wcslen(pszText);
	return CRefStrW(pszText + cchText - cchRight, cchRight);
}

/// <summary>
/// 取文本中间
/// </summary>
/// <param name="pszText">字符串指针</param>
/// <param name="posStart">起始位置</param>
/// <param name="cchMid">中间长度</param>
/// <returns></returns>
EckInline CRefStrW StrMid(PCWSTR pszText, int posStart, int cchMid)
{
	return CRefStrW(pszText + posStart, cchMid);
}

/// <summary>
/// 寻找文本
/// </summary>
/// <param name="pszText">要在其中寻找的字符串指针</param>
/// <param name="pszSub">要寻找的字符串指针</param>
/// <param name="posStart">起始位置</param>
/// <returns>位置，若未找到返回INVALID_STR_POS</returns>
EckInline int FindStr(PCWSTR pszText, PCWSTR pszSub, int posStart = 0)
{
	PCWSTR pszFind = wcsstr(pszText + posStart, pszSub);
	if (pszFind)
		return (int)(pszFind - pszText);
	else
		return 0;
}

/// <summary>
/// 寻找文本。
/// 不区分大小写。
/// 函数先将整个字符串转换为大写，然后对其调用FindStr
/// </summary>
/// <param name="pszText">要在其中寻找的字符串指针</param>
/// <param name="cchText">要在其中寻找的字符串长度</param>
/// <param name="pszSub">要寻找的字符串指针</param>
/// <param name="cchSub">要寻找的字符串长度</param>
/// <param name="posStart">起始位置</param>
/// <returns>位置，若未找到返回INVALID_STR_POS</returns>
EckInline int FindStrNcs(PCWSTR pszText, int cchText, PCWSTR pszSub, int cchSub, int posStart = 0)
{
	return FindStr(ToUpperCase(pszText, cchText), ToUpperCase(pszSub, cchSub), posStart);
}

/// <summary>
/// 寻找文本。
/// 不区分大小写。
/// 函数先将整个字符串转换为大写，然后对其调用FindStr
/// </summary>
/// <param name="rbText">要在其中寻找的字符串</param>
/// <param name="rbSub">要寻找的字符串</param>
/// <param name="posStart">起始位置</param>
/// <returns>位置，若未找到返回INVALID_STR_POS</returns>
EckInline int FindStrNcs(const CRefStrW& rbText, const CRefStrW& rbSub, int posStart = 0)
{
	return FindStrNcs(rbText, rbText.Size(), rbSub, posStart, rbSub.Size());
}

/// <summary>
/// 倒找文本
/// </summary>
/// <param name="pszText">要在其中寻找的字符串指针</param>
/// <param name="cchText">要在其中寻找的字符串长度</param>
/// <param name="pszSub">要寻找的字符串指针</param>
/// <param name="cchSub">要寻找的字符串长度</param>
/// <param name="posStart">起始位置</param>
/// <returns>位置，若未找到返回INVALID_STR_POS</returns>
int FindStrRev(PCWSTR pszText, int cchText, PCWSTR pszSub, int cchSub, int posStart = 0);

/// <summary>
/// 倒找文本
/// </summary>
/// <param name="rbText">要在其中寻找的字符串</param>
/// <param name="rbSub">要寻找的字符串</param>
/// <param name="posStart">起始位置</param>
/// <returns>位置，若未找到返回INVALID_STR_POS</returns>
EckInline int FindStrRev(const CRefStrW& rbText, const CRefStrW& rbSub, int posStart = 0)
{
	return FindStrRev(rbText, rbText.Size(), rbSub, posStart,  rbSub.Size());
}

/// <summary>
/// 倒找文本。
/// 不区分大小写。
/// 函数先将整个字符串转换为大写，然后对其调用FindStrRev
/// </summary>
/// <param name="pszText">要在其中寻找的字符串指针</param>
/// <param name="cchText">要在其中寻找的字符串长度</param>
/// <param name="pszSub">要寻找的字符串指针</param>
/// <param name="cchSub">要寻找的字符串长度</param>
/// <param name="posStart">起始位置</param>
/// <returns>位置，若未找到返回INVALID_STR_POS</returns>
EckInline int FindStrRevNcs(PCWSTR pszText, int cchText, PCWSTR pszSub, int cchSub, int posStart = 0)
{
	auto rbText = ToUpperCase(pszText, cchText), rbSub = ToUpperCase(pszSub, cchSub);
	return FindStrRev(rbText, rbText.Size(), rbSub, posStart,  rbSub.Size());
}

/// <summary>
/// 倒找文本。
/// 不区分大小写。
/// 函数先将整个字符串转换为大写，然后对其调用FindStrRev
/// </summary>
/// <param name="rbText">要在其中寻找的字符串</param>
/// <param name="rbSub">要寻找的字符串</param>
/// <param name="posStart">起始位置</param>
/// <returns>位置，若未找到返回INVALID_STR_POS</returns>
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
/// 分割文本。
/// 函数逐个克隆子文本并存入结果容器
/// </summary>
/// <param name="pszText">要分割的文本</param>
/// <param name="pszDiv">分隔符</param>
/// <param name="aResult">结果容器</param>
/// <param name="cSubTextExpected">返回的最大子文本数</param>
/// <param name="cchText">要分割的文本长度</param>
/// <param name="cchDiv">分隔符长度</param>
void SplitStr(PCWSTR pszText, PCWSTR pszDiv, std::vector<CRefStrW>& aResult, int cSubTextExpected = 0, int cchText = -1, int cchDiv = -1);

struct SPLTEXTINFO
{
	PCWSTR pszStart;
	int cchText;
};

/// <summary>
/// 分割文本。
/// 函数将每个子文本的位置信息存入结果容器，此函数不执行任何复制
/// </summary>
/// <param name="pszText">要分割的文本</param>
/// <param name="pszDiv">分隔符</param>
/// <param name="aResult">结果容器</param>
/// <param name="cSubTextExpected">返回的最大子文本数</param>
/// <param name="cchText">要分割的文本长度</param>
/// <param name="cchDiv">分隔符长度</param>
void SplitStr(PCWSTR pszText, PCWSTR pszDiv, std::vector<SPLTEXTINFO>& aResult, int cSubTextExpected = 0, int cchText = -1, int cchDiv = -1);

/// <summary>
/// 分割文本。
/// 函数将每个分隔符的第一个字符更改为L'\0'，同时将每个子文本的位置信息存入容器
/// </summary>
/// <param name="pszText">要分割的文本，必须可写</param>
/// <param name="pszDiv">分隔符</param>
/// <param name="aResult">结果容器</param>
/// <param name="cSubTextExpected">返回的最大子文本数</param>
/// <param name="cchText">要分割的文本长度</param>
/// <param name="cchDiv">分隔符长度</param>
void SplitStr(PWSTR pszText, PCWSTR pszDiv, std::vector<PWSTR>& aResult, int cSubTextExpected = 0, int cchText = -1, int cchDiv = -1);

/// <summary>
/// 到全角
/// </summary>
/// <param name="pszText">原始文本</param>
/// <param name="cchText">字符数</param>
/// <returns>转换结果</returns>
EckInline CRefStrW ToFullWidth(PCWSTR pszText, int cchText = -1)
{
	CRefStrW rs;
	int cchResult = LCMapStringEx(LOCALE_NAME_USER_DEFAULT, LCMAP_FULLWIDTH, pszText, cchText, NULL, 0, NULL, NULL, 0);
	rs.ReSize(cchResult);
	LCMapStringEx(LOCALE_NAME_USER_DEFAULT, LCMAP_FULLWIDTH, pszText, cchText, rs, cchResult, NULL, NULL, 0);
	return rs;
}

/// <summary>
/// 到半角
/// </summary>
/// <param name="pszText">原始文本</param>
/// <param name="cchText">字符数</param>
/// <returns>转换结果</returns>
EckInline CRefStrW ToHalfWidth(PCWSTR pszText, int cchText = -1)
{
	CRefStrW rs;
	int cchResult = LCMapStringEx(LOCALE_NAME_USER_DEFAULT, LCMAP_HALFWIDTH, pszText, cchText, NULL, 0, NULL, NULL, 0);
	rs.ReSize(cchResult);
	LCMapStringEx(LOCALE_NAME_USER_DEFAULT, LCMAP_HALFWIDTH, pszText, cchText, rs, cchResult, NULL, NULL, 0);
	return rs;
}

/// <summary>
/// 删首空。
/// 函数从pszText开始向后步进到第一个非空格字符，然后返回指向这个字符的指针。
/// 此函数不执行任何修改字符串的操作
/// </summary>
/// <param name="pszText">原始文本</param>
/// <returns>第一个非空格字符的指针</returns>
EckInline PCWSTR LTrimStr(PCWSTR pszText)
{
	WCHAR ch = *pszText;
	while ((ch == L' ' || ch == L'　') && ch != L'\0')
		ch = *++pszText;
	return pszText;
}

/// <summary>
/// 删尾空。
/// 函数从pszText的尾部开始向前步进到第一个非空格字符，然后返回这个字符的位置。
/// 此函数不执行任何修改字符串的操作
/// </summary>
/// <param name="pszText">原始文本</param>
/// <param name="cchText">文本长度</param>
/// <returns>从字符串开头到最后一个非空格字符的长度</returns>
int RTrimStr(PCWSTR pszText, int cchText = -1);

/// <summary>
/// 删首尾空。
/// 函数内部简单地调用LTrimStr和RTrimStr获取首尾空信息。
/// 此函数不执行任何修改字符串的操作
/// </summary>
/// <param name="pszText">原始文本</param>
/// <param name="piEndPos">接收RTrimStr返回值</param>
/// <param name="cchText">文本长度</param>
/// <returns>LTrimStr返回值</returns>
EckInline PCWSTR RLTrimStr(PCWSTR pszText, int* piEndPos, int cchText = -1)
{
	auto pszLeft = LTrimStr(pszText);
	auto posRight = RTrimStr(pszText, cchText);
	*piEndPos = posRight;
	return pszLeft;
}

/// <summary>
/// 删全部空
/// </summary>
/// <param name="pszText">原始文本</param>
/// <param name="cchText">文本长度</param>
/// <returns>处理结果</returns>
CRefStrW AllTrimStr(PCWSTR pszText, int cchText = -1);
ECK_NAMESPACE_END