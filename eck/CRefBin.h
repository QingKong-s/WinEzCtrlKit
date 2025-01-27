#pragma once
#include "CAllocator.h"

#include <initializer_list>

ECK_NAMESPACE_BEGIN
inline constexpr size_t INVALID_BIN_POS = SizeTMax;

inline constexpr size_t BinNPos = SizeTMax;

/// <summary>
/// 寻找字节集
/// </summary>
/// <param name="pMain">要在其中寻找的字节集指针</param>
/// <param name="cbMainSize">要在其中寻找的字节集长度</param>
/// <param name="pSub">要寻找的字节集指针</param>
/// <param name="cbSubSize">要寻找的字节集长度</param>
/// <param name="posStart">起始位置</param>
/// <returns>位置，若未找到返回BinNPos</returns>
[[nodiscard]] inline size_t FindBin(PCVOID pMain, size_t cbMainSize, 
	PCVOID pSub, size_t cbSubSize, size_t posStart = 0)
{
	if (cbMainSize < cbSubSize)
		return BinNPos;
	for (PCBYTE pCurr = (PCBYTE)pMain + posStart, 
		pEnd = (PCBYTE)pMain + cbMainSize - cbSubSize; pCurr <= pEnd; ++pCurr)
	{
		if (memcmp(pCurr, pSub, cbSubSize) == 0)
			return pCurr - (PCBYTE)pMain;
	}
	return BinNPos;
}

/// <summary>
/// 倒找字节集
/// </summary>
/// <param name="pMain">要在其中寻找的字节集指针</param>
/// <param name="cbMainSize">要在其中寻找的字节集长度</param>
/// <param name="pSub">要寻找的字节集指针</param>
/// <param name="cbSubSize">要寻找的字节集长度</param>
/// <param name="posStart">起始位置</param>
/// <returns>位置，若未找到返回BinNPos</returns>
[[nodiscard]] inline size_t FindBinRev(PCVOID pMain, size_t cbMainSize, 
	PCVOID pSub, size_t cbSubSize, size_t posStart = 0)
{
	if (cbMainSize < cbSubSize)
		return BinNPos;
	for (PCBYTE pCurr = (PCBYTE)pMain + cbMainSize - posStart - cbSubSize; pCurr >= pMain; --pCurr)
	{
		if (memcmp(pCurr, pSub, cbSubSize) == 0)
			return pCurr - (PCBYTE)pMain;
	}
	return BinNPos;
}

using TRefBinDefAllocator = CDefAllocator<BYTE>;

template<class TAlloc_ = TRefBinDefAllocator>
class CRefBinT
{
public:
	using TAlloc = TAlloc_;
	using TAllocTraits = CAllocatorTraits<TAlloc>;

	using TIterator = BYTE*;
	using TConstIterator = const BYTE*;
	using TReverseIterator = std::reverse_iterator<TIterator>;
	using TConstReverseIterator = std::reverse_iterator<TConstIterator>;
private:
	BYTE* m_pStream{};
	size_t m_cb{};
	size_t m_cbCapacity{};

	ECKNOUNIQUEADDR TAlloc m_Alloc{};

	EckInlineCe void ResetThat(CRefBinT& x)
	{
		m_pStream = nullptr;
		m_cb = 0;
		m_cbCapacity = 0;
		m_Alloc = {};
	}
public:
	CRefBinT() = default;

	explicit CRefBinT(const TAlloc& Al): m_Alloc{ Al } {}

	explicit CRefBinT(size_t cb, const TAlloc& Al = TAlloc{})
		: m_cb{ cb }, m_cbCapacity{ cb }, m_Alloc{ Al }
	{
		Reserve(cb);
	}

	CRefBinT(PCVOID p, size_t cb, const TAlloc& Al = TAlloc{}): m_Alloc{ Al }
	{
		EckAssert(p ? TRUE : (cb == 0));
		DupStream(p, cb);
	}

	CRefBinT(const CRefBinT& x)
		: m_Alloc{ TAllocTraits::select_on_container_copy_construction(x.m_Alloc) }
	{
		DupStream(x.Data(), x.Size());
	}

	CRefBinT(const CRefBinT& x, const TAlloc& Al): m_Alloc{ Al }
	{
		DupStream(x.Data(), x.Size());
	}

	CRefBinT(CRefBinT&& x) noexcept
		:m_pStream{ x.m_pStream }, m_cb{ x.m_cb }, m_cbCapacity{ x.m_cbCapacity },
		m_Alloc{ std::move(x.m_Alloc) }
	{
		ResetThat(x);
	}

