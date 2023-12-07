/*
* WinEzCtrlKit Library
*
* Utility.h ： 实用函数
*
* Copyright(C) 2023 QingKong
*/

#pragma once
#include "ECK.h"
#include "CRefBin.h"
#include "CRefStr.h"

#include <math.h>
#include <time.h>
#include <process.h>

#include <string_view>

#include <windowsx.h>
#include <d2d1_1.h>
#include <Shlwapi.h>

#define MAKEINTATOMW(i) (PWSTR)((ULONG_PTR)((WORD)(i)))
#define EckBoolNot(x) ((x) = !(x))
// lParam->POINT 用于处理鼠标消息   e.g. POINT pt = GET_PT_LPARAM(lParam);
#define GET_PT_LPARAM(lParam) { GET_X_LPARAM(lParam),GET_Y_LPARAM(lParam) }
// lParam->size 用于处理WM_SIZE   e.g. GET_SIZE_LPARAM(cxClient, cyClient, lParam);
#define GET_SIZE_LPARAM(cx,cy,lParam) (cx) = LOWORD(lParam); (cy) = HIWORD(lParam);

// 计次循环
#define EckCounter(c, Var) for(std::remove_cvref_t<decltype(c)> Var = 0; Var < (c); ++Var)

#define EckCounterNVMakeVarName2(Name) ECKPRIV_COUNT_##Name##___
#define EckCounterNVMakeVarName(Name) EckCounterNVMakeVarName2(Name)

// 计次循环，无变量名参数
#define EckCounterNV(c) EckCounter((c), EckCounterNVMakeVarName(__LINE__))

#define EckOpt(Type, Name) std::optional<Type> Name
#define EckOptNul(Type, Name) std::optional<Type> Name = std::nullopt

ECK_NAMESPACE_BEGIN
namespace Colorref
{
	static constexpr COLORREF
		Red               = 0x0000FF,// 红色
		Green             = 0x00FF00,// 绿色
		Blue              = 0xFF0000,// 蓝色
		Yellow            = 0x00FFFF,// 黄色
		Magenta           = 0xFF00FF,// 品红/洋红
		Cyan              = 0xFFFF00,// 艳青/青色

		Maroon            = 0x000080,// 红褐/暗红
		OfficeGreen       = 0x008000,// 墨绿/暗绿
		Olive             = 0x008080,// 褐绿/暗黄
		NavyBlue          = 0x800000,// 藏青/暗蓝
		Patriarch         = 0x800080,// 紫红/暗洋红
		Teal              = 0x808000,// 深青/暗青

		Silver            = 0xC0C0C0,// 浅灰/亮灰
		MoneyGreen        = 0xC0DCC0,// 美元绿
		LightBlue         = 0xF0CAA6,// 浅蓝/天蓝

		Gray              = 0x808080,// 灰色/暗灰
		NeutralGray       = 0xA4A0A0,// 中性灰
		MilkyWhite        = 0xF0FBFF,// 乳白

		Black             = 0x000000,// 黑色
		White             = 0xFFFFFF,// 白色

		BlueGray          = 0xFF8080,// 蓝灰
		PurplishBlue      = 0xE03058,// 藏蓝
		TenderGreen       = 0x00E080,// 嫩绿
		Turquoise         = 0x80E000,// 青绿
		YellowishBrown    = 0x0060C0,// 黄褐
		Pink              = 0xFFA8FF,// 粉红
		BrightYellow      = 0x00D8D8,// 嫩黄
		JadeWhite         = 0xECECEC,// 银白
		Purple            = 0xFF0090,// 紫色
		Azure             = 0xFF8800,// 天蓝
		Celadon           = 0x80A080,// 灰绿
		CyanBlue          = 0xC06000,// 青蓝
		Orange            = 0x0080FF,// 橙黄
		Peachblow         = 0x8050FF,// 桃红
		HibiscusRed       = 0xC080FF,// 芙红
		DeepGray          = 0x606060// 深灰
		;
}

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

struct CEzCDC
{
	HDC m_hCDC = NULL;
	HBITMAP m_hBmp = NULL;
	HGDIOBJ m_hOld = NULL;

	CEzCDC() = default;
	CEzCDC(HWND hWnd, int cx = 0, int cy = 0)
	{
		Create(hWnd, cx, cy);
	}

