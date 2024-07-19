/*
* WinEzCtrlKit Library
*
* CStreamView.h ： 流视图
* 只读IStream接口的实现，在一些情况下可以避免内存复制
*
* Copyright(C) 2023 QingKong
*/
#pragma once
#include "CRefBin.h"
#include "IMem.h"

#include <Shlwapi.h>

ECK_NAMESPACE_BEGIN
class CStreamView :public IStream, public IMem
{
private:
	ULONG m_cRef = 1;

	PCBYTE m_pMem = NULL;
	PCBYTE m_pSeek = NULL;
	SIZE_T m_cbSize = 0u;
public:
	CStreamView() = default;
	constexpr CStreamView(PCVOID p, SIZE_T cb)
		:m_pMem{ (PCBYTE)p }, m_cbSize{ cb }, m_pSeek{ (PCBYTE)p } {}

	template<class TAlloc>
	CStreamView(const CRefBinT<TAlloc>& rb)
		: m_pMem{ rb.Data() }, m_cbSize{ rb.Size() }, m_pSeek{ rb.Data() } {}

	template<class T, class TAlloc>
	CStreamView(const std::vector<T, TAlloc>& v)
		: m_pMem{ (PCBYTE)v.data() }, m_cbSize{ v.size() * sizeof(T) }, m_pSeek{ (PCBYTE)v.data() } {}

	EckInline constexpr void SetData(PCVOID p, SIZE_T cb)
	{
		m_pMem = (PCBYTE)p;
		m_pSeek = m_pMem;
		m_cbSize = cb;
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
		const QITAB qit[]
		{
			QITABENT(CStreamView, IStream),
			QITABENT(CStreamView, ISequentialStream),
			QITABENT(CStreamView, IMem),
			{}
		};

		return QISearch(this, qit, riid, ppvObject);
	}

	HRESULT STDMETHODCALLTYPE Read(void* pv, ULONG cb, ULONG* pcbRead)
	{
		ULONG Dummy;
		if (!pcbRead) pcbRead = &Dummy;
		if (!pv)
		{
			*pcbRead = 0;
			return STG_E_INVALIDPOINTER;
		}
		if (m_pSeek >= m_pMem + m_cbSize)
		{
			*pcbRead = 0;
			return S_FALSE;
		}
		HRESULT hr;
		if (m_pSeek + cb > m_pMem + m_cbSize)
		{
			cb = (ULONG)(m_pMem + m_cbSize - m_pSeek);
			hr = S_FALSE;
		}
		else
			hr = S_OK;
		memmove(pv, m_pSeek, cb);
		m_pSeek += cb;
		*pcbRead = cb;
		return hr;
	}

	HRESULT STDMETHODCALLTYPE Write(const void* pv, ULONG cb, ULONG* pcbWritten)
	{
		EckDbgBreak();
		return STG_E_ACCESSDENIED;
	}

	HRESULT STDMETHODCALLTYPE Seek(LARGE_INTEGER dlibMove, DWORD dwOrigin, ULARGE_INTEGER* plibNewPosition)
	{
		ULARGE_INTEGER Dummy;
		if (!plibNewPosition) plibNewPosition = &Dummy;
		switch (dwOrigin)
		{
		case SEEK_SET:// 这种情况dlibMove应视为无符号
			m_pSeek = m_pMem + (size_t)dlibMove.QuadPart;
			*plibNewPosition = ToUli(dlibMove);
			return S_OK;

		case SEEK_CUR:
		{
			const ptrdiff_t ocbNew = (ptrdiff_t)dlibMove.QuadPart + (m_pSeek - m_pMem);
			if (ocbNew < 0)// 落在流开始之前
				return STG_E_INVALIDFUNCTION;
			m_pSeek = m_pMem + (size_t)ocbNew;
			plibNewPosition->QuadPart = (ULONGLONG)ocbNew;
		}
		return S_OK;

		case SEEK_END:
			if (dlibMove.QuadPart < -(ptrdiff_t)m_cbSize)// 落在流开始之前
				return STG_E_INVALIDFUNCTION;
			m_pSeek = m_pMem + (size_t)m_cbSize + dlibMove.QuadPart;
			plibNewPosition->QuadPart = (ULONGLONG)(m_cbSize + dlibMove.QuadPart);
			return S_OK;
		}
		return STG_E_INVALIDFUNCTION;
	}

	HRESULT STDMETHODCALLTYPE SetSize(ULARGE_INTEGER libNewSize)
	{
		EckDbgBreak();
		return E_NOTIMPL;
	}

	HRESULT STDMETHODCALLTYPE CopyTo(IStream* pstm, ULARGE_INTEGER cb, ULARGE_INTEGER* pcbRead, ULARGE_INTEGER* pcbWritten)
	{
		if (!pstm)
			return STG_E_INVALIDPOINTER;
		if ((SIZE_T)(m_pSeek - m_pMem) >= m_cbSize)
		{
			if (pcbRead)
				pcbRead->QuadPart = 0u;
			if (pcbWritten)
				pcbWritten->QuadPart = 0u;
			return S_FALSE;
		}

		ULONG cbRead;
		if ((SIZE_T)((m_pSeek - m_pMem) + cb.LowPart) > m_cbSize)
			cbRead = (ULONG)(m_cbSize - (m_pSeek - m_pMem));
		else
			cbRead = cb.LowPart;
		ULONG cbWritten;
		pstm->Write(m_pSeek, cbRead, &cbWritten);
		if (pcbRead)
			pcbRead->QuadPart = cbRead;
		if (pcbWritten)
			pcbWritten->QuadPart = cbWritten;
		return S_OK;
	}

	HRESULT STDMETHODCALLTYPE Commit(DWORD grfCommitFlags)
	{
		return E_NOTIMPL;
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
		pstatstg->cbSize.QuadPart = m_cbSize;
		pstatstg->grfMode = STGM_READ;
		return S_OK;
	}

	HRESULT STDMETHODCALLTYPE Clone(IStream** ppstm)
	{
		const auto pStream = new CStreamView(m_pMem, m_cbSize);
		pStream->m_pSeek = m_pSeek;
		*ppstm = pStream;
		return S_OK;
	}

	HRESULT STDMETHODCALLTYPE MemGetPtr(void** ppvData, SIZE_T* pcbData)
	{
		*ppvData = (void*)m_pMem;
		*pcbData = m_cbSize;
		return S_OK;
	}

	HRESULT STDMETHODCALLTYPE MemLock(void** ppvData, SIZE_T* pcbData)
	{
		*ppvData = NULL;
		*pcbData = 0u;
		return E_NOTIMPL;
	}

	HRESULT STDMETHODCALLTYPE MemUnlock()
	{
		return E_NOTIMPL;
	}
};
ECK_NAMESPACE_END