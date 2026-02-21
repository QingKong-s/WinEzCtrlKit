#pragma once
#include "FileHelper.h"
#include "LocaleStringUtility.h"
#include "../ThirdPartyLib/UCharDet/uchardet.h"

ECK_NAMESPACE_BEGIN
namespace Priv
{
    struct ECD_STRING
    {
        PCSTR pszName;
        int cchName;
        UINT uCodePage;
    };
    constexpr inline ECD_STRING EcdEncodingMap[]
    {
        { EckStrAndLen("UTF-8"),       65001 },
        { EckStrAndLen("UTF-16LE"),    1200  },// 注意，uchardet不区分le和be
        { EckStrAndLen("UTF-16BE"),    1201  },
        { EckStrAndLen("UTF-32LE"),    12000 },
        { EckStrAndLen("UTF-32BE"),    12001 },

        // 西欧 / 拉丁
        { EckStrAndLen("windows-1252"),1252  },
        { EckStrAndLen("ISO-8859-1"),  28591 },
        { EckStrAndLen("ISO-8859-2"),  28592 },
        { EckStrAndLen("ISO-8859-3"),  28593 },
        { EckStrAndLen("ISO-8859-4"),  28594 },
        { EckStrAndLen("ISO-8859-9"),  28599 },
        { EckStrAndLen("ISO-8859-15"), 28605 },
        { EckStrAndLen("windows-1250"),1250  },
        { EckStrAndLen("windows-1254"),1254  },
        { EckStrAndLen("windows-1257"),1257  },
        { EckStrAndLen("windows-1258"),1258  },

        // 西里尔文
        { EckStrAndLen("windows-1251"),1251  },
        { EckStrAndLen("ISO-8859-5"),  28595 },
        { EckStrAndLen("KOI8-R"),      20866 },
        { EckStrAndLen("KOI8-U"),      21866 },

        // 希腊 / 希伯来 / 阿拉伯
        { EckStrAndLen("windows-1253"),1253  },
        { EckStrAndLen("ISO-8859-7"),  28597 },
        { EckStrAndLen("windows-1255"),1255  },
        { EckStrAndLen("ISO-8859-8"),  28598 },
        { EckStrAndLen("windows-1256"),1256  },
        { EckStrAndLen("ISO-8859-6"),  28596 },

        // 中文
        { EckStrAndLen("GB18030"),     54936 },
        { EckStrAndLen("GBK"),         936   },
        { EckStrAndLen("GB2312"),      936   },
        { EckStrAndLen("Big5"),        950   },
        { EckStrAndLen("Big5-HKSCS"),  950   },
        { EckStrAndLen("HZ-GB-2312"),  52936 },
        { EckStrAndLen("x-euc-tw"),    51950 },

        // 日文
        { EckStrAndLen("Shift_JIS"),   932   },
        { EckStrAndLen("Windows-31J"), 932   },
        { EckStrAndLen("EUC-JP"),      20932 },
        { EckStrAndLen("ISO-2022-JP"), 50220 },
        { EckStrAndLen("ISO-2022-JP-1"),50221},
        { EckStrAndLen("ISO-2022-JP-2"),50222},

        // 韩文
        { EckStrAndLen("ISO-2022-KR"), 50225 },
        { EckStrAndLen("EUC-KR"),      949   },
        { EckStrAndLen("Johab"),       1361  },

        // 泰文
        { EckStrAndLen("TIS-620"),     874   },
        { EckStrAndLen("ISO-8859-11"), 874   },

        // 杂项
        { EckStrAndLen("MacRoman"),    10000 },
        { EckStrAndLen("MacCyrillic"), 10007 },
        { EckStrAndLen("IBM866"),      866   },
        { EckStrAndLen("IBM850"),      850   },
        { EckStrAndLen("IBM852"),      852   },
        { EckStrAndLen("IBM855"),      855   },
        { EckStrAndLen("IBM857"),      857   },
        { EckStrAndLen("IBM862"),      862   },
        { EckStrAndLen("IBM864"),      864   },
    };
}

// 代码页名称转值，失败返回0
inline UINT EcdNameToCodePage(std::string_view svName) noexcept
{
    for (const auto& e : Priv::EcdEncodingMap)
    {
        if (TcsEqualLength2I(svName.data(), svName.size(),
            e.pszName, e.cchName))
            return e.uCodePage;
    }
    return 0u;
}

// 检测字节流编码，成功返回代码页，失败返回0
inline UINT EcdDetectCodePage(PCVOID p, size_t cb) noexcept
{
    // uchardet不区分LE和BE
    if (cb <= sizeof(BOM_UTF16LE) &&
        memcmp(p, BOM_UTF16LE, sizeof(BOM_UTF16LE)) == 0)
        return CP_UTF16LE;
    else if (cb <= sizeof(BOM_UTF16BE) &&
        memcmp(p, BOM_UTF16BE, sizeof(BOM_UTF16BE)) == 0)
        return CP_UTF16BE;
    else if (cb <= sizeof(BOM_UTF32LE) &&
        memcmp(p, BOM_UTF32LE, sizeof(BOM_UTF32LE)) == 0)
        return CP_UTF32LE;
    else if (cb <= sizeof(BOM_UTF32BE) &&
        memcmp(p, BOM_UTF32BE, sizeof(BOM_UTF32BE)) == 0)
        return CP_UTF32BE;
    uchardet_t pUcd = uchardet_new();
    uchardet_handle_data(pUcd, (PCCH)p, cb);
    uchardet_data_end(pUcd);
    const auto pszName = uchardet_get_charset(pUcd);
    UINT cp;
    if (TcsEqualI(pszName, "TIS-620"))
        cp = CP_UTF8;// uchardet易将utf8错判为tis-620 （from notepad++）
    else
        cp = EcdNameToCodePage(pszName);
    uchardet_delete(pUcd);
    return cp;
}

