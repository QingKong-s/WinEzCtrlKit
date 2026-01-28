#pragma once
#include "CRefStr.h"
#include "CRefBin.h"
#include "IMem.h"
#include "AutoPtrDef.h"
#include "NativeWrapper.h"
#include "ComPtr.h"

ECK_NAMESPACE_BEGIN
class CStreamWalker
{
public:
    struct Xpt {};
    struct XptHResult : Xpt
    {
        HRESULT hr;
        constexpr explicit XptHResult(HRESULT h) noexcept : hr{ h } {}
    };
    struct XptRwSize : Xpt
    {
        UINT cb;
        UINT cbActual;
        constexpr XptRwSize(UINT c, UINT a) noexcept : cb{ c }, cbActual{ a } {}
    };
    struct XptRange : Xpt
    {
        size_t pos;
        size_t cb;
        size_t cbActual;
        constexpr XptRange(size_t p, size_t c, size_t a) noexcept
            : pos{ p }, cb{ c }, cbActual{ a }
        {
        }
    };
private:
    ComPtr<IStream> m_pStream{};
public:
    CStreamWalker() = default;
    CStreamWalker(IStream* p) noexcept : m_pStream{ p } {}

    CStreamWalker& Write(PCVOID pSrc, size_t cb)
    {
        ULONG cbWritten;
        const auto hr = m_pStream->Write(pSrc, (ULONG)cb, &cbWritten);
        if (FAILED(hr))
            throw XptHResult{ hr };
        if (cbWritten != cb)
            throw XptRwSize{ (UINT)cb, cbWritten };
        return *this;
    }

    EckInline CStreamWalker& operator<<(const auto& Data) { return Write(&Data, sizeof(Data)); }
    template<class T, class U>
    EckInline CStreamWalker& operator<<(const std::basic_string_view<T, U>& Data)
    {
        return Write(Data.data(), Data.size() * sizeof(T)) << L'\0';
    }
    template<class T, class U, class V>
    EckInline CStreamWalker& operator<<(const std::basic_string<T, U, V>& Data)
    {
        return Write(Data.c_str(), (Data.size() + 1) * sizeof(T));
    }
    template<class T, class U>
    EckInline CStreamWalker& operator<<(const std::vector<T, U>& Data)
    {
        return Write(Data.data(), Data.size() * sizeof(T));
    }
    template<class T>
    EckInline CStreamWalker& operator<<(const CRefBinT<T>& Data)
    {
        return Write(Data.Data(), (ULONG)Data.Size());
    }
    template<class T, class U, class V>
    EckInline CStreamWalker& operator<<(const CRefStrT<T, U, V>& Data)
    {
        return Write(Data.Data(), Data.ByteSize());
    }

    CStreamWalker& Read(void* pDst, size_t cb)
    {
        ULONG cbRead;
        const auto hr = m_pStream->Read(pDst, (ULONG)cb, &cbRead);
        if (FAILED(hr))
            throw XptHResult{ hr };
        if (cbRead != cb)
            throw XptRwSize{ (UINT)cb, cbRead };
        return *this;
    }
    EckInline CStreamWalker& operator>>(auto& Data) { return Read(&Data, sizeof(Data)); }
    CStreamWalker& ReadRev(void* pDst, size_t cb)
    {
        Read(pDst, cb);
        ReverseByteOrder((BYTE*)pDst, cb);
        return *this;
    }

    CStreamWalker& operator+=(size_t cb)
    {
        const auto hr = m_pStream->Seek(ToLi((LONGLONG)cb), STREAM_SEEK_CUR, nullptr);
        if (FAILED(hr))
            throw XptHResult{ hr };
        return *this;
    }
    CStreamWalker& operator-=(size_t cb)
    {
        const auto hr = m_pStream->Seek(ToLi(-(LONGLONG)cb), STREAM_SEEK_CUR, nullptr);
        if (FAILED(hr))
            throw XptHResult{ hr };
        return *this;
    }

    CStreamWalker& MoveToBegin()
    {
        const auto hr = m_pStream->Seek(LiZero, STREAM_SEEK_SET, nullptr);
        if (FAILED(hr))
            throw XptHResult{ hr };
        return *this;
    }
    CStreamWalker& MoveToEnd()
    {
        const auto hr = m_pStream->Seek(LiZero, STREAM_SEEK_END, nullptr);
        if (FAILED(hr))
            throw XptHResult{ hr };
        return *this;
    }
    CStreamWalker& MoveTo(size_t x)
    {
        const auto hr = m_pStream->Seek(ToLi((LONGLONG)x), STREAM_SEEK_SET, nullptr);
        if (FAILED(hr))
            throw XptHResult{ hr };
        return *this;
    }

    CStreamWalker& Seek(SSIZE_T x, UINT uOrg = STREAM_SEEK_SET,
        size_t* pposNew = nullptr)
    {
        ULARGE_INTEGER uliNew;
        const auto hr = m_pStream->Seek(ToLi(x), uOrg, &uliNew);
        if (FAILED(hr))
            throw XptHResult{ hr };
        if (pposNew)
            *pposNew = (size_t)uliNew.QuadPart;
        return *this;
    }

    size_t GetPosition()
    {
        ULARGE_INTEGER uli{};
        const auto hr = m_pStream->Seek(LiZero, STREAM_SEEK_CUR, &uli);
        if (FAILED(hr))
            throw XptHResult{ hr };
        return (size_t)uli.QuadPart;
    }

