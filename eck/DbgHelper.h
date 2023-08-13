/*
* WinEzCtrlKit Library
*
* DbgHelper.h �� ���԰�������
*
* Copyright(C) 2023 QingKong
*/
#pragma once
#include "ECK.h"

#include <stdio.h>

#include <strsafe.h>


#define ECKWIDE2___(x)          L##x
// ANSI�ַ��������ַ���
#define ECKWIDE___(x)           ECKWIDE2___(x)

#define ECKTOSTR2___(x)         #x
// ��ANSI�ַ���
#define ECKTOSTR___(x)          ECKTOSTR2___(x)
// �����ַ���
#define ECKTOSTRW___(x)         ECKWIDE___(ECKTOSTR2___(x))

// [Ԥ����] ��ǰ������W
#define ECK_FUNCTIONW           ECKWIDE___(__FUNCTION__)
// [Ԥ����] �к�W
#define ECK_LINEW               ECKTOSTRW___(__LINE__)
// [Ԥ����] ��ǰ�ļ�W
#define ECK_FILEW               __FILEW__

ECK_NAMESPACE_BEGIN
#ifndef NDEBUG

#pragma warning (push)
#pragma warning (disable:6053)// �ԡ�_snwprintf����ǰһ���ÿ���û��Ϊ�ַ�����buf������ַ�������ֹ��
#pragma warning (disable:4996)// ��������ȫ

#define ECK_BS_DBGVAL           64
#define ECK_BS_DBGVALMAXCH      (ECK_BS_DBGVAL - 1)

/// <summary>
/// �������á�
/// ���������ֵ��
/// </summary>
/// <param name="i">��ֵ</param>
/// <param name="bHex">�Ƿ�ʮ������</param>
/// <param name="bNewLine">�Ƿ���ĩβ����</param>
EckInline void DbgPrint(int i, BOOL bHex = FALSE, BOOL bNewLine = TRUE)
{
    WCHAR buf[ECK_BS_DBGVAL];
    PCWSTR pszFmt = (bHex ? L"0x%08X" : L"%d");
    StringCchPrintfW(buf, ECK_BS_DBGVALMAXCH, pszFmt, i);
    OutputDebugStringW(buf);
    if (bNewLine)
        OutputDebugStringW(L"\n");
}

EckInline void DbgPrint(DWORD i, BOOL bHex = FALSE, BOOL bNewLine = TRUE)
{
    WCHAR buf[ECK_BS_DBGVAL];
    PCWSTR pszFmt = (bHex ? L"0x%08X" : L"%lu");
    StringCchPrintfW(buf, ECK_BS_DBGVALMAXCH, pszFmt, i);
    OutputDebugStringW(buf);
    if (bNewLine)
        OutputDebugStringW(L"\n");
}

EckInline void DbgPrint(LONGLONG i, BOOL bHex = FALSE, BOOL bNewLine = TRUE)
{
    WCHAR buf[ECK_BS_DBGVAL];
    PCWSTR pszFmt = (bHex ? L"0x%016I64X" : L"%I64d");
    StringCchPrintfW(buf, ECK_BS_DBGVALMAXCH, pszFmt, i);
    OutputDebugStringW(buf);
    if (bNewLine)
        OutputDebugStringW(L"\n");
}

EckInline void DbgPrint(ULONGLONG i, BOOL bHex = FALSE, BOOL bNewLine = TRUE)
{
    WCHAR buf[ECK_BS_DBGVAL];
    PCWSTR pszFmt = (bHex ? L"0x%016I64X" : L"%I64u");
    StringCchPrintfW(buf, ECK_BS_DBGVALMAXCH, pszFmt, i);
    OutputDebugStringW(buf);
    if (bNewLine)
        OutputDebugStringW(L"\n");
}

EckInline void DbgPrint(double i, BOOL bNewLine = TRUE)
{
    WCHAR buf[ECK_BS_DBGVAL];
    _snwprintf(buf, ECK_BS_DBGVALMAXCH, L"%lf", i);
    OutputDebugStringW(buf);
    if (bNewLine)
        OutputDebugStringW(L"\n");
}

EckInline void DbgPrint(float i, BOOL bNewLine = TRUE)
{
    WCHAR buf[ECK_BS_DBGVAL];
    _snwprintf(buf, ECK_BS_DBGVALMAXCH, L"%f", i);
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

EckInline void DbgPrint(void* i, BOOL bNewLine = TRUE)
{
    DbgPrint((ULONG_PTR)i, TRUE, bNewLine);
}

/// <summary>
/// �������á�
/// ����ַ���
/// </summary>
/// <param name="psz">�ַ���</param>
/// <param name="bNewLine">�Ƿ���ĩβ����</param>
EckInline void DbgPrint(PCWSTR psz, BOOL bNewLine = TRUE)
{
    PWSTR pszBuf = new WCHAR[lstrlenW(psz) + 2];
    wsprintfW(pszBuf, L"%s", psz);
    OutputDebugStringW(pszBuf);
    if (bNewLine)
        OutputDebugStringW(L"\n");
    delete[] pszBuf;
}

/// <summary>
/// �������á�
/// ���������
/// </summary>
/// <param name="bHex">�Ƿ�ʮ������</param>
/// <param name="bNewLine">�Ƿ���ĩβ����</param>
EckInline void DbgPrintLastError(BOOL bHex = FALSE, BOOL bNewLine = TRUE)
{
    DbgPrint(GetLastError(), bHex, bNewLine);
}

/// <summary>
/// �������á�
/// ���������ĸ�ʽ����Ϣ
/// </summary>
/// <param name="uErrCode">������</param>
/// <param name="bNewLine">�Ƿ���ĩβ����</param>
EckInline void DbgPrintFormatMessage(UINT uErrCode, BOOL bNewLine = TRUE)
{
    PWSTR pszInfo;
    FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER, NULL, uErrCode, 0, (PWSTR)&pszInfo, 20, NULL);
    DbgPrint(pszInfo, bNewLine);
    LocalFree(pszInfo);
}

EckInline void DbgPrintFmt(PCWSTR pszFormat, ...)
{
#define ECK_BS_PRINTFMT 1024
    WCHAR sz[ECK_BS_PRINTFMT];
    va_list Args;
    va_start(Args, pszFormat);
    vswprintf(sz, ECK_BS_PRINTFMT - 1, pszFormat, Args);
    OutputDebugStringW(sz);
    OutputDebugStringW(L"\n");
    va_end(Args);
#undef ECK_BS_PRINTFMT
}

#undef ECK_BS_DBGVAL
#undef ECK_BS_DBGVALMAXCH

#define EckDbgPrintGLE eck::DbgPrintLastError
#define EckDbgPrint eck::DbgPrint
#define EckDbgPrintFormatMessage eck::DbgPrintFormatMessage
#define EckDbgPrintFmt eck::DbgPrintFmt
#define EckDbgPrintWithPos(x) \
    EckDbgPrint(L"�����������" ECK_FILEW L" �� " ECK_FUNCTIONW L" ���� (" ECK_LINEW L"��)  ��Ϣ��\n" x) \


#define EckDbgBreak() DebugBreak()

#pragma warning (pop)
#else
#define EckDbgPrintGLE(x)
#define EckDbgPrint(x)
#define EckDbgPrintFormatMessage(x)
#define EckDbgPrintFmt(...)
#define EckDbgPrintWithPos(x)

#define EckDbgBreak()
#endif // !NDEBUG
ECK_NAMESPACE_END