#pragma once
#include "CRefBin.h"
#include "IMem.h"
#include "CUnknown.h"

ECK_NAMESPACE_BEGIN
class CStreamView : public CUnknown<CStreamView, IStream, IMem>
{
private:
    PCBYTE m_pMem{};
    PCBYTE m_pSeek{};
    SIZE_T m_cbSize{};
public:
    CStreamView() = default;
    constexpr CStreamView(PCVOID p, SIZE_T cb) noexcept
        : m_pMem{ (PCBYTE)p }, m_cbSize{ cb }, m_pSeek{ (PCBYTE)p }
    {
    }

    template<class TAlloc>
    constexpr CStreamView(const CRefBinT<TAlloc>& rb) noexcept : CStreamView(rb.Data(), rb.Size()) {}

    template<class T, class TAlloc>
    constexpr CStreamView(const std::vector<T, TAlloc>& v) noexcept : CStreamView(v.data(), v.size() * sizeof(T)) {}

    template<class T>
    constexpr CStreamView(const std::span<T>& s) noexcept : CStreamView(s.data(), s.size() * sizeof(T)) {}

    EckInlineCe void SetData(PCVOID p, SIZE_T cb) noexcept
    {
        m_pMem = (PCBYTE)p;
        m_pSeek = m_pMem;
        m_cbSize = cb;
    }

    EckInline void LeaveRelease() noexcept
    {
#ifdef _DEBUG
        EckAssert(Release() == 0);
#else
        Release();
#endif
    }

    HRESULT STDMETHODCALLTYPE Read(void* pv, ULONG cb, ULONG* pcbRead)
    {
        if (pcbRead)
            *pcbRead = 0;
        if (!pv)
            return STG_E_INVALIDPOINTER;
        if (m_pSeek >= m_pMem + m_cbSize)
            return S_FALSE;

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
        if (pcbRead)
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
        if (plibNewPosition)
            plibNewPosition->QuadPart = (m_pSeek - m_pMem);

        switch (dwOrigin)
        {
        case SEEK_SET:// 这种情况dlibMove应视为无符号
            m_pSeek = m_pMem + (size_t)dlibMove.QuadPart;
            if (plibNewPosition)
                *plibNewPosition = ToUli(dlibMove);
            return S_OK;

        case SEEK_CUR:
        {
            const ptrdiff_t ocbNew = (ptrdiff_t)dlibMove.QuadPart + (m_pSeek - m_pMem);
            if (ocbNew < 0)// 落在流开始之前
                return STG_E_INVALIDFUNCTION;
            m_pSeek = m_pMem + (size_t)ocbNew;
            if (plibNewPosition)
                plibNewPosition->QuadPart = (ULONGLONG)ocbNew;
        }
        return S_OK;

        case SEEK_END:
            if (dlibMove.QuadPart < -(ptrdiff_t)m_cbSize)// 落在流开始之前
                return STG_E_INVALIDFUNCTION;
            m_pSeek = m_pMem + (size_t)m_cbSize + dlibMove.QuadPart;
            if (plibNewPosition)
                plibNewPosition->QuadPart = (ULONGLONG)(m_cbSize + dlibMove.QuadPart);
            return S_OK;
        }
        return STG_E_INVALIDFUNCTION;
    }

    HRESULT STDMETHODCALLTYPE SetSize(ULARGE_INTEGER libNewSize)
    {
        EckDbgBreak();
        return STG_E_ACCESSDENIED;
    }

    HRESULT STDMETHODCALLTYPE CopyTo(IStream* pstm, ULARGE_INTEGER cb, ULARGE_INTEGER* pcbRead, ULARGE_INTEGER* pcbWritten)
    {
        if (pcbRead)
            pcbRead->QuadPart = 0u;
        if (pcbWritten)
            pcbWritten->QuadPart = 0u;
        if (!pstm)
            return STG_E_INVALIDPOINTER;
        if ((SIZE_T)(m_pSeek - m_pMem) >= m_cbSize)
            return S_FALSE;

        ULONG cbRead;
        if ((SIZE_T)((m_pSeek - m_pMem) + cb.LowPart) > m_cbSize)
            cbRead = (ULONG)(m_cbSize - (m_pSeek - m_pMem));
        else
            cbRead = cb.LowPart;
        ULONG cbWritten{};
        const auto hr = pstm->Write(m_pSeek, cbRead, &cbWritten);
        if (pcbRead)
            pcbRead->QuadPart = cbRead;
        if (pcbWritten)
            pcbWritten->QuadPart = cbWritten;
        return hr;
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
        *ppvData = nullptr;
        *pcbData = 0u;
        return E_NOTIMPL;
    }

    HRESULT STDMETHODCALLTYPE MemUnlock()
    {
        return E_NOTIMPL;
    }

    HRESULT STDMETHODCALLTYPE MemIsLocked(BOOL* pbLocked)
    {
        *pbLocked = FALSE;
        return E_NOTIMPL;
    }
};
ECK_NAMESPACE_END