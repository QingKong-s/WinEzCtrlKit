#pragma once
#include "CRefBin.h"
#include "CRefStr.h"
#include "Random.h"

#include <bcrypt.h>

ECK_NAMESPACE_BEGIN
#pragma region CRefBinT运算符
template<class TAlloc, class TChar, class TTraits, class TAlloc1>
EckInline CRefBinT<TAlloc>& operator<<(CRefBinT<TAlloc>& rb, const CRefStrT<TChar, TTraits, TAlloc1>& rs)
{
    rb.PushBack(rs.Data(), rs.ByteSize());
    return rb;
}
template<class TAlloc, class TAlloc1>
EckInline CRefBinT<TAlloc>& operator<<(CRefBinT<TAlloc>& rb, const CRefBinT<TAlloc1>& rb1)
{
    rb.PushBack(rb1.Data(), rb1.Size());
    return rb;
}
template<class TAlloc, class T>
EckInline CRefBinT<TAlloc>& operator<<(CRefBinT<TAlloc>& rb, const T& t)
{
    rb.PushBack(&t, sizeof(T));
    return rb;
}
#pragma endregion CRefBinT运算符

#pragma region 资源
inline std::span<const BYTE> GetResource(PCWSTR pszName, PCWSTR pszType,
    HMODULE hModule = nullptr)
{
    const auto hRes = FindResourceW(hModule, pszName, pszType);
    if (!hRes)
        return {};
    const auto hGlobal = LoadResource(hModule, hRes);
    if (!hGlobal)
        return {};
    const auto pRes = LockResource(hGlobal);
    if (!pRes)
        return {};
    const auto cbRes = SizeofResource(hModule, hRes);
    if (!cbRes)
        return {};
    return { (PCBYTE)pRes,cbRes };
}

inline std::wstring_view GetResourceString(WORD wID, WORD wLangID,
    HMODULE hModule = nullptr)
{
    const auto hRes = FindResourceExW(hModule, RT_STRING,
        MAKEINTRESOURCEW(1 + wID / 16), wLangID);
    if (!hRes)
        return {};
    const auto hgStr = LoadResource(hModule, hRes);
    if (!hgStr)
        return {};
    auto pszBegin = (PCWSTR)LockResource(hgStr);
    for (size_t i = 0; i < size_t(wID % 16); ++i)
        pszBegin += (*pszBegin + 1);
    const int cch = *pszBegin++;
    return { pszBegin,(size_t)cch };
}

EckInline std::wstring_view GetResourceStringForCurrentLocale(
    WORD wID, HMODULE hModule = nullptr)
{
    return GetResourceString(wID, LANGIDFROMLCID(GetThreadLocale()), hModule);
}
#pragma endregion 资源

#pragma region Utf8转换
template<class TCharTraits, class TAlloc>
void StrW2U8(PCWSTR pszText, int cch, CRefStrT<CHAR, TCharTraits, TAlloc>& rsResult)
{
    int cchBuf = WideCharToMultiByte(CP_UTF8, 0, pszText, cch, nullptr, 0, nullptr, nullptr);
    if (!cchBuf)
        return;
    if (cch == -1)
        --cchBuf;
    WideCharToMultiByte(CP_UTF8, 0, pszText, cch,
        rsResult.PushBackNoExtra(cchBuf), cchBuf, nullptr, nullptr);
}
template<class TCharTraits = CCharTraits<CHAR>, class TAlloc = TRefStrDefAlloc<CHAR>>
[[nodiscard]] auto StrW2U8(PCWSTR pszText, int cch = -1)
{
    CRefStrT<CHAR, TCharTraits, TAlloc> rs;
    StrW2U8(pszText, cch, rs);
    return rs;
}

template<class TAlloc>
void StrW2U8(PCWSTR pszText, int cch, CRefBinT<TAlloc>& rbResult)
{
    int cchBuf = WideCharToMultiByte(CP_UTF8, 0, pszText, cch, nullptr, 0, nullptr, nullptr);
    if (!cchBuf)
        return;
    if (cch == -1)
        --cchBuf;
    const auto pszBuf = (PSTR)rbResult.PushBackNoExtra(cchBuf + 1);
    WideCharToMultiByte(CP_UTF8, 0, pszText, cch, pszBuf, cchBuf, nullptr, nullptr);
    *(pszBuf + cchBuf) = '\0';
}

template<class TCharTraits = CCharTraits<CHAR>, class TAlloc = TRefStrDefAlloc<CHAR>,
    class T, class U>
