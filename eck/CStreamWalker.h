#pragma once
#include "CRefStr.h"
#include "CRefBin.h"

ECK_NAMESPACE_BEGIN
struct CStreamWalker
{
	IStream* m_pStream{};
	ULONG m_cbLastReadWrite{};
	HRESULT m_hrLastErr{ S_OK };

	CStreamWalker() = default;

	CStreamWalker(IStream* p)
	{
		m_pStream = p;
	}

	EckInline constexpr ULONG GetLastRWSize() const { return m_cbLastReadWrite; }

	EckInline CStreamWalker& Write(PCVOID pSrc, ULONG cb)
	{
		m_hrLastErr = m_pStream->Write(pSrc, cb, &m_cbLastReadWrite);
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
		return Write(Data.Data(), Data.Size());
	}

	template<class T, class U, class V>
	EckInline CStreamWalker& operator<<(const CRefStrT<T, U, V>& Data)
	{
		return Write(Data.Data(), Data.ByteSize());
	}

	EckInline auto GetStream() { return m_pStream; }

	EckInline CStreamWalker& operator+=(SIZE_T cb)
	{
		m_hrLastErr = m_pStream->Seek({ .QuadPart = (LONGLONG)cb }, STREAM_SEEK_CUR, NULL);
		return *this;
	}

	EckInline CStreamWalker& operator-=(SIZE_T cb)
	{
		m_hrLastErr = m_pStream->Seek({ .QuadPart = -(LONGLONG)cb }, STREAM_SEEK_CUR, NULL);
		return *this;
	}

	EckInline CStreamWalker& Read(void* pDst, ULONG cb)
	{
		m_hrLastErr = m_pStream->Read(pDst, cb, &m_cbLastReadWrite);
		return *this;
	}

	template<class T>
	EckInline CStreamWalker& operator>>(T& Data)
	{
		return Read(&Data, sizeof(Data));
	}

	EckInline CStreamWalker& MoveToBegin()
	{
		m_hrLastErr = m_pStream->Seek(LiZero, STREAM_SEEK_SET, NULL);
		return *this;
	}

	EckInline CStreamWalker& MoveToEnd()
	{
		m_hrLastErr = m_pStream->Seek(LiZero, STREAM_SEEK_END, NULL);
		return *this;
	}

	EckInline CStreamWalker& MoveTo(LARGE_INTEGER li)
	{
		m_hrLastErr = m_pStream->Seek(li, STREAM_SEEK_SET, NULL);
		return *this;
	}

	EckInline CStreamWalker& MoveTo(ULARGE_INTEGER uli)
	{
		m_hrLastErr = m_pStream->Seek(ToLi(uli), STREAM_SEEK_SET, NULL);
		return *this;
	}

	EckInline CStreamWalker& MoveTo(SIZE_T x)
	{
		m_hrLastErr = m_pStream->Seek(ToLi((LONGLONG)x), STREAM_SEEK_SET, NULL);
		return *this;
	}

	EckInline ULARGE_INTEGER GetPos()
	{
		ULARGE_INTEGER uli{};
		m_hrLastErr = m_pStream->Seek(LiZero, STREAM_SEEK_CUR, &uli);
		return uli;
	}

	EckInline ULARGE_INTEGER GetSize()
	{
		STATSTG ss;
		m_hrLastErr = m_pStream->Stat(&ss, STATFLAG_NONAME);
		return ss.cbSize;
	}

	EckInline CRefBin ReadBin(SIZE_T cb)
	{
		CRefBin rb(cb);
		Read(rb.Data(), (ULONG)cb);
		return rb;
	}