	~CEzCDC()
	{
		if (m_hCDC)
		{
			SelectObject(m_hCDC, m_hOld);
			DeleteObject(m_hBmp);
			DeleteDC(m_hCDC);
		}
	}

	HDC Create(HWND hWnd, int cx = 0, int cy = 0);

	void ReSize(HWND hWnd, int cx = 0, int cy = 0);

	EckInline HDC GetDC() const { return m_hCDC; }

	EckInline HBITMAP GetBitmap() const { return m_hBmp; }
};

class IStreamView :public IStream
{
private:
	ULONG m_cRef = 1;

	PCBYTE m_pMem = NULL;
	PCBYTE m_pSeek = NULL;
	SIZE_T m_cbSize = 0u;
public:
	IStreamView(PCVOID p, SIZE_T cb) :m_pMem{ (PCBYTE)p }, m_cbSize{ cb }, m_pSeek{ NULL } {}

	void SetPointer(PCVOID p, SIZE_T cb)
	{
		m_pMem = (PCBYTE)p;
		m_pSeek = m_pMem;
		m_cbSize = cb;
	}

	HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void __RPC_FAR* __RPC_FAR* ppvObject)
	{
		const QITAB qit[]
		{
			QITABENT(IStreamView, IStream),
			QITABENT(IStreamView, ISequentialStream),
			{}
		};

		return QISearch(this, qit, riid, ppvObject);
	}

	ULONG STDMETHODCALLTYPE AddRef(void)
	{
		++m_cRef;
		return m_cRef;
	}

	ULONG STDMETHODCALLTYPE Release(void)
	{
		if (m_cRef == 1)
		{
			delete this;
			return 0;
		}
		return --m_cRef;
	}

	HRESULT STDMETHODCALLTYPE Read(void* pv, ULONG cb, ULONG* pcbRead)
	{
		if (!pv || !pcbRead)
			return STG_E_INVALIDPOINTER;
		HRESULT hr = S_OK;
		if (m_pSeek + cb > m_pMem + m_cbSize)
		{
			cb = (ULONG)(m_cbSize - (m_pSeek - m_pMem));
			hr = S_FALSE;
		}
		memcpy(pv, m_pSeek, cb);
		return hr;
	}

	HRESULT STDMETHODCALLTYPE Write(const void* pv, ULONG cb, ULONG* pcbWritten)
	{
		EckDbgBreak();
		return STG_E_CANTSAVE;
	}

	HRESULT STDMETHODCALLTYPE Seek(LARGE_INTEGER dlibMove, DWORD dwOrigin, ULARGE_INTEGER* plibNewPosition)
	{
		switch (dwOrigin)
		{
		case SEEK_SET:
			if ((SIZE_T)dlibMove.QuadPart > m_cbSize)
				return STG_E_INVALIDFUNCTION;
			m_pSeek = m_pMem + (SIZE_T)dlibMove.QuadPart;
			if (plibNewPosition)
				plibNewPosition->QuadPart = dlibMove.QuadPart;
			return S_OK;
		case SEEK_CUR:
		{
			const auto ocbNew = (SIZE_T)dlibMove.QuadPart + (m_pSeek - m_pMem);
			if (ocbNew > m_cbSize || ocbNew < 0)
				return STG_E_INVALIDFUNCTION;
			m_pSeek = m_pMem + ocbNew;
			if (plibNewPosition)
				plibNewPosition->QuadPart = (LONGLONG)ocbNew;
		}
		return S_OK;
		case SEEK_END:
			if (dlibMove.QuadPart < -(LONGLONG)m_cbSize || dlibMove.QuadPart>0)
				return STG_E_INVALIDFUNCTION;
			m_pSeek = m_pMem + m_cbSize + (SIZE_T)dlibMove.QuadPart;
			if (plibNewPosition)
				plibNewPosition->QuadPart = m_cbSize + dlibMove.QuadPart;
			return S_OK;
		}
		return STG_E_INVALIDFUNCTION;
	}

	HRESULT STDMETHODCALLTYPE SetSize(ULARGE_INTEGER libNewSize)
	{
		EckDbgBreak();
		return STG_E_MEDIUMFULL;
	}

	HRESULT STDMETHODCALLTYPE CopyTo(IStream* pstm, ULARGE_INTEGER cb, ULARGE_INTEGER* pcbRead, ULARGE_INTEGER* pcbWritten)
	{
		if (!pstm)
			return STG_E_INVALIDPOINTER;
		if (m_pSeek + cb.QuadPart > m_pMem + m_cbSize)
			cb.QuadPart = m_cbSize;

		ULONG cbWritten;
		pstm->Write(m_pSeek, cb.LowPart, &cbWritten);
		if (pcbRead)
			pcbRead->QuadPart = cbWritten;
		if (pcbWritten)
			pcbWritten->QuadPart = cbWritten;
		return S_OK;
	}

	HRESULT STDMETHODCALLTYPE Commit(DWORD grfCommitFlags)
	{
		return S_OK;
	}

	HRESULT STDMETHODCALLTYPE Revert(void)
	{
		return S_OK;
	}

	HRESULT STDMETHODCALLTYPE LockRegion(ULARGE_INTEGER libOffset, ULARGE_INTEGER cb, DWORD dwLockType)
	{
		EckDbgBreak();
		return STG_E_INVALIDFUNCTION;
	}

	HRESULT STDMETHODCALLTYPE UnlockRegion(ULARGE_INTEGER libOffset, ULARGE_INTEGER cb, DWORD dwLockType)
	{
		return STG_E_INVALIDFUNCTION;
	}

	HRESULT STDMETHODCALLTYPE Stat(STATSTG* pstatstg, DWORD grfStatFlag)
	{
		return STG_E_ACCESSDENIED;
	}

	HRESULT STDMETHODCALLTYPE Clone(IStream** ppstm)
	{
		EckDbgBreak();
		return STG_E_INSUFFICIENTMEMORY;
	}
};

