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

#include <initializer_list>
#include <vector>

ECK_NAMESPACE_BEGIN
static constexpr SIZE_T INVALID_BIN_POS =
#ifdef _WIN64
ULLONG_MAX
#else
ULONG_MAX
#endif
;

/// <summary>
/// 寻找字节集
/// </summary>
/// <param name="pMain">要在其中寻找的字节集指针</param>
/// <param name="cbMainSize">要在其中寻找的字节集长度</param>
/// <param name="pSub">要寻找的字节集指针</param>
/// <param name="cbSubSize">要寻找的字节集长度</param>
/// <param name="posStart">起始位置</param>
/// <returns>位置，若未找到返回INVALID_BIN_POS</returns>
SIZE_T FindBin(PCVOID pMain, SIZE_T cbMainSize, PCVOID pSub, SIZE_T cbSubSize, SIZE_T posStart = 0);

/// <summary>
/// 倒找字节集
/// </summary>
/// <param name="pMain">要在其中寻找的字节集指针</param>
/// <param name="cbMainSize">要在其中寻找的字节集长度</param>
/// <param name="pSub">要寻找的字节集指针</param>
/// <param name="cbSubSize">要寻找的字节集长度</param>
/// <param name="posStart">起始位置</param>
/// <returns>位置，若未找到返回INVALID_BIN_POS</returns>
SIZE_T FindBinRev(PCVOID pMain, SIZE_T cbMainSize, PCVOID pSub, SIZE_T cbSubSize, SIZE_T posStart = 0);


class CRefBin
{
public:
	using TAlloc = CAllocator<BYTE>;

	BYTE* m_pStream = NULL;
	SIZE_T m_cb = 0u;
	SIZE_T m_cbCapacity = 0u;

	CRefBin() = default;

	/// <summary>
	/// 创建自长度
	/// </summary>
	/// <param name="cb"></param>
	CRefBin(SIZE_T cb);

	/// <summary>
	/// 创建自字节集
	/// </summary>
	/// <param name="p"></param>
	/// <param name="cb"></param>
	CRefBin(PCVOID p, SIZE_T cb);

	CRefBin(const CRefBin& x);

	CRefBin(CRefBin&& x) noexcept
	{
		m_pStream = x.m_pStream;
		m_cb = x.m_cb;
		m_cbCapacity = x.m_cbCapacity;
		ZeroMemory(&x, sizeof(CRefBin));
	}

	/// <summary>
	/// 创建自大括号初始化器
	/// </summary>
	/// <param name="x"></param>
	CRefBin(std::initializer_list<BYTE> x)
	{
		DupStream(x.begin(), x.end() - x.begin());
	}

	~CRefBin()
	{
		TAlloc::Free(m_pStream);
	}

	CRefBin& operator=(CRefBin& x)
	{
		DupStream(x.m_pStream, x.m_cb);
		return *this;
	}

	CRefBin& operator=(CRefBin&& x) noexcept
	{
		TAlloc::Free(m_pStream);
		m_pStream = x.m_pStream;
		m_cb = x.m_cb;
		m_cbCapacity = x.m_cbCapacity;
		ZeroMemory(&x, sizeof(CRefBin));
		return *this;
	}

	BYTE& operator[](SIZE_T x)
	{
		return *(m_pStream + x);
	}

	EckInline operator BYTE* () const
	{
		return m_pStream;
	}

	EckInline CRefBin& operator+(const CRefBin& x)
	{
		PushBack(x, x.m_cb);
		return *this;
	}

	EckInline BYTE* operator+(SIZE_T x) const
	{
		return m_pStream + x;
	}

	EckInline SIZE_T Size() const
	{
		return m_cb;
	}

	EckInline BYTE* Data() const
	{
		return m_pStream;
	}

	/// <summary>
	/// 克隆字节集。
	/// 将指定字节集复制到自身
	/// </summary>
	/// <param name="p">字节集指针</param>
	/// <param name="cb">字节集长度，调用完成后尺寸与此参数相同</param>
	void DupStream(PCVOID p, SIZE_T cb);

