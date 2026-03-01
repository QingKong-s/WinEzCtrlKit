#pragma once
#include "FileHelper.h"
#include "LocaleStringUtility.h"
#include "../ThirdPartyLib/UCharDet/uchardet.h"

ECK_NAMESPACE_BEGIN
namespace Priv
{
    struct ECD_STRING
    {
        std::string_view svName;
        UINT uCodePage;
    };
    constexpr inline ECD_STRING EcdEncodingMap[]
    {
        { "UTF-8"sv,        65001 },
        { "UTF-16LE"sv,     1200  }, // 注意，uchardet不区分le和be
        { "UTF-16BE"sv,     1201  },
        { "UTF-32LE"sv,     12000 },
        { "UTF-32BE"sv,     12001 },

        { "ASCII"sv,        20127 }, // us-ascii
        { "BIG5"sv,         950   },
        { "EUC-JP"sv,       51932 },
        { "EUC-KR"sv,       51949 },
        { "EUC-TW"sv,       51950 }, // EUC Traditional Chinese
        { "GB18030"sv,      54936 }, // 注意不会输出GB2312，以GB18030代替
        { "HZ-GB-2312"sv,   52936 },
        { "IBM852"sv,       852   }, // OEM Latin 2
        { "IBM855"sv,       855   }, // OEM Cyrillic
        { "IBM865"sv,       865   }, // OEM Nordic
        { "IBM866"sv,       866   }, // OEM Russian
        { "ISO-2022-CN"sv,  50227 }, // x-cp50227
        { "ISO-2022-JP"sv,  50220 },
        { "ISO-2022-KR"sv,  50225 },
        { "ISO-8859-1"sv,   28591 }, // Latin-1, Western Europe
        { "ISO-8859-2"sv,   28592 }, // Latin-2, Central Europe
        { "ISO-8859-3"sv,   28593 }, // Latin-3, South Europe
        { "ISO-8859-4"sv,   28594 }, // Latin-4, North Europe
        { "ISO-8859-5"sv,   28595 }, // Cyrillic
        { "ISO-8859-6"sv,   28596 }, // Arabic
        { "ISO-8859-7"sv,   28597 }, // Greek
        { "ISO-8859-8"sv,   28598 }, // Hebrew
        { "ISO-8859-9"sv,   28599 }, // Latin-5, Turkish
        { "ISO-8859-10"sv,  28600 }, // Latin-6, Nordic
        { "ISO-8859-11"sv,  28601 }, // Thai, 兼容TIS-620
        { "ISO-8859-13"sv,  28603 }, // Latin-7, Baltic Rim
        { "ISO-8859-15"sv,  28605 }, // Latin-9, Euro, Latin-1的修订版
        { "ISO-8859-16"sv,  28606 }, // Latin-10, South-Eastern European
        { "KOI8-R"sv,       20866 },
        { "MAC-CENTRALEUROPE"sv, 10029 }, // x-mac-ce
        { "MAC-CYRILLIC"sv, 10007 }, // x-mac-cyrillic
        { "SHIFT_JIS"sv,    932   },
        { "TIS-620"sv,      874   }, // Windows-874, Thai
        { "WINDOWS-1250"sv, 1250  }, // Central Europe
        { "WINDOWS-1251"sv, 1251  }, // Cyrillic
        { "WINDOWS-1252"sv, 1252  }, // Western Europe
        { "WINDOWS-1253"sv, 1253  }, // Greek
        { "WINDOWS-1255"sv, 1255  }, // Hebrew
        { "WINDOWS-1256"sv, 1256  }, // Arabic
        { "WINDOWS-1257"sv, 1257  }, // Baltic
        { "WINDOWS-1258"sv, 1258  }, // Vietnamese
        //{ "VISCII"sv,         0   }, // 不支持
    };
}

// 代码页名称转值，失败返回0
inline UINT EcdNameToCodePage(std::string_view svName) noexcept
{
    for (const auto& e : Priv::EcdEncodingMap)
    {
        if (TcsEqualLength2I(svName.data(), svName.size(),
            e.svName.data(), e.svName.size()))
            return e.uCodePage;
    }
    return 0u;
}

enum : UINT
{
    // 使用advapi32!IsTextUnicode细化UTF16的分析
    ECD_USE_IS_TEXT_UNICODE = 1u << 0,
    // 禁用TIS-620检测，使其映射为UTF-8
    ECD_DISABLE_TIS_620 = 1u << 1,
};

