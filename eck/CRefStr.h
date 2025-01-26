#pragma once
#pragma warning (disable:4996)
#include "StringUtility.h"
#include "CAllocator.h"
#include "Utility.h"

ECK_NAMESPACE_BEGIN



[[nodiscard]] EckInline int FindFirstOf(PCWSTR pszText, PCWSTR pszCharSet, int posStart = 0)
{
	const PCWSTR pszFind = wcspbrk(pszText + posStart, pszCharSet);
	return pszFind ? (int)(pszFind - pszText) : StrNPos;
}

[[nodiscard]] EckInline int FindFirstOf(PCSTR pszText, PCSTR pszCharSet, int posStart = 0)
{
	const PCSTR pszFind = strpbrk(pszText + posStart, pszCharSet);
	return pszFind ? (int)(pszFind - pszText) : StrNPos;
}

/// <summary>
/// 删首空。
/// 函数从pszText开始向后步进到第一个非空格字符，然后返回指向这个字符的指针。
/// 此函数不执行任何修改字符串的操作
/// </summary>
/// <param name="pszText">原始文本</param>
/// <returns>第一个非空格字符的指针</returns>
[[nodiscard]] EckInline PCWSTR LTrimStr(PCWSTR pszText)
{
	WCHAR ch = *pszText;
	while ((ch == L' ' || ch == L'　') && ch != L'\0')
		ch = *++pszText;
	return pszText;
}

/// <summary>
/// 删首空。
/// 函数从pszText开始向后步进到第一个非空格字符，然后返回指向这个字符的指针。
/// 此函数不执行任何修改字符串的操作
/// </summary>
/// <param name="pszText">原始文本</param>
/// <returns>第一个非空格字符的指针</returns>
[[nodiscard]] EckInline PCSTR LTrimStr(PCSTR pszText)
{
	CHAR ch = *pszText;
	while ((ch == ' ' || ch == '　') && ch != '\0')
		ch = *++pszText;
	return pszText;
}

/// <summary>
/// 删尾空。
/// 此函数不执行任何修改字符串的操作
/// </summary>
/// <param name="pszText">原始文本</param>
/// <param name="cchText">文本长度</param>
/// <returns>第一个非空格字符的下一个字符的指针，若字符串全为空格，则返回pszText</returns>
[[nodiscard]] inline PCWSTR RTrimStr(PCWSTR pszText, int cchText = -1)
{
	if (cchText < 0)
		cchText = (int)wcslen(pszText);
	if (!cchText)
		return 0;

	for (PCWSTR pszTemp = pszText + cchText - 1; pszTemp >= pszText; --pszTemp)
	{
		const WCHAR ch = *pszTemp;
		if (ch != L' ' && ch != L'　')
			return pszTemp + 1;
	}

	return pszText;
}

/// <summary>
/// 删尾空。
/// 此函数不执行任何修改字符串的操作
/// </summary>
/// <param name="pszText">原始文本</param>
/// <param name="cchText">文本长度</param>
/// <returns>第一个非空格字符的下一个字符的指针，若字符串全为空格，则返回pszText</returns>
[[nodiscard]] inline PCSTR RTrimStr(PCSTR pszText, int cchText = -1)
{
	if (cchText < 0)
		cchText = (int)strlen(pszText);
	if (!cchText)
		return 0;

	for (PCSTR pszTemp = pszText + cchText - 1; pszTemp >= pszText; --pszTemp)
	{
		const CHAR ch = *pszTemp;
		if (ch != ' ' && ch != '　')
			return pszTemp + 1;
	}

	return pszText;
}

/// <summary>
/// 删首尾空。
/// 函数内部简单地调用LTrimStr和RTrimStr获取首尾空信息。
/// 此函数不执行任何修改字符串的操作
/// </summary>
/// <param name="pszText">原始文本</param>
/// <param name="cchText">文本长度</param>
/// <returns>起始位置和结束位置指针，若无非空格字符，则返回{ NULL,NULL }</returns>
[[nodiscard]] EckInline std::pair<PCWSTR, PCWSTR> RLTrimStr(PCWSTR pszText, int cchText = -1)
{
	const auto pszLeft = LTrimStr(pszText);
	const auto pszRight = RTrimStr(pszText, cchText);
	if (pszRight <= pszLeft)
		return { nullptr,nullptr };
	else
		return { pszLeft,pszRight };
}

/// <summary>
/// 删首尾空。
/// 函数内部简单地调用LTrimStr和RTrimStr获取首尾空信息。
/// 此函数不执行任何修改字符串的操作
/// </summary>
/// <param name="pszText">原始文本</param>
/// <param name="cchText">文本长度</param>
/// <returns>起始位置和结束位置指针，若无非空格字符，则返回{ NULL,NULL }</returns>
[[nodiscard]] EckInline std::pair<PCSTR, PCSTR> RLTrimStr(PCSTR pszText, int cchText = -1)
{
	const auto pszLeft = LTrimStr(pszText);
	const auto pszRight = RTrimStr(pszText, cchText);
	if (pszRight <= pszLeft)
		return { nullptr,nullptr };
	else
		return { pszLeft,pszRight };
}

template<ccpIsStdChar TChar>
using TRefStrDefAlloc = CDefAllocator<TChar, int>;

template<ccpIsStdChar TChar_, class TCharTraits_, class TAlloc_>
class CRefStrT;

template<class TCharTraits, class TAlloc>
CRefStrT<WCHAR, TCharTraits, TAlloc> StrX2W(PCSTR pszText, int cch, int uCP);

template<ccpIsStdChar TChar_>
struct CCharTraits
{
	using TChar = void;
};

template<>
struct CCharTraits<WCHAR>
{
	using TChar = WCHAR;
	EckInline static int Len(PCWSTR psz) { return (int)wcslen(psz); }
	EckInline static PWSTR Copy(PWSTR psz1, PCWSTR psz2, int cch) { return wmemcpy(psz1, psz2, cch); }
	EckInline static PWSTR CopyEnd(PWSTR psz1, PCWSTR psz2, int cch)
	{
		auto r = Copy(psz1, psz2, cch);
		*(psz1 + cch) = L'\0';
		return r;
	}
	EckInline static PWSTR Move(PWSTR psz1, PCWSTR psz2, int cch) { return wmemmove(psz1, psz2, cch); }
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
	EckInline static int Compare(PCWSTR psz1, PCWSTR psz2, int cch) { return wcsncmp(psz1, psz2, cch); }
	EckInline static int CompareI(PCWSTR psz1, PCWSTR psz2) { return wcsicmp(psz1, psz2); }
	EckInline static int CompareI(PCWSTR psz1, PCWSTR psz2, int cch) { return wcsnicmp(psz1, psz2, cch); }
	EckInline static int Find(PCWSTR pszText, PCWSTR pszSub, int posStart = 0) { return FindStr(pszText, pszSub, posStart); }
	EckInline static PCWSTR Find2(PCWSTR pszText, PCWSTR pszSub) { return wcsstr(pszText, pszSub); }
	EckInline static int FormatV(PWSTR pszBuf, PCWSTR pszFmt, va_list vl) { return vswprintf(pszBuf, pszFmt, vl); }
	EckInline static int GetFormatCchV(PCWSTR pszFmt, va_list vl) { return _vscwprintf(pszFmt, vl); }
	EckInline static int FindChar(PCWSTR pszText, WCHAR ch, int posStart = 0) { return eck::FindChar(pszText, ch, posStart); }
	EckInline static WCHAR ToUpper(WCHAR ch) { return towupper(ch); }
	EckInline static WCHAR ToLower(WCHAR ch) { return towlower(ch); }
};

