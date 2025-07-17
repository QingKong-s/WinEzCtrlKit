#pragma once
#include "CRefStr.h"
#include "CRefBin.h"
#include "IMem.h"
#include "AutoPtrDef.h"
#include "NativeWrapper.h"

ECK_NAMESPACE_BEGIN
struct CStreamWalker
{
	IStream* m_pStream{};
	ULONG m_cbLastReadWrite{};
	HRESULT m_hrLastErr{ S_OK };

	CStreamWalker() = default;

	constexpr CStreamWalker(IStream* p) :m_pStream{ p } {}

	EckInline constexpr ULONG GetLastRWSize() const { return m_cbLastReadWrite; }

	EckInline CStreamWalker& Write(PCVOID pSrc, SIZE_T cb)
	{
		m_hrLastErr = m_pStream->Write(pSrc, (ULONG)cb, &m_cbLastReadWrite);
		return *this;
	}

	template<class T>
	EckInline CStreamWalker& operator<<(const T& Data)
	{
		return Write(&Data, sizeof(Data));
	}

	template<class T, class U>
	EckInline CStreamWalker& operator<<(const std::basic_string_view<T, U>& Data)
	{
		return Write(Data.data(), Data.size() * sizeof(T)) << L'\0';
	}

	template<class T, class U>
	EckInline CStreamWalker& operator<<(const std::basic_string<T, U>& Data)
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

	EckInline constexpr auto GetStream() const { return m_pStream; }

	EckInline CStreamWalker& operator+=(SIZE_T cb)
	{
		m_hrLastErr = m_pStream->Seek({ .QuadPart = (LONGLONG)cb }, STREAM_SEEK_CUR, nullptr);
		return *this;
	}

	EckInline CStreamWalker& operator-=(SIZE_T cb)
	{
		m_hrLastErr = m_pStream->Seek({ .QuadPart = -(LONGLONG)cb }, STREAM_SEEK_CUR, nullptr);
		return *this;
	}

	EckInline CStreamWalker& Read(void* pDst, SIZE_T cb)
	{
		m_hrLastErr = m_pStream->Read(pDst, (ULONG)cb, &m_cbLastReadWrite);
		return *this;
	}

	EckInline CStreamWalker& ReadRev(void* pDst, SIZE_T cb)
	{
		Read(pDst, cb);
		ReverseByteOrder((BYTE*)pDst, cb);
		return *this;
	}

	template<class T>
	EckInline CStreamWalker& operator>>(T& Data)
	{
		return Read(&Data, sizeof(Data));
	}

	EckInline CStreamWalker& MoveToBegin()
	{
		m_hrLastErr = m_pStream->Seek(LiZero, STREAM_SEEK_SET, nullptr);
		return *this;
	}

	EckInline CStreamWalker& MoveToEnd()
	{
		m_hrLastErr = m_pStream->Seek(LiZero, STREAM_SEEK_END, nullptr);
		return *this;
	}

	EckInline CStreamWalker& MoveTo(LARGE_INTEGER li)
	{
		m_hrLastErr = m_pStream->Seek(li, STREAM_SEEK_SET, nullptr);
		return *this;
	}

	EckInline CStreamWalker& MoveTo(ULARGE_INTEGER uli)
	{
		m_hrLastErr = m_pStream->Seek(ToLi(uli), STREAM_SEEK_SET, nullptr);
		return *this;
	}

	EckInline CStreamWalker& MoveTo(SIZE_T x)
	{
		m_hrLastErr = m_pStream->Seek(ToLi((LONGLONG)x), STREAM_SEEK_SET, nullptr);
		return *this;
	}

	EckInline CStreamWalker& MoveTo(SSIZE_T x)
	{
		m_hrLastErr = m_pStream->Seek(ToLi((LONGLONG)x), STREAM_SEEK_SET, nullptr);
		return *this;
	}

	EckInline SIZE_T GetPos()
	{
		ULARGE_INTEGER uli{};
		m_hrLastErr = m_pStream->Seek(LiZero, STREAM_SEEK_CUR, &uli);
		return (SIZE_T)uli.QuadPart;
	}

	EckInline ULARGE_INTEGER GetPosUli()
	{
		ULARGE_INTEGER uli{};
		m_hrLastErr = m_pStream->Seek(LiZero, STREAM_SEEK_CUR, &uli);
		return uli;
	}