	CRefBinT(CRefBinT&& x, const TAlloc& Al) noexcept(TAllocTraits::is_always_equal::value)
		: m_pStream{ x.m_pStream }, m_cb{ x.m_cb }, m_cbCapacity{ x.m_cbCapacity },
		m_Alloc{ Al }
	{
		if constexpr (!TAllocTraits::is_always_equal::value)
		{
			if (Al != x.m_Alloc)
			{
				x.m_Alloc.deallocate(m_pStream, m_cbCapacity);
				m_pStream = m_Alloc.allocate(m_cbCapacity);
				memcpy(Data(), x.Data(), x.Size());
			}
		}
		ResetThat(x);
	}

	CRefBinT(std::initializer_list<BYTE> x, const TAlloc& Al = TAlloc{}): m_Alloc{ Al }
	{
		DupStream(x.begin(), x.size());
	}

	~CRefBinT()
	{
		m_Alloc.deallocate(m_pStream, m_cbCapacity);
	}

	CRefBinT& operator=(const CRefBinT& x)
	{
		if constexpr (!TAllocTraits::is_always_equal::value)
		{
			if (m_Alloc != x.m_Alloc)
			{
				m_Alloc = x.m_Alloc;
				m_Alloc.deallocate(m_pStream, m_cbCapacity);
				ResetThat(*this);
			}
			else if constexpr (TAllocTraits::propagate_on_container_copy_assignment::value)
				m_Alloc = x.m_Alloc;
		}
		else if constexpr (TAllocTraits::propagate_on_container_copy_assignment::value)
			m_Alloc = x.m_Alloc;

		DupStream(x.Data(), x.Size());
		return *this;
	}
	CRefBinT& operator=(CRefBinT&& x) noexcept
	{
		if (this == &x)
			return *this;
		if constexpr (TAllocTraits::propagate_on_container_move_assignment::value)
			m_Alloc = std::move(x.m_Alloc);
		else if constexpr (!TAllocTraits::is_always_equal::value)
			if (m_Alloc != x.m_Alloc)
			{
				DupStream(x.Data(), x.Size());
				return *this;
			}

		std::swap(m_pStream, x.m_pStream);
		std::swap(m_cb, x.m_cb);
		std::swap(m_cbCapacity, x.m_cbCapacity);
		return *this;
	}
	EckInline CRefBinT& operator=(std::initializer_list<BYTE> x)
	{
		DupStream(x);
		return *this;
	}
	template<class T, size_t E>
	CRefBinT& operator=(std::span<T, E> x)
	{
		DupStream(x.data(), x.size_bytes());
		return *this;
	}

	template<class TAlloc1>
	EckInline CRefBinT& operator+=(const CRefBinT<TAlloc1>& x)
	{
		PushBack(x.Data(), x.Size());
		return *this;
	}
	template<class T, class TAlloc1>
	EckInline CRefBinT& operator+=(const std::vector<T, TAlloc1>& x)
	{
		PushBack(x.data(), x.size() * sizeof(T));
		return *this;
	}
	template<class T, size_t E>
	EckInline CRefBinT& operator+=(const std::span<T, E>& x)
	{
		PushBack(x.data(), x.size_bytes());
		return *this;
	}

	EckInlineNd TAlloc GetAllocator() const { return m_Alloc; }
	EckInlineNdCe BYTE& At(size_t idx) { EckAssert(idx < Size()); return *(Data() + idx); }
	EckInlineNdCe BYTE At(size_t idx) const { EckAssert(idx < Size()); return *(Data() + idx); }
	EckInlineNdCe BYTE& operator[](size_t idx) { return At(idx); }
	EckInlineNdCe BYTE operator[](size_t idx) const { return At(idx); }
	EckInlineNdCe BYTE& Front() { return At(0); }
	EckInlineNdCe BYTE Front() const { return At(0); }
	EckInlineNdCe BYTE& Back() { return At(Size() - 1); }
	EckInlineNdCe BYTE Back() const { return At(Size() - 1); }
	EckInlineNdCe BYTE* Data() { return m_pStream; }
	EckInlineNdCe const BYTE* Data() const { return m_pStream; }
	EckInlineNdCe size_t Capacity() const { return m_cbCapacity; }
	EckInlineNdCe size_t Size() const { return m_cb; }
	EckInlineNdCe BOOL IsEmpty() const { return Size() == 0; }

