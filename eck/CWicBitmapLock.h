#pragma once
#include "ComPtr.h"

ECK_NAMESPACE_BEGIN
class CWicBitmapLock
{
private:
    UINT m_cx{}, m_cy{};
    UINT m_cbStride{}, m_cbBufferSize{};
    BYTE* m_pData{};
    ComPtr<IWICBitmapLock> m_pLock;
public:
    ECK_DISABLE_COPY_MOVE_DEF_CONS(CWicBitmapLock);

    HRESULT Lock(
        _In_ IWICBitmap* pWicBitmap,
        _In_opt_ const WICRect* prc = nullptr,
        DWORD dwFlags = WICBitmapLockRead) noexcept
    {
        HRESULT hr;
        pWicBitmap->GetSize(&m_cx, &m_cy);
        if (prc)
            hr = pWicBitmap->Lock(prc, dwFlags, &m_pLock);
        else
        {
            const WICRect rc{ .Width = (int)m_cx, .Height = (int)m_cy };
            hr = pWicBitmap->Lock(&rc, dwFlags, &m_pLock);
        }
        if (FAILED(hr))
            return hr;;
        hr = m_pLock->GetStride(&m_cbStride);
        if (FAILED(hr))
            return hr;
        hr = m_pLock->GetDataPointer(&m_cbBufferSize, &m_pData);
        if (FAILED(hr))
            return hr;
        return S_OK;
    }

    EckInlineNdCe UINT GetWidth() const noexcept { return m_cx; }
    EckInlineNdCe UINT GetHeight() const noexcept { return m_cy; }
    EckInlineNdCe UINT GetStride() const noexcept { return m_cbStride; }
    EckInlineNdCe UINT GetBufferSize() const noexcept { return m_cbBufferSize; }
    EckInlineNdCe void* GetData() noexcept { return m_pData; }
};
ECK_NAMESPACE_END