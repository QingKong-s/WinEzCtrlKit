/*
* WinEzCtrlKit Library
*
* DbgHelper.h ： 调试帮助函数
*
* Copyright(C) 2023 QingKong
*/
#pragma once
#include "ECK.h"

#include <string>
#include <stdio.h>

ECK_NAMESPACE_BEGIN
#ifndef NDEBUG
void Assert(PCWSTR pszMsg, PCWSTR pszFile, PCWSTR pszLine);

#pragma warning (push)
#pragma warning (disable:6053)// 对“_snwprintf”的前一调用可能没有为字符串“buf”添加字符串零终止符
#pragma warning (disable:4996)// 函数不安全

inline void DbgPrint(int i, BOOL bHex = FALSE, BOOL bNewLine = TRUE)
{
    WCHAR buf[CchI32ToStrBufNoRadix2];
    const PCWSTR pszFmt = (bHex ? L"0x%08X" : L"%d");
    swprintf_s(buf, pszFmt, i);
    OutputDebugStringW(buf);
    if (bNewLine)
        OutputDebugStringW(L"\n");
}

inline void DbgPrint(DWORD i, BOOL bHex = FALSE, BOOL bNewLine = TRUE)
{
    WCHAR buf[CchI32ToStrBufNoRadix2];
    const PCWSTR pszFmt = (bHex ? L"0x%08X" : L"%lu");
    swprintf_s(buf, pszFmt, i);
    OutputDebugStringW(buf);
    if (bNewLine)
        OutputDebugStringW(L"\n");
}

inline void DbgPrint(LONGLONG i, BOOL bHex = FALSE, BOOL bNewLine = TRUE)
{
    WCHAR buf[CchI64ToStrBufNoRadix2];
    const PCWSTR pszFmt = (bHex ? L"0x%016I64X" : L"%I64d");
    swprintf_s(buf, pszFmt, i);
    OutputDebugStringW(buf);
    if (bNewLine)
        OutputDebugStringW(L"\n");
}

inline void DbgPrint(ULONGLONG i, BOOL bHex = FALSE, BOOL bNewLine = TRUE)
{
    WCHAR buf[CchI64ToStrBufNoRadix2];
    const PCWSTR pszFmt = (bHex ? L"0x%016I64X" : L"%I64u");
    swprintf_s(buf, pszFmt, i);
    OutputDebugStringW(buf);
    if (bNewLine)
        OutputDebugStringW(L"\n");
}

inline void DbgPrint(double i, BOOL bNewLine = TRUE)
{
    WCHAR buf[64];
    swprintf_s(buf, L"%lf", i);
    OutputDebugStringW(buf);
    if (bNewLine)
        OutputDebugStringW(L"\n");
}

inline void DbgPrint(float i, BOOL bNewLine = TRUE)
{
    WCHAR buf[64];
    swprintf_s(buf, L"%f", i);
    OutputDebugStringW(buf);
    if (bNewLine)
        OutputDebugStringW(L"\n");
}

EckInline void DbgPrint(long i, BOOL bHex = FALSE, BOOL bNewLine = TRUE)
{
    DbgPrint((int)i, bHex, bNewLine);
}

EckInline void DbgPrint(UINT i, BOOL bHex = FALSE, BOOL bNewLine = TRUE)
{
    DbgPrint((DWORD)i, bHex, bNewLine);
}

EckInline void DbgPrint(SCHAR i, BOOL bHex = FALSE, BOOL bNewLine = TRUE)
{
    DbgPrint((int)i, bHex, bNewLine);
}

EckInline void DbgPrint(UCHAR i, BOOL bHex = FALSE, BOOL bNewLine = TRUE)
{
    DbgPrint((DWORD)i, bHex, bNewLine);
}

EckInline void DbgPrint(short i, BOOL bHex = FALSE, BOOL bNewLine = TRUE)
{
    DbgPrint((int)i, bHex, bNewLine);
}

EckInline void DbgPrint(USHORT i, BOOL bHex = FALSE, BOOL bNewLine = TRUE)
{
    DbgPrint((DWORD)i, bHex, bNewLine);
}

EckInline void DbgPrint(PCVOID i, BOOL bNewLine = TRUE)
{
    DbgPrint((ULONG_PTR)i, TRUE, bNewLine);
}

EckInline void DbgPrint(PCWSTR psz, BOOL bNewLine = TRUE)
{
    OutputDebugStringW(psz);
    if (bNewLine)
        OutputDebugStringW(L"\n");
}

EckInline void DbgPrint(PCSTR psz, BOOL bNewLine = TRUE)
{
    OutputDebugStringA(psz);
    if (bNewLine)
        OutputDebugStringA("\n");
}

template<class T, class U, class V>
EckInline void DbgPrint(const std::basic_string<T, U, V>& str, BOOL bNewLine = TRUE)
{
	DbgPrint(str.c_str(), bNewLine);
}

inline void DbgPrintFormatMessage(UINT uErrCode, BOOL bNewLine = TRUE)
{
    PWSTR pszInfo;
    FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, nullptr,
        uErrCode, 0, (PWSTR)&pszInfo, 0, nullptr);
    DbgPrint(pszInfo, bNewLine);
    LocalFree(pszInfo);
}

EckInline void DbgPrintLastError(BOOL bHex = FALSE, BOOL bNewLine = TRUE)
{
    const auto u = GetLastError();
    DbgPrint(u, bHex, TRUE);
    DbgPrintFormatMessage(u, bNewLine);
}

void DbgPrintWithPos(PCWSTR pszFile, PCWSTR pszFunc, int iLine, PCWSTR pszMsg);

void DbgPrintWndMap();

void DbgPrintFmt(_Printf_format_string_ PCWSTR pszFormat, ...);

#define EckDbgPrintGLE              ::eck::DbgPrintLastError
#define EckDbgPrint                 ::eck::DbgPrint
#define EckDbgPrintFormatMessage    ::eck::DbgPrintFormatMessage
#define EckDbgPrintFmt              ::eck::DbgPrintFmt
#define EckDbgPrintWithPos(x)       ::eck::DbgPrintWithPos(ECK_FILEW, ECK_FUNCTIONW, __LINE__, x)
#define EckDbgBreak()               DebugBreak()
#define EckDbgCheckMemRange(pBase, cbSize, pCurr)   \
    if(((PCBYTE)(pBase)) + (cbSize) < (pCurr))      \
    {                                               \
        EckDbgPrintFmt(                             \
            L"内存范围检查失败，起始 = %p，尺寸 = %u，当前 = %p，超出 = %u", \
            pBase,                                  \
            (UINT)cbSize,                           \
            pCurr,                                  \
            (UINT)(((SIZE_T)(pCurr)) - ((SIZE_T)(pBase)) - (cbSize))); \
        EckDbgBreak();                              \
    }
#define EckAssert(x)                (void)(!!(x) || (::eck::Assert(ECKWIDE(#x), ECK_FILEW, ECK_LINEW), 0))
#define EckDbgPrintWndMap()         ::eck::DbgPrintWndMap()
#pragma warning (pop)
#else
#define EckDbgPrintGLE(x)           ;
#define EckDbgPrint(...)            ;
#define EckDbgPrintFormatMessage(x) ;
#define EckDbgPrintFmt(...)         ;
#define EckDbgPrintWithPos(x)       ;
#define EckDbgBreak()               ;
#define EckDbgCheckMemRange(a,b,c)  ;
#define EckAssert(x)                ;
#define EckDbgPrintWndMap()         ;
#endif // !NDEBUG
ECK_NAMESPACE_END