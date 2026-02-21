#pragma once
#include "CFile.h"
#include "CRefBin.h"
#include "CRefStr.h"
#include "StringConvert.h"

#define ECK_CIO_NAMESPACE_BEGIN namespace Cio {
#define ECK_CIO_NAMESPACE_END   }

ECK_NAMESPACE_BEGIN
ECK_CIO_NAMESPACE_BEGIN
namespace Priv
{
    template<class TChar>
    EckInlineNdCe UINT CodePageFromCharType() noexcept
    {
        return (std::is_same_v<TChar, WCHAR> ? CP_UTF16LE :
            std::is_same_v<TChar, char8_t> ? CP_UTF8 :
            CP_ACP);
    }

    template<class TChar>
    void PushBackEol(CRefBin& rb, EolType eEol) noexcept
    {
        if constexpr (std::is_same_v<TChar, WCHAR>)
        {
            constexpr WCHAR Eol[]{ L"\r\n" };
            switch (eEol)
            {
            case EolType::CR:
                rb.PushBack(&Eol[0], sizeof(WCHAR));
                break;
            case EolType::LF:
                rb.PushBack(&Eol[1], sizeof(WCHAR));
                break;
            default:
                rb.PushBack(&Eol[0], 2 * sizeof(WCHAR));
                break;
            }
        }
        else
        {
            switch (eEol)
            {
            case EolType::CR:
                rb.PushBackByte('\r');
                break;
            case EolType::LF:
                rb.PushBackByte('\n');
                break;
            default:
                rb.PushBackByte('\r').PushBackByte('\n');
                break;
            }
        }
    }

    template<class TChar>
    size_t ScanEol(PCBYTE pBegin, size_t cb, size_t posBegin = 0) noexcept
    {
        const auto pEnd = (const TChar*)pBegin + cb / sizeof(TChar);
        for (auto p = (const TChar*)(pBegin + posBegin); p < pEnd; ++p)
        {
            if (*p == '\r' || *p == '\n')
                return size_t((PCBYTE)p - pBegin);
        }
        return SizeTMax;
    }
}

// 输入输出整数
template<class TInt, class TChar = char>
struct IoInt
{
    TInt Num{};
    BYTE Radix{ 10 };
    BYTE FillWidth{};
    TChar FillChar{ '0' };
};
// 输入输出字符串
template<class TChar>
struct IoCstStr
{
    const TChar* Str{};
    int Len{};
    UINT Cp : 31{};
    UINT IgnoreCase : 1{};

    IoCstStr() = default;

    template<class TCharString, size_t N>
    IoCstStr(const TCharString(&s)[N], UINT cp) noexcept : Str{ s }, Len{ int(N - 1) }, Cp{ cp } {}
};
// 跳过连续的空白字符
struct IoSpace {};

struct TO_STREAM_CTRL
{
    CRefBin& rbDst;
    UINT cp;
};

struct FROM_STREAM_CTRL
{
    PCBYTE pBegin;  // TypeIo实现不得修改
    PCBYTE pEnd;    // TypeIo实现不得修改
    PCBYTE pCurr;
    int cArg;       // TypeIo实现不得访问
};

namespace TypeIo
{
    enum class Ret
    {
        // 以下为成功代码
        Ok,         // 无额外操作
        Flush,      // 需要冲洗缓冲区
        Continue,   // 应继续读入并分析序列

        OkMax,
        // 以下为错误代码
        NotMatch,   // 扫描时发现占位与流内容不匹配
        BadFormat,  // 扫描失败，格式与期望格式不符
    };

    template<class TChar, class TFmtInt, class TFmtChar>
    Ret ToStream(const TO_STREAM_CTRL& c, IoInt<TFmtInt, TFmtChar> v) noexcept
    {
        const auto cchExtra = TcsCvtCalcBufferSize<TFmtInt>(v.Radix, v.FillWidth);
        const auto p = (TChar*)c.rbDst.PushBack(cchExtra * 2);
        TChar* pEnd;
        TcsFromInt(p, cchExtra, v.Num, v.Radix, TRUE, &pEnd, v.FillWidth, v.FillChar);
        c.rbDst.ReSize((BYTE*)pEnd - c.rbDst.Data());
        return Ret::Ok;
    }
    template<class TChar>
    Ret ToStream(const TO_STREAM_CTRL& c, std::integral auto i) noexcept
    {
        return ToStream<TChar>(c, IoInt{ i });
    }

