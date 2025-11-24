#pragma once
#include "StringUtility.h"

ECK_NAMESPACE_BEGIN
inline constexpr std::wstring_view
IllegalPathCharW{ LR"(\/:*?"<>|)" },
IllegalPathCharWithDotW{ LR"(\/:*?"<>|.)" };
inline constexpr std::string_view
IllegalPathCharA{ R"(\/:*?"<>|)" },
IllegalPathCharWithDotA{ R"(\/:*?"<>|.)" };

template<CcpStdCharPtr TPointer>
void PazLegalize(_In_z_ TPointer pszPath,
    RemoveStdCharPtr_T<TPointer> chReplace = '_', BOOL bReplaceDot = FALSE)
{
    if constexpr (std::is_same_v<RemoveStdCharPtr_T<TPointer>, char>)
    {
        const auto IllegalChars{ bReplaceDot ? IllegalPathCharA : IllegalPathCharWithDotA };
        auto p{ pszPath };
        while (p = TcsChrFirstOf(p, IllegalChars.data()))
            *p++ = chReplace;
    }
    else
    {
        const auto IllegalChars{ bReplaceDot ? IllegalPathCharW : IllegalPathCharWithDotW };
        auto p{ pszPath };
        while (p = TcsChrFirstOf(p, IllegalChars.data()))
            *p++ = chReplace;
    }
}
template<CcpStdCharPtr TPointer>
void PazLegalizeLen(_In_reads_(cchPath) TPointer pszPath, int cchPath,
    RemoveStdCharPtr_T<TPointer> chReplace = '_', BOOL bReplaceDot = FALSE)
{
    if constexpr (std::is_same_v<RemoveStdCharPtr_T<TPointer>, char>)
    {
        const auto IllegalChars{ bReplaceDot ? IllegalPathCharA : IllegalPathCharWithDotA };
        const auto pEnd = pszPath + cchPath;
        auto p{ pszPath };
        while (p = TcsChrFirstOf(p, pEnd - p, IllegalChars.data(), IllegalChars.size()))
            *p++ = chReplace;
    }
    else
    {
        const auto IllegalChars{ bReplaceDot ? IllegalPathCharW : IllegalPathCharWithDotW };
        const auto pEnd = pszPath + cchPath;
        auto p{ pszPath };
        while (p = TcsChrFirstOf(p, pEnd - p, IllegalChars.data(), IllegalChars.size()))
            *p++ = chReplace;
    }
}

template<CcpStdCharPtr TPointer>
HRESULT PazParseCommandLine(
    _In_reads_(cchCmdLine) TPointer pszCmdLine, int cchCmdLine,
    _Out_ TPointer& pszFile, _Out_ int& cchFile,
    _Out_ TPointer& pszParam, _Out_ int& cchParam)
{
    if (!pszCmdLine || !cchCmdLine)
    {
        pszFile = pszParam = nullptr;
        cchFile = cchParam = 0;
        return S_FALSE;
    }
    pszFile = pszCmdLine;
    BOOL bQuote = (*pszCmdLine == '\"');
    if (bQuote)
        ++pszFile;
    const auto pEnd = pszCmdLine + cchCmdLine;
    if (bQuote)
    {
        for (auto p = pszFile; p != pEnd; ++p)
            if (*p == '\"')
            {
                cchFile = int(p - pszFile);
                goto FileNameOk;
            }
        cchFile = 0;// 引号不匹配
        return HRESULT_FROM_WIN32(ERROR_INVALID_COMMAND_LINE);
    }
    else
    {
        for (auto p = pszFile; p != pEnd; ++p)
            if (*p == ' ')
            {
                cchFile = int(p - pszFile);
                goto FileNameOk;
            }
        cchFile = cchCmdLine;
        pszParam = nullptr;
        cchParam = 0;
        return S_OK;
    }
FileNameOk:;// 至此文件名处理完毕
    // 步进到第一个非空格字符
    pszParam = pszFile + cchFile;
    if (*pszParam == '\"')
        ++pszParam;
    for (; pszParam != pEnd; ++pszParam)
        if (*pszParam != ' ')
            break;
    cchParam = int(pEnd - pszParam);
    return S_OK;
}
template<CcpStdCharPtr TPointer>
HRESULT PazParseCommandLineAndCut(
    _In_reads_(cchCmdLine) TPointer pszCmdLine, int cchCmdLine,
    _Out_ TPointer& pszFile, _Out_ int& cchFile,
    _Out_ TPointer& pszParam, _Out_ int& cchParam)
{
    EckAssert(&pszFile != &pszParam && &cchFile != &cchParam);
    const auto hr = PazParseCommandLine(pszCmdLine, cchCmdLine,
        pszFile, cchFile, pszParam, cchParam);
    if (SUCCEEDED(hr))
    {
        if (pszFile)
            *(pszFile + cchFile) = '\0';
        if (pszParam)
            *(pszParam + cchParam) = '\0';
    }
    return hr;
}

EckNfInlineNd BOOL PazIsDotFileName(_In_reads_z_(3) CcpStdCharPtr auto pszFileName)
{
    return pszFileName[0] == '.' && (pszFileName[1] == '\0' ||
        (pszFileName[1] == '.' && pszFileName[2] == '\0'));
}

EckNfInlineNd BOOL PazIsDotFileName(_In_reads_bytes_(cbFileName) PCWCH pszFileName, ULONG cbFileName)
{
    return (cbFileName == sizeof(WCHAR) && pszFileName[0] == L'.') ||
        (cbFileName == sizeof(WCHAR) * 2 && pszFileName[0] == L'.' && pszFileName[1] == L'.');
}
ECK_NAMESPACE_END