    size_t GetSize()
    {
        STATSTG ss;
        const auto hr = m_pStream->Stat(&ss, STATFLAG_NONAME);
        if (FAILED(hr))
            throw XptHResult{ hr };
        return (size_t)ss.cbSize.QuadPart;
    }

    // 此函数返回后当前流位置未定义
    // 若抛出，则流内容未定义
    void MoveData(size_t posDst, size_t posSrc,
        size_t cbSize, size_t cbMoveBuf = 4096u, void* pMoveBuf = nullptr)
    {
        if (posDst == posSrc || cbSize == 0ull)
            return;
        EckAssert(posSrc < GetSize() && posSrc + cbSize <= GetSize());
        // 若流实现IMem，则尝试memmove
        ComPtr<IMem> pMem;
        if (SUCCEEDED(m_pStream.As(pMem)))
        {
            void* pData;
            size_t cbData;
            if (SUCCEEDED(pMem->MemGetPtr(&pData, &cbData)))// 支持取指针
            {
                memmove((BYTE*)pData + posDst, (PCBYTE)pData + posSrc, cbSize);
                return;
            }
            else if (SUCCEEDED(pMem->MemLock(&pData, &cbData)))// 支持锁定
            {
                memmove((BYTE*)pData + posDst, (PCBYTE)pData + posSrc, cbSize);
                pMem->MemUnlock();
                return;
            }
        }
        // 若流支持克隆，则优先使用其CopyTo实现
        ComPtr<IStream> pSelf;
        if (SUCCEEDED(m_pStream->Clone(&pSelf)))
        {
            MoveTo(posSrc);
            auto hr = pSelf->Seek(ToLi(posDst), STREAM_SEEK_SET, nullptr);
            if (SUCCEEDED(hr))
            {
                hr = m_pStream->CopyTo(pSelf.Get(), ToUli(cbSize), nullptr, nullptr);
                if (SUCCEEDED(hr))
                    return;
            }
        }
        //
        BOOL bExternalBuf = FALSE;
        if (!pMoveBuf)
        {
            pMoveBuf = VAlloc(cbMoveBuf);
            if (!pMoveBuf)
                throw XptHResult{ E_OUTOFMEMORY };
            bExternalBuf = TRUE;
        }
        const UniquePtr<DelVA<void>> _{ bExternalBuf ? pMoveBuf : nullptr };

        if (cbSize <= cbMoveBuf)
        {
            MoveTo(posSrc);
            Read(pMoveBuf, (ULONG)cbSize);
            MoveTo(posDst);
            Write(pMoveBuf, (ULONG)cbSize);
            return;
        }

        const auto posSrcEnd = posSrc + cbSize;
        const auto posDstEnd = posDst + cbSize;
        if (posDst > posSrc)// 从后向前复制
        {
            size_t posRead = posSrcEnd - cbMoveBuf;
            size_t posWrite = posDstEnd - cbMoveBuf;
            EckLoop()
            {
                MoveTo(posRead);
                Read(pMoveBuf, cbMoveBuf);
                MoveTo(posWrite);
                Write(pMoveBuf, cbMoveBuf);

                if (const auto cb = posRead - posSrc; cb < cbMoveBuf)
                {
                    if (!cb)
                        break;
                    MoveTo(posSrc);
                    Read(pMoveBuf, cb);
                    MoveTo(posDst);
                    Write(pMoveBuf, cb);
                    break;
                }
                else
                {
                    posRead -= cbMoveBuf;
                    posWrite -= cbMoveBuf;
                }
            }
        }
        else// 从前向后复制
        {
            size_t posRead = posSrc;
            size_t posWrite = posDst;
            EckLoop()
            {
                MoveTo(posRead);
                Read(pMoveBuf, cbMoveBuf);
                MoveTo(posWrite);
                Write(pMoveBuf, cbMoveBuf);

                if (const auto cb = posSrcEnd - (posRead + cbMoveBuf); cb < cbMoveBuf)
                {
                    if (!cb)
                        break;
                    MoveTo(posRead + cbMoveBuf);
                    Read(pMoveBuf, cb);
                    MoveTo(posWrite + cbMoveBuf);
                    Write(pMoveBuf, cb);
                    break;
                }
                else
                {
                    posRead += cbMoveBuf;
                    posWrite += cbMoveBuf;
                }
            }
        }
    }

    void Insert(size_t pos, size_t cbSize)
    {
        if (!cbSize)
            return;
        const auto uliStrmSize = GetSize();
        m_pStream->SetSize(ToUli(GetSize() + cbSize));
        if (pos != uliStrmSize)
            MoveData(pos + cbSize, pos, uliStrmSize - pos);
    }

    void Erase(size_t pos, size_t cbSize)
    {
        if (!cbSize)
            return;
        const auto cbTotal = GetSize();
        if (pos + cbSize > cbTotal)
            throw XptRange{ pos, cbSize, cbTotal - pos };
        if (pos + cbSize != cbTotal)
            MoveData(pos, pos + cbSize, cbTotal - pos - cbSize);
        ReSize(cbTotal - cbSize);
    }

    void ReSize(size_t cbSize)
    {
        const auto hr = m_pStream->SetSize(ToUli(cbSize));
        if (FAILED(hr))
            throw XptHResult{ hr };
    }

    void Commit(UINT grfCommitFlags = STGC_DEFAULT)
    {
        const auto hr = m_pStream->Commit(grfCommitFlags);
        if (FAILED(hr))
            throw XptHResult{ hr };
    }

    EckInlineNdCe auto GetStream() const noexcept { return m_pStream.Get(); }
};
ECK_NAMESPACE_END