	EckInline ULARGE_INTEGER GetSizeUli()
	{
		STATSTG ss;
		m_hrLastErr = m_pStream->Stat(&ss, STATFLAG_NONAME);
		return ss.cbSize;
	}

	EckInline SIZE_T GetSize()
	{
		return (SIZE_T)GetSizeUli().QuadPart;
	}

	EckInline CRefBin ReadBin(SIZE_T cb)
	{
		CRefBin rb(cb);
		Read(rb.Data(), (ULONG)cb);
		return rb;
	}

	void MoveData(ULARGE_INTEGER posDst, ULARGE_INTEGER posSrc,
		ULARGE_INTEGER cbSize, SIZE_T cbMoveBuf = 4096u, void* pMoveBuf = nullptr)
	{
		if (posDst == posSrc || cbSize == 0ull)
		{
			m_hrLastErr = S_FALSE;
			return;
		}
		EckAssert(posSrc < GetSizeUli() && posSrc + cbSize <= GetSizeUli());
		// 若流实现IMem，则尝试memmove
		IMem* pMem;
		if (SUCCEEDED(m_pStream->QueryInterface(&pMem)))
		{
			void* pData;
			SIZE_T cbData;
			if (SUCCEEDED(pMem->MemGetPtr(&pData, &cbData)))// 支持取指针
			{
				memmove((BYTE*)pData + (SIZE_T)posDst.QuadPart,
					(BYTE*)pData + (SIZE_T)posSrc.QuadPart, (SIZE_T)cbSize.QuadPart);
				pMem->Release();
				m_hrLastErr = S_OK;
				return;
			}
			else if (SUCCEEDED(pMem->MemLock(&pData, &cbData)))// 支持锁定
			{
				memmove((BYTE*)pData + (SIZE_T)posDst.QuadPart,
					(BYTE*)pData + (SIZE_T)posSrc.QuadPart, (SIZE_T)cbSize.QuadPart);
				pMem->MemUnlock();
				pMem->Release();
				m_hrLastErr = S_OK;
				return;
			}
			else
				pMem->Release();
		}
		// 若流支持克隆，则优先使用其CopyTo实现
		IStream* pSelf;
		if (SUCCEEDED(m_pStream->Clone(&pSelf)))
		{
			MoveTo(posSrc);
			pSelf->Seek(ToLi(posDst), STREAM_SEEK_SET, nullptr);
			const auto hr = m_pStream->CopyTo(pSelf, cbSize, nullptr, nullptr);
			pSelf->Release();
			if (SUCCEEDED(hr)) return;
		}

		BOOL bExternalBuf = FALSE;
		if (!pMoveBuf)
		{
			pMoveBuf = VAlloc(cbMoveBuf);
			if (!pMoveBuf)
			{
				m_hrLastErr = E_OUTOFMEMORY;
				return;
			}
			bExternalBuf = TRUE;
		}
		UniquePtr<DelVA<void>> _{ bExternalBuf ? pMoveBuf : nullptr };

		if (cbSize <= cbMoveBuf)
		{
			MoveTo(posSrc);
			Read(pMoveBuf, (ULONG)cbSize.QuadPart);
			if (FAILED(m_hrLastErr))
				return;
			MoveTo(posDst);
			Write(pMoveBuf, (ULONG)cbSize.QuadPart);
			return;
		}

		const auto posSrcEnd = posSrc + cbSize;
		const auto posDstEnd = posDst + cbSize;
		if (posDst > posSrc)// 从后向前复制
		{
			LARGE_INTEGER posRead = ToLi(posSrcEnd - cbMoveBuf);
			LARGE_INTEGER posWrite = ToLi(posDstEnd - cbMoveBuf);
			const LARGE_INTEGER liPosSrc = ToLi(posSrc);
			EckLoop()
			{
				MoveTo(posRead);
				Read(pMoveBuf, cbMoveBuf);
				if (FAILED(m_hrLastErr))
					return;
				if (m_cbLastReadWrite != cbMoveBuf)
				{
					m_hrLastErr = E_FAIL;
					return;
				}
				MoveTo(posWrite);
				Write(pMoveBuf, cbMoveBuf);
				if (FAILED(m_hrLastErr))
					return;
				if (m_cbLastReadWrite != cbMoveBuf)
				{
					m_hrLastErr = E_FAIL;
					return;
				}

				const auto posPrev = posRead;
				posRead -= cbMoveBuf;
				posWrite -= cbMoveBuf;
				if (posRead < liPosSrc)
				{
					posRead = liPosSrc;
					posWrite = posRead + ToLi(posDst - posSrc);
					const ULONG cbRead = (ULONG)(posPrev - liPosSrc).QuadPart;
					MoveTo(posRead);
					Read(pMoveBuf, cbRead);
					if (m_cbLastReadWrite != cbRead)
					{
						m_hrLastErr = E_FAIL;
						return;
					}
					MoveTo(posWrite);
					Write(pMoveBuf, cbRead);
					if (m_cbLastReadWrite != cbRead)
					{
						m_hrLastErr = E_FAIL;
						return;
					}
					break;
				}
			}
		}
		else// 从前向后复制
		{
			LARGE_INTEGER posRead = ToLi(posSrc);
			LARGE_INTEGER posWrite = ToLi(posDst);
			const LARGE_INTEGER liPosSrcEnd = ToLi(posSrcEnd);
			EckLoop()
			{
				MoveTo(posRead);
				Read(pMoveBuf, cbMoveBuf);
				if (FAILED(m_hrLastErr))
					return;
				if (m_cbLastReadWrite != cbMoveBuf)
				{
					m_hrLastErr = E_FAIL;
					return;
				}
				MoveTo(posWrite);
				Write(pMoveBuf, cbMoveBuf);
				if (FAILED(m_hrLastErr))
					return;
				if (m_cbLastReadWrite != cbMoveBuf)
				{
					m_hrLastErr = E_FAIL;
					return;
				}

				const auto posPrev = posRead;
				posRead += cbMoveBuf;
				posWrite += cbMoveBuf;
				if (posRead >= liPosSrcEnd)
				{
					const ULONG cbRead = (ULONG)(liPosSrcEnd - posPrev).QuadPart;
					MoveTo(posRead);
					Read(pMoveBuf, cbRead);
					if (m_cbLastReadWrite != cbRead)
					{
						m_hrLastErr = E_FAIL;
						return;
					}
					MoveTo(posWrite);
					Write(pMoveBuf, cbRead);
					if (m_cbLastReadWrite != cbRead)
					{
						m_hrLastErr = E_FAIL;
						return;
					}
					break;
				}
			}
		}
		m_hrLastErr = S_OK;
	}