[[nodiscard]] EckInline CRefStrT<CHAR, TCharTraits, TAlloc> StrW2U8(
    const CRefStrT<WCHAR, T, U>& rs)
{
    return StrW2U8<TCharTraits, TAlloc>(rs.Data(), rs.Size());
}

template<class TCharTraits = CCharTraits<WCHAR>, class TAlloc = TRefStrDefAlloc<WCHAR>>
[[nodiscard]] CRefStrT<WCHAR, TCharTraits, TAlloc> StrU82W(PCSTR pszText, int cch = -1)
{
    int cchBuf = MultiByteToWideChar(CP_UTF8, 0, pszText, cch, nullptr, 0);
    if (!cchBuf)
        return {};
    if (cch == -1)
        --cchBuf;
    CRefStrT<WCHAR, TCharTraits, TAlloc> rs(cchBuf);
    MultiByteToWideChar(CP_UTF8, 0, pszText, cch, rs.Data(), cchBuf);
    return rs;
}

template<class TCharTraits, class TAlloc>
void StrU82W(PCSTR pszText, int cch, CRefStrT<WCHAR, TCharTraits, TAlloc>& rsResult)
{
    int cchBuf = MultiByteToWideChar(CP_UTF8, 0, pszText, cch, nullptr, 0);
    if (!cchBuf)
        return;
    if (cch == -1)
        --cchBuf;
    MultiByteToWideChar(CP_UTF8, 0, pszText, cch,
        rsResult.PushBackNoExtra(cchBuf), cchBuf);
}

template<class TAlloc>
void StrU82W(PCSTR pszText, int cch, CRefBinT<TAlloc>& rbResult)
{
    int cchBuf = MultiByteToWideChar(CP_UTF8, 0, pszText, cch, nullptr, 0);
    if (!cchBuf)
        return;
    if (cch == -1)
        --cchBuf;
    const auto pszBuf = (PWSTR)rbResult.PushBackNoExtra((cchBuf + 1) * sizeof(WCHAR));
    MultiByteToWideChar(CP_UTF8, 0, pszText, cch, pszBuf, cchBuf);
    *(pszBuf + cchBuf) = L'\0';
}

template<class TCharTraits = CCharTraits<WCHAR>, class TAlloc = TRefStrDefAlloc<WCHAR>,
    class T, class U>
[[nodiscard]] EckInline CRefStrT<WCHAR, TCharTraits, TAlloc> StrU82W(
    const CRefStrT<CHAR, T, U>& rs)
{
    return StrU82W<TCharTraits, TAlloc>(rs.Data(), rs.Size());
}

template<class TCharTraits = CCharTraits<WCHAR>, class TAlloc = TRefStrDefAlloc<WCHAR>, class T>
[[nodiscard]] EckInline CRefStrT<WCHAR, TCharTraits, TAlloc> StrU82W(const CRefBinT<T>& rb)
{
    return StrU82W<TCharTraits, TAlloc>((PCSTR)rb.Data(), (int)rb.Size());
}

inline NTSTATUS CalcMd5(_In_reads_bytes_(cbData) PCVOID pData,
    SIZE_T cbData, _Out_writes_bytes_all_(16) void* pResult)
{
    NTSTATUS nts;
    BCRYPT_ALG_HANDLE hAlg;
    DWORD cbHashObject;
    ULONG cbRet;
    BCRYPT_HASH_HANDLE hHash{};
    UCHAR* pHashObject{};
    if (!NT_SUCCESS(nts = BCryptOpenAlgorithmProvider(&hAlg, BCRYPT_MD5_ALGORITHM, nullptr, 0)))
        return nts;
    if (!NT_SUCCESS(nts = BCryptGetProperty(hAlg, BCRYPT_OBJECT_LENGTH, (BYTE*)&cbHashObject,
        sizeof(DWORD), &cbRet, 0)))
        goto TidyUp;
    pHashObject = (UCHAR*)_malloca(cbHashObject);
    if (!NT_SUCCESS(nts = BCryptCreateHash(hAlg, &hHash, pHashObject, cbHashObject,
        nullptr, 0, 0)))
        goto TidyUp;
    if (!NT_SUCCESS(nts = BCryptHashData(hHash, (BYTE*)pData, (ULONG)cbData, 0)))
        goto TidyUp;
    if (!NT_SUCCESS(nts = BCryptFinishHash(hHash, (UCHAR*)pResult, 16, 0)))
        goto TidyUp;
TidyUp:
    if (hHash)
        BCryptDestroyHash(hHash);
    BCryptCloseAlgorithmProvider(hAlg, 0);
    if (pHashObject)
        _freea(pHashObject);
    return nts;
}
#pragma endregion Utf8转换