struct CMAllocDeleter
{
	void operator()(void* p) const
	{
		free(p);
	}
};

/// <summary>
/// 计算下一对齐边界
/// </summary>
/// <param name="pStart">起始地址</param>
/// <param name="pCurr">当前地址</param>
/// <param name="cbAlign">对齐尺寸</param>
/// <returns>当前地址到下一对齐边界的距离，如果当前地址已经落在对齐边界上，则返回0</returns>
constexpr EckInline SIZE_T CalcNextAlignBoundaryDistance(const void* pStart, const void* pCurr, SIZE_T cbAlign)
{
	SIZE_T uDistance = (SIZE_T)pCurr - (SIZE_T)pStart;
	return (((uDistance - 1u) / cbAlign + 1u) * cbAlign - uDistance);
}

/// <summary>
/// 步进到下一对齐边界
/// </summary>
/// <param name="pStart">起始指针</param>
/// <param name="pCurr">当前指针</param>
/// <param name="cbAlign">对齐尺寸</param>
/// <returns>步进后的指针，如果当前指针已经落在对齐边界上，则指针不变</returns>
template<class T>
constexpr EckInline const T* StepToNextAlignBoundary(const T* pStart, const T* pCurr, SIZE_T cbAlign)
{
	return (const T*)((BYTE*)pCurr + CalcNextAlignBoundaryDistance(pStart, pCurr, cbAlign));
}

/// <summary>
/// 步进到下一对齐边界
/// </summary>
/// <param name="pStart">起始指针</param>
/// <param name="pCurr">当前指针</param>
/// <param name="cbAlign">对齐尺寸</param>
/// <returns>步进后的指针，如果当前指针已经落在对齐边界上，则指针不变</returns>
template<class T>
constexpr EckInline T* StepToNextAlignBoundary(T* pStart, T* pCurr, SIZE_T cbAlign)
{
	return (T*)((BYTE*)pCurr + CalcNextAlignBoundaryDistance(pStart, pCurr, cbAlign));
}

template<class T>
constexpr EckInline PCVOID PtrSkipType(const T* p)
{
	return (PCVOID)((PCBYTE)p + sizeof(T));
}

template<class T>
constexpr EckInline void* PtrSkipType(T* p)
{
	return (void*)((PCBYTE)p + sizeof(T));
}

/// <summary>
/// 计算对齐后内存尺寸
/// </summary>
/// <param name="cbSize">尺寸</param>
/// <param name="cbAlign">对齐尺寸</param>
/// <returns>计算结果</returns>
EckInline constexpr SIZE_T AlignMemSize(SIZE_T cbSize, SIZE_T cbAlign)
{
	return (cbSize + cbAlign) & ~cbAlign;
}

