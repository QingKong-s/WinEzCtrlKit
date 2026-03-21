#pragma once
#include "CByteBuffer.h"
#include "IMemoryView.h"
#include "CUnknown.h"

ECK_NAMESPACE_BEGIN
class CStreamView : public CUnknown<CStreamView, IStream, IMemoryView>
{
private:
    PCBYTE m_pMem{};
    PCBYTE m_pSeek{};
    size_t m_cbSize{};
public:
    CStreamView() = default;
    constexpr CStreamView(PCVOID p, size_t cb) noexcept
        : m_pMem{ (PCBYTE)p }, m_cbSize{ cb }, m_pSeek{ (PCBYTE)p }
    {}

    template<class TAllocator>
    constexpr CStreamView(const CByteBufferT<TAllocator>& rb) noexcept : CStreamView(rb.Data(), rb.Size()) {}

    template<class T, class TAllocator>
    constexpr CStreamView(const std::vector<T, TAllocator>& v) noexcept : CStreamView(v.data(), v.size() * sizeof(T)) {}

    template<class T>
    constexpr CStreamView(const std::span<T>& s) noexcept : CStreamView(s.data(), s.size() * sizeof(T)) {}

    EckInlineCe void SetData(PCVOID p, size_t cb) noexcept
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

    EckInline void AssertReference(LONG l) noexcept { EckAssert(m_cRef == l); }

    STDMETHODIMP Read(void* pv, ULONG cb, ULONG* pcbRead) override
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

    STDMETHODIMP Write(const void* pv, ULONG cb, ULONG* pcbWritten) override
    {
        EckDbgBreak();
        return STG_E_ACCESSDENIED;
    }

    STDMETHODIMP Seek(LARGE_INTEGER dlibMove, DWORD dwOrigin, ULARGE_INTEGER* plibNewPosition) override
    {
        if (plibNewPosition)
            plibNewPosition->QuadPart = (m_pSeek - m_pMem);

        switch (dwOrigin)
        {
        case SEEK_SET:// 这种情况dlibMove应视为无符号
            m_pSeek = m_pMem + (size_t)dlibMove.QuadPart;
            if (plibNewPosition)
                *plibNewPosition = ToULargeInt(dlibMove);
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

    STDMETHODIMP SetSize(ULARGE_INTEGER libNewSize) override
    {
        EckDbgBreak();
        return STG_E_ACCESSDENIED;
    }

    STDMETHODIMP CopyTo(IStream* pstm, ULARGE_INTEGER cb, ULARGE_INTEGER* pcbRead, ULARGE_INTEGER* pcbWritten) override
    {
        if (pcbRead)
            pcbRead->QuadPart = 0u;
        if (pcbWritten)
            pcbWritten->QuadPart = 0u;
        if (!pstm)
            return STG_E_INVALIDPOINTER;
        if (size_t(m_pSeek - m_pMem) >= m_cbSize)
            return S_FALSE;

        ULONG cbRead;
        if (size_t((m_pSeek - m_pMem) + cb.LowPart) > m_cbSize)
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

    STDMETHODIMP Commit(DWORD grfCommitFlags) override
    {
        return E_NOTIMPL;
    }

    STDMETHODIMP Revert() override
    {
        return E_NOTIMPL;
    }

    STDMETHODIMP LockRegion(ULARGE_INTEGER libOffset, ULARGE_INTEGER cb, DWORD dwLockType) override
    {
        return E_NOTIMPL;
    }

    STDMETHODIMP UnlockRegion(ULARGE_INTEGER libOffset, ULARGE_INTEGER cb, DWORD dwLockType) override
    {
        return E_NOTIMPL;
    }

    STDMETHODIMP Stat(STATSTG* pstatstg, DWORD grfStatFlag) override
    {
        ZeroMemory(pstatstg, sizeof(STATSTG));
        pstatstg->type = STGTY_STREAM;
        pstatstg->cbSize.QuadPart = m_cbSize;
        pstatstg->grfMode = STGM_READ;
        return S_OK;
    }

    STDMETHODIMP Clone(IStream** ppstm) override
    {
        const auto pStream = new CStreamView(m_pMem, m_cbSize);
        pStream->m_pSeek = m_pSeek;
        *ppstm = pStream;
        return S_OK;
    }

    STDMETHODIMP MemGetPointer(void** ppvData, size_t* pcbData) override
    {
        *ppvData = (void*)m_pMem;
        *pcbData = m_cbSize;
        return S_OK;
    }

    STDMETHODIMP MemLock(void** ppvData, size_t* pcbData) override
    {
        *ppvData = nullptr;
        *pcbData = 0u;
        return E_NOTIMPL;
    }

    STDMETHODIMP MemUnlock() override
    {
        return E_NOTIMPL;
    }

    STDMETHODIMP MemIsLocked(BOOL* pbLocked) override
    {
        *pbLocked = FALSE;
        return E_NOTIMPL;
    }
};
ECK_NAMESPACE_END