#pragma region 包装器
EckInline void FreeSTRRET(const STRRET& strret)
{
    if (strret.uType == STRRET_WSTR)
        CoTaskMemFree(strret.pOleStr);
}

EckInline const auto* UserSharedData() { return USER_SHARED_DATA; }

EckInline HANDLE DuplicateStdThreadHandle(std::thread& thr, NTSTATUS* pnts = nullptr)
{
    HANDLE hThread{};
    const auto nts = NtDuplicateObject(NtCurrentProcess(), thr.native_handle(),
        NtCurrentProcess(), &hThread,
        0, 0, DUPLICATE_SAME_ATTRIBUTES | DUPLICATE_SAME_ACCESS);
    if (pnts) *pnts = nts;
    return hThread;
}

EckInline NTSTATUS WaitObject(HANDLE hObject, LONGLONG llMilliseconds)
{
    return NtWaitForSingleObject(hObject, FALSE, (LARGE_INTEGER*)&llMilliseconds);
}

EckInline NTSTATUS WaitObject(HANDLE hObject, LARGE_INTEGER* pliMilliseconds = nullptr)
{
    return NtWaitForSingleObject(hObject, FALSE, pliMilliseconds);
}

EckInline int GetKeyNameTextByVk(WORD wVk, PWSTR pszBuf, int cchBuf,
    BOOL bExtended = FALSE, BOOL bDontCare = FALSE)
{
    return GetKeyNameTextW((MapVirtualKeyW(wVk, MAPVK_VK_TO_VSC) << 16) |
        ((!!bExtended) << 24) |
        ((!!bDontCare) << 25),
        pszBuf, cchBuf);
}

inline BOOL GetUserLocaleName(CRefStrW& rsLocaleName)
{
    WCHAR sz[LOCALE_NAME_MAX_LENGTH];
    const auto r = GetUserDefaultLocaleName(sz, ARRAYSIZE(sz));
    if (r <= 1)
        return FALSE;
    rsLocaleName.ReSize(r - 1);
    TcsCopyLenEnd(rsLocaleName.Data(), sz, r - 1);
    return TRUE;
}
#pragma endregion 包装器

#pragma region 其他
inline void RandomBytes(_Out_writes_bytes_all_(cb) void* p, size_t cb)
{
    CPcg32 Pcg{};
    for (BYTE* pb = (BYTE*)p, *pbEnd = pb + cb; pb < pbEnd; ++pb)
        *pb = Pcg.Next<BYTE>();
}
inline void RandomBytes(CRefBin& rb)
{
    RandomBytes(rb.Data(), rb.Size());
}
template<size_t N>
inline void RandomBytes(_Out_ BYTE(&arr)[N])
{
    RandomBytes(arr, N);
}

// iType  0 - 空格分割的十六进制  1 - 易语言字节集调试输出
inline CRefStrW FormatBin(PCVOID pData_, size_t cb, int iType)
{
    const auto pData = (PCBYTE)pData_;
    CRefStrW rsResult{};
    if (!pData || !cb)
    {
        if (iType == 1)
            rsResult.DupString(EckStrAndLen(L"{ }"));
        return rsResult;
    }

    switch (iType)
    {
    case 0:
    {
        rsResult.Reserve((int)cb * 3 + 10);
        for (SIZE_T i = 0u; i < cb; ++i)
            rsResult.AppendFormat(L"%02hhX ", pData[i]);
    }
    break;
    case 1:
    {
        rsResult.Reserve((int)cb * 4 + 10);
        rsResult.PushBack(EckStrAndLen(L"{ "));
        rsResult.AppendFormat(L"%hhu", pData[0]);
        for (SIZE_T i = 1u; i < cb; ++i)
            rsResult.AppendFormat(L",%hhu", pData[i]);
        rsResult.PushBack(EckStrAndLen(L" }"));
    }
    break;
    }
    return rsResult;
}
#pragma endregion 其他

// 弃用
EckInline BOOL IsFILETIMEZero(const FILETIME& ft)
{
    return ft.dwLowDateTime == 0 && ft.dwHighDateTime == 0;
}
EckInline bool operator==(const FILETIME& ft1, const FILETIME& ft2)
{
    return CompareFileTime(&ft1, &ft2) == 0;
}
EckInline bool operator>(const FILETIME& ft1, const FILETIME& ft2)
{
    return CompareFileTime(&ft1, &ft2) == 1;
}
EckInline bool operator<(const FILETIME& ft1, const FILETIME& ft2)
{
    return CompareFileTime(&ft1, &ft2) == -1;
}
ECK_NAMESPACE_END