/// <summary>
/// CRT创建线程。
/// （_beginthreadex wrapper）
/// </summary>
/// <param name="lpStartAddress">起始地址</param>
/// <param name="lpParameter">参数</param>
/// <param name="lpThreadId">线程ID变量指针</param>
/// <param name="dwCreationFlags">标志</param>
/// <returns>线程句柄</returns>
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
constexpr EckInline T i32ToP(U i)
{
	return (T)((ULONG_PTR)i);
}

template<class T, class U>
constexpr EckInline T pToI32(U p)
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
constexpr EckInline BOOL IsBitSet(T1 dw1, T2 dw2)
{
	return (dw1 & dw2) == dw2;
}

/// <summary>
/// 取运行目录
/// </summary>
const CRefStrW& GetRunningPath();

/// <summary>
/// 读入文件
/// </summary>
/// <param name="pszFile">文件路径</param>
/// <returns>返回字节集</returns>
CRefBin ReadInFile(PCWSTR pszFile);

/// <summary>
/// 写到文件
/// </summary>
/// <param name="pData">字节流</param>
/// <param name="cb">字节流长度</param>
BOOL WriteToFile(PCWSTR pszFile, PCVOID pData, DWORD cb);

EckInline BOOL WriteToFile(PCWSTR pszFile, const CRefBin& rb)
{
	return WriteToFile(pszFile, rb.Data(), (DWORD)rb.Size());
}

/// <summary>
/// 字节集到友好字符串表示
/// </summary>
/// <param name="Bin">字节集</param>
/// <param name="iType">类型，0 - 空格分割的十六进制  1 - 易语言字节集调试输出</param>
/// <returns>返回结果</returns>
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

template<class TCharTraits = CCharTraits<CHAR>, class TAlloc = CAllocatorProcHeap<CHAR, int>>
CRefStrT<CHAR, TCharTraits, TAlloc> StrW2X(PCWSTR pszText, int cch = -1, int uCP = CP_ACP)
{
	int cchBuf = WideCharToMultiByte(uCP, WC_COMPOSITECHECK, pszText, cch, NULL, 0, NULL, NULL);
	CRefStrT<CHAR, TCharTraits, TAlloc> rs(cchBuf);
	WideCharToMultiByte(uCP, WC_COMPOSITECHECK, pszText, cch, rs.Data(), cchBuf, NULL, NULL);
	*(rs.Data() + cchBuf) = '\0';
	return rs;
}

template<class TCharTraits = CCharTraits<WCHAR>, class TAlloc = CAllocatorProcHeap<WCHAR, int>>
CRefStrT<WCHAR, TCharTraits, TAlloc> StrX2W(PCSTR pszText, int cch = -1, int uCP = CP_ACP)
{
	int cchBuf = MultiByteToWideChar(uCP, MB_PRECOMPOSED, pszText, cch, NULL, 0);
	CRefStrT<WCHAR, TCharTraits, TAlloc> rs(cchBuf);
	MultiByteToWideChar(uCP, MB_PRECOMPOSED, pszText, cch, rs.Data(), cchBuf);
	*(rs.Data() + cchBuf) = '\0';
	return rs;
}

EckInline void DbBufPrepare(HWND hWnd, HDC& hCDC, HBITMAP& hBitmap, HGDIOBJ& hOldBmp, int cx = 8, int cy = 8)
{
	HDC hDC = GetDC(hWnd);
	hCDC = CreateCompatibleDC(hDC);
	hBitmap = CreateCompatibleBitmap(hDC, cx, cy);
	hOldBmp = SelectObject(hCDC, hBitmap);
	ReleaseDC(hWnd, hDC);
}

EckInline void DbBufReSize(HWND hWnd, HDC& hCDC, HBITMAP& hBitmap, HGDIOBJ& hOldBmp, int cx, int cy)
{
	SelectObject(hCDC, hOldBmp);
	DeleteObject(hBitmap);

	HDC hDC = GetDC(hWnd);
	hBitmap = CreateCompatibleBitmap(hDC, cx, cy);
	hOldBmp = SelectObject(hCDC, hBitmap);
	ReleaseDC(hWnd, hDC);
}

EckInline void DbBufFree(HDC hCDC, HBITMAP hBitmap, HGDIOBJ hOldBmp)
{
	SelectObject(hCDC, hOldBmp);
	DeleteObject(hBitmap);
	DeleteDC(hCDC);
}

template<class T>
EckInline constexpr BOOL Sign(T v)
{
	return v >= 0;
}

