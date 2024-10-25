/*
* WinEzCtrlKit Library
*
* CRefBinStream.h ： 字节集流
*
* Copyright(C) 2023-2024 QingKong
*/
#pragma once
#include "CRefBin.h"
#include "IMem.h"

ECK_NAMESPACE_BEGIN
/// <summary>
/// 字节集流。
/// CRefBinT的IStream实现。
/// 非线程安全
/// </summary>
template<class TAlloc>
class CRefBinStreamT :public IStream, public IMem
{
private:
	ULONG m_cRef{ 1ul };
	BOOL m_bLocked{};

	CRefBinT<TAlloc>& m_rb;
	size_t m_posSeek{};	// 相对于m_rb的起始位置
	size_t m_posBegin{};// 若要强制追加数据，则此字段记录追加起始位置
public:
	CRefBinStreamT(CRefBinT<TAlloc>& rb) :m_rb{ rb } {}

	// 强制从指定位置追加数据
	EckInline constexpr void SetBeginPos(size_t pos)
	{
		m_posBegin = pos;
	}

	// 强制从当前尾部追加数据
	EckInline constexpr void SetBeginPos()
	{
		SetBeginPos(m_rb.Size());
		if (m_posSeek < m_posBegin)
			m_posSeek = m_posBegin;
	}

	EckInline void LeaveRelease()
	{
#ifdef _DEBUG
		EckAssert(Release() == 0);
#else
		Release();
#endif
	}

	ULONG STDMETHODCALLTYPE AddRef(void)
	{
		EckAssert(m_cRef);
		return ++m_cRef;
	}

	ULONG STDMETHODCALLTYPE Release(void)
	{
		EckAssert(m_cRef);
		if (m_cRef == 1)
		{
			delete this;
			return 0;
		}
		return --m_cRef;
	}

	HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppvObject)
	{
		const static QITAB qit[]
		{
			QITABENT(CRefBinStreamT, IStream),
			QITABENT(CRefBinStreamT, ISequentialStream),
			QITABENT(CRefBinStreamT, IMem),
			{}
		};

		return QISearch(this, qit, riid, ppvObject);
	}

	HRESULT STDMETHODCALLTYPE Read(void* pv, ULONG cb, ULONG* pcbRead)
	{
		if (pcbRead)
			*pcbRead = 0;
		if (!pv)
			return STG_E_INVALIDPOINTER;
		if (m_posSeek > m_rb.Size())
			return S_FALSE;

		HRESULT hr;
		if (m_posSeek + cb > m_rb.Size())
		{
			cb = (ULONG)(m_rb.Size() - m_posSeek);
			hr = S_FALSE;
		}
		else
			hr = S_OK;
		memmove(pv, m_rb.Data() + m_posSeek, cb);
		m_posSeek += cb;
		if (pcbRead)
			*pcbRead = cb;
		return hr;
	}

	HRESULT STDMETHODCALLTYPE Write(const void* pv, ULONG cb, ULONG* pcbWritten)
	{
		if (pcbWritten)
			*pcbWritten = 0;
		if (!pv)
			return STG_E_INVALIDPOINTER;
		if (m_bLocked)
			return STG_E_ACCESSDENIED;

		if (m_posSeek + cb > m_rb.Size())
			m_rb.ReSizeExtra(m_posSeek + cb);
		memmove(m_rb.Data() + m_posSeek, pv, cb);
		m_posSeek += cb;
		if (pcbWritten)
			*pcbWritten = cb;
		return S_OK;
	}

	HRESULT STDMETHODCALLTYPE Seek(LARGE_INTEGER dlibMove, DWORD dwOrigin, ULARGE_INTEGER* plibNewPosition)
	{
		if (plibNewPosition)
			plibNewPosition->QuadPart = m_posSeek - m_posBegin;

		switch (dwOrigin)
		{
		case SEEK_SET:// 这种情况dlibMove应视为无符号
			m_posSeek = (size_t)dlibMove.QuadPart + m_posBegin;
			if (plibNewPosition)
				*plibNewPosition = ToUli(dlibMove);
			return S_OK;

		case SEEK_CUR:
		{
			const ptrdiff_t ocbNew = (ptrdiff_t)dlibMove.QuadPart + m_posSeek;
			if (ocbNew < (ptrdiff_t)m_posBegin)// 落在流开始之前
				return STG_E_INVALIDFUNCTION;
			m_posSeek = (size_t)ocbNew;
			if (plibNewPosition)
				plibNewPosition->QuadPart = m_posSeek - m_posBegin;
		}
		return S_OK;

		case SEEK_END:
			if (m_posBegin >= m_rb.Size())
				return E_FAIL;
			if (dlibMove.QuadPart < -((ptrdiff_t)m_rb.Size() - (ptrdiff_t)m_posBegin))// 落在流开始之前
				return STG_E_INVALIDFUNCTION;
			m_posSeek = m_rb.Size() + (size_t)dlibMove.QuadPart;
			if (plibNewPosition)
				plibNewPosition->QuadPart = m_posSeek - m_posBegin;
			return S_OK;
		}
		return STG_E_INVALIDFUNCTION;
	}

	HRESULT STDMETHODCALLTYPE SetSize(ULARGE_INTEGER libNewSize)
	{
		if (m_bLocked)
			return STG_E_ACCESSDENIED;
		else
			m_rb.ReSizeExtra((size_t)libNewSize.QuadPart + m_posBegin);
		return S_OK;
	}

	HRESULT STDMETHODCALLTYPE CopyTo(IStream* pstm, ULARGE_INTEGER cb,
		ULARGE_INTEGER* pcbRead, ULARGE_INTEGER* pcbWritten)
	{
		if (pcbRead)
			pcbRead->QuadPart = 0u;
		if (pcbWritten)
			pcbWritten->QuadPart = 0u;
		if (!pstm)
			return STG_E_INVALIDPOINTER;
		if (m_posSeek >= m_rb.Size())
			return S_FALSE;

		ULONG cbRead;
		if (m_posSeek + cb.LowPart > m_rb.Size())
			cbRead = (ULONG)(m_rb.Size() - m_posSeek);
		else
			cbRead = cb.LowPart;
		ULONG cbWritten{};
		const auto hr = pstm->Write(m_rb.Data() + m_posSeek, cbRead, &cbWritten);
		if (pcbRead)
			pcbRead->QuadPart = cbRead;
		if (pcbWritten)
			pcbWritten->QuadPart = cbWritten;
		return hr;
	}

	HRESULT STDMETHODCALLTYPE Commit(DWORD grfCommitFlags)
	{
		return S_OK;
	}

	HRESULT STDMETHODCALLTYPE Revert(void)
	{
		return E_NOTIMPL;
	}

	HRESULT STDMETHODCALLTYPE LockRegion(ULARGE_INTEGER libOffset, ULARGE_INTEGER cb, DWORD dwLockType)
	{
		return E_NOTIMPL;
	}

	HRESULT STDMETHODCALLTYPE UnlockRegion(ULARGE_INTEGER libOffset, ULARGE_INTEGER cb, DWORD dwLockType)
	{
		return E_NOTIMPL;
	}

	HRESULT STDMETHODCALLTYPE Stat(STATSTG* pstatstg, DWORD grfStatFlag)
	{
		ZeroMemory(pstatstg, sizeof(STATSTG));
		pstatstg->type = STGTY_STREAM;
		pstatstg->cbSize.QuadPart = m_rb.Size();
		if (m_bLocked)
			pstatstg->grfMode = STGM_READ;
		else
			pstatstg->grfMode = STGM_READWRITE;
		return S_OK;
	}

	HRESULT STDMETHODCALLTYPE Clone(IStream** ppstm)
	{
		const auto p = new CRefBinStreamT(m_rb);
		p->m_posSeek = m_posSeek;
		p->m_posBegin = m_posBegin;
		p->m_bLocked = m_bLocked;
		*ppstm = p;
		return S_OK;
	}

	HRESULT STDMETHODCALLTYPE MemGetPtr(void** ppvData, SIZE_T* pcbData)
	{
		*ppvData = m_rb.Data();
		*pcbData = m_rb.Size();
		return S_OK;
	}

	HRESULT STDMETHODCALLTYPE MemLock(void** ppvData, SIZE_T* pcbData)
	{
		*ppvData = m_rb.Data();
		*pcbData = m_rb.Size();
		m_bLocked = TRUE;
		return S_OK;
	}

	HRESULT STDMETHODCALLTYPE MemUnlock()
	{
		m_bLocked = FALSE;
		return S_OK;
	}

	HRESULT STDMETHODCALLTYPE MemIsLocked(BOOL* pIsLocked)
	{
		*pIsLocked = m_bLocked;
		return S_OK;
	}
};

using CRefBinStream = CRefBinStreamT<TRefBinDefAllocator>;
ECK_NAMESPACE_END