    template<class TChar, class TCharString>
    Ret ToStream(const TO_STREAM_CTRL& c, const IoCstStr<TCharString>& Str) noexcept
    {
        if constexpr (std::is_same_v<TChar, TCharString>)
            c.rbDst.PushBack(Str.Str, Str.Len * sizeof(TChar));
        else if constexpr (std::is_same_v<TCharString, WCHAR>)
        {
            const auto cchBuf = WideCharToMultiByte(c.cp, 0, Str.Str, Str.Len,
                nullptr, 0, nullptr, nullptr);
            if (cchBuf)
                WideCharToMultiByte(c.cp, 0, Str.Str, Str.Len,
                    (PCH)c.rbDst.PushBack(cchBuf), cchBuf, nullptr, nullptr);
        }
        else if constexpr (std::is_same_v<TChar, WCHAR>)
        {
            const auto cchBuf = MultiByteToWideChar(
                c.cp, MB_PRECOMPOSED, Str.Str, Str.Len, nullptr, 0);
            if (cchBuf)
                MultiByteToWideChar(c.cp, MB_PRECOMPOSED, Str.Str, Str.Len,
                    (PWCH)c.rbDst.PushBack(cchBuf), cchBuf);
        }
        else
            c.rbDst.PushBack(Str.Str, Str.Len * sizeof(TChar));
        return *(Str.Str + Str.Len - 1) == '\n' ? Ret::Flush : Ret::Ok;
    }
    template<class TChar, class TCharString, size_t N>
    Ret ToStream(const TO_STREAM_CTRL& c, const TCharString(&s)[N]) noexcept
    {
        constexpr UINT cpInput = Priv::CodePageFromCharType<TCharString>();
        return ToStream<TChar>(c, IoCstStr<TCharString>{ s, cpInput });
    }


    template<class TChar>
    Ret FromStream(FROM_STREAM_CTRL& c, std::integral auto& i) noexcept
    {
        const auto pEnd = (const TChar*)c.pEnd;
        const auto pCurr = (const TChar*)c.pCurr;
        const TChar* pScanEnd;
        const auto r = TcsToInt(pCurr, pEnd - pCurr, i, 0, &pScanEnd);
        if (r == TcsCvtErr::Ok || r == TcsCvtErr::Overflow)
        {
            c.pCurr = (PCBYTE)pScanEnd;
            return Ret::Ok;
        }
        else
            return Ret::BadFormat;
    }

    template<class TChar, class TCharString>
        requires requires { sizeof(TChar) == sizeof(TCharString); }
    Ret FromStream(FROM_STREAM_CTRL& c, const IoCstStr<TCharString>& Str) noexcept
    {
        const auto pEnd = (const TChar*)c.pEnd;
        const auto pCurr = (const TChar*)c.pCurr;
        if (Str.IgnoreCase ?
            TcsIsStartWithLen2I(pCurr, pEnd - pCurr, (const TChar*)Str.Str, Str.Len) :
            TcsIsStartWithLen2(pCurr, pEnd - pCurr, (const TChar*)Str.Str, Str.Len))
        {
            c.pCurr += Str.Len;
            return Ret::Ok;
        }
        return Ret::NotMatch;
    }

    template<class TChar, class TCharString, size_t N>
    Ret FromStream(FROM_STREAM_CTRL& c, const TCharString(&s)[N]) noexcept
    {
        constexpr UINT cpInput = Priv::CodePageFromCharType<TCharString>();
        return FromStream<TChar>(c, IoCstStr<TCharString>{ s, cpInput });
    }

    template<class TChar>
    Ret FromStream(FROM_STREAM_CTRL& c, IoSpace) noexcept
    {
        const auto pEnd = (const TChar*)c.pEnd;
        auto pCurr = (const TChar*)c.pCurr;
        Ret r{ Ret::Continue };
        for (; pCurr < pEnd; ++pCurr)
        {
            if (*pCurr != ' ' && *pCurr != '\r' && *pCurr != '\n' && *pCurr != '\t')
            {
                r = Ret::Ok;
                break;
            }
        }
        c.pCurr = (PCBYTE)pCurr;
        return r;
    }
}


template<class TChar>
class CWriter : public CFile
{
private:
    CRefBin m_rbBuf{};
    UINT m_cp{ 0 };
    USHORT m_cbMaxBuffer{ 512 };
    EolType m_eEol{ EolType::CRLF };

    BOOL PushBackBuffer(auto&& Arg, auto&&... Args) noexcept
    {
        const auto b = (TypeIo::ToStream<TChar>(
            TO_STREAM_CTRL{ m_rbBuf, m_cp }, Arg) == TypeIo::Ret::Flush);
        if constexpr (sizeof...(Args))
            return PushBackBuffer(Args...) || b;
        else
            return b;
    }
public:
    auto& Flush() noexcept
    {
        if (!m_rbBuf.IsEmpty())
        {
            Write(m_rbBuf.Data(), (DWORD)m_rbBuf.Size());
            m_rbBuf.Clear();
        }
        return *this;
    }
    auto& WriteRaw(_In_reads_bytes_(cb) PCVOID p, DWORD cb, DWORD* pcbWritten = nullptr) noexcept
    {
        if (m_rbBuf.IsEmpty())
            Write(p, cb, pcbWritten);
        else
        {
            m_rbBuf.PushBack(p, cb);
            Flush();
        }
        return *this;
    }