	EckInline void DupStream(PCVOID p, size_t cb)
	{
		ReSizeExtra(cb);
		memcpy(Data(), p, cb);
	}
	EckInline void DupStream(std::initializer_list<BYTE> x)
	{
		DupStream(x.begin(), x.size());
	}
	template<class T, size_t E>
	EckInline void DupStream(std::span<T, E> x)
	{
		DupStream(x.data(), x.size_bytes());
	}

	/// <summary>
	/// 依附指针。
	/// 先前的内存将被释放
	/// </summary>
	/// <param name="p">指针，必须可通过当前分配器解分配</param>
	/// <param name="cbCapacity">容量</param>
	/// <param name="cb">当前长度</param>
	void Attach(BYTE* p, size_t cbCapacity, size_t cb)
	{
		m_Alloc.deallocate(m_pStream, m_cbCapacity);
		if (!p)
		{
			m_pStream = nullptr;
			m_cb = m_cbCapacity = 0u;
		}
		else
		{
			m_pStream = p;
			m_cbCapacity = cbCapacity;
			m_cb = cb;
		}
	}

	/// <summary>
	/// 拆离指针
	/// </summary>
	/// <returns></returns>
	[[nodiscard]] EckInline BYTE* Detach(size_t& cbCapacity, size_t& cb)
	{
		const auto pTemp = m_pStream;
		m_pStream = nullptr;

		cbCapacity = m_cbCapacity;
		m_cbCapacity = 0u;

		cb = m_cb;
		m_cb = 0u;
		return pTemp;
	}

	/// <summary>
	/// 复制到
	/// </summary>
	/// <param name="pDst">目标指针</param>
	/// <param name="cbMax">最大长度，若过大则自动缩减</param>
	/// <returns>复制的长度</returns>
	EckInline size_t CopyTo(void* pDst, size_t cbMax) const
	{
		if (cbMax > m_cb)
			cbMax = m_cb;
		memcpy(pDst, m_pStream, cbMax);
		return cbMax;
	}

	/// <summary>
	/// 保留内存
	/// </summary>
	/// <param name="cb">尺寸</param>
	EckInline void Reserve(size_t cb)
	{
		if (m_cbCapacity >= cb)
			return;

		const auto pOld = m_pStream;
		m_pStream = m_Alloc.allocate(cb);
		if (pOld)
		{
			memcpy(Data(), pOld, Size());
			m_Alloc.deallocate(pOld, m_cbCapacity);
		}

		m_cbCapacity = cb;
	}

	EckInline void ReSize(size_t cb)
	{
		Reserve(cb);
		m_cb = cb;
	}
	EckInline void ReSizeExtra(size_t cb)
	{
		if (m_cbCapacity < cb)
			Reserve(TAllocTraits::MakeCapacity(cb));
		m_cb = cb;
	}

	/// <summary>
	/// 替换
	/// </summary>
	/// <param name="posStart">替换位置</param>
	/// <param name="cbReplacing">替换长度</param>
	/// <param name="pNew">用作替换的字节集指针</param>
	/// <param name="cbNew">用作替换的字节集长度</param>
	EckInline void Replace(size_t posStart, size_t cbReplacing, PCVOID pNew = nullptr, size_t cbNew = 0u)
	{
		EckAssert(cbNew ? (!!pNew) : TRUE);
		const size_t cbOrg = Size();
		ReSizeExtra(Size() + cbNew - cbReplacing);
		memmove(
			Data() + posStart + cbNew,
			Data() + posStart + cbReplacing,
			cbOrg - posStart - cbReplacing);
		memcpy(Data() + posStart, pNew, cbNew);
	}

	/// <summary>
	/// 替换
	/// </summary>
	/// <param name="posStart">替换位置</param>
	/// <param name="cbReplacing">替换长度</param>
	/// <param name="rb">用作替换的字节集</param>
	template<class TAlloc1>
	EckInline void Replace(size_t posStart, size_t cbReplacing, const CRefBinT<TAlloc1>& rb)
	{
		Replace(posStart, cbReplacing, rb.Data(), rb.Size());
	}
	EckInline void Replace(size_t posStart, size_t cbReplacing, std::initializer_list<BYTE> x)
	{
		Replace(posStart, cbReplacing, x.begin(), x.size());
	}

