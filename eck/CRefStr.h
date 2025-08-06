#pragma once
#pragma warning (disable:4996)
#include "StringUtility.h"
#include "CAllocator.h"
#include "Utility.h"

ECK_NAMESPACE_BEGIN
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
	EckInline static constexpr void AssignChar(PWSTR psz, WCHAR ch) { *psz = ch; }
	EckInline static constexpr WCHAR CharTerminatingNull() { return L'\0'; }
	EckInline static constexpr void Cut(PWSTR psz, int cch) { AssignChar(psz + cch, CharTerminatingNull()); }
	EckInline static constexpr WCHAR CharSpace() { return L' '; }
	EckInline static int FormatV(PWSTR pszBuf, PCWSTR pszFmt, va_list vl) { return vswprintf(pszBuf, pszFmt, vl); }
	EckInline static int GetFormatCchV(PCWSTR pszFmt, va_list vl) { return _vscwprintf(pszFmt, vl); }
};

template<>
struct CCharTraits<CHAR>
{
	using TChar = CHAR;
	EckInline static constexpr void AssignChar(PSTR psz, CHAR ch) { *psz = ch; }
	EckInline static constexpr CHAR CharTerminatingNull() { return '\0'; }
	EckInline static constexpr void Cut(PSTR psz, int cch) { AssignChar(psz + cch, CharTerminatingNull()); }
	EckInline static constexpr CHAR CharSpace() { return ' '; }
	EckInline static int FormatV(PSTR pszBuf, PCSTR pszFmt, va_list vl) { return vsprintf(pszBuf, pszFmt, vl); }
	EckInline static int GetFormatCchV(PCSTR pszFmt, va_list vl) { return _vscprintf(pszFmt, vl); }
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
	constexpr static int EnsureNotLocalBufferSize = ARRAYSIZE(m_szLocal) * 3 / 2;

	CRefStrT() = default;

	explicit CRefStrT(const TAlloc& Al) : m_Alloc{ Al } {}

	explicit CRefStrT(int cchInit)
	{
		ReSize(cchInit);
	}

	CRefStrT(int cchInit, const TAlloc& Al) : m_Alloc{ Al }
	{
		ReSize(cchInit);
	}

	CRefStrT(TConstPointer psz, int cchText, const TAlloc& Al = TAlloc{}) : m_Alloc{ Al }
	{
		DupString(psz, cchText);
	}

