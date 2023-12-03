/*
* WinEzCtrlKit Library
*
* CRefStrT.h ： 字符串
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
#include <execution>

ECK_NAMESPACE_BEGIN
static constexpr int INVALID_STR_POS = -1;

inline constexpr int StrNPos = -1;

template<class TChar>
struct CCharTraits
{};

template<>
struct CCharTraits<WCHAR>
{
	EckInline static int Len(PCWSTR psz) { return (int)wcslen(psz); }
	EckInline static PWSTR Copy(PWSTR psz1, PCWSTR psz2, int cch) { return (PWSTR)memcpy(psz1, psz2, cch * sizeof(WCHAR)); }
	EckInline static PWSTR CopyEnd(PWSTR psz1, PCWSTR psz2, int cch)
	{
		auto r = Copy(psz1, psz2, cch);
		*(psz1 + cch) = L'\0';
		return r;
	}
	EckInline static PWSTR Move(PWSTR psz1, PCWSTR psz2, int cch) { return (PWSTR)memmove(psz1, psz2, cch * sizeof(WCHAR)); }
	EckInline static PWSTR MoveEnd(PWSTR psz1, PCWSTR psz2, int cch)
	{
		auto r = Move(psz1, psz2, cch);
		*(psz1 + cch) = L'\0';
		return r;
	}
	EckInline static constexpr void AssignChar(PWSTR psz, WCHAR ch) { *psz = ch; }
	EckInline static constexpr WCHAR CharTerminatingNull() { return L'\0'; }
	EckInline static constexpr void Cut(PWSTR psz, int cch) { AssignChar(psz + cch, CharTerminatingNull()); }
	EckInline static constexpr WCHAR CharSpace() { return L' '; }
	EckInline static int Compare(PCWSTR psz1, PCWSTR psz2) { return wcscmp(psz1, psz2); }
};

template<>
struct CCharTraits<CHAR>
{
	EckInline static int Len(PCSTR psz) { return (int)strlen(psz); }
	EckInline static PSTR Copy(PSTR psz1, PCSTR psz2, int cch) { return (PSTR)memcpy(psz1, psz2, cch); }
	EckInline static PSTR CopyEnd(PSTR psz1, PCSTR psz2, int cch)
	{
		auto r = Copy(psz1, psz2, cch);
		*(psz1 + cch) = '\0';
		return r;
	}
	EckInline static PSTR Move(PSTR psz1, PCSTR psz2, int cch) { return (PSTR)memmove(psz1, psz2, cch * sizeof(CHAR)); }
	EckInline static PSTR MoveEnd(PSTR psz1, PCSTR psz2, int cch)
	{
		auto r = Move(psz1, psz2, cch);
		*(psz1 + cch) = '\0';
		return r;
	}
	EckInline static constexpr void AssignChar(PSTR psz, CHAR ch) { *psz = ch; }
	EckInline static constexpr CHAR CharTerminatingNull() { return '\0'; }
	EckInline static constexpr void Cut(PSTR psz, int cch) { AssignChar(psz + cch, CharTerminatingNull()); }
	EckInline static constexpr CHAR CharSpace() { return ' '; }
	EckInline static int Compare(PCSTR psz1, PCSTR psz2) { return strcmp(psz1, psz2); }
};

template<class TChar_, class TCharTraits_, class TAlloc_ = CAllocatorProcHeap<TChar_, int>>
class CRefStrT
{
public:
	using TChar = TChar_;
	using TCharTraits = TCharTraits_;
	using TAlloc = TAlloc_;

	static_assert(std::is_same_v<TChar, CHAR> || std::is_same_v<TChar, WCHAR>);

	using TAllocTraits = CAllocatorTraits<TAlloc>;
	using TPointer = TChar*;
	using TConstPointer = const TChar*;

	using TIterator = TPointer;
	using TConstIterator = TConstPointer;
	using TReverseIterator = std::reverse_iterator<TIterator>;
	using TConstReverseIterator = std::reverse_iterator<TConstIterator>;
private:
	TPointer m_pszText = NULL;
	int m_cchText = 0;
	int m_cchCapacity = 0;

	[[no_unique_address]] TAlloc m_Alloc{};
public:
	CRefStrT() = default;

	TAlloc& GetAllocator() { return m_Alloc; }

	/// <summary>
	/// 创建自长度
	/// </summary>
	/// <param name="cchInit">字符串长度</param>
	explicit CRefStrT(int cchInit)
	{
		m_cchCapacity = TAllocTraits::MakeCapacity(cchInit + 1);
		m_pszText = m_Alloc.allocate(m_cchCapacity);
		m_cchText = cchInit;
	}

	/// <summary>
	/// 创建自字符串
	/// </summary>
	/// <param name="psz">字符串指针</param>
	/// <param name="cchText">字符串长度</param>
	CRefStrT(TConstPointer psz, int cchText = -1)
	{
		if (!psz || !cchText)
			return;
		if (cchText < 0)
			cchText = TCharTraits::Len(psz);
		if (!cchText)
			return;

		m_cchCapacity = TAllocTraits::MakeCapacity(cchText + 1);
		m_pszText = m_Alloc.allocate(m_cchCapacity);
		m_cchText = cchText;
		TCharTraits::CopyEnd(m_pszText, psz, cchText);
	}

	CRefStrT(const CRefStrT& x)
	{
		m_Alloc = TAllocTraits::select_on_container_copy_construction(x.m_Alloc);
		m_cchText = x.Size();
		m_cchCapacity = TAllocTraits::MakeCapacity(m_cchText + 1);
		m_pszText = m_Alloc.allocate(m_cchCapacity);
		TCharTraits::CopyEnd(Data(), x.Data(), x.Size());
	}

	CRefStrT(CRefStrT&& x) noexcept
		:m_pszText{ x.m_pszText }, m_cchText{ x.m_cchText }, m_cchCapacity{ x.m_cchCapacity },
		m_Alloc{ std::move(x.m_Alloc) }
	{
		x.m_pszText = NULL;
		x.m_cchText = x.m_cchCapacity = 0;
	}

	template<class TTraits, class TAlloc1>
	CRefStrT(const std::basic_string<TChar, TTraits, TAlloc1>& x)
	{
		DupString(x.c_str(), (int)x.size());
	}

	~CRefStrT()
	{
		m_Alloc.deallocate(m_pszText, m_cchCapacity);
	}

	EckInline CRefStrT& operator=(TConstPointer pszSrc)
	{
		DupString(pszSrc);
		return *this;
	}

	EckInline CRefStrT& operator=(const CRefStrT& x)
	{
		if constexpr (!TAllocTraits::is_always_equal::value)
		{
			if (m_Alloc != x.m_Alloc)
			{
				m_Alloc = x.m_Alloc;
				m_Alloc.deallocate(m_pszText, m_cchCapacity);
				m_pszText = NULL;
				m_cchText = m_cchCapacity = 0;
			}
			else if constexpr (TAllocTraits::propagate_on_container_copy_assignment::value)
				m_Alloc = x.m_Alloc;
		}
		else if constexpr (TAllocTraits::propagate_on_container_copy_assignment::value)
			m_Alloc = x.m_Alloc;

		DupString(x.Data(), x.Size());
		return *this;
	}

	EckInline CRefStrT& operator=(CRefStrT&& x) noexcept
	{
		if (this == &x)
			return *this;
		if constexpr (TAllocTraits::propagate_on_container_move_assignment::value)
			m_Alloc = std::move(x.m_Alloc);
		else if constexpr (!TAllocTraits::is_always_equal::value)
			if (m_Alloc != x.m_Alloc)
			{
				DupString(x.Data(), x.Size());
				return *this;
			}
		
		std::swap(m_pszText, x.m_pszText);
		std::swap(m_cchText, x.m_cchText);
		std::swap(m_cchCapacity, x.m_cchCapacity);
		return *this;
	}

	template<class TTraits, class TAlloc1>
	EckInline CRefStrT& operator=(const std::basic_string<TChar, TTraits, TAlloc1>& x)
	{
		DupString(x.c_str(), (int)x.size());
		return *this;
	}

	EckInline TChar& operator[](int x)
	{
		return *(Data() + x);
	}

	EckInline TChar operator[](int x) const
	{
		return *(Data() + x);
	}

	EckInline CRefStrT& operator<<(const CRefStrT& x)
	{
		PushBack(x.Data(), x.Size());
		return *this;
	}

	template<class TTraits, class TAlloc1>
	EckInline CRefStrT& operator<<(const std::basic_string<TChar, TTraits, TAlloc1>& x)
	{
		PushBack(x.c_str(), (int)x.size());
		return *this;
	}

	EckInline int Size() const
	{
		return m_cchText;
	}

	EckInline size_t ByteSize() const
	{
		return (Data() ? ((m_cchText + 1) * sizeof(TChar)) : 0u);
	}

	EckInline TPointer Data()
	{
		return m_pszText;
	}

	EckInline TConstPointer Data() const
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
	int DupString(TConstPointer pszSrc, int cchSrc = -1)
	{
		if (!pszSrc)
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
		ReSize(cchSrc);
		TCharTraits::CopyEnd(Data(), pszSrc, cchSrc);
		return cchSrc;
	}

	/// <summary>
	/// 依附指针
	/// </summary>
	/// <param name="psz">指针，必须可通过当前分配器解分配</param>
	/// <param name="cchCapacity">容量</param>
	/// <param name="cchText">字符数</param>
	void Attach(TPointer psz, int cchCapacity, int cchText = -1)
	{
		m_Alloc.deallocate(m_pszText, m_cchCapacity);
		if (!psz)
		{
			m_cchCapacity = 0;
			m_cchText = 0;
			m_pszText = NULL;
		}
		else
		{
			m_cchCapacity = cchCapacity;
			if (cchText < 0)
				m_cchText = TCharTraits::Len(psz);
			else
				m_cchText = cchText;
			m_pszText = psz;
		}
	}

	/// <summary>
	/// 拆离指针
	/// </summary>
	/// <returns>指针，必须通过当前分配器解分配</returns>
	EckInline TPointer Detach()
	{
		auto pTemp = m_pszText;
		m_cchCapacity = 0;
		m_cchText = 0;
		m_pszText = NULL;
		return pTemp;
	}

	/// <summary>
	/// 尾插
	/// </summary>
	/// <param name="pszSrc">字符串指针</param>
	/// <param name="cchSrc">字符串长度</param>
	/// <returns>实际复制的字符数</returns>
	EckInline int PushBack(TConstPointer pszSrc, int cchSrc)
	{
		if (!pszSrc)
			return 0;
		const auto cNew = m_cchText + cchSrc + 1;
		Reserve(cNew + cNew / 2);
		TCharTraits::CopyEnd(Data() + Size(), pszSrc, cchSrc);
		return cchSrc;
	}

	EckInline int PushBack(TConstPointer pszSrc)
	{
		if (!pszSrc)
			return 0;
		return PushBack(pszSrc, TCharTraits::Len(pszSrc));
	}

	/// <summary>
	/// 尾删
	/// </summary>
	/// <param name="cch">删除长度</param>
	EckInline void PopBack(int cch)
	{
		EckAssert(Size() >= cch);
		m_cchText -= cch;
		TCharTraits::Cut(Data(), Size());
	}

	/// <summary>
	/// 复制到
	/// </summary>
	/// <param name="pszDst">目的字符串指针</param>
	/// <param name="cch">字符数</param>
	/// <returns>实际复制的字符数</returns>
	EckInline int CopyTo(TPointer pszDst, int cch = -1) const
	{
		if (cch < 0 || cch > m_cchText)
			cch = Size();
		TCharTraits::CopyEnd(pszDst, Data(), cch);
		return cch;
	}

	/// <summary>
	/// 保留内存
	/// </summary>
	/// <param name="cch">字符数</param>
	void Reserve(int cch)
	{
		if (m_cchCapacity >= cch + 1)
			return;

		const auto pOld = m_pszText;
		m_pszText = m_Alloc.allocate(cch + 1);
		if (pOld)
		{
			TCharTraits::Copy(m_pszText, pOld, m_cchText);
			m_Alloc.deallocate(pOld, m_cchCapacity);
		}
		else
			TCharTraits::Cut(Data(), 0);
		m_cchCapacity = cch + 1;
	}

	/// <summary>
	/// 重置尺寸
	/// </summary>
	/// <param name="cch">字符数</param>
	EckInline void ReSize(int cch)
	{
		Reserve(cch + 1);
		m_cchText = cch;
		TCharTraits::Cut(Data(), cch);
	}

	/// <summary>
	/// 重新计算字符串长度
	/// </summary>
	/// <returns>长度</returns>
	EckInline int ReCalcLen()
	{
		return m_cchText = TCharTraits::Len(Data());
	}

	/// <summary>
	/// 替换
	/// </summary>
	/// <param name="posStart">替换位置</param>
	/// <param name="cchReplacing">替换长度</param>
	/// <param name="pszNew">用作替换的字符串指针</param>
	/// <param name="cchNew">用作替换的字符串长度</param>
	void Replace(int posStart, int cchReplacing, PCWSTR pszNew, int cchNew)
	{
		EckAssert(pszNew ? TRUE : cchNew == 0);
		if (cchNew < 0)
			cchNew = TCharTraits::Len(pszNew);
		TCharTraits::Move(
			Data() + posStart + cchNew,
			Data() + posStart + cchReplacing,
			Size() - posStart - cchReplacing);
		if (pszNew)
			TCharTraits::Copy(Data() + posStart, pszNew, cchNew);
		ReSize(m_cchText + cchNew - cchReplacing);
	}

	/// <summary>
	/// 替换
	/// </summary>
	/// <param name="posStart">替换位置</param>
	/// <param name="cchReplacing">替换长度</param>
	/// <param name="rsNew">用作替换的字符串</param>
	EckInline void Replace(int posStart, int cchReplacing, const CRefStrT& rsNew)
	{
		Replace(posStart, cchReplacing, rsNew.Data(), rsNew.Size());
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
	void ReplaceSubStr(PCWSTR pszReplaced, int cchReplaced, PCWSTR pszSrc, int cchSrc,
		int posStart = 0, int cReplacing = 0)
	{
		EckAssert(pszReplaced);
		EckAssert(pszSrc ? TRUE : cchSrc == 0);
		if (cchReplaced < 0)
			cchReplaced = TCharTraits::Len(pszReplaced);
		if (cchSrc < 0)
			cchSrc = TCharTraits::Len(pszSrc);
		int pos = 0;
		for (int c = 1;; ++c)
		{
			pos = FindStr(Data(), pszReplaced, posStart + pos);
			if (pos == StrNPos)
				break;
			Replace(pos, cchReplaced, pszSrc, cchSrc);
			pos += cchSrc;
			if (c == cReplacing)
				break;
		}
	}

	/// <summary>
	/// 子文本替换
	/// </summary>
	/// <param name="rsReplaced">被替换的字符串</param>
	/// <param name="rsSrc">用作替换的字符串</param>
	/// <param name="posStart">起始位置</param>
	/// <param name="cReplacing">替换进行的次数，0为执行所有替换</param>
	EckInline void ReplaceSubStr(const CRefStrT& rsReplaced, const CRefStrT& rsSrc,
		int posStart = 0, int cReplacing = 0)
	{
		ReplaceSubStr(rsReplaced.Data(), rsReplaced.Size(), rsSrc.Data(), rsSrc.Size(), posStart, cReplacing);
	}

	/// <summary>
	/// 取空白文本
	/// </summary>
	/// <param name="cch">长度</param>
	/// <param name="posStart">起始位置</param>
	EckInline void MakeEmpty(int cch, int posStart = 0)
	{
		static constexpr TChar szSpace[2]{ TCharTraits::CharSpace(),TCharTraits::CharTerminatingNull() };
		MakeRepeatedStrSequence(szSpace, 1, cch, posStart);
	}

	/// <summary>
	/// 取重复文本
	/// </summary>
	/// <param name="pszText">字符串指针</param>
	/// <param name="cchText">字符串长度</param>
	/// <param name="cCount">重复次数</param>
	/// <param name="posStart">起始位置</param>
	void MakeRepeatedStrSequence(TConstPointer pszText, int cchText, int cCount, int posStart = 0)
	{
		EckAssert(pszText);
		if (cchText < 0)
			cchText = TCharTraits::Len(pszText);
		ReSize(posStart + cchText * cCount);
		TPointer pszCurr = Data() + posStart;
		for (int i = 0; i < cCount; ++i, pszCurr += cchText)
			TCharTraits::Copy(pszCurr, pszText, cchText);
		TCharTraits::Cut(Data(), Size());
	}

	/// <summary>
	/// 取重复文本
	/// </summary>
	/// <param name="rsText">字符串</param>
	/// <param name="cCount">重复次数</param>
	/// <param name="posStart">起始位置</param>
	EckInline void MakeRepeatedStrSequence(const CRefStrT& rsText, int cCount, int posStart = 0)
	{
		MakeRepeatedStrSequence(rsText.Data(), rsText.Size(), cCount, posStart);
	}

	EckInline void Clear()
	{
		ReSize(0);
	}

	EckInline TIterator begin() { return Data(); }
	EckInline TIterator end() { return begin() + Size(); }
	EckInline TConstIterator begin() const { return Data(); }
	EckInline TConstIterator end() const { return begin() + Size(); }
	EckInline TConstIterator cbegin() const { begin(); }
	EckInline TConstIterator cend() const { end(); }
	EckInline TReverseIterator rbegin() { return TReverseIterator(begin()); }
	EckInline TReverseIterator rend() { return TReverseIterator(end()); }
	EckInline TConstReverseIterator rbegin() const { return TConstReverseIterator(begin()); }
	EckInline TConstReverseIterator rend() const { return TConstReverseIterator(end()); }
	EckInline TConstReverseIterator crbegin() const { return rbegin(); }
	EckInline TConstReverseIterator crend() const { return rend(); }
};

using CRefStrW = CRefStrT<WCHAR, CCharTraits<WCHAR>>;
using CRefStrA = CRefStrT<CHAR, CCharTraits<CHAR>>;

#define EckCRefStrTemp CRefStrT<TChar, TCharTraits, TAlloc>

template<class TChar, class TCharTraits, class TAlloc = CAllocatorProcHeap<TChar, int>>
EckInline EckCRefStrTemp operator+(const EckCRefStrTemp& rs1, const EckCRefStrTemp& rs2)
{
	EckCRefStrTemp x(rs1.Size() + rs2.Size());
	TCharTraits::Copy(x.Data(), rs1.Data(), rs1.Size());
	TCharTraits::CopyEnd(x.Data() + rs1.Size(), rs2.Data(), rs2.Size());
	return x;
}

template<class TChar, class TCharTraits, class TAlloc = CAllocatorProcHeap<TChar, int>>
EckInline EckCRefStrTemp operator+(const EckCRefStrTemp& rs, PCWSTR psz)
{
	const int cch = (psz ? TCharTraits::Len(psz) : 0);
	EckCRefStrTemp x(rs.Size() + cch);
	TCharTraits::Copy(x.Data(), rs.Data(), rs.Size());
	TCharTraits::CopyEnd(x.Data() + rs.Size(), psz, cch);
	return x;
}

template<class TChar, class TCharTraits, class TAlloc = CAllocatorProcHeap<TChar, int>>
EckInline bool operator==(const EckCRefStrTemp& rs1, PCWSTR psz2)
{
	if (!rs1.Data() && !psz2)
		return true;
	else if (!rs1.Data() || !psz2)
		return false;
	else
		return TCharTraits::Compare(rs1.Data(), psz2) == 0;
}

template<class TChar, class TCharTraits, class TAlloc = CAllocatorProcHeap<TChar, int>>
EckInline std::weak_ordering operator<=>(const EckCRefStrTemp& rs1, PCWSTR psz2)
{
	if (!rs1.Data() && !psz2)
		return std::weak_ordering::equivalent;
	else if (!rs1.Data() && psz2)
		return std::weak_ordering::greater;
	else if (rs1.Data() && !psz2)
		return std::weak_ordering::less;
	else
		return TCharTraits::Compare(rs1.Data(), psz2) <=> 0;
}

template<class TChar, class TCharTraits, class TAlloc = CAllocatorProcHeap<TChar, int>>
EckInline bool operator==(const EckCRefStrTemp& rs1, const EckCRefStrTemp& rs2)
{
	return operator==(rs1, rs2.Data());
}

template<class TChar, class TCharTraits, class TAlloc = CAllocatorProcHeap<TChar, int>>
EckInline std::weak_ordering operator<=>(const EckCRefStrTemp& rs1, const EckCRefStrTemp& rs2)
{
	return operator<=>(rs1, rs2.Data());
}


inline constexpr auto c_cbI32ToStrBufNoRadix2 = std::max({
	_MAX_ITOSTR_BASE16_COUNT,_MAX_ITOSTR_BASE10_COUNT,_MAX_ITOSTR_BASE8_COUNT,
	_MAX_LTOSTR_BASE16_COUNT ,_MAX_LTOSTR_BASE10_COUNT ,_MAX_LTOSTR_BASE8_COUNT,
	_MAX_ULTOSTR_BASE16_COUNT,_MAX_ULTOSTR_BASE10_COUNT,_MAX_ULTOSTR_BASE8_COUNT });
inline constexpr auto c_cbI64ToStrBufNoRadix2 = std::max({
	_MAX_I64TOSTR_BASE16_COUNT,_MAX_I64TOSTR_BASE10_COUNT,_MAX_I64TOSTR_BASE8_COUNT,
	_MAX_U64TOSTR_BASE16_COUNT,_MAX_U64TOSTR_BASE10_COUNT,_MAX_U64TOSTR_BASE8_COUNT });

inline constexpr auto c_cbI32ToStrBuf = std::max({ c_cbI32ToStrBufNoRadix2,
	_MAX_ITOSTR_BASE2_COUNT,_MAX_LTOSTR_BASE2_COUNT,_MAX_ULTOSTR_BASE2_COUNT });
inline constexpr auto c_cbI64ToStrBuf = std::max({ c_cbI64ToStrBufNoRadix2,
	_MAX_I64TOSTR_BASE2_COUNT,_MAX_U64TOSTR_BASE2_COUNT });


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
	LCMapStringEx(LOCALE_NAME_USER_DEFAULT, LCMAP_LOWERCASE, pszText, cchText, rs.Data(), cchResult, NULL, NULL, 0);
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
	LCMapStringEx(LOCALE_NAME_USER_DEFAULT, LCMAP_UPPERCASE, pszText, cchText, rs.Data(), cchResult, NULL, NULL, 0);
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
		return INVALID_STR_POS;
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
	return FindStr(ToUpperCase(pszText, cchText).Data(), ToUpperCase(pszSub, cchSub).Data(), posStart);
}

/// <summary>
/// 寻找文本。
/// 不区分大小写。
/// 函数先将整个字符串转换为大写，然后对其调用FindStr
/// </summary>
/// <param name="rsText">要在其中寻找的字符串</param>
/// <param name="rsSub">要寻找的字符串</param>
/// <param name="posStart">起始位置</param>
/// <returns>位置，若未找到返回INVALID_STR_POS</returns>
EckInline int FindStrNcs(const CRefStrW& rsText, const CRefStrW& rsSub, int posStart = 0)
{
	return FindStrNcs(rsText.Data(), rsText.Size(), rsSub.Data(), posStart, rsSub.Size());
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
/// <param name="rsText">要在其中寻找的字符串</param>
/// <param name="rsSub">要寻找的字符串</param>
/// <param name="posStart">起始位置</param>
/// <returns>位置，若未找到返回INVALID_STR_POS</returns>
EckInline int FindStrRev(const CRefStrW& rsText, const CRefStrW& rsSub, int posStart = 0)
{
	return FindStrRev(rsText.Data(), rsText.Size(), rsSub.Data(), posStart,  rsSub.Size());
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
	auto rsText = ToUpperCase(pszText, cchText), rsSub = ToUpperCase(pszSub, cchSub);
	return FindStrRev(rsText.Data(), rsText.Size(), rsSub.Data(), posStart,  rsSub.Size());
}

/// <summary>
/// 倒找文本。
/// 不区分大小写。
/// 函数先将整个字符串转换为大写，然后对其调用FindStrRev
/// </summary>
/// <param name="rsText">要在其中寻找的字符串</param>
/// <param name="rsSub">要寻找的字符串</param>
/// <param name="posStart">起始位置</param>
/// <returns>位置，若未找到返回INVALID_STR_POS</returns>
EckInline int FindStrRevNcs(const CRefStrW& rsText, const CRefStrW& rsSub, int posStart = 0)
{
	return FindStrRevNcs(rsText.Data(), rsText.Size(), rsSub.Data(), posStart, rsSub.Size());
}

template<class TProcesser>
void SplitStr(PCWSTR pszText, PCWSTR pszDiv, int cSubTextExpected, int cchText, int cchDiv, TProcesser Processer)
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
		if (Processer(pszPrevFirst, (int)(pszFind - pszPrevFirst)))
			return;
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
	LCMapStringEx(LOCALE_NAME_USER_DEFAULT, LCMAP_FULLWIDTH, pszText, cchText, rs.Data(), cchResult, NULL, NULL, 0);
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
	LCMapStringEx(LOCALE_NAME_USER_DEFAULT, LCMAP_HALFWIDTH, pszText, cchText, rs.Data(), cchResult, NULL, NULL, 0);
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

#undef EckCRefStrTemp
ECK_NAMESPACE_END