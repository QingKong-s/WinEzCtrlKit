/*
* WinEzCtrlKit Library
*
* CBitSet.h ： 位集合
*
* Copyright(C) 2023-2024 QingKong
*/
#pragma once
#include "ECK.h"

ECK_NAMESPACE_BEGIN
template<size_t N>
class CBitSet
{
public:
#ifdef _WIN64
	using TWord = std::conditional_t<N <= 32, ULONG, ULONGLONG>;
#else
	using TWord = ULONG;
#endif
	constexpr static ptrdiff_t BitsPerWord = 8 * sizeof(TWord);
private:
	TWord m_Bits[(N - 1) / BitsPerWord + 1]{};
public:
	constexpr static ptrdiff_t NB = ARRAYSIZE(m_Bits) - 1;
private:
	template<ccpIsStdChar TChar>
	void ParseBinText(const TChar* pszBinText, int cchBinText, TChar ch1, TChar ch0, TChar chX)
	{
		EckAssert(pszBinText && cchBinText > 0 && ch1 != ch0);
		size_t idxInWord{}, idxCurrWord{};
		TWord u{};
		for (auto p = pszBinText + cchBinText - 1; p >= pszBinText; --p)
		{
			if (*p == chX)
				continue;
			u |= (TWord(*p == ch1) << idxInWord);
			if (++idxInWord == BitsPerWord)
			{
				m_Bits[idxCurrWord++] = u;
				u = TWord{};
				idxInWord = 0;
			}
		}

		if (idxInWord)
			m_Bits[idxCurrWord++] = u;

		for (size_t i = idxCurrWord; i < ARRAYSIZE(m_Bits); ++i)
			m_Bits[i] = TWord{};
	}
public:
	class CProxy
	{
		friend class CBitSet;
	private:
		CBitSet& m_That;
		size_t m_idx;

		CProxy(CBitSet& that, size_t idx) : m_That(that), m_idx(idx) {}
	public:
		EckInline operator bool() const { return m_That.Test(m_idx); }

		EckInline CProxy& operator=(bool b) { m_That.Set(m_idx, b); return *this; }

		EckInline CProxy& operator~() { m_That.Flip(m_idx); return *this; }

		EckInline CProxy& operator=(const CProxy& x) { m_That.Set(m_idx, x); }
	};

	CBitSet() = default;

	CBitSet(_In_ PCWSTR pszBin, int cchText = -1)
	{
		if (cchText < 0)
			cchText = (int)wcslen(pszBin);
		ParseBinText(pszBin, cchText, L'1', L'0', L'\'');
	}

	CBitSet(_In_ PCSTR pszBin, int cchText = -1)
	{
		if (cchText < 0)
			cchText = (int)strlen(pszBin);
		ParseBinText(pszBin, cchText, '1', '0', '\'');
	}

	CBitSet(_In_reads_bytes_(cb) PCVOID pBin, size_t cb)
	{
		if (cb > N)
			cb = N;
		memcpy(m_Bits, pBin, cb);
	}

	constexpr void Set()
	{
		for (auto& e : m_Bits)
			e = ~TWord{};
		Trim();
	}

	constexpr void Set(size_t n)
	{
		EckAssert(n < N);
		m_Bits[n / BitsPerWord] |= (TWord{ 1 } << n % BitsPerWord);
	}

	EckInline constexpr void Set(size_t n, BOOL b)
	{
		if (b)
			Set(n);
		else
			Clear(n);
	}

	EckInline constexpr void Clear(size_t n)
	{
		EckAssert(n < N);
		m_Bits[n / BitsPerWord] &= ~(TWord{ 1 } << n % BitsPerWord);
	}

	EckInline constexpr void Clear()
	{
		for (auto& e : m_Bits)
			e = TWord{};
		Trim();
	}

	EckInline constexpr CBitSet& operator&=(const CBitSet& x)
	{
		EckCounter(ARRAYSIZE(m_Bits), i)
			m_Bits[i] &= x.m_Bits[i];
		return *this;
	}