	void MoveData(ULARGE_INTEGER posDst, ULARGE_INTEGER posSrc, ULARGE_INTEGER cbSize)
	{
		EckAssert(posSrc < GetSize() && posSrc + cbSize <= GetSize());
		if (posDst == posSrc || cbSize == 0ull)
			return;

		IStream* pSelf;
		if (SUCCEEDED(m_pStream->Clone(&pSelf)))// 若流支持克隆，则优先使用其CopyTo实现
		{
			MoveTo(posSrc);
			pSelf->Seek(ToLi(posDst), STREAM_SEEK_SET, NULL);
			const auto hr = m_pStream->CopyTo(pSelf, cbSize, NULL, NULL);
			pSelf->Release();
			if (SUCCEEDED(hr))
				return;
		}
		
		constexpr SIZE_T cbBuf = 4096u;
		void* pBuf = VirtualAlloc(NULL, cbBuf, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
		if (!pBuf)
		{
			m_hrLastErr = HRESULT_FROM_WIN32(GetLastError());
			return;
		}
		UniquePtrVA<void> _(pBuf);

		if (cbSize <= cbBuf)
		{
			MoveTo(posSrc);
			Read(pBuf, (ULONG)cbSize.QuadPart);
			if (FAILED(m_hrLastErr))
				return;
			MoveTo(posDst);
			Write(pBuf, (ULONG)cbSize.QuadPart);
			return;
		}

		const auto posSrcEnd = posSrc + cbSize;
		const auto posDstEnd = posDst + cbSize;
		if (posDst > posSrc)// 从后向前复制
		{
			LARGE_INTEGER posRead = ToLi(posSrcEnd - cbBuf);
			LARGE_INTEGER posWrite = ToLi(posDstEnd - cbBuf);
			const LARGE_INTEGER liPosSrc = ToLi(posSrc);
			for (;;)
			{
				MoveTo(posRead);
				Read(pBuf, cbBuf);
				if (FAILED(m_hrLastErr))
					return;
				if (m_cbLastReadWrite != cbBuf)
				{
					m_hrLastErr = E_FAIL;
					return;
				}
				MoveTo(posWrite);
				Write(pBuf, cbBuf);
				if (FAILED(m_hrLastErr))
					return;
				if (m_cbLastReadWrite != cbBuf)
				{
					m_hrLastErr = E_FAIL;
					return;
				}

				const auto posPrev = posRead;
				posRead -= cbBuf;
				posWrite -= cbBuf;
				if (posRead < liPosSrc)
				{
					posRead = liPosSrc;
					posWrite = posRead + ToLi(posDst - posSrc);
					const ULONG cbRead = (ULONG)(posPrev - liPosSrc).QuadPart;
					MoveTo(posRead);
					Read(pBuf, cbRead);
					if (m_cbLastReadWrite != cbRead)
					{
						m_hrLastErr = E_FAIL;
						return;
					}
					MoveTo(posWrite);
					Write(pBuf, cbRead);
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
			for (;;)
			{
				MoveTo(posRead);
				Read(pBuf, cbBuf);
				if (FAILED(m_hrLastErr))
					return;
				if (m_cbLastReadWrite != cbBuf)
				{
					m_hrLastErr = E_FAIL;
					return;
				}
				MoveTo(posWrite);
				Write(pBuf, cbBuf);
				if (FAILED(m_hrLastErr))
					return;
				if (m_cbLastReadWrite != cbBuf)
				{
					m_hrLastErr = E_FAIL;
					return;
				}

				const auto posPrev = posRead;
				posRead += cbBuf;
				posWrite += cbBuf;
				if (posRead >= liPosSrcEnd)
				{
					const ULONG cbRead = (ULONG)(liPosSrcEnd - posPrev).QuadPart;
					MoveTo(posRead);
					Read(pBuf, cbRead);
					if (m_cbLastReadWrite != cbRead)
					{
						m_hrLastErr = E_FAIL;
						return;
					}
					MoveTo(posWrite);
					Write(pBuf, cbRead);
					if (m_cbLastReadWrite != cbRead)
					{
						m_hrLastErr = E_FAIL;
						return;
					}
					break;
				}
			}
		}
	}

	void Insert(ULARGE_INTEGER pos, ULARGE_INTEGER cbSize)
	{
		if (cbSize == 0ull)
			return;
		const auto uliStrmSize = GetSize();
		m_pStream->SetSize(GetSize() + cbSize);
		if (pos != uliStrmSize)
			MoveData(pos + cbSize, pos, uliStrmSize - pos);
	}

	void Erase(ULARGE_INTEGER pos, ULARGE_INTEGER cbSize)
	{
		if (cbSize == 0ull)
			return;
		EckAssert(pos < GetSize() && pos + cbSize <= GetSize());
		MoveData(pos, pos + cbSize, cbSize);
	}
};
ECK_NAMESPACE_END