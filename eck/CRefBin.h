/*
* WinEzCtrlKit Library
*
* CRefBin.h �� �ֽڼ�
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
/// Ѱ���ֽڼ�
/// </summary>
/// <param name="pMain">Ҫ������Ѱ�ҵ��ֽڼ�ָ��</param>
/// <param name="cbMainSize">Ҫ������Ѱ�ҵ��ֽڼ�����</param>
/// <param name="pSub">ҪѰ�ҵ��ֽڼ�ָ��</param>
/// <param name="cbSubSize">ҪѰ�ҵ��ֽڼ�����</param>
/// <param name="posStart">��ʼλ��</param>
/// <returns>λ�ã���δ�ҵ�����INVALID_BIN_POS</returns>
SIZE_T FindBin(PCVOID pMain, SIZE_T cbMainSize, PCVOID pSub, SIZE_T cbSubSize, SIZE_T posStart = 0);

/// <summary>
/// �����ֽڼ�
/// </summary>
/// <param name="pMain">Ҫ������Ѱ�ҵ��ֽڼ�ָ��</param>
/// <param name="cbMainSize">Ҫ������Ѱ�ҵ��ֽڼ�����</param>
/// <param name="pSub">ҪѰ�ҵ��ֽڼ�ָ��</param>
/// <param name="cbSubSize">ҪѰ�ҵ��ֽڼ�����</param>
/// <param name="posStart">��ʼλ��</param>
/// <returns>λ�ã���δ�ҵ�����INVALID_BIN_POS</returns>
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
	/// �����Գ���
	/// </summary>
	/// <param name="cb"></param>
	CRefBin(SIZE_T cb);

	/// <summary>
	/// �������ֽڼ�
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
	/// �����Դ����ų�ʼ����
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
	/// ��¡�ֽڼ���
	/// ��ָ���ֽڼ����Ƶ�����
	/// </summary>
	/// <param name="p">�ֽڼ�ָ��</param>
	/// <param name="cb">�ֽڼ����ȣ�������ɺ�ߴ���˲�����ͬ</param>
	void DupStream(PCVOID p, SIZE_T cb);

	/// <summary>
	/// ����ָ�롣
	/// ������������ͬ
	/// </summary>
	/// <param name="p">ָ��</param>
	/// <param name="cbCapacity">����</param>
	/// <param name="cb">��ǰ����</param>
	/// <returns>��ǰ��ָ��</returns>
	BYTE* Attach(BYTE* p, SIZE_T cbCapacity, SIZE_T cb);

	/// <summary>
	/// ����ָ��
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
	/// ���Ƶ�
	/// </summary>
	/// <param name="pDst">Ŀ��ָ��</param>
	/// <param name="cbMax">��󳤶ȣ����������Զ�����</param>
	/// <returns>���Ƶĳ���</returns>
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
	/// �����ڴ�
	/// </summary>
	/// <param name="cb">�ߴ�</param>
	void Reserve(SIZE_T cb);

	/// <summary>
	/// �ó���
	/// </summary>
	/// <param name="cb">�ߴ�</param>
	EckInline void ReSize(SIZE_T cb)
	{
		Reserve(TAlloc::MakeCapacity(cb));
		m_cb = cb;
	}

	/// <summary>
	/// �ó��ȡ�
	/// ����ִ�гߴ�����Ԥ�������
	/// </summary>
	/// <param name="cb">�ߴ�</param>
	EckInline void ReSizeAbs(SIZE_T cb)
	{
		Reserve(cb);
		m_cb = cb;
	}

	/// <summary>
	/// �滻
	/// </summary>
	/// <param name="posStart">�滻λ��</param>
	/// <param name="cbReplacing">�滻����</param>
	/// <param name="pNew">�����滻���ֽڼ�ָ��</param>
	/// <param name="cbNew">�����滻���ֽڼ�����</param>
	void Replace(SIZE_T posStart, SIZE_T cbReplacing, PCVOID pNew = NULL, SIZE_T cbNew = 0u);

	/// <summary>
	/// �滻
	/// </summary>
	/// <param name="posStart">�滻λ��</param>
	/// <param name="cbReplacing">�滻����</param>
	/// <param name="rb">�����滻���ֽڼ�</param>
	EckInline void Replace(SIZE_T posStart, SIZE_T cbReplacing, const CRefBin& rb)
	{
		Replace(posStart, cbReplacing, rb.m_pStream, rb.m_cb);
	}

	/// <summary>
	/// ���ֽڼ��滻
	/// </summary>
	/// <param name="pReplacedBin">���滻���ֽڼ�ָ��</param>
	/// <param name="cbReplacedBin">���滻���ֽڼ�����</param>
	/// <param name="pSrcBin">�����滻���ֽڼ�ָ��</param>
	/// <param name="cbSrcBin">�����滻���ֽڼ�����</param>
	/// <param name="posStart">��ʼλ��</param>
	/// <param name="cReplacing">�滻���еĴ�����0Ϊִ�������滻</param>
	void ReplaceSubBin(PCVOID pReplacedBin, SIZE_T cbReplacedBin, PCVOID pSrcBin, SIZE_T cbSrcBin, SIZE_T posStart = 0, int cReplacing = -1);

	/// <summary>
	/// ���ֽڼ��滻
	/// </summary>
	/// <param name="rbReplacedBin">���滻���ֽڼ�</param>
	/// <param name="rbSrcBin">�����滻���ֽڼ�</param>
	/// <param name="posStart">��ʼλ��</param>
	/// <param name="cReplacing">�滻���еĴ�����0Ϊִ�������滻</param>
	EckInline void ReplaceSubBin(const CRefBin& rbReplacedBin, const CRefBin& rbSrcBin, SIZE_T posStart = 0, int cReplacing = -1)
	{
		ReplaceSubBin(rbReplacedBin, rbReplacedBin.m_cb, rbSrcBin, rbSrcBin.m_cb, posStart, cReplacing);
	}

	/// <summary>
	/// ȡ�հ��ֽڼ�
	/// </summary>
	/// <param name="cbSize">����</param>
	/// <param name="posStart">��ʼλ��</param>
	EckInline void MakeEmpty(SIZE_T cbSize, SIZE_T posStart = 0u)
	{
		ReSize(cbSize + posStart);
		ZeroMemory(m_pStream + posStart, cbSize);
	}

	/// <summary>
	/// ȡ�ظ��ֽڼ�
	/// </summary>
	/// <param name="pBin">�ֽڼ�ָ��</param>
	/// <param name="cbBin">�ֽڼ�����</param>
	/// <param name="cCount">�ظ�����</param>
	/// <param name="posStart">��ʼλ��</param>
	void MakeRepeatedBinSequence(PCVOID pBin, SIZE_T cbBin, SIZE_T cCount, SIZE_T posStart = 0u);

	/// <summary>
	/// β��
	/// </summary>
	/// <param name="p">Ҫ������ֽڼ�ָ��</param>
	/// <param name="cb">Ҫ������ֽڼ�����</param>
	EckInline void PushBack(PCVOID p, SIZE_T cb)
	{
		ReSize(m_cb + cb);
		memcpy(m_pStream + m_cb - cb, p, cb);
	}

	/// <summary>
	/// βɾ
	/// </summary>
	/// <param name="cb">Ҫɾ���ĳ���</param>
	EckInline void PopBack(SIZE_T cb)
	{
		ReSize(m_cb - cb);
	}
};

/// <summary>
/// ���ֽڼ�
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
/// �ֽڼ���
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
/// �ֽڼ���
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
/// ȡ�ֽڼ����
/// </summary>
/// <param name="p">�ֽڼ�</param>
/// <param name="cbLeft">��߳���</param>
/// <returns></returns>
EckInline CRefBin BinLeft(PCVOID p, SIZE_T cbLeft)
{
	CRefBin rb;
	rb.DupStream(p, cbLeft);
	return rb;
}

/// <summary>
/// ȡ�ֽڼ��ұ�
/// </summary>
/// <param name="p">�ֽڼ�</param>
/// <param name="cbSize">�ֽڼ�����</param>
/// <param name="cbRight">�ұ߳���</param>
/// <returns></returns>
EckInline CRefBin BinRight(PCVOID p,SIZE_T cbSize, SIZE_T cbRight)
{
	CRefBin rb;
	rb.DupStream((PCBYTE)p + cbSize - cbRight, cbRight);
	return rb;
}

/// <summary>
/// ȡ�ֽڼ��м�
/// </summary>
/// <param name="p">�ֽڼ�</param>
/// <param name="posStart">��ʼλ��</param>
/// <param name="cbMid">�м䳤��</param>
/// <returns></returns>
EckInline CRefBin BinMid(PCVOID p, SIZE_T posStart, SIZE_T cbMid)
{
	CRefBin rb;
	rb.DupStream((PCBYTE)p + posStart, cbMid);
	return rb;
}

/// <summary>
/// Ѱ���ֽڼ�
/// </summary>
/// <param name="rbMain">Ҫ������Ѱ�ҵ��ֽڼ�</param>
/// <param name="rbSub">ҪѰ�ҵ��ֽڼ�</param>
/// <param name="posStart">��ʼλ��</param>
/// <returns>λ�ã���δ�ҵ�����INVALID_BIN_POS</returns>
EckInline SIZE_T FindBin(const CRefBin& rbMain, const CRefBin& rbSub, SIZE_T posStart = 0)
{
	return FindBin(rbMain, rbMain.m_cb, rbSub, rbSub.m_cb, posStart);
}

/// <summary>
/// �����ֽڼ�
/// </summary>
/// <param name="rbMain">Ҫ������Ѱ�ҵ��ֽڼ�</param>
/// <param name="rbSub">ҪѰ�ҵ��ֽڼ�</param>
/// <param name="posStart">��ʼλ��</param>
/// <returns>λ�ã���δ�ҵ�����INVALID_BIN_POS</returns>
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
/// �ָ��ֽڼ���
/// �˺�����ִ���κθ��Ʋ���
/// </summary>
/// <param name="p">Ҫ�ָ���ֽڼ�ָ��</param>
/// <param name="cbSize">Ҫ�ָ���ֽڼ�����</param>
/// <param name="pDiv">�����ָ���ֽڼ�ָ��</param>
/// <param name="cbDiv">�����ָ���ֽڼ�����</param>
/// <param name="aResult">�������</param>
/// <param name="cBinExpected">���ص�������ֽڼ�����</param>
void SplitBin(PCVOID p, SIZE_T cbSize, PCVOID pDiv, SIZE_T cbDiv, std::vector<SPLITBININFO>& aResult, int cBinExpected = 0);

/// <summary>
/// �ָ��ֽڼ���
/// �˺�����ִ���κθ��Ʋ���
/// </summary>
/// <param name="rb">Ҫ�ָ���ֽڼ�</param>
/// <param name="rbDiv">�����ָ���ֽڼ�</param>
/// <param name="aResult">�������</param>
/// <param name="cBinExpected">���ص�������ֽڼ�����</param>
EckInline void SplitBin(const CRefBin& rb, const CRefBin& rbDiv, std::vector<SPLITBININFO>& aResult, int cBinExpected = 0)
{
	SplitBin(rb, rb.m_cb, rbDiv, rbDiv.m_cb, aResult, cBinExpected);
}

/// <summary>
/// �ָ��ֽڼ�
/// </summary>
/// <param name="p">Ҫ�ָ���ֽڼ�ָ��</param>
/// <param name="cbSize">Ҫ�ָ���ֽڼ�����</param>
/// <param name="pDiv">�����ָ���ֽڼ�ָ��</param>
/// <param name="cbDiv">�����ָ���ֽڼ�����</param>
/// <param name="aResult">�������</param>
/// <param name="cBinExpected">���ص�������ֽڼ�����</param>
void SplitBin(PCVOID p, SIZE_T cbSize, PCVOID pDiv, SIZE_T cbDiv, std::vector<CRefBin>& aResult, int cBinExpected = 0);

/// <summary>
/// �ָ��ֽڼ�
/// </summary>
/// <param name="rb">Ҫ�ָ���ֽڼ�</param>
/// <param name="rbDiv">�����ָ���ֽڼ�</param>
/// <param name="aResult">�������</param>
/// <param name="cBinExpected">���ص�������ֽڼ�����</param>
EckInline void SplitBin(const CRefBin& rb, const CRefBin& rbDiv, std::vector<CRefBin>& aResult, int cBinExpected = 0)
{
	SplitBin(rb, rb.m_cb, rbDiv, rbDiv.m_cb, aResult, cBinExpected);
}
ECK_NAMESPACE_END