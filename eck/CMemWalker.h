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
ECK_NAMESPACE_END