	/// <summary>
	/// 子字节集替换
	/// </summary>
	/// <param name="pReplacedBin">被替换的字节集指针</param>
	/// <param name="cbReplacedBin">被替换的字节集长度</param>
	/// <param name="pSrcBin">用作替换的字节集指针</param>
	/// <param name="cbSrcBin">用作替换的字节集长度</param>
	/// <param name="posStart">起始位置</param>
	/// <param name="cReplacing">替换进行的次数，0为执行所有替换</param>
	void ReplaceSubBin(PCVOID pReplacedBin, size_t cbReplacedBin,
		PCVOID pSrcBin, size_t cbSrcBin, size_t posStart = 0, int cReplacing = 0)
	{
		size_t pos = 0u;
		for (int c = 1;; ++c)
		{
			pos = FindBin(m_pStream, m_cb, pReplacedBin, cbReplacedBin, posStart + pos);
			if (pos == BinNPos)
				break;
			Replace(pos, cbReplacedBin, pSrcBin, cbSrcBin);
			pos += cbSrcBin;
			if (c == cReplacing)
				break;
		}
	}

	/// <summary>
	/// 子字节集替换
	/// </summary>
	/// <param name="rbReplacedBin">被替换的字节集</param>
	/// <param name="rbSrcBin">用作替换的字节集</param>
	/// <param name="posStart">起始位置</param>
	/// <param name="cReplacing">替换进行的次数，0为执行所有替换</param>
	template<class TAlloc1, class TAlloc2>
	EckInline void ReplaceSubBin(const CRefBinT<TAlloc1>& rbReplacedBin,
		const CRefBinT<TAlloc2>& rbSrcBin, size_t posStart = 0, int cReplacing = -1)
	{
		ReplaceSubBin(rbReplacedBin.Data(), rbReplacedBin.Size(), rbSrcBin.Data(), rbSrcBin.Size(),
			posStart, cReplacing);
	}
	EckInline void ReplaceSubBin(std::initializer_list<BYTE> ilReplacedBin,
		std::initializer_list<BYTE> ilSrcBin, size_t posStart = 0, int cReplacing = -1)
	{
		ReplaceSubBin(ilReplacedBin.begin(), ilReplacedBin.size(), ilSrcBin.begin(), ilSrcBin.size(),
			posStart, cReplacing);
	}

	EckInline void MakeSpace(size_t cbSize, size_t posStart = 0u)
	{
		ReSizeExtra(cbSize + posStart);
		ZeroMemory(Data() + posStart, cbSize);
	}

	void MakeRepeatedBinSequence(size_t cCount, PCVOID pBin, size_t cbBin, size_t posStart = 0u)
	{
		ReSizeExtra(posStart + cCount * cbBin);
		BYTE* pCurr{ Data() + posStart };
		size_t i;
		for (size_t i = 0; i < cCount; ++i, pCurr += cbBin)
			memcpy(pCurr, pBin, cbBin);
	}

	EckInline CRefBinT& PushBack(PCVOID p, size_t cb)
	{
		ReSizeExtra(m_cb + cb);
		memcpy(Data() + Size() - cb, p, cb);
		return *this;
	}
	EckInline CRefBinT& PushBack(const CRefBinT& rb)
	{
		return PushBack(rb.Data(), rb.Size());
	}
	EckInline CRefBinT& PushBack(std::initializer_list<BYTE> x)
	{
		return PushBack(x.begin(), x.size());
	}
	template<class T, class TAlloc1>
	EckInline CRefBinT& PushBack(const std::vector<T, TAlloc1>& x)
	{
		return PushBack(x.data(), x.size() * sizeof(T));
	}
	template<class T, size_t E>
	EckInline CRefBinT& PushBack(std::span<T, E> x)
	{
		return PushBack(x.data(), x.size_bytes());
	}
	EckInline CRefBinT& PushBackByte(BYTE by)
	{
		ReSizeExtra(Size() + 1);
		*(Data() + Size() - 1) = by;
		return *this;
	}

	EckInline BYTE* PushBack(size_t cb)
	{
		ReSizeExtra(m_cb + cb);
		return Data() + m_cb - cb;
	}
	template<class T>
	EckInline T* PushBackType()
	{
		return (T*)PushBack(sizeof(T));
	}
	EckInline BYTE* PushBackNoExtra(size_t cb)
	{
		ReSize(m_cb + cb);
		return Data() + m_cb - cb;
	}

	EckInline void PopBack(size_t cb)
	{
		EckAssert(m_cb >= cb);
		m_cb -= cb;
	}

	EckInline constexpr void Clear() { m_cb = 0u; }