// 检测字节流编码，成功返回代码页，失败返回0
inline UINT EcdDetectCodePage(
    _In_reads_bytes_(cb) PCVOID p, size_t cb,
    UINT uFlags = ECD_DISABLE_TIS_620) noexcept
{
    if (cb < 4)
        return 0u;
#undef ECK_TEMP_HIT
#define ECK_TEMP_HIT(Bom) memcmp(p, Bom, sizeof(Bom)) == 0

    /* */if (ECK_TEMP_HIT(BOM_UTF32LE))
        return CP_UTF32LE;
    else if (ECK_TEMP_HIT(BOM_UTF32BE))
        return CP_UTF32BE;
    else if (ECK_TEMP_HIT(BOM_UTF16LE))
        return CP_UTF16LE;
    else if (ECK_TEMP_HIT(BOM_UTF16BE))
        return CP_UTF16BE;
    else if (ECK_TEMP_HIT(BOM_UTF8))
        return CP_UTF8;

#undef ECK_TEMP_HIT

    if (uFlags & ECD_USE_IS_TEXT_UNICODE)
    {
        int i = IS_TEXT_UNICODE_REVERSE_STATISTICS;
        if (IsTextUnicode(p, (int)cb, &i))
            return CP_UTF16BE;
        i = IS_TEXT_UNICODE_STATISTICS;
        if (IsTextUnicode(p, (int)cb, &i))
            return CP_UTF16LE;
    }

    const auto pUcd = uchardet_new();
    uchardet_handle_data(pUcd, (PCCH)p, cb);
    uchardet_data_end(pUcd);
    const auto pszName = uchardet_get_charset(pUcd);
    UINT cp;
    if ((uFlags & ECD_DISABLE_TIS_620) &&
        TcsEqualI(pszName, "TIS-620"))
    {
        BOOL bValid;
        (void)LcsUtf8CountChar((PCSTR)p, cb, bValid);
        if (bValid)
            cp = CP_UTF8;
        else
            cp = EcdNameToCodePage(pszName);
    }
    else
        cp = EcdNameToCodePage(pszName);
    uchardet_delete(pUcd);
    return cp;
}

// 将已知编码的字节流转换为另一种编码
// 成功返回TRUE，失败返回FALSE
inline BOOL EcdConvert(
    PCVOID p, size_t cb,
    UINT cpSrc, UINT cpDst,
    CByteBuffer& rbDst, CByteBuffer& rbWork,
    UINT uFlagsMb2Wc = MB_PRECOMPOSED, UINT uFlagsWc2Mb = 0) noexcept
{
    EckAssert(cpSrc != 0u && cpDst != 0u && cpSrc != cpDst);
    int cchU16 = MultiByteToWideChar(cpSrc, uFlagsMb2Wc,
        (PCCH)p, (int)cb, nullptr, 0);
    if (!cchU16)
        return FALSE;
    if (cpDst == CP_UTF16LE || cpDst == CP_UTF16BE)
    {
        const auto pNew = (PWCH)rbDst.PushBackNoExtra((cchU16 + 1) * sizeof(WCHAR));
        cchU16 = MultiByteToWideChar(cpSrc, uFlagsMb2Wc,
            (PCCH)p, (int)cb, pNew, cchU16);
        if (!cchU16)
            return FALSE;
        *(pNew + cchU16) = 0;
        if (cpDst == CP_UTF16BE)
            LcsUtf16ReverseByteOrder(pNew, cchU16);
        return TRUE;
    }
    rbWork.ReSize(cchU16 * sizeof(WCHAR));
    cchU16 = MultiByteToWideChar(cpSrc, uFlagsMb2Wc,
        (PCCH)p, (int)cb, (PWCH)rbWork.Data(), cchU16);
    if (!cchU16)
        return FALSE;
    if (cpDst == CP_UTF32LE || cpDst == CP_UTF32BE)
    {
        auto pU32 = (UINT*)rbDst.PushBackNoExtra(cchU16 * sizeof(UINT));
        BOOL bValid;
        const auto cchU32 = LcsUtf16LeToUtf32(
            (PCWSTR)rbWork.Data(), cchU16,
            pU32, bValid, (cpDst == CP_UTF32LE));
        rbDst.ReSize((BYTE*)(pU32 + cchU32) - rbDst.Data());
        EckAssert(bValid);
        return bValid;
    }

    int cbMb = WideCharToMultiByte(cpDst, uFlagsWc2Mb,
        (PCWCH)rbWork.Data(), cchU16, nullptr, 0, nullptr, nullptr);
    if (!cbMb)
        return FALSE;
    rbDst.ReSize(cbMb + 1);
    cbMb = WideCharToMultiByte(cpDst, uFlagsWc2Mb,
        (PCWCH)rbWork.Data(), cchU16, (PCH)rbDst.Data(), cbMb, nullptr, nullptr);
    if (!cbMb)
        return FALSE;
    *((PCH)rbDst.Data() + cbMb) = 0;
    return TRUE;
}

// 检查rbDst中的字节流编码，如果与目标编码不同则执行转换
// 成功返回TRUE，失败返回FALSE
inline BOOL EcdLoadTextStream(UINT cpDst, CByteBuffer& rbDst) noexcept
{
    if (rbDst.IsEmpty())
        return TRUE;
    const auto cpSrc = EcdDetectCodePage(rbDst.Data(), rbDst.Size());
    if (!cpSrc || cpSrc == cpDst)
        return TRUE;
    CByteBuffer rb, rbWork;
    if (!EcdConvert(rbDst.Data(), rbDst.Size(), cpSrc, cpDst, rb, rbWork))
        return FALSE;
    rbDst = std::move(rb);
    return TRUE;
}

// 读入指定文件，如果文件内容编码与目标编码不同则执行转换
// 成功返回TRUE，失败返回FALSE
inline BOOL EcdLoadTextFile(UINT cpDst, CByteBuffer& rbDst,
    PCWSTR pszFile, NTSTATUS* pnts = nullptr) noexcept
{
    rbDst = ReadInFile(pszFile, pnts);
    if (rbDst.IsEmpty())
        return TRUE;
    return EcdLoadTextStream(cpDst, rbDst);
}
ECK_NAMESPACE_END