EckInline constexpr DWORD ReverseDWORD(const BYTE* p)
{
	return ((p[0] << 24) | (p[1] << 16) | (p[2] << 8) | p[3]);
}

/// <summary>
/// 反转4字节整数字节序
/// </summary>
/// <param name="dw">输入</param>
/// <returns>转换结果</returns>
EckInline constexpr DWORD ReverseDWORD(DWORD dw)
{
	return ReverseDWORD((BYTE*)&dw);
}

EckInline constexpr SIZE_T Cch2Cb(int cch)
{
	return (cch + 1) * sizeof(WCHAR);
}

EckInline D2D1_RECT_F MakeD2DRcF(const RECT& rc)
{
	return D2D1::RectF((float)rc.left, (float)rc.top, (float)rc.right, (float)rc.bottom);
}

EckInline constexpr RECT MakeRect(const D2D1_RECT_F& rc)
{
	return { (LONG)rc.left, (LONG)rc.top, (LONG)rc.right, (LONG)rc.bottom };
}

EckInline constexpr UINT ReverseColorref(UINT cr)
{
	BYTE* p = (BYTE*)&cr;
	return (((p[0] << 16) | (p[1] << 8) | p[2]) & 0x00FFFFFF);
}

EckInline void DoEvents()
{
	MSG msg;
	while (PeekMessageW(&msg, NULL, 0u, 0u, PM_REMOVE))
	{
		if (msg.message == WM_QUIT)
		{
			PostQuitMessage((int)msg.wParam);
			return;
		}
		TranslateMessage(&msg);
		DispatchMessageW(&msg);
	}
}

EckInline constexpr UINT Gcd(UINT a, UINT b)
{
	UINT c;
	for (;;)
	{
		c = a % b;
		if (c)
		{
			a = b;
			b = c;
		}
		else
			return b;
	}
}

EckInline constexpr GpRectF ToGpRectF(const RECT& rc)
{
	return { (REAL)rc.left,(REAL)rc.top,(REAL)(rc.right - rc.left),(REAL)(rc.bottom - rc.top) };
}

EckInline constexpr HRESULT HResultFromBool(BOOL b)
{
	return b ? S_OK : S_FALSE;
}

inline CRefStrW GetClipboardString(HWND hWnd = NULL)
{
	if (!OpenClipboard(hWnd))
	{
		EckDbgPrintFmt(L"剪贴板打开失败，当前所有者 = %p", GetClipboardOwner());
		return {};
	}
	if (IsClipboardFormatAvailable(CF_UNICODETEXT))
	{
		const HANDLE hData = GetClipboardData(CF_UNICODETEXT);
		if (hData)
		{
			const void* pData = GlobalLock(hData);
			if (pData)
			{
				const auto cb = GlobalSize(hData);
				const int cch = (int)(cb / sizeof(WCHAR));
				CRefStrW rs(cch);
				memcpy(rs.Data(), pData, cch * sizeof(WCHAR));
				GlobalUnlock(hData);
				CloseClipboard();
				return rs;
			}
		}
	}
	CloseClipboard();
	return {};
}

inline BOOL SetClipboardString(PCWSTR pszText, int cch = -1, HWND hWnd = NULL)
{
	if (!OpenClipboard(hWnd))
	{
		EckDbgPrintFmt(L"剪贴板打开失败，当前所有者 = %p", GetClipboardOwner());
		return FALSE;
	}
	EmptyClipboard();
	if (cch < 0)
		cch = (int)wcslen(pszText);
	const SIZE_T cb = Cch2Cb(cch);
	
	HGLOBAL hGlobal = GlobalAlloc(GMEM_MOVEABLE, cb);
	if (hGlobal)
	{
		void* pData = GlobalLock(hGlobal);
		if (pData)
		{
			memcpy(pData, pszText, cb);
			GlobalUnlock(hGlobal);
			SetClipboardData(CF_UNICODETEXT, hGlobal);
			CloseClipboard();
			return TRUE;
		}
	}
	CloseClipboard();
	return FALSE;
}

template<class T>
EckInline T Abs(T x)
{
	if (x >= 0) return x;
	else return -x;
}

template<class T>
EckInline T SetSign(T x, T iSign)
{
	if (iSign > 0) return Abs(x);
	if (iSign < 0) return -Abs(x);
	else return x;
}
ECK_NAMESPACE_END