	EckInline constexpr CBitSet& operator|=(const CBitSet& x)
	{
		EckCounter(ARRAYSIZE(m_Bits), i)
			m_Bits[i] |= x.m_Bits[i];
		return *this;
	}

	EckInline constexpr CBitSet& operator^=(const CBitSet& x)
	{
		EckCounter(ARRAYSIZE(m_Bits), i)
			m_Bits[i] ^= x.m_Bits[i];
		return *this;
	}

	EckInline constexpr bool operator==(const CBitSet& x) const
	{
		EckCounter(ARRAYSIZE(m_Bits), i)
			if (m_Bits[i] != x.m_Bits[i])
				return FALSE;
		return TRUE;
	}

	EckInline constexpr bool operator!=(const CBitSet& x) const
	{
		return !(*this == x);
	}

	constexpr CBitSet& operator<<=(size_t n)
	{
		const auto t = n / BitsPerWord;
		if (t)
			for (ptrdiff_t i = NB; 0 <= i; --i)
				m_Bits[i] = t <= i ? m_Bits[i - t] : 0;

		if (n %= BitsPerWord)
		{
			for (ptrdiff_t i = NB; 0 < i; --i)
				m_Bits[i] = (m_Bits[i] << n) | (m_Bits[i - 1] >> (BitsPerWord - n));
			m_Bits[0] <<= n;
		}
		Trim();
		return *this;
	}

	constexpr CBitSet& operator>>=(size_t n)
	{
		const auto t = n / BitsPerWord;
		if (t)
			for (ptrdiff_t i = 0; i <= NB; ++i)
				m_Bits[i] = t <= NB - i ? m_Bits[i + t] : 0;

		if (n %= BitsPerWord)
		{
			for (ptrdiff_t i = 0; i < NB; ++i)
				m_Bits[i] = (m_Bits[i] >> n) | (m_Bits[i + 1] << (BitsPerWord - n));
			m_Bits[NB] >>= n;
		}
		return *this;
	}

	constexpr [[nodiscard]] CBitSet operator~() const
	{
		CBitSet x{ *this };
		x.Flip();
		return x;
	}

	EckInline [[nodiscard]] CProxy operator[](size_t n)
	{
		return CProxy(*this, n);
	}

	EckInline [[nodiscard]] constexpr bool operator[](size_t n) const
	{
		return Test(n);
	}

	EckInline constexpr void Trim()
	{
#pragma warning(suppress:6285)// 是否要使用按位与
		constexpr bool b = (N == 0 || N % BitsPerWord != 0);
		if constexpr (b)
			m_Bits[NB] &= (TWord{ 1 } << N % BitsPerWord) - 1;
	}

	EckInline [[nodiscard]] size_t PopCount() const
	{
		size_t n{};
		if constexpr (sizeof(TWord) == 8)
		{
#ifdef _WIN64
			EckCounter(ARRAYSIZE(m_Bits), i)
				n += __popcnt64(m_Bits[i]);
#else
			EckCounter(ARRAYSIZE(m_Bits), i)
				n += (__popcnt(m_Bits[i]) + __popcnt(m_Bits[i] >> 32));
#endif
		}
		else
		{
			EckCounter(ARRAYSIZE(m_Bits), i)
				n += __popcnt(m_Bits[i]);
		}
		return n;
	}

	[[nodiscard]] constexpr BOOL AllOne() const
	{
		constexpr BOOL bNoPadding = N % BitsPerWord == 0;
		for (size_t i = 0; i < NB + bNoPadding; ++i)
			if (m_Bits[i] != ~TWord{})
				return FALSE;
		return bNoPadding || m_Bits[NB] == (TWord{ 1 } << (N % BitsPerWord)) - 1;
	}

	[[nodiscard]] constexpr BOOL AnyOne() const
	{
		for (size_t i = 0; i <= NB; ++i)
			if (m_Bits[i] != 0)
				return TRUE;
		return FALSE;
	}

