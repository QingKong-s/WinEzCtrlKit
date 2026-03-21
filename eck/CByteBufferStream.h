#pragma once
#include "CByteBuffer.h"
#include "IMemoryView.h"
#include "CUnknown.h"

ECK_NAMESPACE_BEGIN
template<class TAllocator>
class CByteBufferStreamT : public CUnknown<CByteBufferStreamT<TAllocator>, IStream, IMemoryView>
{
private:
    BOOL m_bLocked{};

    CByteBufferT<TAllocator>& m_rb;
    size_t m_posSeek{};	// 相对于m_rb的起始位置
    size_t m_posBegin{};// 若要强制追加数据，则此字段记录追加起始位置
public:
    CByteBufferStreamT(CByteBufferT<TAllocator>& rb) :m_rb{ rb } {}

    // 强制从指定位置追加数据
    EckInlineCe void SetBeginPosition(size_t pos) noexcept
    {
        m_posBegin = pos;
    }

    // 强制从当前尾部追加数据
    EckInlineCe void SetBeginPosition() noexcept
    {
        SetBeginPosition(m_rb.Size());
        if (m_posSeek < m_posBegin)
            m_posSeek = m_posBegin;
    }

    EckInline void LeaveRelease() noexcept
    {
#ifdef _DEBUG
        EckAssert(this->Release() == 0);
#else
        this->Release();
#endif
    }

    EckInline void AssertReference(LONG l) noexcept { EckAssert(this->m_cRef == l); }

    STDMETHODIMP Read(void* pv, ULONG cb, ULONG* pcbRead) override
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

    STDMETHODIMP Write(const void* pv, ULONG cb, ULONG* pcbWritten) override
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

    STDMETHODIMP Seek(LARGE_INTEGER dlibMove, DWORD dwOrigin, ULARGE_INTEGER* plibNewPosition) override
    {
        if (plibNewPosition)
            plibNewPosition->QuadPart = m_posSeek - m_posBegin;

        switch (dwOrigin)
        {
        case SEEK_SET:// 这种情况dlibMove应视为无符号
            m_posSeek = (size_t)dlibMove.QuadPart + m_posBegin;
            if (plibNewPosition)
                *plibNewPosition = ToULargeInt(dlibMove);
            return S_OK;

        case SEEK_CUR:
        {
            const auto ocbNew = ptrdiff_t(dlibMove.QuadPart + m_posSeek);
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

    STDMETHODIMP SetSize(ULARGE_INTEGER libNewSize) override
    {
        if (m_bLocked)
            return STG_E_ACCESSDENIED;
        else
            m_rb.ReSizeExtra((size_t)libNewSize.QuadPart + m_posBegin);
        return S_OK;
    }

    STDMETHODIMP CopyTo(IStream* pstm, ULARGE_INTEGER cb,
        ULARGE_INTEGER* pcbRead, ULARGE_INTEGER* pcbWritten) override
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

    STDMETHODIMP Commit(DWORD grfCommitFlags) override
    {
        return S_OK;
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
        pstatstg->cbSize.QuadPart = m_rb.Size();
        if (m_bLocked)
            pstatstg->grfMode = STGM_READ;
        else
            pstatstg->grfMode = STGM_READWRITE;
        return S_OK;
    }

    STDMETHODIMP Clone(IStream** ppstm) override
    {
        const auto p = new CByteBufferStreamT(m_rb);
        p->m_posSeek = m_posSeek;
        p->m_posBegin = m_posBegin;
        p->m_bLocked = m_bLocked;
        *ppstm = p;
        return S_OK;
    }

    STDMETHODIMP MemGetPointer(void** ppvData, size_t* pcbData) override
    {
        *ppvData = m_rb.Data();
        *pcbData = m_rb.Size();
        return S_OK;
    }

    STDMETHODIMP MemLock(void** ppvData, size_t* pcbData) override
    {
        *ppvData = m_rb.Data();
        *pcbData = m_rb.Size();
        m_bLocked = TRUE;
        return S_OK;
    }

    STDMETHODIMP MemUnlock() override
    {
        m_bLocked = FALSE;
        return S_OK;
    }

    STDMETHODIMP MemIsLocked(BOOL* pbLocked) override
    {
        *pbLocked = m_bLocked;
        return S_OK;
    }
};

using CByteBufferStream = CByteBufferStreamT<TByteBufferDefaultAllocator>;
ECK_NAMESPACE_END