template<>
struct CCharTraits<CHAR>
{
	using TChar = CHAR;
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
	EckInline static int Compare(PCSTR psz1, PCSTR psz2, int cch) { return strncmp(psz1, psz2, cch); }
	EckInline static int CompareI(PCSTR psz1, PCSTR psz2) { return stricmp(psz1, psz2); }
	EckInline static int CompareI(PCSTR psz1, PCSTR psz2, int cch) { return strnicmp(psz1, psz2, cch); }
	EckInline static int Find(PCSTR pszText, PCSTR pszSub, int posStart = 0) { return FindStr(pszText, pszSub, posStart); }
	EckInline static PCSTR Find2(PCSTR pszText, PCSTR pszSub) { return strstr(pszText, pszSub); }
	EckInline static int FormatV(PSTR pszBuf, PCSTR pszFmt, va_list vl) { return vsprintf(pszBuf, pszFmt, vl); }
	EckInline static int GetFormatCchV(PCSTR pszFmt, va_list vl) { return _vscprintf(pszFmt, vl); }
	EckInline static int FindChar(PCSTR pszText, CHAR ch, int posStart = 0) { return eck::FindChar(pszText, ch, posStart); }
	EckInline static CHAR ToUpper(CHAR ch) { return (CHAR)toupper(ch); }
	EckInline static CHAR ToLower(CHAR ch) { return (CHAR)tolower(ch); }
};

template<ccpIsStdChar TChar>
using TRefStrDefTraits = CCharTraits<TChar>;

template<
	ccpIsStdChar TChar_,
	class TCharTraits_ = TRefStrDefTraits<TChar_>,
	class TAlloc_ = TRefStrDefAlloc<TChar_>
>
class CRefStrT
{
public:
	using TChar = TChar_;
	using TCharTraits = TCharTraits_;
	using TAlloc = TAlloc_;

	using TAllocTraits = CAllocatorTraits<TAlloc>;
	using TPointer = TChar*;
	using TConstPointer = const TChar*;

	using TIterator = TPointer;
	using TConstIterator = TConstPointer;
	using TReverseIterator = std::reverse_iterator<TIterator>;
	using TConstReverseIterator = std::reverse_iterator<TConstIterator>;

	using TNtString = std::conditional_t<std::is_same_v<TChar, CHAR>, ANSI_STRING, UNICODE_STRING>;

	static_assert(std::is_same_v<typename TAllocTraits::size_type, int>);
private:
	union
	{
		TPointer m_pszText;
		TChar m_szLocal[16 / sizeof(TChar)]{};
	};
	int m_cchText{};
	int m_cchCapacity{ ARRAYSIZE(m_szLocal) };

	ECKNOUNIQUEADDR TAlloc m_Alloc{};

	static constexpr void ResetThat(CRefStrT& x)
	{
		x.m_pszText = nullptr;// 保证清空指针和截断sso缓冲区
		x.m_cchText = 0;
		x.m_cchCapacity = LocalBufferSize;
	}
public:
	constexpr static int LocalBufferSize = ARRAYSIZE(m_szLocal);

	CRefStrT() = default;

	explicit CRefStrT(const TAlloc& Al) : m_Alloc{ Al } {}

	/// <summary>
	/// 创建自长度
	/// </summary>
	/// <param name="cchInit">字符串长度</param>
	explicit CRefStrT(int cchInit)
	{
		ReSize(cchInit);
	}

	/// <summary>
	/// 创建自长度
	/// </summary>
	/// <param name="cchInit">字符串长度</param>
	/// <param name="Al">分配器</param>
	CRefStrT(int cchInit, const TAlloc& Al) : m_Alloc{ Al }
	{
		ReSize(cchInit);
	}

	/// <summary>
	/// 创建自字符串
	/// </summary>
	/// <param name="psz">字符串指针</param>
	/// <param name="cchText">字符串长度</param>
	/// <param name="Al">分配器</param>
	CRefStrT(TConstPointer psz, int cchText, const TAlloc& Al = TAlloc{}) : m_Alloc{ Al }
	{
		DupString(psz, cchText);
	}

	/// <summary>
	/// 创建自字符串
	/// </summary>
	/// <param name="psz">字符串指针</param>
	/// <param name="Al">分配器</param>
	CRefStrT(TConstPointer psz, const TAlloc& Al = TAlloc{})
		: CRefStrT(psz, psz ? TCharTraits::Len(psz) : 0, Al) {
	}

	CRefStrT(const CRefStrT& x)
		: m_Alloc{ TAllocTraits::select_on_container_copy_construction(x.m_Alloc) }
	{
		DupString(x.Data(), x.Size());
	}

	CRefStrT(const CRefStrT& x, const TAlloc& Al) : m_Alloc{ Al }
	{
		DupString(x.Data(), x.Size());
	}

	CRefStrT(CRefStrT&& x) noexcept
		: m_cchText{ x.m_cchText }, m_cchCapacity{ x.m_cchCapacity }, m_Alloc{ std::move(x.m_Alloc) }
	{
		std::copy(std::begin(x.m_szLocal), std::end(x.m_szLocal), std::begin(m_szLocal));
		ResetThat(x);
	}

	CRefStrT(CRefStrT&& x, const TAlloc& Al)
		: m_cchText{ x.m_cchText }, m_cchCapacity{ x.m_cchCapacity }, m_Alloc{ Al }
	{
		if constexpr (!TAllocTraits::is_always_equal::value)
		{
			if (Al != x.m_Alloc)
			{
				DupString(x.Data(), x.Size());
				ResetThat(x);
				return;
			}
		}
		std::copy(std::begin(x.m_szLocal), std::end(x.m_szLocal), std::begin(m_szLocal));
		ResetThat(x);
	}

	template<class TTraits, class TAlloc1>
	explicit CRefStrT(const std::basic_string<TChar, TTraits, TAlloc1>& s, const TAlloc& Al = TAlloc{})
		: CRefStrT(s.data(), (int)s.size(), Al) {
	}

	explicit CRefStrT(TNtString nts, const TAlloc& Al = TAlloc{})
		:CRefStrT(nts.Buffer, (int)nts.Length, Al) {
	}

	template<class TTraits>
	CRefStrT(std::basic_string_view<TChar, TTraits> sv, const TAlloc& Al = TAlloc{})
		: CRefStrT(sv.data(), (int)sv.size(), Al) {
	}

	~CRefStrT()
	{
		if (!IsLocal())
			m_Alloc.deallocate(m_pszText, m_cchCapacity);
	}

	EckInline CRefStrT& operator=(TConstPointer pszSrc)
	{
		if (pszSrc)
			DupString(pszSrc);
		else
			Clear();
		return *this;
	}