	/// <summary>
	/// 依附指针。
	/// 分配器必须相同
	/// </summary>
	/// <param name="p">指针</param>
	/// <param name="cbCapacity">容量</param>
	/// <param name="cb">当前长度</param>
	/// <returns>先前的指针</returns>
	BYTE* Attach(BYTE* p, SIZE_T cbCapacity, SIZE_T cb);

	/// <summary>
	/// 拆离指针
	/// </summary>
	/// <returns></returns>
	EckInline BYTE* Detach()
	{
		auto pOld = m_pStream;
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
	EckInline SIZE_T CopyTo(void* pDst, SIZE_T cbMax) const
	{
		if (cbMax > m_cb)
			cbMax = m_cb;
		if (!m_pStream || !pDst || !cbMax)
			return 0u;
		memcpy(pDst, m_pStream, cbMax);
		return cbMax;
	}

	/// <summary>
	/// 保留内存
	/// </summary>
	/// <param name="cb">尺寸</param>
	void Reserve(SIZE_T cb);

	/// <summary>
	/// 置长度
	/// </summary>
	/// <param name="cb">尺寸</param>
	EckInline void ReSize(SIZE_T cb)
	{
		Reserve(TAlloc::MakeCapacity(cb));
		m_cb = cb;
	}

	/// <summary>
	/// 置长度。
	/// 不会执行尺寸扩容预分配操作
	/// </summary>
	/// <param name="cb">尺寸</param>
	EckInline void ReSizeAbs(SIZE_T cb)
	{
		Reserve(cb);
		m_cb = cb;
	}

	/// <summary>
	/// 替换
	/// </summary>
	/// <param name="posStart">替换位置</param>
	/// <param name="cbReplacing">替换长度</param>
	/// <param name="pNew">用作替换的字节集指针</param>
	/// <param name="cbNew">用作替换的字节集长度</param>
	void Replace(SIZE_T posStart, SIZE_T cbReplacing, PCVOID pNew = NULL, SIZE_T cbNew = 0u);

	/// <summary>
	/// 替换
	/// </summary>
	/// <param name="posStart">替换位置</param>
	/// <param name="cbReplacing">替换长度</param>
	/// <param name="rb">用作替换的字节集</param>
	EckInline void Replace(SIZE_T posStart, SIZE_T cbReplacing, const CRefBin& rb)
	{
		Replace(posStart, cbReplacing, rb.m_pStream, rb.m_cb);
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
	void ReplaceSubBin(PCVOID pReplacedBin, SIZE_T cbReplacedBin, PCVOID pSrcBin, SIZE_T cbSrcBin, SIZE_T posStart = 0, int cReplacing = -1);

	/// <summary>
	/// 子字节集替换
	/// </summary>
	/// <param name="rbReplacedBin">被替换的字节集</param>
	/// <param name="rbSrcBin">用作替换的字节集</param>
	/// <param name="posStart">起始位置</param>
	/// <param name="cReplacing">替换进行的次数，0为执行所有替换</param>
	EckInline void ReplaceSubBin(const CRefBin& rbReplacedBin, const CRefBin& rbSrcBin, SIZE_T posStart = 0, int cReplacing = -1)
	{
		ReplaceSubBin(rbReplacedBin, rbReplacedBin.m_cb, rbSrcBin, rbSrcBin.m_cb, posStart, cReplacing);
	}

	/// <summary>
	/// 取空白字节集
	/// </summary>
	/// <param name="cbSize">长度</param>
	/// <param name="posStart">起始位置</param>
	EckInline void MakeEmpty(SIZE_T cbSize, SIZE_T posStart = 0u)
	{
		ReSize(cbSize + posStart);
		ZeroMemory(m_pStream + posStart, cbSize);
	}

	/// <summary>
	/// 取重复字节集
	/// </summary>
	/// <param name="pBin">字节集指针</param>
	/// <param name="cbBin">字节集长度</param>
	/// <param name="cCount">重复次数</param>
	/// <param name="posStart">起始位置</param>
	void MakeRepeatedBinSequence(PCVOID pBin, SIZE_T cbBin, SIZE_T cCount, SIZE_T posStart = 0u);

	/// <summary>
	/// 尾插
	/// </summary>
	/// <param name="p">要插入的字节集指针</param>
	/// <param name="cb">要插入的字节集长度</param>
	EckInline void PushBack(PCVOID p, SIZE_T cb)
	{
		ReSize(m_cb + cb);
		memcpy(m_pStream + m_cb - cb, p, cb);
	}

	/// <summary>
	/// 尾删
	/// </summary>
	/// <param name="cb">要删除的长度</param>
	EckInline void PopBack(SIZE_T cb)
	{
		ReSize(m_cb - cb);
	}
};

/// <summary>
/// 到字节集
/// </summary>
/// <typeparam name="T"></typeparam>
/// <param name="x"></param>
/// <returns></returns>
template<class T>
EckInline CRefBin ToBin(T x)
{
	CRefBin rb;
	rb.DupStream(&x, sizeof(T));
	return rb;
}

/// <summary>
/// 字节集到
/// </summary>
/// <typeparam name="T"></typeparam>
/// <param name="rb"></param>
/// <returns></returns>
template<class T>
EckInline T BinToData(const CRefBin& rb)
{
	assert(sizeof(T) <= rb.m_cb);
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
EckInline T BinToData(PCVOID* p)
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
EckInline CRefBin BinLeft(PCVOID p, SIZE_T cbLeft)
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
EckInline CRefBin BinRight(PCVOID p,SIZE_T cbSize, SIZE_T cbRight)
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
EckInline CRefBin BinMid(PCVOID p, SIZE_T posStart, SIZE_T cbMid)
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
/// <returns>位置，若未找到返回INVALID_BIN_POS</returns>
EckInline SIZE_T FindBin(const CRefBin& rbMain, const CRefBin& rbSub, SIZE_T posStart = 0)
{
	return FindBin(rbMain, rbMain.m_cb, rbSub, rbSub.m_cb, posStart);
}

/// <summary>
/// 倒找字节集
/// </summary>
/// <param name="rbMain">要在其中寻找的字节集</param>
/// <param name="rbSub">要寻找的字节集</param>
/// <param name="posStart">起始位置</param>
/// <returns>位置，若未找到返回INVALID_BIN_POS</returns>
EckInline SIZE_T FindBinRev(const CRefBin& rbMain, const CRefBin& rbSub, SIZE_T posStart = 0)
{
	return FindBinRev(rbMain, rbMain.m_cb, rbSub, rbSub.m_cb, posStart);
}

struct SPLITBININFO
{
	PCVOID pBin;
	SIZE_T cbBin;
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
void SplitBin(PCVOID p, SIZE_T cbSize, PCVOID pDiv, SIZE_T cbDiv, std::vector<SPLITBININFO>& aResult, int cBinExpected = 0);

/// <summary>
/// 分割字节集。
/// 此函数不执行任何复制操作
/// </summary>
/// <param name="rb">要分割的字节集</param>
/// <param name="rbDiv">用作分割的字节集</param>
/// <param name="aResult">结果容器</param>
/// <param name="cBinExpected">返回的最大子字节集个数</param>
EckInline void SplitBin(const CRefBin& rb, const CRefBin& rbDiv, std::vector<SPLITBININFO>& aResult, int cBinExpected = 0)
{
	SplitBin(rb, rb.m_cb, rbDiv, rbDiv.m_cb, aResult, cBinExpected);
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
void SplitBin(PCVOID p, SIZE_T cbSize, PCVOID pDiv, SIZE_T cbDiv, std::vector<CRefBin>& aResult, int cBinExpected = 0);

/// <summary>
/// 分割字节集
/// </summary>
/// <param name="rb">要分割的字节集</param>
/// <param name="rbDiv">用作分割的字节集</param>
/// <param name="aResult">结果容器</param>
/// <param name="cBinExpected">返回的最大子字节集个数</param>
EckInline void SplitBin(const CRefBin& rb, const CRefBin& rbDiv, std::vector<CRefBin>& aResult, int cBinExpected = 0)
{
	SplitBin(rb, rb.m_cb, rbDiv, rbDiv.m_cb, aResult, cBinExpected);
}
ECK_NAMESPACE_END