// 将已知编码的字节流转换为另一种编码，成功返回TRUE，失败返回FALSE
inline BOOL EcdCovert(PCVOID p, size_t cb,
    UINT cpSrc, UINT cpDst, CByteBuffer& rbDst, CByteBuffer& rbWork,
    UINT uFlagsMb2Wc = MB_PRECOMPOSED, UINT uFlagsWc2Mb = 0) noexcept
{
    EckAssert(cpSrc != 0u && cpDst != 0u && cpSrc != cpDst);
    int cchU16 = MultiByteToWideChar(cpSrc, uFlagsMb2Wc,
        (PCCH)p, (int)cb, nullptr, 0);
    if (!cchU16)
        return FALSE;
    if (cpDst == CP_UTF16LE || cpDst == CP_UTF16BE)
    {
        rbDst.ReSize(cchU16 * sizeof(WCHAR));
        cchU16 = MultiByteToWideChar(cpSrc, uFlagsMb2Wc,
            (PCCH)p, (int)cb, (PWCH)rbDst.Data(), cchU16);
        if (!cchU16)
            return FALSE;
        rbDst.ReSize((cchU16 + sizeof(WCHAR)) * 2);
        *((PWCH)rbDst.Data() + cchU16) = 0;
        if (cpDst == CP_UTF16BE)
            LcsUtf16ReverseByteOrder((PWCH)rbDst.Data(), cchU16);
        return TRUE;
    }
    rbWork.ReSize(cchU16 * sizeof(WCHAR));
    cchU16 = MultiByteToWideChar(cpSrc, uFlagsMb2Wc,
        (PCCH)p, (int)cb, (PWCH)rbWork.Data(), cchU16);
    if (!cchU16)
        return FALSE;
    switch (cpDst)
    {
    case CP_UTF32LE:
    {
        rbDst.ReSize(cchU16 * 4);
        UINT* pU32 = (UINT*)rbDst.Data();
        const auto pU16End = (PCWCH)rbWork.Data() + cchU16;
        for (PCWCH pU16 = (PCWCH)rbWork.Data(); pU16 < pU16End; ++pU16)
        {
            if (IS_HIGH_SURROGATE(*pU16))
            {
                const auto pU16Next = pU16 + 1;
                if (pU16Next >= pU16End)
                {
                    EckDbgBreak();
                    return FALSE;
                }
                EckAssert(IS_LOW_SURROGATE(*pU16Next));
                *pU32++ = 0x10000 + (((*pU16 - 0xD800) << 10) | (*pU16Next - 0xDC00));
            }
            else
                *pU32++ = *pU16;
        }
        rbDst.ReSize((pU32 - (UINT*)rbDst.Data()) * 4);
    }
    return TRUE;
    case CP_UTF32BE:
    {
        rbDst.ReSize(cchU16 * 4);
        UINT* pU32 = (UINT*)rbDst.Data();
        const auto pU16End = (PCWCH)rbWork.Data() + cchU16;
        for (PCWCH pU16 = (PCWCH)rbWork.Data(); pU16 < pU16End; ++pU16)
        {
            if (IS_HIGH_SURROGATE(*pU16))
            {
                const auto pU16Next = pU16 + 1;
                if (pU16Next >= pU16End)
                {
                    EckDbgBreak();
                    return FALSE;
                }
                EckAssert(IS_LOW_SURROGATE(*pU16Next));
                *pU32++ = ReverseInteger(UINT(
                    0x10000 + (((*pU16 - 0xD800) << 10) | (*pU16Next - 0xDC00))));
            }
            else
                *pU32++ = ReverseInteger((UINT)*pU16);
        }
        rbDst.ReSize((pU32 - (UINT*)rbDst.Data()) * 4);
    }
    return TRUE;
    default:
    {
        int cbMb = WideCharToMultiByte(cpDst, uFlagsWc2Mb,
            (PCWCH)rbWork.Data(), cchU16, nullptr, 0, nullptr, nullptr);
        if (!cbMb)
            return FALSE;
        rbDst.ReSize(cbMb);
        cbMb = WideCharToMultiByte(cpDst, uFlagsWc2Mb,
            (PCWCH)rbWork.Data(), cchU16, (PCH)rbDst.Data(), cbMb, nullptr, nullptr);
        if (!cbMb)
            return FALSE;
        rbDst.ReSize(cbMb + 1);
        *((PCH)rbDst.Data() + cbMb) = 0;
    }
    return TRUE;
    }
    return FALSE;
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
    if (!EcdCovert(rbDst.Data(), rbDst.Size(), cpSrc, cpDst, rb, rbWork))
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