	EckInline CRefStrT& operator=(const CRefStrT& x)
	{
		if (this == &x)
			return *this;
		if constexpr (!TAllocTraits::is_always_equal::value)
		{
			if (m_Alloc != x.m_Alloc)
			{
				if (!IsLocal())
					m_Alloc.deallocate(m_pszText, m_cchCapacity);
				ResetThat(*this);
				m_Alloc = x.m_Alloc;
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
		TChar szTemp[LocalBufferSize];
		std::copy(std::begin(x.m_szLocal), std::end(x.m_szLocal), std::begin(szTemp));
		std::copy(std::begin(m_szLocal), std::end(m_szLocal), std::begin(x.m_szLocal));
		std::copy(std::begin(szTemp), std::end(szTemp), std::begin(m_szLocal));

		std::swap(m_cchText, x.m_cchText);
		std::swap(m_cchCapacity, x.m_cchCapacity);
		return *this;
	}

	template<class TTraits, class TAlloc1>
	EckInline CRefStrT& operator=(const std::basic_string<TChar, TTraits, TAlloc1>& x)
	{
		DupString(x.data(), (int)x.size());
		return *this;
	}

	template<class TTraits>
	EckInline CRefStrT& operator=(std::basic_string_view<TChar, TTraits> x)
	{
		DupString(x.data(), (int)x.size());
		return *this;
	}

	EckInlineNdCe TChar& At(int x) { EckAssert(x >= 0 && x < Size()); return *(Data() + x); }
	EckInlineNdCe TChar At(int x) const { EckAssert(x >= 0 && x < Size()); return *(Data() + x); }
	EckInlineNdCe TChar& operator[](int x) { return At(x); }
	EckInlineNdCe TChar operator[](int x) const { return At(x); }

	EckInlineNdCe TChar& Front() { return At(0); }
	EckInlineNdCe TChar Front() const { return At(0); }
	EckInlineNdCe TChar& Back() { return At(Size() - 1); }
	EckInlineNdCe TChar Back() const { return At(Size() - 1); }

	EckInlineNd TAlloc GetAllocator() const { return m_Alloc; }
	EckInlineNdCe int Size() const noexcept { return m_cchText; }
	EckInlineNdCe BOOL IsEmpty() const { return Size() == 0; }
	EckInlineNdCe size_t ByteSize() const noexcept { return (m_cchText + 1) * sizeof(TChar); }
	EckInlineNdCe size_t ByteSizePure() const noexcept { return m_cchText * sizeof(TChar); }
	EckInlineNdCe int Capacity() const noexcept { return m_cchCapacity; }
	EckInlineNdCe BOOL IsLocal() const noexcept { return Capacity() == LocalBufferSize; }
	EckInlineNdCe size_t ByteCapacity() const noexcept { return m_cchCapacity * sizeof(TChar); }
	EckInlineNdCe TPointer Data() noexcept { return IsLocal() ? m_szLocal : m_pszText; }
	EckInlineNdCe TConstPointer Data() const noexcept { return IsLocal() ? m_szLocal : m_pszText; }

	/// <summary>
	/// 克隆字符串。
	/// 将指定字符串复制到自身
	/// </summary>
	/// <param name="pszSrc">字符串指针</param>
	/// <param name="cchSrc">字符串长度</param>
	/// <returns>实际复制的字符数</returns>
	int DupString(TConstPointer pszSrc, int cchSrc = -1)
	{
		EckAssert(pszSrc ? TRUE : cchSrc == 0);
		if (cchSrc < 0)
			cchSrc = TCharTraits::Len(pszSrc);
		ReSizeExtra(cchSrc);
		TCharTraits::CopyEnd(Data(), pszSrc, cchSrc);
		return cchSrc;
	}

	EckInline int DupString(TNtString nts)
	{
		return DupString((TConstPointer)nts.Buffer, (int)nts.Length);
	}

	template<class TTraits, class TAlloc1>
	EckInline int DupString(const std::basic_string<TChar, TTraits, TAlloc1>& x)
	{
		return DupString(x.data(), (int)x.size());
	}

	template<class TTraits>
	EckInline int DupString(std::basic_string_view<TChar, TTraits> sv)
	{
		return DupString(sv.data(), (int)sv.size());
	}

	EckInline int DupBSTR(BSTR bstr)
	{
		if (bstr)
			return DupString(TConstPointer(((PCBYTE)bstr) + 4), (int)SysStringLen(bstr));
		else
		{
			Clear();
			return 0;
		}
	}

	int DupSTRRET(const STRRET& strret, PITEMIDLIST pidl = nullptr)
	{
		switch (strret.uType)
		{
		case STRRET_WSTR:
			return DupString(strret.pOleStr);
		case STRRET_OFFSET:
			EckAssert(pidl);
			return DupString((TConstPointer)((PCBYTE)pidl + strret.uOffset));
		case STRRET_CSTR:
			if constexpr (std::is_same_v<TChar, CHAR>)
				return DupString(strret.cStr);
			else
			{
				const int cch = MultiByteToWideChar(CP_ACP, 0, strret.cStr, -1, nullptr, 0);
				if (cch > 1)
				{
					ReSize(cch);
					return MultiByteToWideChar(CP_ACP, 0, strret.cStr, -1, Data(), cch);
				}
				else
					Clear();
			}
		default:
			return 0;
		}
	}

	/// <summary>
	/// 依附指针。
	/// 先前的内存将被释放
	/// </summary>
	/// <param name="psz">指针，必须可通过当前分配器解分配</param>
	/// <param name="cchCapacity">容量</param>
	/// <param name="cchText">字符数</param>
	void Attach(TPointer psz, int cchCapacity, int cchText)
	{
		EckAssert(cchCapacity > 0 && cchText >= 0);
		m_Alloc.deallocate(m_pszText, m_cchCapacity);
		if (!psz)
		{
			m_pszText = nullptr;
			m_cchText = m_cchCapacity = 0;
		}
		else
		{
			m_cchCapacity = cchCapacity;
			m_cchText = cchText;
			m_pszText = psz;
		}
	}

	/// <summary>
	/// 拆离指针
	/// </summary>
	/// <returns>指针，必须通过与当前分配器相等的分配器解分配</returns>
	[[nodiscard]] EckInline TPointer Detach(int& cchCapacity, int& cchText)
	{
		const auto pOld = m_pszText;
		m_pszText = nullptr;

		cchCapacity = m_cchCapacity;
		m_cchCapacity = 0;

		cchText = m_cchText;
		m_cchText = 0;
		return pOld;
	}

	/// <summary>
	/// 尾插
	/// </summary>
	/// <param name="pszSrc">字符串指针</param>
	/// <param name="cchSrc">字符串长度</param>
	/// <returns>实际复制的字符数</returns>
	CRefStrT& PushBack(TConstPointer pszSrc, int cchSrc = -1)
	{
		if (!pszSrc)
			return *this;
		if (cchSrc < 0)
			cchSrc = TCharTraits::Len(pszSrc);
		ReSizeExtra(Size() + cchSrc);
		TCharTraits::CopyEnd(Data() + Size() - cchSrc, pszSrc, cchSrc);
		return *this;
	}

	EckInline CRefStrT& PushBack(const CRefStrT& rs)
	{
		return PushBack(rs.Data(), rs.Size());
	}

	EckInline TPointer PushBack(int cch)
	{
		EckAssert(cch >= 0);
		ReSizeExtra(Size() + cch);
		return Data() + Size() - cch;
	}

	template<class TTraits, class TAlloc1>
	EckInline CRefStrT& PushBack(const std::basic_string<TChar, TTraits>& s)
	{
		return PushBack(s.data(), (int)s.size());
	}

	template<class TTraits>
	EckInline CRefStrT& PushBack(std::basic_string_view<TChar, TTraits> sv)
	{
		return PushBack(sv.data(), (int)sv.size());
	}

	EckInline void PushBackChar(TChar ch)
	{
		ReSizeExtra(Size() + 1);
		TCharTraits::AssignChar(Data() + Size() - 1, ch);
	}

	EckInline TPointer PushBackNoExtra(int cch)
	{
		EckAssert(cch >= 0);
		ReSize(Size() + cch);
		return Data() + Size() - cch;
	}

	EckInline CRefStrT& operator+=(TConstPointer psz)
	{
		PushBack(psz);
		return *this;
	}

	EckInline CRefStrT& operator+=(const CRefStrT& x)
	{
		PushBack(x.Data(), x.Size());
		return *this;
	}

	template<class TTraits, class TAlloc1>
	EckInline CRefStrT& operator+=(const std::basic_string<TChar, TTraits, TAlloc1>& x)
	{
		PushBack(x.data(), (int)x.size());
		return *this;
	}

	template<class TTraits>
	EckInline CRefStrT& operator+=(std::basic_string_view<TChar, TTraits> x)
	{
		PushBack(x.data(), (int)x.size());
		return *this;
	}

	EckInline CRefStrT& operator+=(TChar ch)
	{
		PushBackChar(ch);
		return *this;
	}

	/// <summary>
	/// 尾删
	/// </summary>
	/// <param name="cch">删除长度</param>
	EckInline void PopBack(int cch = 1)
	{
		EckAssert(Size() >= cch && cch >= 0);
		if (!cch)
			return;
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
		if (cch < 0 || cch > Size())
			cch = Size();
		TCharTraits::CopyEnd(pszDst, Data(), cch);
		return cch;
	}
private:
	void ReserveReal(int cch)
	{
		if (m_cchCapacity >= cch)
			return;
		if (IsLocal() && cch <= LocalBufferSize)
			return;
		const auto pOld = IsLocal() ? m_szLocal : m_pszText;
		const auto pNew = m_Alloc.allocate(cch);
		if (pOld)
		{
			TCharTraits::Copy(pNew, pOld, m_cchText + 1);// 多拷一个结尾NULL
			if (!IsLocal())
				m_Alloc.deallocate(pOld, m_cchCapacity);
		}
		else
			TCharTraits::Cut(pNew, 0);
		m_pszText = pNew;
		m_cchCapacity = cch;
	}
public:
	/// <summary>
	/// 保留内存
	/// </summary>
	/// <param name="cch">字符数</param>
	EckInline void Reserve(int cch) { ReserveReal(cch + 1); }

	/// <summary>
	/// 重置尺寸
	/// </summary>
	/// <param name="cch">字符数</param>
	void ReSize(int cch)
	{
		EckAssert(cch >= 0);
		ReserveReal(cch + 1);
		m_cchText = cch;
		TCharTraits::Cut(Data(), cch);
	}

	void ReSizeExtra(int cch)
	{
		EckAssert(cch >= 0);
		if (m_cchCapacity < cch + 1)
			ReserveReal(TAllocTraits::MakeCapacity(cch + 1));
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
	void Replace(int posStart, int cchReplacing, TConstPointer pszNew, int cchNew)
	{
		EckAssert(pszNew ? TRUE : cchNew == 0);
		if (cchNew < 0)
			cchNew = TCharTraits::Len(pszNew);
		const int cchOrg = Size();
		const int cchAfter = Size() + cchNew - cchReplacing;
		Reserve(cchAfter);
		TCharTraits::Move(
			Data() + posStart + cchNew,
			Data() + posStart + cchReplacing,
			cchOrg - posStart - cchReplacing);
		if (pszNew)
			TCharTraits::Copy(Data() + posStart, pszNew, cchNew);
		ReSize(cchAfter);
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
	void ReplaceSubStr(TConstPointer pszReplaced, int cchReplaced, TConstPointer pszSrc, int cchSrc,
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
			pos = TCharTraits::Find(Data(), pszReplaced, posStart + pos);
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
		ReSizeExtra(posStart + cchText * cCount);
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

	/// <summary>
	/// 清除。
	/// 将尺寸设置为0并在开始处截断（如果有数据）
	/// </summary>
	/// <returns></returns>
	EckInline constexpr void Clear()
	{
		m_cchText = 0;
		TCharTraits::Cut(Data(), 0);
	}

	/// <summary>
	/// 插入字符串
	/// </summary>
	/// <param name="pos">位置</param>
	/// <param name="pszText">字符串</param>
	/// <param name="cchText">字符数</param>
	EckInline void Insert(int pos, TConstPointer pszText, int cchText = -1)
	{
		EckAssert(pos <= Size() && pos >= 0);
		EckAssert(pszText ? TRUE : (cchText == 0));
		if (cchText < 0)
			cchText = TCharTraits::Len(pszText);
		ReSizeExtra(Size() + cchText);
		TCharTraits::MoveEnd(
			Data() + pos + cchText,
			Data() + pos,
			Size() - cchText - pos);
		TCharTraits::Copy(Data() + pos, pszText, cchText);
	}

	template<class TTraits, class TAlloc1>
	EckInline void Insert(int pos, const CRefStrT<TChar, TTraits, TAlloc1>& rs)
	{
		Insert(pos, rs.Data(), rs.Size());
	}

	template<class TTraits, class TAlloc1>
	EckInline void Insert(int pos, const std::basic_string<TChar, TTraits, TAlloc1>& s)
	{
		Insert(pos, s.data(), (int)s.size());
	}

	template<class TTraits>
	EckInline void Insert(int pos, std::basic_string_view<TChar, TTraits> sv)
	{
		Insert(pos, sv.data(), (int)sv.size());
	}

	EckInline TPointer Insert(int pos, int cchText)
	{
		EckAssert(pos <= Size() && pos >= 0);
		ReSizeExtra(Size() + cchText);
		TCharTraits::MoveEnd(
			Data() + pos + cchText,
			Data() + pos,
			Size() - cchText - pos);
		return Data() + pos;
	}

	EckInline void InsertChar(int pos, TChar ch)
	{
		EckAssert(pos <= Size() && pos >= 0);
		ReSizeExtra(Size() + 1);
		TCharTraits::MoveEnd(
			Data() + pos + 1,
			Data() + pos,
			Size() - 1 - pos);
		TCharTraits::AssignChar(Data() + pos, ch);
	}

	/// <summary>
	/// 擦除字符
	/// </summary>
	/// <param name="pos">位置</param>
	/// <param name="cch">要擦除的字符数</param>
	EckInline void Erase(int pos, int cch = 1)
	{
		EckAssert(Size() >= pos + cch);
		TCharTraits::MoveEnd(
			Data() + pos,
			Data() + pos + cch,
			Size() - pos - cch);
		m_cchText -= cch;
	}

	// 裁剪多余空间
	void ShrinkToFit()
	{
		EckAssert(m_cchCapacity >= m_cchText + 1);
		if (m_cchCapacity == m_cchText + 1)
			return;
		const auto pOld = m_pszText;
		m_pszText = m_Alloc.allocate(m_cchText + 1);
		TCharTraits::Copy(Data(), pOld, m_cchText + 1);
		m_Alloc.deallocate(pOld, m_cchCapacity);
		m_cchCapacity = m_cchText + 1;
	}

	void ExtendToCapacity()
	{
		if (Capacity())
		{
			m_cchText = Capacity() - 1;
			TCharTraits::Cut(Data(), m_cchText);
		}
	}

	EckInline int Format(_Printf_format_string_ TConstPointer pszFmt, ...)
	{
		va_list vl;
		va_start(vl, pszFmt);
		const int cch = FormatV(pszFmt, vl);
		va_end(vl);
		return cch;
	}

	EckInline int FormatV(_Printf_format_string_ TConstPointer pszFmt, va_list vl)
	{
		const int cch = TCharTraits::GetFormatCchV(pszFmt, vl);
		if (cch <= 0)
			return 0;
		ReSizeExtra(cch);
		TCharTraits::FormatV(Data(), pszFmt, vl);
		return cch;
	}

	EckInline int AppendFormat(_Printf_format_string_ TConstPointer pszFmt, ...)
	{
		va_list vl;
		va_start(vl, pszFmt);
		const int cch = AppendFormatV(pszFmt, vl);
		va_end(vl);
		return cch;
	}

	EckInline int AppendFormatV(_Printf_format_string_ TConstPointer pszFmt, va_list vl)
	{
		const int cch = TCharTraits::GetFormatCchV(pszFmt, vl);
		if (cch <= 0)
			return 0;
		TCharTraits::FormatV(PushBack(cch), pszFmt, vl);
		return cch;
	}

	/// <summary>
	/// 取BSTR。
	/// 调用方必须在使用完返回值后对其调用SysFreeString以解分配
	/// </summary>
	/// <returns>BSTR</returns>
	EckInlineNd BSTR ToBSTR() const
	{
		if constexpr (std::is_same_v<TChar, WCHAR>)
			return SysAllocStringLen(Data(), Size());
		else
		{
			auto rs = StrX2W<CCharTraits<WCHAR>, CAllocatorProcHeap<WCHAR, int>>(Data(), Size(), CP_ACP);
			return SysAllocStringLen(rs.Data(), rs.Size());
		}
	}

	template<class TTraits = std::char_traits<TChar>, class TAlloc1 = std::allocator<TChar>>
	EckInlineNd auto ToStdString() const
	{
		return std::basic_string<TChar, TTraits, TAlloc1>(Data(), (size_t)Size());
	}

	EckInlineNdCe auto ToStringView() const
	{
		return std::basic_string_view<TChar>(Data(), Size());
	}

	EckInlineNdCe auto ToSpan() const
	{
		return std::span<TChar>(Data(), ByteSize());
	}

	EckInlineNdCe TNtString ToNtString()
	{
		return TNtString{ (USHORT)ByteSizePure(),(USHORT)ByteCapacity(),Data() };
	}

	EckInlineNdCe RTL_UNICODE_STRING_BUFFER ToNtStringBuf()
	{
		if constexpr (std::is_same_v<TChar, WCHAR>)
		{
#if ECKCXX20
			return RTL_UNICODE_STRING_BUFFER
			{
				.String = ToNtString(),
				.ByteBuffer = {.Buffer = (PUCHAR)Data(),.Size = ByteCapacity() },
			};
#else
			RTL_UNICODE_STRING_BUFFER Buf{ ToNtString() };
			Buf.ByteBuffer.Buffer = (PUCHAR)Data();
			Buf.ByteBuffer.Size = ByteCapacity();
			return Buf;
#endif// ECKCXX20
		}
		else
			return {};
	}

	EckInlineNd int Find(TConstPointer pszSub, int posStart = 0) const
	{
		if (IsEmpty())
			return StrNPos;
		return FindStr(Data(), pszSub, posStart);
	}

	template<class TTraits, class TAlloc1>
	EckInlineNd int Find(const CRefStrT<TChar, TTraits, TAlloc1>& rs, int posStart = 0) const
	{
		if (IsEmpty())
			return StrNPos;
		return FindStr(Data(), rs.Data(), posStart);
	}

	template<class TTraits, class TAlloc1>
	EckInlineNd int Find(const std::basic_string<TChar, TTraits, TAlloc1>& s, int posStart = 0) const
	{
		if (IsEmpty())
			return StrNPos;
		return FindStr(Data(), s.data(), posStart);
	}

	template<class TTraits>
	EckInlineNd int Find(std::basic_string<TChar, TTraits> sv, int posStart = 0) const
	{
		if (IsEmpty())
			return StrNPos;
		return FindStr(Data(), sv.data(), posStart);
	}

	EckInlineNd int RFind(TConstPointer pszSub, int cchSub = -1, int posStart = -1) const
	{
		if (IsEmpty())
			return StrNPos;
		return FindStrRev(Data(), Size(), pszSub, cchSub, posStart);
	}

	EckInlineNd int FindChar(TChar ch, int posStart = 0) const
	{
		if (IsEmpty())
			return StrNPos;
		return TCharTraits::FindChar(Data(), ch, posStart);
	}

	void LTrim()
	{
		if (IsEmpty())
			return;
		const auto pszBegin = LTrimStr(Data());
		const int cchNew = Size() - (int)(pszBegin - Data());
		TCharTraits::Move(Data(), pszBegin, cchNew);
		ReSize(cchNew);
	}

	void RTrim()
	{
		if (IsEmpty())
			return;
		const auto pszEnd = RTrimStr(Data(), Size());
		ReSize((int)(pszEnd - Data()));
	}

	void RLTrim()
	{
		if (IsEmpty())
			return;
		auto [psz0, psz1] = RLTrimStr(Data(), Size());
		if (!psz0)
			return;
		const int cchNew = (int)(psz1 - psz0);
		TCharTraits::Move(Data(), psz0, cchNew);
		ReSize(cchNew);
	}

	[[nodiscard]] BOOL IsStartOf(TConstPointer psz, int cch = -1) const
	{
		if (cch < 0)
			cch = TCharTraits::Len(psz);
		if (cch == 0 || Size() < cch)
			return FALSE;
		return TCharTraits::Compare(Data(), psz, cch) == 0;
	}

	[[nodiscard]] BOOL IsStartOfI(TConstPointer psz, int cch = -1) const
	{
		if (cch < 0)
			cch = TCharTraits::Len(psz);
		if (cch == 0 || Size() < cch)
			return FALSE;
		return TCharTraits::CompareI(Data(), psz, cch) == 0;
	}

	[[nodiscard]] EckInline int Compare(TConstPointer psz) const
	{
		return TCharTraits::Compare(Data(), psz);
	}

	[[nodiscard]] EckInline int CompareI(TConstPointer psz) const
	{
		return TCharTraits::CompareI(Data(), psz);
	}

	EckInlineNd CRefStrT SubStr(int posStart, int cch) const
	{
		EckAssert(posStart >= 0 && posStart < Size());
		if (cch < 0)
			cch = Size() - posStart;
		EckAssert(cch != 0 && posStart + cch <= Size());
		return CRefStrT(Data() + posStart, cch);
	}

	EckInlineNd auto SubStringView(int posStart, int cch) const
	{
		EckAssert(posStart >= 0 && posStart < Size());
		if (cch < 0)
			cch = Size() - posStart;
		EckAssert(cch != 0 && posStart + cch <= Size());
		return std::basic_string_view<TChar>(Data() + posStart, cch);
	}

	[[nodiscard]] EckInline TIterator begin() { return Data(); }
	[[nodiscard]] EckInline TIterator end() { return begin() + Size(); }
	[[nodiscard]] EckInline TConstIterator begin() const { return Data(); }
	[[nodiscard]] EckInline TConstIterator end() const { return begin() + Size(); }
	[[nodiscard]] EckInline TConstIterator cbegin() const { begin(); }
	[[nodiscard]] EckInline TConstIterator cend() const { end(); }
	[[nodiscard]] EckInline TReverseIterator rbegin() { return TReverseIterator(begin()); }
	[[nodiscard]] EckInline TReverseIterator rend() { return TReverseIterator(end()); }
	[[nodiscard]] EckInline TConstReverseIterator rbegin() const { return TConstReverseIterator(begin()); }
	[[nodiscard]] EckInline TConstReverseIterator rend() const { return TConstReverseIterator(end()); }
	[[nodiscard]] EckInline TConstReverseIterator crbegin() const { return rbegin(); }
	[[nodiscard]] EckInline TConstReverseIterator crend() const { return rend(); }
};

using CRefStrW = CRefStrT<WCHAR, CCharTraits<WCHAR>>;
using CRefStrA = CRefStrT<CHAR, CCharTraits<CHAR>>;

#define EckCRefStrTemp CRefStrT<TChar, TCharTraits, TAlloc>

#pragma region Operator
template<class TChar, class TCharTraits, class TAlloc>
EckInline EckCRefStrTemp operator+(const EckCRefStrTemp& rs1, const EckCRefStrTemp& rs2)
{
	EckCRefStrTemp x(rs1.Size() + rs2.Size());
	TCharTraits::Copy(x.Data(), rs1.Data(), rs1.Size());
	TCharTraits::CopyEnd(x.Data() + rs1.Size(), rs2.Data(), rs2.Size());
	return x;
}

template<class TChar, class TCharTraits, class TAlloc>
EckInline EckCRefStrTemp operator+(const EckCRefStrTemp& rs, const TChar* psz)
{
	const int cch = (psz ? TCharTraits::Len(psz) : 0);
	EckCRefStrTemp x(rs.Size() + cch);
	TCharTraits::Copy(x.Data(), rs.Data(), rs.Size());
	TCharTraits::CopyEnd(x.Data() + rs.Size(), psz, cch);
	return x;
}

template<class TChar, class TCharTraits, class TAlloc>
[[nodiscard]] EckInline bool operator==(const EckCRefStrTemp& rs1, const TChar* psz2)
{
	if (!rs1.Data() && !psz2)
		return true;
	else if (!rs1.Data() || !psz2)
		return false;
	else
		return TCharTraits::Compare(rs1.Data(), psz2) == 0;
}

#if ECKCXX20
template<class TChar, class TCharTraits, class TAlloc>
[[nodiscard]] EckInline std::weak_ordering operator<=>(const EckCRefStrTemp& rs1, const TChar* psz2)
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
#else// !ECKCXX20
template<class TChar, class TCharTraits, class TAlloc>
[[nodiscard]] EckInline bool operator<(const EckCRefStrTemp& rs1, const TChar* psz2)
{
	if (!rs1.Data() && !psz2)
		return false;
	else if (!rs1.Data() && psz2)
		return false;
	else if (rs1.Data() && !psz2)
		return true;
	else
		return TCharTraits::Compare(rs1.Data(), psz2) < 0;
}

template<class TChar, class TCharTraits, class TAlloc>
[[nodiscard]] EckInline bool operator>(const EckCRefStrTemp& rs1, const TChar* psz2)
{
	if (!rs1.Data() && !psz2)
		return false;
	else if (!rs1.Data() && psz2)
		return true;
	else if (rs1.Data() && !psz2)
		return false;
	else
		return TCharTraits::Compare(rs1.Data(), psz2) > 0;
}

template<class TChar, class TCharTraits, class TAlloc>
[[nodiscard]] EckInline bool operator<=(const EckCRefStrTemp& rs1, const TChar* psz2)
{
	if (!rs1.Data() && !psz2)
		return true;
	else if (!rs1.Data() && psz2)
		return false;
	else if (rs1.Data() && !psz2)
		return true;
	else
		return TCharTraits::Compare(rs1.Data(), psz2) <= 0;
}

template<class TChar, class TCharTraits, class TAlloc>
[[nodiscard]] EckInline bool operator>=(const EckCRefStrTemp& rs1, const TChar* psz2)
{
	if (!rs1.Data() && !psz2)
		return true;
	else if (!rs1.Data() && psz2)
		return true;
	else if (rs1.Data() && !psz2)
		return false;
	else
		return TCharTraits::Compare(rs1.Data(), psz2) >= 0;
}
#endif// ECKCXX20

template<class TChar, class TCharTraits, class TAlloc>
[[nodiscard]] EckInline bool operator==(const EckCRefStrTemp& rs1, const EckCRefStrTemp& rs2)
{
	return operator==(rs1, rs2.Data());
}

#if ECKCXX20
template<class TChar, class TCharTraits, class TAlloc>
[[nodiscard]] EckInline std::weak_ordering operator<=>(const EckCRefStrTemp& rs1, const EckCRefStrTemp& rs2)
{
	return operator<=>(rs1, rs2.Data());
}
#else// !ECKCXX20
template<class TChar, class TCharTraits, class TAlloc>
[[nodiscard]] EckInline bool operator<(const EckCRefStrTemp& rs1, const EckCRefStrTemp& rs2)
{
	return operator<(rs1, rs2.Data());
}

template<class TChar, class TCharTraits, class TAlloc>
[[nodiscard]] EckInline bool operator>(const EckCRefStrTemp& rs1, const EckCRefStrTemp& rs2)
{
	return operator>(rs1, rs2.Data());
}

template<class TChar, class TCharTraits, class TAlloc>
[[nodiscard]] EckInline bool operator<=(const EckCRefStrTemp& rs1, const EckCRefStrTemp& rs2)
{
	return operator<=(rs1, rs2.Data());
}

template<class TChar, class TCharTraits, class TAlloc>
[[nodiscard]] EckInline bool operator>=(const EckCRefStrTemp& rs1, const EckCRefStrTemp& rs2)
{
	return operator>=(rs1, rs2.Data());
}
#endif// ECKCXX20
#pragma endregion Operator

template<class TCharTraits, class TAlloc>
EckInline void DbgPrint(const CRefStrT<WCHAR, TCharTraits, TAlloc>& rs, int iType = 1, BOOL bNewLine = TRUE)
{
	OutputDebugStringW(rs.Data());
	if (bNewLine)
		OutputDebugStringW(L"\n");
}

template<class TCharTraits, class TAlloc>
EckInline void DbgPrint(const CRefStrT<CHAR, TCharTraits, TAlloc>& rs, int iType = 1, BOOL bNewLine = TRUE)
{
	OutputDebugStringA(rs.Data());
	if (bNewLine)
		OutputDebugStringA("\n");
}

[[nodiscard]] EckInline CRefStrW ToStr(int x, int iRadix = 10)
{
	CRefStrW rs(CchI32ToStrBuf);
	_itow(x, rs.Data(), iRadix);
	rs.ReCalcLen();
	return rs;
}

[[nodiscard]] EckInline CRefStrW ToStr(UINT x, int iRadix = 10)
{
	CRefStrW rs(CchI32ToStrBuf);
	_ultow(x, rs.Data(), iRadix);
	rs.ReCalcLen();
	return rs;
}

[[nodiscard]] EckInline CRefStrW ToStr(LONGLONG x, int iRadix = 10)
{
	CRefStrW rs(CchI64ToStrBuf);
	_i64tow(x, rs.Data(), iRadix);
	rs.ReCalcLen();
	return rs;
}

[[nodiscard]] EckInline CRefStrW ToStr(ULONGLONG x, int iRadix = 10)
{
	CRefStrW rs(CchI64ToStrBuf);
	_ui64tow(x, rs.Data(), iRadix);
	rs.ReCalcLen();
	return rs;
}

[[nodiscard]] EckInline CRefStrW ToStr(double x, int iPrecision = 6)
{
	CRefStrW rs{};
	rs.Format(L"%.*g", iPrecision, x);
	return rs;
}

namespace Literals
{
	EckInline auto operator""_rs(PCWSTR psz, size_t cch)
	{
		return CRefStrW(psz, (int)cch);
	}

	EckInline auto operator""_rs(PCSTR psz, size_t cch)
	{
		return CRefStrA(psz, (int)cch);
	}
}

/// <summary>
/// 到小写
/// </summary>
/// <param name="pszText">字符串指针</param>
/// <param name="cchText">字符串长度</param>
/// <returns></returns>
[[nodiscard]] EckInline CRefStrW ToLowerCase(PCWSTR pszText, int cchText = -1)
{
	CRefStrW rs;
	int cchResult = LCMapStringEx(LOCALE_NAME_USER_DEFAULT, LCMAP_LOWERCASE, pszText, cchText, nullptr, 0, nullptr, nullptr, 0);
	rs.ReSize(cchResult);
	LCMapStringEx(LOCALE_NAME_USER_DEFAULT, LCMAP_LOWERCASE, pszText, cchText, rs.Data(), cchResult, nullptr, nullptr, 0);
	return rs;
}

/// <summary>
/// 到大写
/// </summary>
/// <param name="pszText">字符串指针</param>
/// <param name="cchText">字符串长度</param>
/// <returns></returns>
[[nodiscard]] EckInline CRefStrW ToUpperCase(PCWSTR pszText, int cchText = -1)
{
	CRefStrW rs;
	int cchResult = LCMapStringEx(LOCALE_NAME_USER_DEFAULT, LCMAP_UPPERCASE, pszText, cchText, nullptr, 0, nullptr, nullptr, 0);
	rs.ReSize(cchResult);
	LCMapStringEx(LOCALE_NAME_USER_DEFAULT, LCMAP_UPPERCASE, pszText, cchText, rs.Data(), cchResult, nullptr, nullptr, 0);
	return rs;
}

/// <summary>
/// 到小写
/// </summary>
/// <param name="pszText">字符串指针</param>
/// <param name="cchText">字符串长度</param>
/// <returns></returns>
[[nodiscard]] EckInline CRefStrW ToLowerCaseCrt(PCWSTR pszText, int cchText = -1)
{
	if (cchText < 0)
		cchText = (int)wcslen(pszText);
	CRefStrW rs(cchText);
	EckCounter(cchText, i)
		rs[i] = towlower(pszText[i]);
	return rs;
}

/// <summary>
/// 到大写
/// </summary>
/// <param name="pszText">字符串指针</param>
/// <param name="cchText">字符串长度</param>
/// <returns></returns>
[[nodiscard]] EckInline CRefStrW ToUpperCaseCrt(PCWSTR pszText, int cchText = -1)
{
	if (cchText < 0)
		cchText = (int)wcslen(pszText);
	CRefStrW rs(cchText);
	EckCounter(cchText, i)
		rs[i] = towupper(pszText[i]);
	return rs;
}

/// <summary>
/// 到小写
/// </summary>
/// <param name="pszText">字符串指针</param>
/// <param name="cchText">字符串长度</param>
/// <returns></returns>
[[nodiscard]] EckInline CRefStrA ToLowerCaseCrt(PCSTR pszText, int cchText = -1)
{
	if (cchText < 0)
		cchText = (int)strlen(pszText);
	CRefStrA rs(cchText);
	EckCounter(cchText, i)
		rs[i] = tolower(pszText[i]);
	return rs;
}

/// <summary>
/// 到大写
/// </summary>
/// <param name="pszText">字符串指针</param>
/// <param name="cchText">字符串长度</param>
/// <returns></returns>
[[nodiscard]] EckInline CRefStrA ToUpperCaseCrt(PCSTR pszText, int cchText = -1)
{
	if (cchText < 0)
		cchText = (int)strlen(pszText);
	CRefStrA rs(cchText);
	EckCounter(cchText, i)
		rs[i] = toupper(pszText[i]);
	return rs;
}

/// <summary>
/// 取文本左边
/// </summary>
/// <param name="pszText">字符串指针</param>
/// <param name="cchLeft">左边长度</param>
/// <returns></returns>
[[nodiscard]] EckInline CRefStrW StrLeft(PCWSTR pszText, int cchLeft)
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
[[nodiscard]] EckInline CRefStrW StrRight(PCWSTR pszText, int cchRight, int cchText = -1)
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
[[nodiscard]] EckInline CRefStrW StrMid(PCWSTR pszText, int posStart, int cchMid)
{
	return CRefStrW(pszText + posStart, cchMid);
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
/// <returns>位置，若未找到返回StrNPos</returns>
[[nodiscard]] EckInline int FindStrNcs(PCWSTR pszText, int cchText, PCWSTR pszSub, int cchSub, int posStart = 0)
{
	return FindStr(ToUpperCaseCrt(pszText, cchText).Data(), ToUpperCaseCrt(pszSub, cchSub).Data(), posStart);
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
/// <returns>位置，若未找到返回StrNPos</returns>
[[nodiscard]] EckInline int FindStrNcs(PCSTR pszText, int cchText, PCSTR pszSub, int cchSub, int posStart = 0)
{
	return FindStr(ToUpperCaseCrt(pszText, cchText).Data(), ToUpperCaseCrt(pszSub, cchSub).Data(), posStart);
}

/// <summary>
/// 寻找文本。
/// 不区分大小写。
/// 函数先将整个字符串转换为大写，然后对其调用FindStr
/// </summary>
/// <param name="rsText">要在其中寻找的字符串</param>
/// <param name="rsSub">要寻找的字符串</param>
/// <param name="posStart">起始位置</param>
/// <returns>位置，若未找到返回StrNPos</returns>
[[nodiscard]] EckInline int FindStrNcs(const CRefStrW& rsText, const CRefStrW& rsSub, int posStart = 0)
{
	return FindStrNcs(rsText.Data(), rsText.Size(), rsSub.Data(), posStart, rsSub.Size());
}

/// <summary>
/// 寻找文本。
/// 不区分大小写。
/// 函数先将整个字符串转换为大写，然后对其调用FindStr
/// </summary>
/// <param name="rsText">要在其中寻找的字符串</param>
/// <param name="rsSub">要寻找的字符串</param>
/// <param name="posStart">起始位置</param>
/// <returns>位置，若未找到返回StrNPos</returns>
[[nodiscard]] EckInline int FindStrNcs(const CRefStrA& rsText, const CRefStrA& rsSub, int posStart = 0)
{
	return FindStrNcs(rsText.Data(), rsText.Size(), rsSub.Data(), posStart, rsSub.Size());
}

/// <summary>
/// 倒找文本
/// </summary>
/// <param name="rsText">要在其中寻找的字符串</param>
/// <param name="rsSub">要寻找的字符串</param>
/// <param name="posStart">起始位置</param>
/// <returns>位置，若未找到返回StrNPos</returns>
[[nodiscard]] EckInline int FindStrRev(const CRefStrW& rsText, const CRefStrW& rsSub, int posStart = -1)
{
	return FindStrRev(rsText.Data(), rsText.Size(), rsSub.Data(), posStart, rsSub.Size());
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
/// <returns>位置，若未找到返回StrNPos</returns>
[[nodiscard]] EckInline int FindStrRevNcs(PCWSTR pszText, int cchText, PCWSTR pszSub, int cchSub, int posStart = -1)
{
	auto rsText = ToUpperCase(pszText, cchText), rsSub = ToUpperCase(pszSub, cchSub);
	return FindStrRev(rsText.Data(), rsText.Size(), rsSub.Data(), posStart, rsSub.Size());
}

/// <summary>
/// 倒找文本。
/// 不区分大小写。
/// 函数先将整个字符串转换为大写，然后对其调用FindStrRev
/// </summary>
/// <param name="rsText">要在其中寻找的字符串</param>
/// <param name="rsSub">要寻找的字符串</param>
/// <param name="posStart">起始位置</param>
/// <returns>位置，若未找到返回StrNPos</returns>
[[nodiscard]] EckInline int FindStrRevNcs(const CRefStrW& rsText, const CRefStrW& rsSub, int posStart = -1)
{
	return FindStrRevNcs(rsText.Data(), rsText.Size(), rsSub.Data(), posStart, rsSub.Size());
}

template<class TProcesser, class TChar>
inline void SplitStr(const TChar* pszText, const TChar* pszDiv, int cSubTextExpected, int cchText,
	int cchDiv, TProcesser&& Processer)
{
	if (cchText < 0)
		cchText = CCharTraits<TChar>::Len(pszText);
	if (cchDiv < 0)
		cchDiv = CCharTraits<TChar>::Len(pszDiv);
	if (cSubTextExpected <= 0)
		cSubTextExpected = INT_MAX;

	auto pszFind = CCharTraits<TChar>::Find2(pszText, pszDiv);
	auto pszPrevFirst = pszText;
	int c = 0;
	while (pszFind)
	{
		if (Processer(pszPrevFirst, (int)(pszFind - pszPrevFirst)))
			return;
		++c;
		if (c == cSubTextExpected)
			return;
		pszPrevFirst = pszFind + cchDiv;
		pszFind = CCharTraits<TChar>::Find2(pszPrevFirst, pszDiv);
	}

	Processer(pszPrevFirst, (int)(pszText + cchText - pszPrevFirst));
}

template<class TProcesser>
inline void SplitStrWithMultiChar(PCWSTR pszText, PCWSTR pszDiv, int cSubTextExpected, int cchText,
	int cchDiv, TProcesser&& Processer)
{
	if (cchText < 0)
		cchText = (int)wcslen(pszText);
	if (cchDiv < 0)
		cchDiv = (int)wcslen(pszDiv);
	if (cSubTextExpected <= 0)
		cSubTextExpected = INT_MAX;

	PCWSTR pszFind = wcspbrk(pszText, pszDiv);
	PCWSTR pszPrevFirst = pszText;
	int c = 0;
	while (pszFind)
	{
		if (Processer(pszPrevFirst, (int)(pszFind - pszPrevFirst)))
			return;
		++c;
		if (c == cSubTextExpected)
			return;
		pszPrevFirst = pszFind + 1;
		pszFind = wcspbrk(pszPrevFirst, pszDiv);
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
EckInline void SplitStr(PCWSTR pszText, PCWSTR pszDiv, std::vector<CRefStrW>& aResult,
	int cSubTextExpected = 0, int cchText = -1, int cchDiv = -1)
{
	SplitStr(pszText, pszDiv, cSubTextExpected, cchText, cchDiv, [&](PCWSTR pszStart, int cchSub)
		{
			aResult.push_back(CRefStrW(pszStart, cchSub));
			return FALSE;
		});
}

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
EckInline void SplitStr(PCWSTR pszText, PCWSTR pszDiv, std::vector<SPLTEXTINFO>& aResult,
	int cSubTextExpected = 0, int cchText = -1, int cchDiv = -1)
{
	SplitStr(pszText, pszDiv, cSubTextExpected, cchText, cchDiv, [&](PCWSTR pszStart, int cchSub)
		{
			aResult.push_back({ pszStart,cchSub });
			return FALSE;
		});
}

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
EckInline void SplitStr(PWSTR pszText, PCWSTR pszDiv, std::vector<PWSTR>& aResult,
	int cSubTextExpected = 0, int cchText = -1, int cchDiv = -1)
{
	SplitStr(pszText, pszDiv, cSubTextExpected, cchText, cchDiv, [&](PCWSTR pszStart, int cchSub)
		{
			PWSTR psz = (PWSTR)pszStart;
			*(psz + cchSub) = L'\0';
			aResult.push_back(psz);
			return FALSE;
		});
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
EckInline void SplitStrWithMultiChar(PCWSTR pszText, PCWSTR pszDiv, std::vector<CRefStrW>& aResult,
	int cSubTextExpected = 0, int cchText = -1, int cchDiv = -1)
{
	SplitStrWithMultiChar(pszText, pszDiv, cSubTextExpected, cchText, cchDiv, [&](PCWSTR pszStart, int cchSub)
		{
			aResult.push_back(CRefStrW(pszStart, cchSub));
			return FALSE;
		});
}

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
EckInline void SplitStrWithMultiChar(PCWSTR pszText, PCWSTR pszDiv, std::vector<SPLTEXTINFO>& aResult,
	int cSubTextExpected = 0, int cchText = -1, int cchDiv = -1)
{
	SplitStrWithMultiChar(pszText, pszDiv, cSubTextExpected, cchText, cchDiv, [&](PCWSTR pszStart, int cchSub)
		{
			aResult.push_back({ pszStart,cchSub });
			return FALSE;
		});
}

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
EckInline void SplitStrWithMultiChar(PWSTR pszText, PCWSTR pszDiv, std::vector<PWSTR>& aResult,
	int cSubTextExpected = 0, int cchText = -1, int cchDiv = -1)
{
	SplitStrWithMultiChar(pszText, pszDiv, cSubTextExpected, cchText, cchDiv, [&](PCWSTR pszStart, int cchSub)
		{
			PWSTR psz = (PWSTR)pszStart;
			*(psz + cchSub) = L'\0';
			aResult.push_back(psz);
			return FALSE;
		});
}

/// <summary>
/// 到全角
/// </summary>
/// <param name="pszText">原始文本</param>
/// <param name="cchText">字符数</param>
/// <returns>转换结果</returns>
[[nodiscard]] EckInline CRefStrW ToFullWidth(PCWSTR pszText, int cchText = -1)
{
	CRefStrW rs;
	int cchResult = LCMapStringEx(LOCALE_NAME_USER_DEFAULT, LCMAP_FULLWIDTH, pszText, cchText, nullptr, 0, nullptr, nullptr, 0);
	rs.ReSize(cchResult);
	LCMapStringEx(LOCALE_NAME_USER_DEFAULT, LCMAP_FULLWIDTH, pszText, cchText, rs.Data(), cchResult, nullptr, nullptr, 0);
	return rs;
}

/// <summary>
/// 到半角
/// </summary>
/// <param name="pszText">原始文本</param>
/// <param name="cchText">字符数</param>
/// <returns>转换结果</returns>
[[nodiscard]] EckInline CRefStrW ToHalfWidth(PCWSTR pszText, int cchText = -1)
{
	CRefStrW rs;
	int cchResult = LCMapStringEx(LOCALE_NAME_USER_DEFAULT, LCMAP_HALFWIDTH, pszText, cchText, nullptr, 0, nullptr, nullptr, 0);
	rs.ReSize(cchResult);
	LCMapStringEx(LOCALE_NAME_USER_DEFAULT, LCMAP_HALFWIDTH, pszText, cchText, rs.Data(), cchResult, nullptr, nullptr, 0);
	return rs;
}

/// <summary>
/// 删全部空
/// </summary>
/// <param name="pszText">原始文本</param>
/// <param name="cchText">文本长度</param>
/// <returns>处理结果</returns>
[[nodiscard]] inline CRefStrW AllTrimStr(PCWSTR pszText, int cchText = -1)
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
		posTemp = [](PCWSTR pszText)->int
			{
				PCWSTR pszOrg = pszText;
				WCHAR ch = *pszText;
				while (ch != L' ' && ch != L'　' && ch != L'\0')
					ch = *++pszText;
				return (int)(pszText - pszOrg);
			}(pszText);
		rs.PushBack(pszText, posTemp);

		posTemp += (int)(LTrimStr(pszText) - pszText);
		pszText += posTemp;
	}

	return rs;
}

template<class TCharTraits = CCharTraits<CHAR>, class TAlloc = TRefStrDefAlloc<CHAR>>
[[nodiscard]] CRefStrT<CHAR, TCharTraits, TAlloc> StrW2X(PCWSTR pszText, int cch = -1, int uCP = CP_ACP)
{
	int cchBuf = WideCharToMultiByte(uCP, WC_COMPOSITECHECK, pszText, cch, nullptr, 0, nullptr, nullptr);
	if (!cchBuf)
		return {};
	if (cch == -1)
		--cchBuf;
	CRefStrT<CHAR, TCharTraits, TAlloc> rs(cchBuf);
	WideCharToMultiByte(uCP, WC_COMPOSITECHECK, pszText, cch, rs.Data(), cchBuf, nullptr, nullptr);
	*(rs.Data() + cchBuf) = '\0';
	return rs;
}

template<class TCharTraits = CCharTraits<CHAR>, class TAlloc = TRefStrDefAlloc<CHAR>,
	class T, class U>
[[nodiscard]] EckInline CRefStrT<CHAR, TCharTraits, TAlloc> StrW2X(
	const CRefStrT<WCHAR, T, U>& rs, int uCP = CP_ACP)
{
	return StrW2X(rs.Data(), rs.Size(), uCP);
}

template<class TCharTraits = CCharTraits<WCHAR>, class TAlloc = TRefStrDefAlloc<WCHAR>>
[[nodiscard]] CRefStrT<WCHAR, TCharTraits, TAlloc> StrX2W(PCSTR pszText, int cch = -1, int uCP = CP_ACP)
{
	int cchBuf = MultiByteToWideChar(uCP, MB_PRECOMPOSED, pszText, cch, nullptr, 0);
	if (!cchBuf)
		return {};
	if (cch == -1)
		--cchBuf;
	CRefStrT<WCHAR, TCharTraits, TAlloc> rs(cchBuf);
	MultiByteToWideChar(uCP, MB_PRECOMPOSED, pszText, cch, rs.Data(), cchBuf);
	*(rs.Data() + cchBuf) = '\0';
	return rs;
}

template<class TCharTraits = CCharTraits<WCHAR>, class TAlloc = TRefStrDefAlloc<WCHAR>,
	class T, class U>
[[nodiscard]] EckInline CRefStrT<WCHAR, TCharTraits, TAlloc> StrX2W(
	const CRefStrT<CHAR, T, U>& rs, int uCP = CP_ACP)
{
	return StrX2W(rs.Data(), rs.Size(), uCP);
}

/// <summary>
/// 字节集到友好字符串表示
/// </summary>
/// <param name="Bin">字节集</param>
/// <param name="iType">类型，0 - 空格分割的十六进制  1 - 易语言字节集调试输出</param>
/// <returns>返回结果</returns>
inline CRefStrW BinToFriendlyString(PCBYTE pData, SIZE_T cb, int iType)
{
	CRefStrW rsResult{};
	if (!pData || !cb)
	{
		if (iType == 1)
			rsResult = L"{ }";
		return rsResult;
	}

	switch (iType)
	{
	case 0:
	{
		rsResult.Reserve((int)cb * 3 + 10);
		for (SIZE_T i = 0u; i < cb; ++i)
			rsResult.AppendFormat(L"%02hhX ", pData[i]);
	}
	break;
	case 1:
	{
		rsResult.Reserve((int)cb * 4 + 10);
		rsResult.PushBack(L"{ ");
		rsResult.AppendFormat(L"%hhu", pData[0]);
		for (SIZE_T i = 1u; i < cb; ++i)
			rsResult.AppendFormat(L"%hhu", pData[0]);
		rsResult.PushBack(L" }");
	}
	break;
	}

	return rsResult;
}

EckInline CRefStrW Format(PCWSTR pszFmt, ...)
{
	CRefStrW rs{};
	va_list vl;
	va_start(vl, pszFmt);
	rs.FormatV(pszFmt, vl);
	va_end(vl);
	return rs;
}

EckInline CRefStrA Format(PCSTR pszFmt, ...)
{
	CRefStrA rs{};
	va_list vl;
	va_start(vl, pszFmt);
	rs.FormatV(pszFmt, vl);
	va_end(vl);
	return rs;
}


#undef EckCRefStrTemp
ECK_NAMESPACE_END

template<class TChar, class TCharTraits, class TAlloc>
struct std::hash<::eck::CRefStrT<TChar, TCharTraits, TAlloc>>
{
	[[nodiscard]] EckInline constexpr size_t operator()(
		const ::eck::CRefStrT<TChar, TCharTraits, TAlloc>& rs) const noexcept
	{
		return ::eck::Fnv1aHash((::eck::PCBYTE)rs.Data(), rs.Size() * sizeof(TChar));
	}
};