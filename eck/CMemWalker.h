/*
* WinEzCtrlKit Library
*
* CMemWalker.h ： 指针跟踪爬行器
*
* Copyright(C) 2023 QingKong
*/
#pragma once
#include "CRefBin.h"
#include "CRefStr.h"

ECK_NAMESPACE_BEGIN
struct CMemWriter
{
	BYTE* m_pMem = NULL;
#ifdef _DEBUG
	BYTE* m_pBase = NULL;
	SIZE_T m_cbMax = 0u;
#endif

	CMemWriter(void* p)
	{
		m_pMem = (BYTE*)p;
	}

	CMemWriter(PCVOID p, SIZE_T cbMax)
	{
		m_pMem = (BYTE*)p;
#ifdef _DEBUG
		m_pBase = m_pMem;
		m_cbMax = cbMax;
#endif
	}

	EckInline CMemWriter& Write(PCVOID pSrc, SIZE_T cb)
	{
#ifdef _DEBUG
		if (m_pBase)
		{
			EckDbgCheckMemRange(m_pBase, m_cbMax, m_pMem + cb);
		}
#endif
		memcpy(m_pMem, pSrc, cb);
		m_pMem += cb;
		return *this;
	}

	template<class T>
	EckInline CMemWriter& operator<<(const T& Data)
	{
		return Write(&Data, sizeof(Data));
	}

	template<class T, class U>
	EckInline CMemWriter& operator<<(const std::basic_string_view<T, U>& Data)
	{
		return Write(Data.data(), Data.size() * sizeof(T)) << L'\0';
	}

	template<class T, class U>
	EckInline CMemWriter& operator<<(const std::basic_string<T, U>& Data)
	{
		return Write(Data.c_str(), (Data.size() + 1) * sizeof(T));
	}

	template<class T, class U>
	EckInline CMemWriter& operator<<(const std::vector<T, U>& Data)
	{
		return Write(Data.data(), Data.size() * sizeof(T));
	}

	template<class T>
	EckInline CMemWriter& operator<<(const CRefBinT<T>& Data)
	{
		return Write(Data.Data(), Data.Size());
	}

	template<class T, class U, class V>
	EckInline CMemWriter& operator<<(const CRefStrT<T, U, V>& Data)
	{
		return Write(Data.Data(), Data.ByteSize());
	}

	EckInline operator BYTE*& () { return m_pMem; }

	EckInline BYTE* Data() { return m_pMem; }

	EckInline CMemWriter& operator+=(SIZE_T cb)
	{
		m_pMem += cb;
		return *this;
	}

	EckInline CMemWriter& operator-=(SIZE_T cb)
	{
		m_pMem -= cb;
		return *this;
	}

	template<class T>
	CMemWriter& SkipPointer(T*& p)
	{
		p = (T*)m_pMem;
		m_pMem += sizeof(T);
		return *this;
	}
};

struct CMemReader
{
	const BYTE* m_pMem = NULL;
#ifdef _DEBUG
	const BYTE* m_pBase = NULL;
	SIZE_T m_cbMax = 0u;
#endif

	CMemReader(PCVOID p, SIZE_T cbMax = 0u)
	{
		SetPtr(p, cbMax);
	}

	void SetPtr(PCVOID p, SIZE_T cbMax = 0u)
	{
		m_pMem = (const BYTE*)p;
#ifdef _DEBUG
		if (cbMax)
		{
			m_pBase = m_pMem;
			m_cbMax = cbMax;
		}
#endif
	}

	EckInline CMemReader& Read(void* pDst, SIZE_T cb)
	{
#ifdef _DEBUG
		if (m_pBase)
		{
			EckDbgCheckMemRange(m_pBase, m_cbMax, m_pMem + cb);
		}
#endif
		memcpy(pDst, m_pMem, cb);
		m_pMem += cb;
		return *this;
	}

	template<class T>
	EckInline CMemReader& operator>>(T& Data)
	{
		return Read(&Data, sizeof(Data));
	}

	template<class T, class U, class V>
	EckInline CMemWriter& operator>>(const CRefStrT<T, U, V>& x)
	{
		const int cch = U::Len(Data());
		x.ReSize(cch);
		return Read(x.Data(), cch * sizeof(WCHAR));
	}

	EckInline operator const BYTE*& () { return m_pMem; }

	EckInline const BYTE* Data() { return m_pMem; }

	EckInline CMemReader& operator+=(SIZE_T cb)
	{
		m_pMem += cb;
		return *this;
	}

	EckInline CMemReader& operator-=(SIZE_T cb)
	{
		m_pMem -= cb;
		return *this;
	}

	template<class T>
	CMemReader& SkipPointer(T*& p)
	{
		p = (T*)m_pMem;
		m_pMem += sizeof(T);
		return *this;
	}
};