	CRefStrT(TConstPointer psz, const TAlloc& Al = TAlloc{})
		: CRefStrT(psz, psz ? (int)TcsLen(psz) : 0, Al) {
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
	CRefStrT(const std::basic_string<TChar, TTraits, TAlloc1>& s, const TAlloc& Al = TAlloc{})
		: CRefStrT(s.data(), (int)s.size(), Al) {
	}

	template<class TTraits>
	CRefStrT(std::basic_string_view<TChar, TTraits> sv, const TAlloc& Al = TAlloc{})
		: CRefStrT(sv.data(), (int)sv.size(), Al) {
	}

	CRefStrT(TNtString nts, const TAlloc& Al = TAlloc{})
		:CRefStrT(nts.Buffer, (int)nts.Length, Al) {
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

	EckInline CRefStrT& operator=(TNtString x) { DupString(x.Buffer, (int)x.Length); }

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
			cchSrc = (int)TcsLen(pszSrc);
		ReSizeExtra(cchSrc);
		TcsCopyLenEnd(Data(), pszSrc, cchSrc);
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
		m_Alloc.deallocate(m_pszText, std::max(m_cchCapacity, LocalBufferSize + 1));
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
		if (IsLocal())
		{
			cchCapacity = m_cchCapacity;
			const auto p = m_Alloc.allocate(cchCapacity);
			// 必须在分配完毕后更新cchText，因为cchCapacity与cchText可能为同一个变量
			cchText = m_cchText;
			TcsCopyLen(p, m_szLocal, m_cchText + 1);
			return p;
		}
		else
		{
			const auto pOld = m_pszText;
			m_pszText = nullptr;

			cchCapacity = m_cchCapacity;
			m_cchCapacity = 0;

			cchText = m_cchText;
			m_cchText = 0;
			return pOld;
		}
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
			cchSrc = (int)TcsLen(pszSrc);
		ReSizeExtra(Size() + cchSrc);
		TcsCopyLenEnd(Data() + Size() - cchSrc, pszSrc, cchSrc);
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

	EckInline CRefStrT& PushBackChar(TChar ch)
	{
		ReSizeExtra(Size() + 1);
		*(Data() + Size() - 1) = ch;
		return *this;
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
	EckInline CRefStrT& PopBack(int cch = 1)
	{
		EckAssert(Size() >= cch && cch >= 0);
		if (!cch)
			return *this;
		m_cchText -= cch;
		TCharTraits::Cut(Data(), Size());
		return *this;
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
		TcsCopyLenEnd(pszDst, Data(), cch);
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
			TcsCopyLen(pNew, pOld, m_cchText + 1);// 多拷一个结尾NULL
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
		return m_cchText = (int)TcsLen(Data());
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
			cchNew = (int)TcsLen(pszNew);
		const int cchOrg = Size();
		const int cchAfter = Size() + cchNew - cchReplacing;
		Reserve(cchAfter);
		TcsMoveLen(
			Data() + posStart + cchNew,
			Data() + posStart + cchReplacing,
			cchOrg - posStart - cchReplacing);
		if (pszNew)
			TcsCopyLen(Data() + posStart, pszNew, cchNew);
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
			cchReplaced = (int)TcsLen(pszReplaced);
		if (cchSrc < 0)
			cchSrc = (int)TcsLen(pszSrc);
		int pos = 0;
		for (int c = 1;; ++c)
		{
			pos = FindStrLen(Data(), Size(), pszReplaced, cchReplaced, posStart + pos);
			if (pos < 0)
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

	EckInline void MakeSpace(int cch, int posStart = 0)
	{
		ReSize(posStart + cch);
		TcsSet(Data() + posStart, cch, TCharTraits::CharSpace());
	}

	void MakeRepeatedSequence(int cCount, TConstPointer pszText,
		int cchText = -1, int posStart = 0)
	{
		EckAssert(pszText);
		if (cchText < 0)
			cchText = (int)TcsLen(pszText);
		ReSize(posStart + cchText * cCount);
		auto pszCurr = Data() + posStart;
		for (int i = 0; i < cCount; ++i, pszCurr += cchText)
			TcsCopyLen(pszCurr, pszText, cchText);
		TCharTraits::Cut(Data(), Size());
	}
	EckInline void MakeRepeatedSequence(int cCount, const CRefStrT& rsText, int posStart = 0)
	{
		MakeRepeatedStrSequence(rsText.Data(), rsText.Size(), cCount, posStart);
	}

	EckInline constexpr void Clear()
	{
		m_cchText = 0;
		TCharTraits::Cut(Data(), 0);
	}

	EckInline void Insert(int pos, TConstPointer pszText, int cchText = -1)
	{
		EckAssert(pos <= Size() && pos >= 0);
		EckAssert(pszText ? TRUE : (cchText == 0));
		if (cchText < 0)
			cchText = (int)TcsLen(pszText);
		ReSizeExtra(Size() + cchText);
		TcsMoveLenEnd(
			Data() + pos + cchText,
			Data() + pos,
			Size() - cchText - pos);
		TcsCopyLen(Data() + pos, pszText, cchText);
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
		TcsMoveLenEnd(
			Data() + pos + cchText,
			Data() + pos,
			Size() - cchText - pos);
		return Data() + pos;
	}

	EckInline void InsertChar(int pos, TChar ch)
	{
		EckAssert(pos <= Size() && pos >= 0);
		ReSizeExtra(Size() + 1);
		TcsMoveLenEnd(
			Data() + pos + 1,
			Data() + pos,
			Size() - 1 - pos);
		TCharTraits::AssignChar(Data() + pos, ch);
	}

	EckInline void Erase(int pos, int cch = 1)
	{
		EckAssert(Size() >= pos + cch);
		TcsMoveLenEnd(
			Data() + pos,
			Data() + pos + cch,
			Size() - pos - cch);
		m_cchText -= cch;
	}

	void ShrinkToFit()
	{
		EckAssert(m_cchCapacity >= m_cchText + 1);
		if (m_cchCapacity == m_cchText + 1)
			return;
		const auto pOld = m_pszText;
		m_pszText = m_Alloc.allocate(m_cchText + 1);
		TcsCopyLen(Data(), pOld, m_cchText + 1);
		m_Alloc.deallocate(pOld, m_cchCapacity);
		m_cchCapacity = m_cchText + 1;
	}

	void ExtendToCapacity()
	{
		if (Capacity())
		{
			m_cchText = Capacity() - 1;
			TCharTraits::Cut(Data(), Size());
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

	EckInlineNd int Find(TConstPointer pszSub, int cchSub = -1, int posStart = 0) const
	{
		if (cchSub < 0)
			cchSub = (int)TcsLen(pszSub);
		return FindStrLen(Data(), Size(), pszSub, cchSub, posStart);
	}
	template<class TTraits, class TAlloc1>
	EckInlineNd int Find(const CRefStrT<TChar, TTraits, TAlloc1>& rs, int posStart = 0) const
	{
		return Find(rs.Data(), rs.Size(), posStart);
	}
	template<class TTraits, class TAlloc1>
	EckInlineNd int Find(const std::basic_string<TChar, TTraits, TAlloc1>& s, int posStart = 0) const
	{
		return Find(s.data(), (int)s.size(), posStart);
	}
	template<class TTraits>
	EckInlineNd int Find(std::basic_string_view<TChar, TTraits> sv, int posStart = 0) const
	{
		return Find(sv.data(), (int)sv.size(), posStart);
	}

	EckInlineNd int FindI(TConstPointer pszSub, int cchSub = -1, int posStart = 0) const
	{
		if (cchSub < 0)
			cchSub = (int)TcsLen(pszSub);
		return FindStrLenI(Data(), Size(), pszSub, cchSub, posStart);
	}
	template<class TTraits, class TAlloc1>
	EckInlineNd int FindI(const CRefStrT<TChar, TTraits, TAlloc1>& rs, int posStart = 0) const
	{
		return FindI(rs.Data(), rs.Size(), posStart);
	}
	template<class TTraits, class TAlloc1>
	EckInlineNd int FindI(const std::basic_string<TChar, TTraits, TAlloc1>& s, int posStart = 0) const
	{
		return FindI(s.data(), (int)s.size(), posStart);
	}
	template<class TTraits>
	EckInlineNd int FindI(std::basic_string_view<TChar, TTraits> sv, int posStart = 0) const
	{
		return FindI(sv.data(), (int)sv.size(), posStart);
	}

	EckInlineNd int RFind(TConstPointer pszSub, int cchSub = -1, int posStart = -1) const
	{
		if (cchSub < 0)
			cchSub = (int)TcsLen(pszSub);
		return FindStrRev(Data(), Size(), pszSub, cchSub, posStart);
	}
	template<class TTraits, class TAlloc1>
	EckInlineNd int RFind(const CRefStrT<TChar, TTraits, TAlloc1>& rs, int posStart = -1) const
	{
		return RFind(rs.Data(), rs.Size(), posStart);
	}
	template<class TTraits, class TAlloc1>
	EckInlineNd int RFind(const std::basic_string<TChar, TTraits, TAlloc1>& s, int posStart = -1) const
	{
		return RFind(s.data(), (int)s.size(), posStart);
	}
	template<class TTraits>
	EckInlineNd int RFind(std::basic_string_view<TChar, TTraits> sv, int posStart = -1) const
	{
		return RFind(sv.data(), (int)sv.size(), posStart);
	}

	EckInlineNd int RFindI(TConstPointer pszSub, int cchSub = -1, int posStart = -1) const
	{
		if (cchSub < 0)
			cchSub = (int)TcsLen(pszSub);
		return FindStrRevI(Data(), Size(), pszSub, cchSub, posStart);
	}
	template<class TTraits, class TAlloc1>
	EckInlineNd int RFindI(const CRefStrT<TChar, TTraits, TAlloc1>& rs, int posStart = -1) const
	{
		return RFindI(rs.Data(), rs.Size(), posStart);
	}
	template<class TTraits, class TAlloc1>
	EckInlineNd int RFindI(const std::basic_string<TChar, TTraits, TAlloc1>& s, int posStart = -1) const
	{
		return RFindI(s.data(), (int)s.size(), posStart);
	}
	template<class TTraits>
	EckInlineNd int RFindI(std::basic_string_view<TChar, TTraits> sv, int posStart = -1) const
	{
		return RFindI(sv.data(), (int)sv.size(), posStart);
	}

	EckInlineNd int FindChar(TChar ch, int posStart = 0) const
	{
		if (IsEmpty())
			return StrNPos;
		return FindCharLen(Data(), Size(), ch, posStart);
	}
	EckInlineNd int RFindChar(TChar ch, int posStart = -1) const
	{
		if (IsEmpty())
			return StrNPos;
		return FindCharRevLen(Data(), Size(), ch, posStart);
	}

	EckInlineNd int FindFirstOf(TConstPointer pszChars, int cchChars = -1, int posStart = 0) const
	{
		if (cchChars < 0)
			cchChars = (int)TcsLen(pszChars);
		return FindCharFirstOf(Data(), Size(), pszChars, cchChars, posStart);
	}
	EckInlineNd int FindFirstNotOf(TConstPointer pszChars, int cchChars = -1, int posStart = 0) const
	{
		if (cchChars < 0)
			cchChars = (int)TcsLen(pszChars);
		return FindCharFirstNotOf(Data(), Size(), pszChars, cchChars, posStart);
	}
	EckInlineNd int FindLastOf(TConstPointer pszChars, int cchChars = -1, int posStart = -1) const
	{
		if (cchChars < 0)
			cchChars = (int)TcsLen(pszChars);
		return FindCharLastOf(Data(), Size(), pszChars, cchChars, posStart);
	}
	EckInlineNd int FindLastNotOf(TConstPointer pszChars, int cchChars = -1, int posStart = -1) const
	{
		if (cchChars < 0)
			cchChars = (int)TcsLen(pszChars);
		return FindCharLastNotOf(Data(), Size(), pszChars, cchChars, posStart);
	}

	void LTrim()
	{
		if (IsEmpty())
			return;
		const auto pszBegin = LTrimStr(Data(), Size());
		const int cchNew = Size() - int(pszBegin - Data());
		TcsMoveLen(Data(), pszBegin, cchNew);
		ReSize(cchNew);
	}
	void RTrim()
	{
		if (IsEmpty())
			return;
		const auto pszEnd = RTrimStr(Data(), Size());
		ReSize(int(pszEnd - Data()));
	}
	void LRTrim()
	{
		if (IsEmpty())
			return;
		const auto pszBegin = LTrimStr(Data(), Size());
		const auto pszEnd = RTrimStr(Data(), Size());
		if (pszEnd < pszBegin)
			Clear();
		else
		{
			const int cchNew = int(pszEnd - pszBegin);
			TcsMoveLen(Data(), pszBegin, cchNew);
			ReSize(cchNew);
		}
	}
	void AllTrim()
	{
		if (IsEmpty())
			return;
		const auto pData = Data();
		TPointer p0, p1{ pData }, pCurr{ pData };
		EckLoop()
		{
			if constexpr (std::is_same_v<TChar, WCHAR>)
				p0 = TcsChrFirstNotOf(p1, Size() - (p1 - pData), EckStrAndLen(SpaceCharsW));
			else
				p0 = TcsChrFirstNotOf(p1, Size() - (p1 - pData), EckStrAndLen(SpaceCharsA));
			if (!p0)
				break;
			if constexpr (std::is_same_v<TChar, WCHAR>)
				p1 = TcsChrFirstOf(p0, Size() - (p0 - pData), EckStrAndLen(SpaceCharsW));
			else
				p1 = TcsChrFirstOf(p0, Size() - (p0 - pData), EckStrAndLen(SpaceCharsA));
			if (!p1)
				p1 = pData + Size();
			TcsMoveLen(pCurr, p0, p1 - p0);
			pCurr += (p1 - p0);
		}
		ReSize(int(pCurr - pData));
	}

	[[nodiscard]] BOOL IsStartWith(TConstPointer psz, int cch = -1) const
	{
		if (cch < 0)
			cch = (int)TcsLen(psz);
		if (cch == 0 || Size() < cch)
			return FALSE;
		return TcsEqualLen(Data(), psz, cch);
	}
	[[nodiscard]] BOOL IsStartWithI(TConstPointer psz, int cch = -1) const
	{
		if (cch < 0)
			cch = (int)TcsLen(psz);
		if (cch == 0 || Size() < cch)
			return FALSE;
		return TcsEqualLenI(Data(), psz, cch);
	}
	[[nodiscard]] BOOL IsEndWith(TConstPointer psz, int cch = -1) const
	{
		if (cch < 0)
			cch = (int)TcsLen(psz);
		if (cch == 0 || Size() < cch)
			return FALSE;
		return TcsEqualLen(Data() + Size() - cch, psz, cch);
	}
	[[nodiscard]] BOOL IsEndWithI(TConstPointer psz, int cch = -1) const
	{
		if (cch < 0)
			cch = (int)TcsLen(psz);
		if (cch == 0 || Size() < cch)
			return FALSE;
		return TcsEqualLenI(Data() + Size() - cch, psz, cch);
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
	EckInlineNd auto SubSpan(int posStart, int cch) const
	{
		EckAssert(posStart >= 0 && posStart < Size());
		if (cch < 0)
			cch = Size() - posStart;
		EckAssert(cch != 0 && posStart + cch <= Size());
		return std::span<TChar>(Data() + posStart, cch);
	}

	EckInline void ToLower()
	{
		const auto pEnd = Data() + Size();
		for (auto p = Data(); p < pEnd; ++p)
			*p = TchToLower(*p);
	}
	EckInline void ToUpper()
	{
		const auto pEnd = Data() + Size();
		for (auto p = Data(); p < pEnd; ++p)
			*p = TchToUpper(*p);
	}

	EckInline int Compare(TConstPointer psz, int cch = -1) const
	{
		if (cch < 0)
			cch = (int)TcsLen(psz);
		return TcsCompareLen2(Data(), Size(), psz, cch);
	}
	EckInline int CompareI(TConstPointer psz, int cch = -1) const
	{
		if (cch < 0)
			cch = (int)TcsLen(psz);
		return TcsCompareLen2I(Data(), Size(), psz, cch);
	}

	// 返回文件名的位置
	[[nodiscard]] int PazFindFileSpec() const
	{
		if (IsEmpty())
			return -1;
		auto pEnd = Data() + Size() - 1;
		if (Back() == '\\' || Back() == '/')
			--pEnd;// 如果以反斜杠结尾，则跳过
		for (auto p = pEnd; p != Data(); --p)
		{
			const auto ch = *p;
			if (ch == '\\' || ch == '/')
			{
				if (p < Data() + 2)// NT路径或UNC路径的起始
					return -1;
				return int(p - Data());
			}
		}
		return -1;
	}

	BOOL PazRemoveFileSpec()
	{
		const auto pos = PazFindFileSpec();
		if (pos < 0)
			return FALSE;
		ReSize(pos);
		return TRUE;
	}

	BOOL PazRenameFileSpec(TConstPointer pszNewName, int cchNewName = -1)
	{
		const auto pos = PazFindFileSpec();
		if (pos < 0)
			return FALSE;
		if (cchNewName < 0)
			cchNewName = (int)TcsLen(pszNewName);
		ReSize(pos + cchNewName + 1);
		*(Data() + pos) = '\\';
		TcsCopyLen(Data() + pos + 1, pszNewName, cchNewName);
		return TRUE;
	}

	[[nodiscard]] int PazFindExtension() const
	{
		if (IsEmpty())
			return -1;
		int pos{ -1 };
		for (auto p = Data() + Size() - 1; p != Data(); --p)
		{
			const auto ch = *p;
			if (ch == '.')
				return int(p - Data());
			else if (ch == ' ' /*扩展名内不能有空格*/ ||
				ch == '\\' || ch == '/')
				return -1;
		}
		return -1;
	}

	BOOL PazRemoveExtension()
	{
		const auto pos = PazFindExtension();
		if (pos < 0)
			return FALSE;
		ReSize(pos);
		return TRUE;
	}

	void PazRenameExtension(TConstPointer pszNewExt, int cchNewExt = -1)
	{
		const auto pos = PazFindExtension();
		if (pos < 0)
			PushBack(pszNewExt, cchNewExt);
		else
		{
			if (cchNewExt < 0)
				cchNewExt = (int)TcsLen(pszNewExt);
			ReSize(pos + cchNewExt);
			TcsCopyLen(Data() + pos, pszNewExt, cchNewExt);
		}
	}

	// 如果没有反斜杠，则在末尾添加。返回操作前是否已有反斜杠
	BOOL PazAddBackslash()
	{
		if (IsEmpty())
			return FALSE;
		if (Back() == '\\' || Back() == '/')
			return TRUE;
		PushBackChar('\\');
		return FALSE;
	}
	// 去掉末尾的反斜杠
	BOOL PazRemoveBackslash()
	{
		if (IsEmpty())
			return FALSE;
		if (Back() == '\\' || Back() == '/')
		{
			PopBack();
			return TRUE;
		}
		return FALSE;
	}
	// 将所有斜杠替换为反斜杠
	void PazConvertToBackslash()
	{
		for (auto& ch : *this)
			if (ch == '/')
				ch = '\\';
	}
	// 将所有反斜杠替换为斜杠
	void PazConvertToSlash()
	{
		for (auto& ch : *this)
			if (ch == '\\')
				ch = '/';
	}

	HRESULT PazParseCommandLine(_Out_ TPointer& pszFile, _Out_ int& cchFile,
		_Out_ TPointer& pszParam, _Out_ int& cchParam)
	{
		if (IsEmpty())
		{
			pszFile = pszParam = nullptr;
			cchFile = cchParam = 0;
			return S_FALSE;
		}
		pszFile = Data();
		BOOL bQuote = (Front() == '\"');
		if (bQuote)
			++pszFile;
		if (bQuote)
		{
			for (auto p = pszFile; p != Data() + Size(); ++p)
				if (*p == '\"')
				{
					cchFile = int(p - pszFile);
					goto FileNameOk;
				}
			cchFile = 0;// 引号不匹配
			return HRESULT_FROM_WIN32(ERROR_INVALID_COMMAND_LINE);
		}
		else
		{
			for (auto p = pszFile; p != Data() + Size(); ++p)
				if (*p == ' ')
				{
					cchFile = int(p - pszFile);
					goto FileNameOk;
				}
			cchFile = Size();
			pszParam = nullptr;
			cchParam = 0;
			return S_OK;
		}
	FileNameOk:;// 至此文件名处理完毕
		// 步进到第一个非空格字符
		pszParam = pszFile + cchFile;
		if (*pszParam == '\"')
			++pszParam;
		for (; pszParam != Data() + Size(); ++pszParam)
			if (*pszParam != ' ')
				break;
		cchParam = int(Data() + Size() - pszParam);
		return S_OK;
	}

	HRESULT PazParseCommandLineAndCut(_Out_ TPointer& pszFile, _Out_ int& cchFile,
		_Out_ TPointer& pszParam, _Out_ int& cchParam)
	{
		EckAssert(&pszFile != &pszParam && &cchFile != &cchParam);
		const auto hr = PazParseCommandLine(pszFile, cchFile,
			pszParam, cchParam);
		if (SUCCEEDED(hr))
		{
			*(pszFile + cchFile) = '\0';
			*(pszParam + cchParam) = '\0';
		}
		return hr;
	}

	EckInlineNd TIterator begin() { return Data(); }
	EckInlineNd TIterator end() { return begin() + Size(); }
	EckInlineNd TConstIterator begin() const { return Data(); }
	EckInlineNd TConstIterator end() const { return begin() + Size(); }
	EckInlineNd TConstIterator cbegin() const { begin(); }
	EckInlineNd TConstIterator cend() const { end(); }
	EckInlineNd TReverseIterator rbegin() { return TReverseIterator(begin()); }
	EckInlineNd TReverseIterator rend() { return TReverseIterator(end()); }
	EckInlineNd TConstReverseIterator rbegin() const { return TConstReverseIterator(begin()); }
	EckInlineNd TConstReverseIterator rend() const { return TConstReverseIterator(end()); }
	EckInlineNd TConstReverseIterator crbegin() const { return rbegin(); }
	EckInlineNd TConstReverseIterator crend() const { return rend(); }
};

using CRefStrW = CRefStrT<WCHAR, CCharTraits<WCHAR>>;
using CRefStrA = CRefStrT<CHAR, CCharTraits<CHAR>>;

#undef EckTemp
#define EckTemp CRefStrT<TChar, TCharTraits, TAlloc>

#pragma region Operator
template<class TChar, class TCharTraits, class TAlloc>
EckInline EckTemp operator+(const EckTemp& rs1, const EckTemp& rs2)
{
	EckTemp x(rs1.Size() + rs2.Size());
	TcsCopyLen(x.Data(), rs1.Data(), rs1.Size());
	TcsCopyLenEnd(x.Data() + rs1.Size(), rs2.Data(), rs2.Size());
	return x;
}
template<class TChar, class TCharTraits, class TAlloc>
EckInline EckTemp operator+(const EckTemp& rs, const TChar* psz)
{
	const int cch = (psz ? (int)TcsLen(psz) : 0);
	EckTemp x(rs.Size() + cch);
	TcsCopyLen(x.Data(), rs.Data(), rs.Size());
	TcsCopyLenEnd(x.Data() + rs.Size(), psz, cch);
	return x;
}

template<class TChar, class TCharTraits, class TAlloc>
[[nodiscard]] EckInline bool operator==(const EckTemp& rs1, const TChar* psz2)
{
	if (rs1.IsEmpty())
		return !psz2;
	else if (!psz2)
		return false;
	else
		return TcsCompareLen2(rs1.Data(), rs1.Size(), psz2, (int)TcsLen(psz2)) == 0;
}
template<class TChar, class TCharTraits, class TAlloc>
[[nodiscard]] EckInline bool operator==(const TChar* psz2, const EckTemp& rs1)
{
	return operator==(rs1, psz2);
}

template<class TChar, class TCharTraits, class TAlloc>
[[nodiscard]] EckInline std::weak_ordering operator<=>(const EckTemp& rs1, const TChar* psz2)
{
	if (rs1.IsEmpty())
		return psz2 ? std::weak_ordering::less : std::weak_ordering::equivalent;
	else if (!psz2)
		return std::weak_ordering::greater;
	else
		return TcsCompareLen2(rs1.Data(), rs1.Size(), psz2, (int)TcsLen(psz2)) <=> 0;
}
template<class TChar, class TCharTraits, class TAlloc>
[[nodiscard]] EckInline std::weak_ordering operator<=>(const TChar* psz2, const EckTemp& rs1)
{
	if (!psz2)
		return rs1.IsEmpty() ? std::weak_ordering::equivalent : std::weak_ordering::less;
	else if (rs1.IsEmpty())
		return std::weak_ordering::greater;
	else
		return TcsCompareLen2(psz2, (int)TcsLen(psz2), rs1.Data(), rs1.Size()) <=> 0;
}

template<class TChar, class TCharTraits, class TAlloc>
[[nodiscard]] EckInline bool operator==(const EckTemp& rs1, const EckTemp& rs2)
{
	return TcsCompareLen2(rs1.Data(), rs1.Size(), rs2.Data(), rs2.Size()) == 0;
}

template<class TChar, class TCharTraits, class TAlloc>
[[nodiscard]] EckInline std::weak_ordering operator<=>(const EckTemp& rs1, const EckTemp& rs2)
{
	return TcsCompareLen2(rs1.Data(), rs1.Size(), rs2.Data(), rs2.Size()) <=> 0;
}
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

[[nodiscard]] EckInline void ToFullWidth(CRefStrW& rs, PCWSTR pszText, int cchText = -1)
{
	const int cchResult = LCMapStringEx(LOCALE_NAME_USER_DEFAULT, LCMAP_FULLWIDTH,
		pszText, cchText, nullptr, 0, nullptr, nullptr, 0);
	LCMapStringEx(LOCALE_NAME_USER_DEFAULT, LCMAP_FULLWIDTH,
		pszText, cchText, rs.PushBack(cchResult), cchResult, nullptr, nullptr, 0);
}
[[nodiscard]] EckInline void ToHalfWidth(CRefStrW& rs, PCWSTR pszText, int cchText = -1)
{
	const int cchResult = LCMapStringEx(LOCALE_NAME_USER_DEFAULT, LCMAP_HALFWIDTH,
		pszText, cchText, nullptr, 0, nullptr, nullptr, 0);
	LCMapStringEx(LOCALE_NAME_USER_DEFAULT, LCMAP_HALFWIDTH,
		pszText, cchText, rs.PushBack(cchResult), cchResult, nullptr, nullptr, 0);
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