    auto& Print(auto&&... Args) noexcept
    {
        if constexpr (sizeof...(Args))
        {
            const auto bFlush = PushBackBuffer(Args...);
            if (bFlush || m_rbBuf.Size() >= m_cbMaxBuffer)
                Flush();
        }
        return *this;
    }
    auto& PrintLine(auto&&... Args) noexcept
    {
        if constexpr (sizeof...(Args))
            PushBackBuffer(Args...);
        Priv::PushBackEol<TChar>(m_rbBuf, m_eEol);
        Flush();
        return *this;
    }
#ifdef _DEBUG
    auto DbgGetBuffer() const noexcept { return (const TChar*)m_rbBuf.Data(); }
#endif
};

template<class TChar>
class CReader : public CFile
{
private:
    CRefBin m_rbBuf{};
    size_t m_posCurr{};
    size_t m_posEol{ SizeTMax };
    USHORT m_cbMaxBuffer{ 64 };
    EolType m_eEol{ EolType::CRLF };

    template<class T, class... Ts>
    void IntScan(FROM_STREAM_CTRL& c, T&& Arg, Ts&&... Args) noexcept
    {
        ++c.cArg;
        TypeIo::Ret r;
        while ((r = TypeIo::FromStream<TChar>(c, std::forward<T>(Arg))) == TypeIo::Ret::Continue)
        {
            EnsureReadToEol(TRUE);
            const auto pos = c.pCurr - c.pBegin;
            c.pBegin = m_rbBuf.Data();
            c.pEnd = m_rbBuf.Data() + m_rbBuf.Size();
            c.pCurr = c.pBegin + pos;
        }
        if (r > TypeIo::Ret::OkMax)
            return;
        if constexpr (sizeof...(Args))
            IntScan(c, std::forward<Ts>(Args)...);
    }

    void EnsureReadToEol(BOOL bEmpty) noexcept
    {
        NTSTATUS nts;
        DWORD cbRead;
        size_t posBegin;
        if (bEmpty)
        {
            posBegin = m_rbBuf.Size();
            Read(m_rbBuf.PushBack(m_cbMaxBuffer), m_cbMaxBuffer, &cbRead, &nts);
            m_rbBuf.ReSize(posBegin + cbRead);
        }

        if (m_posEol == SizeTMax)
            m_posEol = Priv::ScanEol<TChar>(m_rbBuf.Data(), m_rbBuf.Size());
        while (m_posEol == SizeTMax)
        {
            posBegin = m_rbBuf.Size();
            Read(m_rbBuf.PushBack(m_cbMaxBuffer), m_cbMaxBuffer, &cbRead, &nts);
            if (cbRead)
            {
                m_rbBuf.ReSize(posBegin + cbRead);
                m_posEol = Priv::ScanEol<TChar>(m_rbBuf.Data(), m_rbBuf.Size(), posBegin);
            }
            else
                break;
        }
        if (m_eEol == EolType::CRLF &&
            m_posEol == m_rbBuf.Size() - 1 &&
            m_rbBuf[m_posEol] == '\r')// 避免截断CRLF
        {
            posBegin = m_rbBuf.Size();
            Read(m_rbBuf.PushBack(m_cbMaxBuffer), m_cbMaxBuffer, &cbRead, &nts);
            if (cbRead)
                m_rbBuf.ReSize(posBegin + cbRead);
        }
    }
public:
    template<class... T>
    int Scan(T&&... Args) noexcept
    {
        if constexpr (!!sizeof...(Args))
        {
            EnsureReadToEol(m_rbBuf.IsEmpty());
            FROM_STREAM_CTRL c{ m_rbBuf.Data(),m_rbBuf.Data() + m_rbBuf.Size(),m_rbBuf.Data() };
            IntScan(c, std::forward<T>(Args)...);
            m_rbBuf.Erase(0, c.pCurr - c.pBegin);
            return c.cArg == sizeof...(Args) ? 0 : c.cArg;
        }
        return 0;
    }

    template<class TTrait, class TAlloc>
    BOOL ScanLine(CRefStrT<TChar, TTrait, TAlloc>& rs) noexcept
    {
        EnsureReadToEol(FALSE);
        if (m_posEol != SizeTMax)
        {
            rs.PushBack((const TChar*)m_rbBuf.Data(), int(m_posEol / sizeof(TChar)));
            if (rs[m_posEol] == '\r' &&
                m_posEol + 1 < rs.Size() && rs[m_posEol] == '\n')
                ++m_posEol;// CRLF检测
            m_rbBuf.Erase(0, m_posEol + 1);
            m_posEol = SizeTMax;
        }
    }
};
ECK_CIO_NAMESPACE_END
ECK_NAMESPACE_END