struct CMemWalker
{
	BYTE* m_pMem = NULL;
	BYTE* m_pBase = NULL;
	SIZE_T m_cbMax = 0u;

	CMemWalker(PVOID p, SIZE_T cbMax)
	{
		m_pMem = (BYTE*)p;
		m_pBase = m_pMem;
		m_cbMax = cbMax;
	}

	EckInline CMemWalker& Write(PCVOID pSrc, SIZE_T cb)
	{
		EckDbgCheckMemRange(m_pBase, m_cbMax, m_pMem + cb);
		memcpy(m_pMem, pSrc, cb);
		m_pMem += cb;
		return *this;
	}

	EckInline CMemWalker& WriteRev(PCVOID pSrc, SIZE_T cb)
	{
		Write(pSrc, cb);
		ReverseByteOrder(Data() - cb, cb);
		return *this;
	}

	template<class T>
	EckInline CMemWalker& operator<<(const T& Data)
	{
		return Write(&Data, sizeof(T));
	}

	template<class T>
	EckInline CMemWalker& WriteRev(const T& Data)
	{
		return WriteRev(&Data, sizeof(T));
	}

	template<class T, class U>
	EckInline CMemWalker& operator<<(const std::basic_string_view<T, U>& Data)
	{
		return Write(Data.data(), Data.size() * sizeof(T)) << L'\0';
	}

	template<class T, class U>
	EckInline CMemWalker& operator<<(const std::basic_string<T, U>& Data)
	{
		return Write(Data.c_str(), (Data.size() + 1) * sizeof(T));
	}

	template<class T, class U>
	EckInline CMemWalker& operator<<(const std::vector<T, U>& Data)
	{
		return Write(Data.data(), Data.size() * sizeof(T));
	}

	template<class T>
	EckInline CMemWalker& operator<<(const CRefBinT<T>& Data)
	{
		return Write(Data.Data(), Data.Size());
	}

	template<class T, class U, class V>
	EckInline CMemWalker& operator<<(const CRefStrT<T, U, V>& Data)
	{
		return Write(Data.Data(), Data.ByteSize());
	}

	EckInline BYTE* Data() { return m_pMem; }

	EckInline CMemWalker& operator+=(SIZE_T cb)
	{
		m_pMem += cb;
		return *this;
	}

	EckInline CMemWalker& operator-=(SIZE_T cb)
	{
		m_pMem -= cb;
		return *this;
	}

	template<class T>
	CMemWalker& SkipPointer(T*& p)
	{
		p = (T*)m_pMem;
		m_pMem += sizeof(T);
		EckDbgCheckMemRange(m_pBase, m_cbMax, m_pMem);
		return *this;
	}

	void SetPtr(PVOID p, SIZE_T cbMax)
	{
		m_pMem = (BYTE*)p;
		m_pBase = m_pMem;
		m_cbMax = cbMax;
	}

	EckInline CMemWalker& Read(void* pDst, SIZE_T cb)
	{
		EckDbgCheckMemRange(m_pBase, m_cbMax, m_pMem + cb);
		memcpy(pDst, m_pMem, cb);
		m_pMem += cb;
		return *this;
	}

	EckInline CMemWalker& ReadRev(void* pDst, SIZE_T cb)
	{
		Read(pDst, cb);
		ReverseByteOrder((BYTE*)pDst, cb);
		return *this;
	}

	template<class T>
	EckInline CMemWalker& operator>>(T& Data)
	{
		return Read(&Data, sizeof(Data));
	}

	template<class T>
	EckInline CMemWalker& ReadRev(T& Data)
	{
		return ReadRev(&Data, sizeof(T));
	}

	template<class T, class U, class V>
	EckInline CMemWalker& operator>>(CRefStrT<T, U, V>& x)
	{
		const int cch = U::Len((const T*)Data());
		x.ReSize(cch);
		return Read(x.Data(), (cch + 1) * sizeof(T));
	}

	EckInline constexpr CMemWalker& MoveToBegin()
	{
		m_pMem = m_pBase;
		return *this;
	}

	EckInline constexpr CMemWalker& MoveToEnd()
	{
		m_pMem = m_pBase + m_cbMax;
		return *this;
	}

	EckInline constexpr CMemWalker& MoveTo(SIZE_T pos)
	{
		m_pMem = m_pBase + pos;
		EckDbgCheckMemRange(m_pBase, m_cbMax, m_pMem);
		return *this;
	}

	EckInline constexpr SIZE_T GetLeaveSize() const
	{
		return m_pBase + m_cbMax - m_pMem;
	}
	
	EckInline constexpr BOOL IsEnd() const
	{
		return m_pMem >= m_pBase + m_cbMax;
	}

	EckInline constexpr SIZE_T GetPos() const
	{
		return m_pMem - m_pBase;
	}
};
ECK_NAMESPACE_END