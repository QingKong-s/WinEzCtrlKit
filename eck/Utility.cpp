#include "Utility.h"
#include "CRefBin.h"
#include "CRefStr.h"

ECK_NAMESPACE_BEGIN
extern CRefStrW g_rsCurrDir;

HDC CEzCDC::Create(HWND hWnd, int cx, int cy)
{
	this->~CEzCDC();
	HDC hDC = ::GetDC(hWnd);
	if (cx < 0 || cy < 0)
	{
		RECT rc;
		GetClientRect(hWnd, &rc);
		cx = rc.right;
		cy = rc.bottom;
	}
	m_hCDC = CreateCompatibleDC(hDC);
	m_hBmp = CreateCompatibleBitmap(hDC, cx, cy);
	m_hOld = SelectObject(m_hCDC, m_hBmp);
	ReleaseDC(hWnd, hDC);
	return m_hCDC;
}

void CEzCDC::ReSize(HWND hWnd, int cx, int cy)
{
	if (m_hCDC)
	{
		if (cx < 0 || cy < 0)
		{
			RECT rc;
			GetClientRect(hWnd, &rc);
			cx = rc.right;
			cy = rc.bottom;
		}

		HDC hDC = ::GetDC(hWnd);
		SelectObject(m_hCDC, m_hOld);
		DeleteObject(m_hBmp);

		m_hBmp = CreateCompatibleBitmap(hDC, cx, cy);
		m_hOld = SelectObject(m_hCDC, m_hBmp);
		ReleaseDC(hWnd, hDC);
	}
}




const CRefStrW& GetRunningPath()
{
	return g_rsCurrDir;
}

CRefBin ReadInFile(PCWSTR pszFile)
{
	CRefBin rb;
	HANDLE hFile = CreateFileW(pszFile, GENERIC_READ, FILE_SHARE_READ, NULL,
		OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE)
		return rb;

	LARGE_INTEGER i64{};
	GetFileSizeEx(hFile, &i64);
	if (i64.QuadPart > 1'073'741'824i64)// 大于1G，不读
	{
		EckDbgPrintFmt(L"So LARGE File! Size = %i64", i64.QuadPart);
		CloseHandle(hFile);
		return rb;
	}

	rb.ReSize((SIZE_T)i64.QuadPart);
	DWORD dwRead;
	if (!ReadFile(hFile, rb, i64.LowPart, &dwRead, NULL))
		rb.ReSize(0);

	CloseHandle(hFile);
	return rb;
}

BOOL WriteToFile(PCWSTR pszFile, PCVOID pData, DWORD cb)
{
	HANDLE hFile = CreateFileW(pszFile, GENERIC_WRITE, FILE_SHARE_READ, NULL,
		OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE)
		return FALSE;

	DWORD dwRead;
	BOOL b = WriteFile(hFile, pData, cb, &dwRead, NULL);

	CloseHandle(hFile);
	return b;
}

CRefStrW BinToFriendlyString(BYTE* pData, SIZE_T cb, int iType)
{
	CRefStrW rsResult;
	if (!pData || !cb)
	{
		if (iType == 1)
			rsResult = L"{ }";
		return rsResult;
	}
#define B2SBUFSIZE 32
	WCHAR szBuf[B2SBUFSIZE];
	switch (iType)
	{
	case 0:
	{
		rsResult.Reserve((int)cb * 3 + 10);

		for (SIZE_T i = 0u; i < cb; ++i)
		{
			swprintf(szBuf, B2SBUFSIZE - 1, L"%02hhX ", pData[i]);
			rsResult.PushBack(szBuf);
		}
	}
	break;

	case 1:
	{
		rsResult.Reserve((int)cb * 4 + 10);

		rsResult.PushBack(L"{ ");
		swprintf(szBuf, B2SBUFSIZE - 1, L"%hhu", pData[0]);
		rsResult.PushBack(szBuf);
		for (SIZE_T i = 1u; i < cb; ++i)
		{
			swprintf(szBuf, B2SBUFSIZE - 1, L",%hhu", pData[i]);
			rsResult.PushBack(szBuf);
		}
		rsResult.PushBack(L" }");
	}
	break;
	}
#undef B2SBUFSIZE
	return rsResult;
}

RECT MakeRect(POINT pt1, POINT pt2)
{
	RECT rc;
	if (pt1.x >= pt2.x)
	{
		rc.left = pt2.x;
		rc.right = pt1.x;
	}
	else
	{
		rc.left = pt1.x;
		rc.right = pt2.x;
	}

	if (pt1.y >= pt2.y)
	{
		rc.top = pt2.y;
		rc.bottom = pt1.y;
	}
	else
	{
		rc.top = pt1.y;
		rc.bottom = pt2.y;
	}

	return rc;
}

PSTR StrW2X(PCWSTR pszText, int cch, int uCP)
{
	int cchBuf = WideCharToMultiByte(uCP, WC_COMPOSITECHECK, pszText, cch, NULL, 0, NULL, NULL);
	auto pszBuf = CAllocator<char>::Alloc(cchBuf + 1);
	WideCharToMultiByte(uCP, WC_COMPOSITECHECK, pszText, cch, pszBuf, cchBuf, NULL, NULL);
	*(pszBuf + cchBuf) = '\0';
	return pszBuf;
}

PWSTR StrX2W(PCSTR pszText, int cch, int uCP)
{
	int cchBuf = MultiByteToWideChar(uCP, MB_PRECOMPOSED, pszText, cch, NULL, 0);
	auto pszBuf = CAllocator<WCHAR>::Alloc(cchBuf + 1);
	MultiByteToWideChar(uCP, MB_PRECOMPOSED, pszText, cch, pszBuf, cchBuf);
	*(pszBuf + cchBuf) = '\0';
	return pszBuf;
}

CRefStrW StrX2WRs(PCSTR pszText, int cch, int uCP)
{
	int cchBuf = MultiByteToWideChar(uCP, MB_PRECOMPOSED, pszText, cch, NULL, 0);
	CRefStrW rs(cchBuf);
	MultiByteToWideChar(uCP, MB_PRECOMPOSED, pszText, cch, rs.Data(), cchBuf);
	*(rs.Data() + cchBuf) = '\0';
	return rs;
}
ECK_NAMESPACE_END