	EckInline [[nodiscard]] constexpr BOOL NoneOne() const
	{
		return !AnyOne();
	}

	EckInline [[nodiscard]] constexpr BOOL Test(size_t n) const
	{
		EckAssert(n < N);
		return (m_Bits[n / BitsPerWord] & (TWord{ 1 } << n % BitsPerWord)) != 0;
	}

	EckInline constexpr void Flip()
	{
		for (auto& e : m_Bits)
			e = ~e;
		Trim();
	}

	EckInline constexpr void Flip(size_t n)
	{
		EckAssert(n < N);
		m_Bits[n / BitsPerWord] ^= (TWord{ 1 } << n % BitsPerWord);
	}

	constexpr void ReverseByte()
	{
		BYTE* const p = (BYTE*)m_Bits;
		for (size_t i = 0; i < N / 8 / 2; ++i)
			std::swap(p[i], p[N / 8 - 1 - i]);
	}

	EckInline [[nodiscard]] constexpr const TWord* Data() const
	{
		return m_Bits;
	}

	EckInline [[nodiscard]] constexpr TWord* Data()
	{
		return m_Bits;
	}

	EckInline [[nodiscard]] CBitSet SubSet(size_t pos, size_t cBits)
	{
		EckAssert(pos + cBits <= N);
		CBitSet x{};
		memcpy(x.Data(), Data() + pos / BitsPerWord, (cBits + BitsPerWord - 1) / BitsPerWord);
		if (pos % BitsPerWord)
		{
			EckCounter((pos + cBits) / BitsPerWord, i)
			{
				x.Data()[i] = (x.Data()[i] >> pos % BitsPerWord) |
					(x.Data()[i + 1] << (BitsPerWord - pos % BitsPerWord));
			}
		}
		x.Trim();
		return x;
	}

	template<size_t NPos, size_t NBits>
		requires requires { NPos < N&& NPos + NBits <= N; }
	[[nodiscard]] CBitSet<NBits> SubSet()
	{
		CBitSet<NBits> x{};
		memcpy(x.Data(), Data() + NPos / BitsPerWord, (NBits + BitsPerWord - 1) / BitsPerWord);
		if (NPos % BitsPerWord)
		{
			EckCounter((NPos + NBits) / BitsPerWord, i)
			{
				x.Data()[i] = (x.Data()[i] >> NPos % BitsPerWord) |
					(x.Data()[i + 1] << (BitsPerWord - NPos % BitsPerWord));
			}
		}
		x.Trim();
		return x;
	}

	EckInline [[nodiscard]] ULONG SubULong(size_t pos) const
	{
		EckAssert(pos + sizeof(ULONG) * 8 < N);
		TWord x{};
		CopyBits(m_Bits, &x, pos, 0, sizeof(ULONG) * 8);
		return (ULONG)x;
	}

	EckInline [[nodiscard]] ULONGLONG SubULongLong(size_t pos) const
	{
		EckAssert(pos + sizeof(ULONGLONG) * 8 < N);
		ULONGLONG x{};
		CopyBits(m_Bits, (TWord*)&x, pos, 0, sizeof(ULONGLONG) * 8);
		return x;
	}

	EckInline [[nodiscard]] ULONG ToULong() const
	{
		EckAssert(sizeof(ULONG) * 8 <= N);
		return *(ULONG*)m_Bits;
	}

	EckInline [[nodiscard]] ULONGLONG ToULongLong() const
	{
		EckAssert(sizeof(ULONGLONG) * 8 <= N);
		return *(ULONGLONG*)m_Bits;
	}

	void Set(PCVOID pBin, size_t cb)
	{
		EckAssert(cb * 8 <= N);
		if (cb > N)
			cb = N;
		memcpy(m_Bits, pBin, cb);
		for (BYTE* p = (BYTE*)m_Bits + cb; p <= (BYTE*)m_Bits + ARRAYSIZE(m_Bits); ++p)
			*p = 0;
	}
};
ECK_NAMESPACE_END