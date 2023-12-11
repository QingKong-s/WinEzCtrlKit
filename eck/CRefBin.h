/*
* WinEzCtrlKit Library
*
* CRefBin.h ： 字节集
*
* Copyright(C) 2023 QingKong
*/

#pragma once
#include "ECK.h"
#include "CAllocator.h"
#include "CRefStr.h"

#include <initializer_list>
#include <vector>

ECK_NAMESPACE_BEGIN
static constexpr size_t INVALID_BIN_POS = std::numeric_limits<size_t>{}.max();

inline constexpr size_t BinNPos = std::numeric_limits<size_t>{}.max();

/// <summary>
/// 寻找字节集
/// </summary>
/// <param name="pMain">要在其中寻找的字节集指针</param>
/// <param name="cbMainSize">要在其中寻找的字节集长度</param>
/// <param name="pSub">要寻找的字节集指针</param>
/// <param name="cbSubSize">要寻找的字节集长度</param>
/// <param name="posStart">起始位置</param>
/// <returns>位置，若未找到返回BinNPos</returns>
inline size_t FindBin(PCVOID pMain, size_t cbMainSize, PCVOID pSub, size_t cbSubSize, size_t posStart = 0)
{
	if (cbMainSize < cbSubSize)
		return BinNPos;
	for (PCBYTE pCurr = (PCBYTE)pMain + posStart, pEnd = (PCBYTE)pMain + cbMainSize - cbSubSize; pCurr <= pEnd; ++pCurr)
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
inline size_t FindBinRev(PCVOID pMain, size_t cbMainSize, PCVOID pSub, size_t cbSubSize, size_t posStart = 0)
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

template<class TAlloc_ = CAllocatorProcHeap<BYTE>>
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
	BYTE* m_pStream = NULL;
	size_t m_cb = 0u;
	size_t m_cbCapacity = 0u;

	[[no_unique_address]] TAlloc m_Alloc{};
public:
	CRefBinT() = default;

	/// <summary>
	/// 创建自长度
	/// </summary>
	/// <param name="cb"></param>
	explicit CRefBinT(size_t cb)
	{
		m_cb = cb;
		m_cbCapacity = cb;
		m_pStream = m_Alloc.allocate(m_cbCapacity);
	}

	/// <summary>
	/// 创建自字节集
	/// </summary>
	/// <param name="p"></param>
	/// <param name="cb"></param>
	CRefBinT(PCVOID p, size_t cb)
	{
		EckAssert(cb ? (!!p) : TRUE);
		m_cb = cb;
		m_cbCapacity = TAllocTraits::MakeCapacity(cb);
		m_pStream = m_Alloc.allocate(m_cbCapacity);
		memcpy(Data(), p, cb);
	}

	CRefBinT(const CRefBinT& x)
	{
		EckAssert(x.Size() ? (!!x.Data()) : TRUE);
		m_Alloc = TAllocTraits::select_on_container_copy_construction(x.m_Alloc);
		m_cb = x.Size();
		m_cbCapacity = TAllocTraits::MakeCapacity(x.Size());
		m_pStream = m_Alloc.allocate(m_cbCapacity);
		memcpy(Data(), x.Data(), x.Size());
	}

	CRefBinT(CRefBinT&& x) noexcept
		:m_pStream{ x.m_pStream }, m_cb{ x.m_cb }, m_cbCapacity{ x.m_cbCapacity },
		m_Alloc{ std::move(x.m_Alloc) }
	{
		x.m_pStream = NULL;
		x.m_cb = x.m_cbCapacity = 0u;
	}

	/// <summary>
	/// 创建自大括号初始化器
	/// </summary>
	/// <param name="x"></param>
	explicit CRefBinT(std::initializer_list<BYTE> x)
	{
		m_cb = x.size();
		m_cbCapacity = TAllocTraits::MakeCapacity(x.size());
		m_pStream = m_Alloc.allocate(m_cbCapacity);
		memcpy(Data(), x.begin(), x.size());
	}

	~CRefBinT()
	{
		m_Alloc.deallocate(m_pStream, m_cbCapacity);
	}

	CRefBinT& operator=(CRefBinT& x)
	{
		if constexpr (!TAllocTraits::is_always_equal::value)
		{
			if (m_Alloc != x.m_Alloc)
			{
				m_Alloc = x.m_Alloc;
				m_Alloc.deallocate(m_pStream, m_cbCapacity);
				m_pStream = NULL;
				m_cb = m_cbCapacity = 0;
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

	BYTE& operator[](size_t idx) { EckAssert(idx < Size()); return *(Data() + idx); }

	BYTE operator[](size_t idx) const { EckAssert(idx < Size()); return *(Data() + idx); }

	template<class T>
	EckInline CRefBinT& operator<<(const T& x)
	{
		PushBack(&x, sizeof(T));
		return *this;
	}

	template<class TAlloc1>
	EckInline CRefBinT& operator<<(const CRefBinT<TAlloc1>& x)
	{
		PushBack(x.Data(), x.Size());
		return *this;
	}

	template<class T, class TAlloc1>
	EckInline CRefBinT& operator<<(const std::vector<T, TAlloc1>& x)
	{
		PushBack(x.data(), x.size() * sizeof(T));
		return *this;
	}

	template<class TChar, class TTraits, class TAlloc1>
	EckInline CRefBinT& operator<<(const std::basic_string<TChar, TTraits, TAlloc1>& x)
	{
		PushBack(x.c_str(), x.size() * sizeof(TChar));
		return *this;
	}

	template<class TChar, class TTraits, class TAlloc1>
	EckInline CRefBinT& operator<<(const CRefStrT<TChar, TTraits, TAlloc1>& x)
	{
		PushBack(x.Data(), x.Size() * sizeof(TChar));
		return *this;
	}

	template<class T>
	T& At(size_t idx) { EckAssert(idx * sizeof(T) <= Size() - sizeof(T)); return *((T*)Data() + idx); }

	template<class T>
	const T& At(size_t idx) const { EckAssert(idx * sizeof(T) <= Size() - sizeof(T)); return *((const T*)Data() + idx); }

	EckInline BYTE* Data() { return m_pStream; }

	EckInline const BYTE* Data() const { return m_pStream; }

	EckInline size_t Size() const { return m_cb; }

	/// <summary>
	/// 克隆字节集。
	/// 将指定字节集复制到自身
	/// </summary>
	/// <param name="p">字节集指针</param>
	/// <param name="cb">字节集长度，调用完成后尺寸与此参数相同</param>
	EckInline void DupStream(PCVOID p, size_t cb)
	{
		ReSizeExtra(cb);
		memcpy(Data(), p, cb);
	}

	template<class TAlloc1>
	EckInline void DupStream(const CRefBinT<TAlloc1>& rb)
	{
		DupStream(rb.Data(), rb.Size());
	}

	EckInline void DupStream(std::initializer_list<BYTE> x)
	{
		DupStream(x.begin(), x.size());
	}

	/// <summary>
	/// 依附指针。
	/// 分配器必须相同
	/// </summary>
	/// <param name="p">指针</param>
	/// <param name="cbCapacity">容量</param>
	/// <param name="cb">当前长度</param>
	/// <returns>先前的指针</returns>
	BYTE* Attach(BYTE* p, size_t cbCapacity, size_t cb)
	{
		const auto pOld = m_pStream;
		if (!p)
		{
			m_pStream = NULL;
			m_cb = m_cbCapacity = 0u;
			return pOld;
		}
		m_pStream = p;
		m_cbCapacity = cbCapacity;
		m_cb = cb;
		return pOld;
	}

	/// <summary>
	/// 拆离指针
	/// </summary>
	/// <returns></returns>
	EckInline BYTE* Detach()
	{
		const auto pOld = m_pStream;
		m_pStream = NULL;
		m_cbCapacity = 0u;
		m_cb = 0u;
		return pOld;
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

	/// <summary>
	/// 置长度
	/// </summary>
	/// <param name="cb">尺寸</param>
	EckInline void ReSize(size_t cb)
	{
		Reserve(cb);
		m_cb = cb;
	}

	EckInline void ReSizeExtra(size_t cb)
	{
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
	EckInline void Replace(size_t posStart, size_t cbReplacing, PCVOID pNew = NULL, size_t cbNew = 0u)
	{
		EckAssert(cbNew ? (!!pNew) : TRUE);
		const size_t cbOrg = m_cb;
		ReSizeExtra(Size() + cbNew - cbReplacing);
		memmove(Data() + posStart + cbNew, Data() + posStart + cbReplacing, cbOrg - posStart - cbReplacing);
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
		PCVOID pSrcBin, size_t cbSrcBin, size_t posStart = 0, int cReplacing = -1)
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

	/// <summary>
	/// 取空白字节集
	/// </summary>
	/// <param name="cbSize">长度</param>
	/// <param name="posStart">起始位置</param>
	EckInline void MakeEmpty(size_t cbSize, size_t posStart = 0u)
	{
		ReSizeExtra(cbSize + posStart);
		RtlZeroMemory(Data() + posStart, cbSize);
	}

	/// <summary>
	/// 取重复字节集
	/// </summary>
	/// <param name="pBin">字节集指针</param>
	/// <param name="cbBin">字节集长度</param>
	/// <param name="cCount">重复次数</param>
	/// <param name="posStart">起始位置</param>
	void MakeRepeatedBinSequence(PCVOID pBin, size_t cbBin, size_t cCount, size_t posStart = 0u)
	{
		ReSizeExtra(posStart + cCount * cbBin);
		BYTE* pCurr;
		size_t i;
		for (i = 0, pCurr = m_pStream + posStart; i < cCount; ++i, pCurr += cbBin)
			memcpy(pCurr, pBin, cbBin);
	}

	/// <summary>
	/// 尾插
	/// </summary>
	/// <param name="p">要插入的字节集指针</param>
	/// <param name="cb">要插入的字节集长度</param>
	EckInline void PushBack(PCVOID p, size_t cb)
	{
		ReSizeExtra(m_cb + cb);
		memcpy(Data() + Size() - cb, p, cb);
	}

	EckInline void PushBack(const CRefBinT& rb)
	{
		PushBack(rb.Data(), rb.Size());
	}

	EckInline BYTE* PushBack(size_t cb)
	{
		ReSizeExtra(m_cb + cb);
		return Data() + m_cb - cb;
	}

	template<class T>
	EckInline T* PushBack()
	{
		return (T*)PushBack(sizeof(T));
	}

	/// <summary>
	/// 尾删
	/// </summary>
	/// <param name="cb">要删除的长度</param>
	EckInline void PopBack(size_t cb)
	{
		EckAssert(m_cb >= cb);
		m_cb -= cb;
	}

	EckInline void Clear() { m_cb = 0u; }

	EckInline void Zero() { RtlZeroMemory(Data(), Size()); }

	/// <summary>
	/// 从左闭右开区间创建字节集
	/// </summary>
	template<class TAlloc1 = TAlloc>
	EckInline CRefBinT<TAlloc> SubBin(size_t posBegin, size_t posEnd)
	{
		return CRefBinT<TAlloc>(Data() + posBegin, posEnd - posBegin);
	}

	EckInline void Insert(size_t pos, PCVOID p, size_t cb)
	{
		EckAssert(pos <= Size());
		EckAssert(p ? TRUE : (cb == 0));
		ReSizeExtra(Size() + cb);
		memmove(
			Data() + pos + cb,
			Data() + pos,
			Size() - cb - pos);
		memcpy(Data() + pos, p, cb);
	}

	template<class TAlloc1 = TAlloc>
	EckInline void Insert(size_t pos, const CRefBinT<TAlloc1>& rb)
	{
		Insert(pos, rb.Data(), rb.Size());
	}

	EckInline void Insert(size_t pos, std::initializer_list<BYTE> il)
	{
		Insert(pos, il.begin(), il.size());
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

using CRefBin = CRefBinT<CAllocatorProcHeap<BYTE>>;

template<class TAlloc = CAllocatorProcHeap<BYTE>, class TAlloc1, class TAlloc2>
CRefBinT<TAlloc> operator+(const CRefBinT<TAlloc1>& rb1, const CRefBinT<TAlloc2>& rb2)
{
	CRefBinT<TAlloc> rb(rb1.Size() + rb2.Size());
	memcpy(rb.Data(), rb1.Data(), rb1.Size());
	memcpy(rb.Data() + rb1.Size(), rb2.Data(), rb2.Size());
	return rb;
}

CRefStrW BinToFriendlyString(PCBYTE pData, SIZE_T cb, int iType);
template<class TAlloc>
EckInline void DbgPrint(const CRefBinT<TAlloc>& rb,int iType=1, BOOL bNewLine = TRUE)
{
	auto rs = BinToFriendlyString(rb.Data(), rb.Size(), iType);
	OutputDebugStringW(rs.Data());
	if (bNewLine)
		OutputDebugStringW(L"\n");
}

/// <summary>
/// 到字节集
/// </summary>
/// <typeparam name="T"></typeparam>
/// <param name="x"></param>
/// <returns></returns>
template<class T>
EckInline CRefBin ToBin(T x)
{
	return CRefBin(&x, sizeof(T));
}

/// <summary>
/// 字节集到
/// </summary>
/// <typeparam name="T"></typeparam>
/// <param name="rb"></param>
/// <returns></returns>
template<class T, class TAlloc>
EckInline T BinToData(const CRefBinT<TAlloc>& rb)
{
	EckAssert(sizeof(T) <= rb.Size());
	T x;
	rb.CopyTo(&x, sizeof(T));
	return x;
}

/// <summary>
/// 字节集到
/// </summary>
/// <typeparam name="T"></typeparam>
/// <param name="p"></param>
/// <returns></returns>
template<class T>
EckInline T BinToData(PCVOID p)
{
	T x;
	memcpy(&x, p, sizeof(T));
	return x;
}

/// <summary>
/// 取字节集左边
/// </summary>
/// <param name="p">字节集</param>
/// <param name="cbLeft">左边长度</param>
/// <returns></returns>
EckInline CRefBin BinLeft(PCVOID p, size_t cbLeft)
{
	CRefBin rb;
	rb.DupStream(p, cbLeft);
	return rb;
}

/// <summary>
/// 取字节集右边
/// </summary>
/// <param name="p">字节集</param>
/// <param name="cbSize">字节集长度</param>
/// <param name="cbRight">右边长度</param>
/// <returns></returns>
EckInline CRefBin BinRight(PCVOID p, size_t cbSize, size_t cbRight)
{
	CRefBin rb;
	rb.DupStream((PCBYTE)p + cbSize - cbRight, cbRight);
	return rb;
}

/// <summary>
/// 取字节集中间
/// </summary>
/// <param name="p">字节集</param>
/// <param name="posStart">起始位置</param>
/// <param name="cbMid">中间长度</param>
/// <returns></returns>
EckInline CRefBin BinMid(PCVOID p, size_t posStart, size_t cbMid)
{
	CRefBin rb;
	rb.DupStream((PCBYTE)p + posStart, cbMid);
	return rb;
}

/// <summary>
/// 寻找字节集
/// </summary>
/// <param name="rbMain">要在其中寻找的字节集</param>
/// <param name="rbSub">要寻找的字节集</param>
/// <param name="posStart">起始位置</param>
/// <returns>位置，若未找到返回BinNPos</returns>
template<class TAlloc1, class TAlloc2>
EckInline size_t FindBin(const CRefBinT<TAlloc1>& rbMain, const CRefBinT<TAlloc1>& rbSub, size_t posStart = 0)
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
EckInline size_t FindBinRev(const CRefBin& rbMain, const CRefBin& rbSub, size_t posStart = 0)
{
	return FindBinRev(rbMain.Data(), rbMain.Size(), rbSub.Data(), rbSub.Size(), posStart);
}

template<class TProcesser>
inline void SplitBin(PCVOID p, SIZE_T cbSize, PCVOID pDiv, SIZE_T cbDiv, int cBinExpected, TProcesser Processer)
{
	SIZE_T pos = FindBin(p, cbSize, pDiv, cbDiv);
	SIZE_T posPrevFirst = 0u;
	int c = 0;
	while (pos != BinNPos)
	{
		Processer((BYTE*)p + posPrevFirst, pos - posPrevFirst);
		++c;
		if (c == cBinExpected)
			return;
		posPrevFirst = pos + cbDiv;
		pos = FindBin(p, cbSize, pDiv, cbDiv, posPrevFirst);
	}

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