	EckInline void Zero() { RtlZeroMemory(Data(), Size()); }

	// 从左闭右开区间创建字节集
	template<class TAlloc1 = TAlloc>
	[[nodiscard]] EckInline CRefBinT<TAlloc1> SubBin(size_t posBegin, size_t posEnd)
	{
		return CRefBinT<TAlloc1>(Data() + posBegin, posEnd - posBegin);
	}

	EckInline CRefBinT& Insert(size_t pos, PCVOID p, size_t cb)
	{
		EckAssert(pos <= Size());
		EckAssert(p ? TRUE : (cb == 0));
		ReSizeExtra(Size() + cb);
		memmove(
			Data() + pos + cb,
			Data() + pos,
			Size() - cb - pos);
		memcpy(Data() + pos, p, cb);
		return *this;
	}
	template<class TAlloc1 = TAlloc>
	EckInline CRefBinT& Insert(size_t pos, const CRefBinT<TAlloc1>& rb)
	{
		return Insert(pos, rb.Data(), rb.Size());
	}
	EckInline CRefBinT& Insert(size_t pos, std::initializer_list<BYTE> il)
	{
		return Insert(pos, il.begin(), il.size());
	}
	template<class T, class TAlloc1>
	EckInline CRefBinT& Insert(size_t pos, const std::vector<T, TAlloc1>& x)
	{
		return Insert(pos, x.data(), x.size() * sizeof(T));
	}
	template<class T, size_t E>
	EckInline CRefBinT& Insert(size_t pos, std::span<T, E> x)
	{
		return Insert(pos, x.data(), x.size_bytes());
	}
	EckInline CRefBinT& Insert(size_t pos, BYTE by)
	{
		return Insert(pos, &by, 1u);
	}

	EckInline void Erase(size_t pos, size_t cb)
	{
		EckAssert(Size() >= pos + cb);
		memmove(
			Data() + pos,
			Data() + pos + cb,
			Size() - pos - cb);
		m_cb -= cb;
	}

	void ShrinkToFit()
	{
		EckAssert(m_cbCapacity >= m_cb);
		if (m_cbCapacity == m_cb)
			return;
		const auto pOld = m_pStream;
		m_pStream = m_Alloc.allocate(m_cb);
		memcpy(Data(), pOld, Size());
		m_Alloc.deallocate(pOld, m_cbCapacity);
		m_cbCapacity = m_cb;
	}

