/*
* WinEzCtrlKit Library
*
* Utility.h �� ʵ�ú���
*
* Copyright(C) 2023 QingKong
*/
// �ƴ�ѭ��

#pragma once
#include "ECK.h"
#include "CRefBin.h"
#include "CRefStr.h"

#include <math.h>
#include <time.h>
#include <process.h>

#include <string_view>

#include <windowsx.h>

#define MAKEINTATOMW(i) (PWSTR)((ULONG_PTR)((WORD)(i)))
#define EckBoolNot(x) ((x) = !(x))
// lParam->POINT ���ڴ��������Ϣ   e.g. POINT pt = GET_PT_LPARAM(lParam);
#define GET_PT_LPARAM(lParam) { GET_X_LPARAM(lParam),GET_Y_LPARAM(lParam) }
// lParam->size ���ڴ���WM_SIZE   e.g. GET_SIZE_LPARAM(cxClient, cyClient, lParam);
#define GET_SIZE_LPARAM(cx,cy,lParam) (cx) = LOWORD(lParam); (cy) = HIWORD(lParam);

#define EckCounter(c, Var) for(decltype(c) Var = 0; Var < c; ++Var)

#define EckCounterNVMakeVarName2(Name) ECKPRIV_COUNT_##Name##___
#define EckCounterNVMakeVarName(Name) EckCounterNVMakeVarName2(Name)

#define EckCounterNV(c) EckCounter(c, EckCounterNVMakeVarName(__LINE__))

ECK_NAMESPACE_BEGIN
namespace Colorref
{
	static constexpr COLORREF
		Red               = 0x0000FF,// ��ɫ
		Green             = 0x00FF00,// ��ɫ
		Blue              = 0xFF0000,// ��ɫ
		Yellow            = 0x00FFFF,// ��ɫ
		Magenta           = 0xFF00FF,// Ʒ��/���
		Cyan              = 0xFFFF00,// ����/��ɫ

		Maroon            = 0x000080,// ���/����
		OfficeGreen       = 0x008000,// ī��/����
		Olive             = 0x008080,// ����/����
		NavyBlue          = 0x800000,// ����/����
		Patriarch         = 0x800080,// �Ϻ�/�����
		Teal              = 0x808000,// ����/����

		Silver            = 0xC0C0C0,// ǳ��/����
		MoneyGreen        = 0xC0DCC0,// ��Ԫ��
		LightBlue         = 0xF0CAA6,// ǳ��/����

		Gray              = 0x808080,// ��ɫ/����
		NeutralGray       = 0xA4A0A0,// ���Ի�
		MilkyWhite        = 0xF0FBFF,// ���

		Black             = 0x000000,// ��ɫ
		White             = 0xFFFFFF,// ��ɫ

		BlueGray          = 0xFF8080,// ����
		PurplishBlue      = 0xE03058,// ����
		TenderGreen       = 0x00E080,// ����
		Turquoise         = 0x80E000,// ����
		YellowishBrown    = 0x0060C0,// �ƺ�
		Pink              = 0xFFA8FF,// �ۺ�
		BrightYellow      = 0x00D8D8,// �ۻ�
		JadeWhite         = 0xECECEC,// ����
		Purple            = 0xFF0090,// ��ɫ
		Azure             = 0xFF8800,// ����
		Celadon           = 0x80A080,// ����
		CyanBlue          = 0xC06000,// ����
		Orange            = 0x0080FF,// �Ȼ�
		Peachblow         = 0x8050FF,// �Һ�
		HibiscusRed       = 0xC080FF,// ܽ��
		DeepGray          = 0x606060// ���
		;
}

struct CMemWriter
{
	BYTE* m_pMem = NULL;
	CMemWriter(void* p)
	{
		m_pMem = (BYTE*)p;
	}