	EckInline void MoveData(SIZE_T posDst, SIZE_T posSrc, SIZE_T cbSize,
		SIZE_T cbMoveBuf = 4096u, void* pMoveBuf = nullptr)
	{
		MoveData(ToUli(posDst), ToUli(posSrc), ToUli(cbSize), cbMoveBuf, pMoveBuf);
	}

	void Insert(ULARGE_INTEGER pos, ULARGE_INTEGER cbSize)
	{
		if (cbSize == 0ull)
			return;
		const auto uliStrmSize = GetSizeUli();
		m_pStream->SetSize(GetSizeUli() + cbSize);
		if (pos != uliStrmSize)
			MoveData(pos + cbSize, pos, uliStrmSize - pos);
	}

	EckInline void Insert(SIZE_T pos, SIZE_T cbSize) { Insert(ToUli(pos), ToUli(cbSize)); }

	void Erase(ULARGE_INTEGER pos, ULARGE_INTEGER cbSize)
	{
		if (cbSize == 0ull)
			return;
		const auto cbTotal = GetSizeUli();
		EckAssert(pos < cbTotal && pos + cbSize <= cbTotal);
		MoveData(pos, pos + cbSize, cbTotal - pos - cbSize);
		ReSize(GetSizeUli() - cbSize);
	}

	EckInline void Erase(SIZE_T pos, SIZE_T cbSize) { Erase(ToUli(pos), ToUli(cbSize)); }

	EckInline BOOL ReSize(ULARGE_INTEGER cbSize)
	{
		return SUCCEEDED(m_hrLastErr = m_pStream->SetSize(cbSize));
	}

	EckInline BOOL ReSize(SIZE_T cbSize) { return ReSize(ToUli(cbSize)); }

	EckInlineNdCe HRESULT GetLastErr() const { return m_hrLastErr; }

	EckInlineNdCe IStream* operator->() const { return m_pStream; }
};
ECK_NAMESPACE_END