	EckInline void ExtendToCapacity() { m_cb = Capacity(); }

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

using CRefBin = CRefBinT<TRefBinDefAllocator>;

template<class TAlloc = TRefBinDefAllocator, class TAlloc1, class TAlloc2>
CRefBinT<TAlloc> operator+(const CRefBinT<TAlloc1>& rb1, const CRefBinT<TAlloc2>& rb2)
{
	CRefBinT<TAlloc> rb(rb1.Size() + rb2.Size());
	memcpy(rb.Data(), rb1.Data(), rb1.Size());
	memcpy(rb.Data() + rb1.Size(), rb2.Data(), rb2.Size());
	return rb;
}

template<class TAlloc>
[[nodiscard]] EckInline bool operator==(const CRefBinT<TAlloc>& rb1, std::initializer_list<BYTE> il)
{
	if (!rb1.Data() && !il.size())
		return true;
	else if (!rb1.Data() || !il.size() || rb1.Size() != il.size())
		return false;
	else
		return memcmp(rb1.Data(), il.begin(), il.size()) == 0;
}

/// <summary>
/// 寻找字节集
/// </summary>
/// <param name="rbMain">要在其中寻找的字节集</param>
/// <param name="rbSub">要寻找的字节集</param>
/// <param name="posStart">起始位置</param>
/// <returns>位置，若未找到返回BinNPos</returns>
template<class TAlloc1, class TAlloc2>
[[nodiscard]] EckInline size_t FindBin(const CRefBinT<TAlloc1>& rbMain, const CRefBinT<TAlloc1>& rbSub, size_t posStart = 0)
{
	return FindBin(rbMain.Data(), rbMain.Size(), rbSub.Data(), rbSub.Size(), posStart);
}

/// <summary>
/// 倒找字节集
/// </summary>
/// <param name="rbMain">要在其中寻找的字节集</param>
/// <param name="rbSub">要寻找的字节集</param>
/// <param name="posStart">起始位置</param>
/// <returns>位置，若未找到返回BinNPos</returns>
[[nodiscard]] EckInline size_t FindBinRev(const CRefBin& rbMain, const CRefBin& rbSub, size_t posStart = 0)
{
	return FindBinRev(rbMain.Data(), rbMain.Size(), rbSub.Data(), rbSub.Size(), posStart);
}

template<class TProcesser>
inline void SplitBin(PCVOID p, SIZE_T cbSize, PCVOID pDiv, SIZE_T cbDiv, int cBinExpected, TProcesser Processer)
{
	SIZE_T pos = FindBin(p, cbSize, pDiv, cbDiv);
	if (pos == BinNPos)
	{
		Processer((BYTE*)p, cbSize);
		return;
	}
	SIZE_T posPrevFirst = 0u;
	int c = 0;
	do
	{
		Processer((BYTE*)p + posPrevFirst, pos - posPrevFirst);
		++c;
		if (c == cBinExpected)
			return;
		posPrevFirst = pos + cbDiv;
		pos = FindBin(p, cbSize, pDiv, cbDiv, posPrevFirst);
	} while (pos != BinNPos);

	Processer((BYTE*)p + posPrevFirst, pos + cbSize - posPrevFirst);
}


struct SPLITBININFO
{
	PCVOID pBin;
	size_t cbBin;
};

/// <summary>
/// 分割字节集。
/// 此函数不执行任何复制操作
/// </summary>
/// <param name="p">要分割的字节集指针</param>
/// <param name="cbSize">要分割的字节集长度</param>
/// <param name="pDiv">用作分割的字节集指针</param>
/// <param name="cbDiv">用作分割的字节集长度</param>
/// <param name="aResult">结果容器</param>
/// <param name="cBinExpected">返回的最大子字节集个数</param>
EckInline void SplitBin(PCVOID p, size_t cbSize, PCVOID pDiv, size_t cbDiv,
	std::vector<SPLITBININFO>& aResult, int cBinExpected = 0)
{
	SplitBin(p, cbSize, pDiv, cbDiv, cBinExpected,
		[&](PCVOID p, SIZE_T cb)
		{
			aResult.push_back({ p,cb });
		});
}

/// <summary>
/// 分割字节集。
/// 此函数不执行任何复制操作
/// </summary>
/// <param name="rb">要分割的字节集</param>
/// <param name="rbDiv">用作分割的字节集</param>
/// <param name="aResult">结果容器</param>
/// <param name="cBinExpected">返回的最大子字节集个数</param>
EckInline void SplitBin(const CRefBin& rb, const CRefBin& rbDiv,
	std::vector<SPLITBININFO>& aResult, int cBinExpected = 0)
{
	SplitBin(rb.Data(), rb.Size(), rbDiv.Data(), rbDiv.Size(), aResult, cBinExpected);
}

/// <summary>
/// 分割字节集
/// </summary>
/// <param name="p">要分割的字节集指针</param>
/// <param name="cbSize">要分割的字节集长度</param>
/// <param name="pDiv">用作分割的字节集指针</param>
/// <param name="cbDiv">用作分割的字节集长度</param>
/// <param name="aResult">结果容器</param>
/// <param name="cBinExpected">返回的最大子字节集个数</param>
EckInline void SplitBin(PCVOID p, size_t cbSize, PCVOID pDiv, size_t cbDiv,
	std::vector<CRefBin>& aResult, int cBinExpected = 0)
{
	SplitBin(p, cbSize, pDiv, cbDiv, cBinExpected,
		[&](PCVOID p, SIZE_T cb)
		{
			aResult.push_back(CRefBin(p, cb));
		});
}

/// <summary>
/// 分割字节集
/// </summary>
/// <param name="rb">要分割的字节集</param>
/// <param name="rbDiv">用作分割的字节集</param>
/// <param name="aResult">结果容器</param>
/// <param name="cBinExpected">返回的最大子字节集个数</param>
EckInline void SplitBin(const CRefBin& rb, const CRefBin& rbDiv,
	std::vector<CRefBin>& aResult, int cBinExpected = 0)
{
	SplitBin(rb.Data(), rb.Size(), rbDiv.Data(), rbDiv.Size(), aResult, cBinExpected);
}
ECK_NAMESPACE_END

template<class TAlloc>
struct std::hash<::eck::CRefBinT<TAlloc>>
{
	[[nodiscard]] EckInline size_t operator()(
		const ::eck::CRefBinT<TAlloc>& rb) const noexcept
	{
		return ::eck::Fnv1aHash(rb.Data(), rb.Size());
	}
};