	EckInline CMemWriter& Write(PCVOID pSrc, SIZE_T cb)
	{
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

	EckInline CMemWriter& operator<<(const CRefBin& Data)
	{
		return Write(Data, Data.m_cb);
	}

	EckInline CMemWriter& operator<<(const CRefStrW& Data)
	{
		return Write(Data, (Data.m_cchText + 1) * sizeof(WCHAR));
	}

	EckInline operator BYTE*& () { return m_pMem; }

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
	BYTE* m_pMem = NULL;
	CMemReader(PCVOID p) { m_pMem = (BYTE*)p; }

	EckInline CMemReader& Read(void* pDst, SIZE_T cb)
	{
		memcpy(pDst, m_pMem, cb);
		m_pMem += cb;
		return *this;
	}

	template<class T>
	EckInline CMemReader& operator>>(T& Data)
	{
		return Read(&Data, sizeof(Data));
	}

	EckInline CMemReader& operator>>(CRefStrW& Data)
	{
		int cch = (int)wcslen((PCWSTR)m_pMem);
		Data.ReSize(cch);
		return Read(Data, (cch + 1) * sizeof(WCHAR));
	}

	EckInline operator BYTE*& () { return m_pMem; }

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

/// <summary>
/// ������һ����߽�
/// </summary>
/// <param name="pStart">��ʼ��ַ</param>
/// <param name="pCurr">��ǰ��ַ</param>
/// <param name="cbAlign">����ߴ�</param>
/// <returns>��ǰ��ַ����һ����߽�ľ��룬�����ǰ��ַ�Ѿ����ڶ���߽��ϣ��򷵻�0</returns>
EckInline SIZE_T CalcNextAlignBoundaryDistance(const void* pStart, const void* pCurr, SIZE_T cbAlign)
{
	SIZE_T uDistance = (SIZE_T)pCurr - (SIZE_T)pStart;
	return (((uDistance - 1u) / cbAlign + 1u) * cbAlign - uDistance);
}

/// <summary>
/// ��������һ����߽�
/// </summary>
/// <param name="pStart">��ʼָ��</param>
/// <param name="pCurr">��ǰָ��</param>
/// <param name="cbAlign">����ߴ�</param>
/// <returns>�������ָ�룬�����ǰָ���Ѿ����ڶ���߽��ϣ���ָ�벻��</returns>
template<class T>
EckInline T* StepToNextAlignBoundary(const T* pStart,const T* pCurr, SIZE_T cbAlign)
{
	return (T*)((BYTE*)pCurr + CalcNextAlignBoundaryDistance(pStart, pCurr, cbAlign));
}

/// <summary>
/// ���������ڴ�ߴ�
/// </summary>
/// <param name="cbSize">�ߴ�</param>
/// <param name="cbAlign">����ߴ�</param>
/// <returns>������</returns>
EckInline constexpr SIZE_T AlignMemSize(SIZE_T cbSize, SIZE_T cbAlign)
{
	return (cbSize + cbAlign) & ~cbAlign;
}

/// <summary>
/// CRT�����̡߳�
/// ��_beginthreadex wrapper��
/// </summary>
/// <param name="lpStartAddress">��ʼ��ַ</param>
/// <param name="lpParameter">����</param>
/// <param name="lpThreadId">�߳�ID����ָ��</param>
/// <param name="dwCreationFlags">��־</param>
/// <returns>�߳̾��</returns>
EckInline HANDLE CRTCreateThread(_beginthreadex_proc_type lpStartAddress, void* lpParameter = NULL,
	UINT* lpThreadId = NULL, UINT dwCreationFlags = 0)
{
	return ((HANDLE)_beginthreadex(0, 0, lpStartAddress, lpParameter, dwCreationFlags, lpThreadId));
}

EckInline constexpr ARGB ColorrefToARGB(COLORREF cr, BYTE byAlpha = 0xFF)
{
	BYTE* pcr = (BYTE*)&cr;
	BYTE byTemp;
	byTemp = pcr[0];
	pcr[0] = pcr[2];
	pcr[2] = byTemp;
	pcr[3] = byAlpha;
	return cr;
}

template<class T, class U>
EckInline T i32ToP(U i)
{
	return (T)((ULONG_PTR)i);
}

template<class T, class U>
EckInline T pToI32(U p)
{
	return (T)((ULONG_PTR)p);
}

EckInline BOOL IsFILETIMEZero(const FILETIME* pft)
{
	return pft->dwLowDateTime == 0 && pft->dwHighDateTime == 0;
}

EckInline BOOL operator==(const FILETIME& ft1, const FILETIME& ft2)
{
	return CompareFileTime(&ft1, &ft2) == 0;
}

EckInline BOOL operator>(const FILETIME& ft1, const FILETIME& ft2)
{
	return CompareFileTime(&ft1, &ft2) == 1;
}

EckInline BOOL operator<(const FILETIME& ft1, const FILETIME& ft2)
{
	return CompareFileTime(&ft1, &ft2) == -1;
}

template<class T1, class T2>
EckInline BOOL IsBitSet(T1 dw1, T2 dw2)
{
	return ((dw1 & dw2) == dw2);
}

template<class T1, class T2>
EckInline T1 ReInterpretNum(T2 n)
{
	return *(T1*)&n;
}

/// <summary>
/// ȡ����Ŀ¼
/// </summary>
const CRefStrW& GetRunningPath();

/// <summary>
/// �����ļ�
/// </summary>
/// <param name="pszFile">�ļ�·��</param>
/// <returns>�����ֽڼ�</returns>
CRefBin ReadInFile(PCWSTR pszFile);

/// <summary>
/// д���ļ�
/// </summary>
/// <param name="pData">�ֽ���</param>
/// <param name="cb">�ֽ�������</param>
BOOL WriteToFile(PCWSTR pszFile, PCVOID pData, DWORD cb);

EckInline BOOL WriteToFile(PCWSTR pszFile, const CRefBin& rb)
{
	return WriteToFile(pszFile, rb, (DWORD)rb.m_cb);
}

/// <summary>
/// �ֽڼ����Ѻ��ַ�����ʾ
/// </summary>
/// <param name="Bin">�ֽڼ�</param>
/// <param name="iType">���ͣ�0 - �ո�ָ��ʮ������  1 - �������ֽڼ��������</param>
/// <returns>���ؽ��</returns>
CRefStrW BinToFriendlyString(BYTE* pData, SIZE_T cb, int iType);

EckInline void ScreenToClient(HWND hWnd, RECT* prc)
{
	::ScreenToClient(hWnd, (POINT*)prc);
	::ScreenToClient(hWnd, ((POINT*)prc) + 1);
}

EckInline void ClientToScreen(HWND hWnd, RECT* prc)
{
	::ClientToScreen(hWnd, (POINT*)prc);
	::ClientToScreen(hWnd, ((POINT*)prc) + 1);
}

RECT MakeRect(POINT pt1, POINT pt2);

EckInline float RoundToF(float fVal, int cDigits)
{
	float fTemp = powf(10, (float)cDigits);
	return roundf(fVal * fTemp) / fTemp;
}

EckInline double RoundTo(double fVal, int cDigits)
{
	double fTemp = pow(10, (double)cDigits);
	return round(fVal * fTemp) / fTemp;
}

EckInline void RandSeed(UINT uSeed)
{
	srand(uSeed);
}

EckInline void RandSeed()
{
	srand((UINT)time(NULL));
} 

EckInline int Rand(int iMin = INT_MIN, int iMax = INT_MAX)
{
	return rand() % ((LONGLONG)iMax - (LONGLONG)iMin + 1ll) + (LONGLONG)iMin;
}

EckInline BOOL IsRectsIntersect(const RECT* prc1, const RECT* prc2)
{
	return
		std::max(prc1->left, prc2->left) < std::min(prc1->right, prc2->right) &&
		std::max(prc1->top, prc2->top) < std::min(prc1->bottom, prc2->bottom);
}
ECK_NAMESPACE_END