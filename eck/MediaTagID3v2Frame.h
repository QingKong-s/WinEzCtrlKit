#pragma once
#include "MediaTag.h"
#include "Compress.h"

#define ECK_ID3V2_FRAME_NAMESPACE_BEGIN namespace ID3v2 {
#define ECK_ID3V2_FRAME_NAMESPACE_END   }

ECK_NAMESPACE_BEGIN
ECK_MEDIATAG_NAMESPACE_BEGIN
ECK_ID3V2_FRAME_NAMESPACE_BEGIN
enum class EventType : BYTE
{
    Padding,				// 填充(无意义)
    EndOfInitialSilence,	// 初始静音结束
    IntroStart,				// 前奏开始
    MainPartStart,			// 主体部分开始
    OutroStart,				// 序曲开始
    OutroEnd,				// 片尾曲结束
    VerseStart,				// 诗句开始
    RefrainStart,			// 重唱开始
    InterludeStart,			// 插曲开始
    ThemeStart,				// 主题开始
    VariationStart,			// 变奏开始
    KeyChange,				// 调性变化
    TimeChange,				// 时间变化
    MomentaryUnwantedNoise,	// 瞬时噪音("啪"、"噼啪"和"噗噗"声)
    SustainedNoise,			// 持续噪音
    SustainedNoiseEnd,		// 持续噪音结束
    IntroEnd,				// 前奏结束
    MainPartEnd,			// 主体部分结束
    VerseEnd,				// 诗句结束
    RefrainEnd,				// 重唱结束
    ThemeEnd,				// 主题结束
    Profanity,				// 脏话 (仅ID3v2.4)
    ProfanityEnd,			// 脏话结束 (仅ID3v2.4)

    InvalidBeginV2_4,		// ！无效区开始 (仅ID3v2.4)
    InvalidEnd = 0xFC,		// ！无效区结束

    AudioEnd = 0xFD,		// 音频结束(静音开始)
    AudioFileEnds,			// 音频文件结束

    InvalidBeginV2_3 = Profanity,// 无效区开始 (仅ID3v2.3)
};

enum class TimestampFmt : BYTE
{
    MpegFrame,
    Milliseconds,
    Max
};

enum class TextEncoding : BYTE
{
    Latin1,
    UTF16V2_3,
    UTF16LE = UTF16V2_3,
    MaxV2_3,
    UTF16BE = MaxV2_3,
    UTF8,
    Max,

    Default = UTF16LE
};

enum class LrcContentType : BYTE
{
    Other,				// 其他
    Lyrics,				// 歌词
    TextTranscription,	// 文字转录
    Movement,			// 乐章/乐段名称(如"Adagio")
    Events,				// 事件(如"堂吉诃德登场")
    Chord,				// 和弦(如"Bb F Fsus")
    Trivia,				// 小节/弹出式信息
    WebpageUrl,			// 网页URL
    ImageUrl,			// 图片URL
    Max
};

enum class ChannelType : BYTE
{
    Other,			// 其他
    MasterVolume,	// 主音量
    FrontRight,		// 前右
    FrontLeft,		// 前左
    BackRight,		// 后右
    BackLeft,		// 后左
    FrontCentre,	// 前中
    BackCentre,		// 后中
    Subwoofer,		// 低音炮
    Max
};

enum class Interpolation : BYTE
{
    Band,	// 不进行插值。从一个调整级别到另一个调整级别的跳变发生在两个调整点之间的中间位置。
    Linear,	// 调整点之间的插值是线性的。
    Max
};

enum class ReceivedWay : BYTE
{
    Other,
    StandardCdAlbumWithOtherSongs,
    CompressedAudioOnCd,
    FileOverTheInternet,
    StreamOverTheInternet,
    AsNoteSheets,
    AsNoteSheetsInABookWithOtherSheets,
    MusicOnOtherMedia,
    NonMusicalMerchandise,
    Max
};

#define ECK_DECL_ID3FRAME_METHOD_CLONE(x)					\
    FRAME* Clone() const noexcept override { return new x{ *this }; }\
    x(x&&) = default;										\
    x(const x&) = default;

#define ECK_DECL_ID3FRAME_METHOD_CLONE_DEF_CONS(x)			\
    [[nodiscard]] FRAME* Clone() const noexcept override { return new x{ *this }; }\
    x(x&&) = default;										\
    x(const x&) = default;									\
    x() = default;

#define ECK_DECL_ID3FRAME_METHOD(x)			\
    ECK_DECL_ID3FRAME_METHOD_CLONE(x)		\
    x() { memcpy(Id, #x, 4); }

#define ECK_DECL_ID3FRAME_METHOD_ID(x)		\
    ECK_DECL_ID3FRAME_METHOD_CLONE(x)		\
    x() = default;							\
    x(_In_reads_bytes_(4) PCSTR psz) { memcpy(Id, psz, 4); }


struct FRAME
{
    struct SERIAL_CTX
    {
        CByteBuffer& rbWork;
        const ID3v2_HEADER* pTagHdr;
        const ID3v2_FRAME_HEADER* pFrameHdr;
        BOOLEAN bFrameHdr;
    };

    CHAR Id[4]{};		// 帧标识
    BYTE byGroupId{};   // 组标识，对应GRID
    BYTE byCryption{};  // 加密方式，对应ENCR

    BITBOOL bTagAlterDiscard : 1{};
    BITBOOL bFileAlterDiscard : 1{};
    BITBOOL bReadOnly : 1{};
    BITBOOL bHasGroupId : 1{};
    BITBOOL bCompressed : 1{};
    BITBOOL bEncrypted : 1{};
    BITBOOL bUnsynchronized : 1{};
    BITBOOL bHasDataLengthIndicator : 1{};

    BYTE byFlags{};// MIIF_

    virtual ~FRAME() = default;

    virtual Result Serialize(CByteBuffer& rb, const SERIAL_CTX& Ctx) noexcept = 0;
    virtual Result Deserialize(CByteBuffer& rb, const SERIAL_CTX& Ctx) noexcept = 0;

    virtual FRAME* Clone() const noexcept = 0;

    EckInlineNd BOOL EqualId(_In_reads_(4) PCCH Id_) const noexcept
    {
        return memcmp(Id, Id_, 4) == 0;
    }

    static void TagUnsynchronize(CByteBuffer& rb, size_t posBegin) noexcept
    {
        EckAssert(rb.Size() >= posBegin);
        for (size_t i = posBegin; i < rb.Size() - 1; )
        {
            if (rb[i] == 0xFF &&
                (rb[i + 1] == 0 || IsBitSet(rb[i + 1], 0b1110'0000_by)))
            {
                rb.Insert(i + 1, 0);
                i += 2;
            }
        }
        if (posBegin != rb.Size() && rb.Back() == 0xFF)
            rb.PushBackByte(0);
    }

    static BOOL TagIsValidFrameId(PCCH Id) noexcept
    {
        return isalnum(Id[0]) && isalnum(Id[1]) && isalnum(Id[2]) && isalnum(Id[3]) &&
            memcmp(Id, "ID3", 3) != 0 &&
            memcmp(Id, "3DI", 3) != 0;
    }
protected:
    // 准备空间，bFrameHdr控制是否写入帧头
    CMemoryWalker PreSerialize(CByteBuffer& rb,
        const SERIAL_CTX& Ctx, size_t cbFrame) const noexcept
    {
        if (!Ctx.bFrameHdr)
            return { rb.PushBack(cbFrame), cbFrame };
        const size_t cbTotal = cbFrame + sizeof(ID3v2_FRAME_HEADER);
        CMemoryWalker w{ rb.PushBack(cbTotal), cbTotal };

        ID3v2_FRAME_HEADER* pFrameHdr;
        w.SkipPointer(pFrameHdr);
        memcpy(pFrameHdr->Id, Id, 4);
        pFrameHdr->Flags[0] = pFrameHdr->Flags[1] = 0;
        if (Ctx.pTagHdr->Ver == 3)
        {
            // 0 = 状态，1 = 格式
            pFrameHdr->Size[0] = pFrameHdr->Size[1] = 0;
            if (bTagAlterDiscard)
                pFrameHdr->Flags[0] |= ID3V23FF_TAG_ALTER_DISCARD;
            if (bFileAlterDiscard)
                pFrameHdr->Flags[0] |= ID3V23FF_FILE_ALTER_DISCARD;
            if (bReadOnly)
                pFrameHdr->Flags[0] |= ID3V23FF_READ_ONLY;

            if (bHasGroupId)
                pFrameHdr->Flags[1] |= ID3V23FF_HAS_GROUP_IDENTITY;
            if (bCompressed)
                pFrameHdr->Flags[1] |= ID3V23FF_COMPRESSION;
            if (bEncrypted)
                pFrameHdr->Flags[1] |= ID3V23FF_ENCRYPTION;

            *(UINT*)pFrameHdr->Size = ReverseInteger((UINT)cbFrame);
        }
        else
        {
            pFrameHdr->Size[0] = pFrameHdr->Size[1] = 0;

            if (bTagAlterDiscard)
                pFrameHdr->Flags[0] |= ID3V24FF_TAG_ALTER_DISCARD;
            if (bFileAlterDiscard)
                pFrameHdr->Flags[0] |= ID3V24FF_FILE_ALTER_DISCARD;
            if (bReadOnly)
                pFrameHdr->Flags[0] |= ID3V24FF_READ_ONLY;

            if (bHasGroupId)
                pFrameHdr->Flags[1] |= ID3V24FF_HAS_GROUP_IDENTITY;
            if (bCompressed)
                pFrameHdr->Flags[1] |= ID3V24FF_COMPRESSION;
            if (bEncrypted)
                pFrameHdr->Flags[1] |= ID3V24FF_ENCRYPTION;
            if (bUnsynchronized)
                pFrameHdr->Flags[1] |= ID3V24FF_UNSYNCHRONIZATION;
            if (bHasDataLengthIndicator)
                pFrameHdr->Flags[1] |= ID3V24FF_HAS_DATA_LENGTH_INDICATOR;

            TagUIntToSyncSafeInt(pFrameHdr->Size, (UINT)cbFrame);
        }
        return w;
    }
    // 完成序列化，处理非同步和压缩等
    Result PostSerialize(CByteBuffer& rb,
        const SERIAL_CTX& Ctx, size_t cbFrame) const noexcept
    {
        const size_t posFrameData = Ctx.bFrameHdr ?
            sizeof(ID3v2_FRAME_HEADER) : 0;
        EckAssert(rb.Size() >= posFrameData + cbFrame);
        if (bCompressed)
        {
            Ctx.rbWork.Clear();
            ZLibCompress(rb.Data() + rb.Size() - cbFrame, cbFrame, Ctx.rbWork);
            rb.Replace(rb.Size() - cbFrame, cbFrame,
                Ctx.rbWork.Data(), Ctx.rbWork.Size());
        }

        if (Ctx.pTagHdr->Ver == 4)
        {
            if (Ctx.pTagHdr->Flags & ID3V2HF_UNSYNCHRONIZATION ||
                bUnsynchronized)
                TagUnsynchronize(rb, posFrameData);
        }
        else
        {
            if (Ctx.pTagHdr->Flags & ID3V2HF_UNSYNCHRONIZATION)
                TagUnsynchronize(rb, posFrameData);
        }
        return Result::Ok;
    }

    // 使用pFrameHdr中的信息更新当前状态
    // 如果bFrameHdr为TRUE，则rb包含帧头
    // 调用方从返回爬行器的当前位置开始读取帧内容，不能假定当前位置一定在起始处
    CMemoryReader PreDeserialize(CByteBuffer& rb, const SERIAL_CTX& Ctx)
    {
        EckAssert(Ctx.bFrameHdr ? rb.Size() >= sizeof(ID3v2_FRAME_HEADER) : TRUE);
        memcpy(Id, Ctx.pFrameHdr->Id, 4);
        if (Ctx.pTagHdr->Ver == 3)
        {
            if (Ctx.pFrameHdr->Flags[0] & ID3V23FF_TAG_ALTER_DISCARD)
                bTagAlterDiscard = TRUE;
            if (Ctx.pFrameHdr->Flags[0] & ID3V23FF_FILE_ALTER_DISCARD)
                bFileAlterDiscard = TRUE;
            if (Ctx.pFrameHdr->Flags[0] & ID3V23FF_READ_ONLY)
                bReadOnly = TRUE;

            if (Ctx.pFrameHdr->Flags[1] & ID3V23FF_HAS_GROUP_IDENTITY)
                bHasGroupId = TRUE;
            if (Ctx.pFrameHdr->Flags[1] & ID3V23FF_COMPRESSION)
                bCompressed = TRUE;
            if (Ctx.pFrameHdr->Flags[1] & ID3V23FF_ENCRYPTION)
                bEncrypted = TRUE;

            bUnsynchronized = bHasDataLengthIndicator = FALSE;

            if (Ctx.pTagHdr->Flags & ID3V2HF_UNSYNCHRONIZATION)
                rb.ReplaceSub({ 0xFF, 0x00 }, { 0xFF });
        }
        else
        {
            if (Ctx.pFrameHdr->Flags[0] & ID3V24FF_TAG_ALTER_DISCARD)
                bTagAlterDiscard = TRUE;
            if (Ctx.pFrameHdr->Flags[0] & ID3V24FF_FILE_ALTER_DISCARD)
                bFileAlterDiscard = TRUE;
            if (Ctx.pFrameHdr->Flags[0] & ID3V24FF_READ_ONLY)
                bReadOnly = TRUE;

            if (Ctx.pFrameHdr->Flags[1] & ID3V24FF_HAS_GROUP_IDENTITY)
                bHasGroupId = TRUE;
            if (Ctx.pFrameHdr->Flags[1] & ID3V24FF_COMPRESSION)
                bCompressed = TRUE;
            if (Ctx.pFrameHdr->Flags[1] & ID3V24FF_ENCRYPTION)
                bEncrypted = TRUE;
            if (Ctx.pFrameHdr->Flags[1] & ID3V24FF_UNSYNCHRONIZATION)
                bUnsynchronized = TRUE;
            if (Ctx.pFrameHdr->Flags[1] & ID3V24FF_HAS_DATA_LENGTH_INDICATOR)
                bHasDataLengthIndicator = TRUE;

            if (Ctx.pTagHdr->Flags & ID3V2HF_UNSYNCHRONIZATION ||
                bUnsynchronized)
                rb.ReplaceSub({ 0xFF, 0x00 }, { 0xFF });
        }

        CMemoryReader w{ rb.Data(), rb.Size() };
        if (Ctx.bFrameHdr)
            w += sizeof(ID3v2_FRAME_HEADER);
        // 读附加信息
        UINT cbExtra{}, cbBody, cbOrg{};
        if (Ctx.pTagHdr->Ver == 3)
        {
            cbBody = ReverseInteger(*(UINT*)Ctx.pFrameHdr->Size);
            if (cbBody != w.GetRemainingSize())
                w.SeekToEnd() += 1;// throw
            if (Ctx.pFrameHdr->Flags[1] & ID3V23FF_HAS_GROUP_IDENTITY)
                w >> byGroupId;
            if (Ctx.pFrameHdr->Flags[1] & ID3V23FF_ENCRYPTION)
                w >> byCryption;
            if (Ctx.pFrameHdr->Flags[1] & ID3V23FF_COMPRESSION)
                w >> cbOrg;
        }
        else/* if (Ctx.pTagHdr->Ver == 4)*/
        {
            cbBody = TagSyncSafeIntToUInt(Ctx.pFrameHdr->Size);
            if (cbBody != w.GetRemainingSize())
                w.SeekToEnd() += 1;// throw
            if (Ctx.pFrameHdr->Flags[1] & ID3V24FF_HAS_GROUP_IDENTITY)
                w >> byGroupId;
            if (Ctx.pFrameHdr->Flags[1] & ID3V24FF_ENCRYPTION)
                w >> byCryption;
            if (Ctx.pFrameHdr->Flags[1] & ID3V24FF_HAS_DATA_LENGTH_INDICATOR)
                w >> cbOrg;
        }

        if (bCompressed && w.GetRemainingSize())
        {
            Ctx.rbWork.Clear();
            ZLibDecompress(w.Data(), w.GetRemainingSize(), Ctx.rbWork);
            std::swap(rb, Ctx.rbWork);
            return { rb.Data(), rb.Size() };
        }
        else
            return w;
    }

    Result PostDeserialize(CByteBuffer& rb, const SERIAL_CTX& Ctx) noexcept
    {
        return Result::Ok;
    }

    // 给定Utf16 LE字符串，转换为指定编码，并将结果追加到rbResult
    static void CovertTextEncoding(
        CByteBuffer& rb,
        const CStringW& rsStr,
        TextEncoding eEncoding,
        BOOL bTerminator = FALSE) noexcept
    {
        if (rsStr.IsEmpty())
        {
            if (bTerminator)
                switch (eEncoding)
                {
                case TextEncoding::Latin1:
                case TextEncoding::UTF8:
                    *rb.PushBack(1) = 0;
                    break;
                case TextEncoding::UTF16LE:
                case TextEncoding::UTF16BE:
                    *(WCHAR*)rb.PushBack(2) = 0;
                    break;
                }
            return;
        }

        switch (eEncoding)
        {
        case TextEncoding::Latin1:
            EcdWideToMultiByte(rb, rsStr.Data(), rsStr.Size(), CP_ACP);
            if (!bTerminator)
                rb.PopBack(1);
            break;

        case TextEncoding::UTF16LE:
        {
            const size_t cch = rsStr.ByteSize() - (bTerminator ? 0 : sizeof(WCHAR));
            BYTE* const p = rb.PushBack(cch + sizeof(BOM_UTF16LE));
            memcpy(p, BOM_UTF16LE, 2);
            memcpy(p + 2, rsStr.Data(), cch);
        }
        break;

        case TextEncoding::UTF16BE:
        {
            const size_t cch = rsStr.ByteSize() - (bTerminator ? 0 : sizeof(WCHAR));
            BYTE* p = rb.PushBack(cch + sizeof(BOM_UTF16BE));
            memcpy(p, BOM_UTF16BE, 2);
            p += 2;
            for (const auto ch : rsStr)
            {
                *((WCHAR*)p) = _byteswap_ushort(ch);
                p += 2;
            }
            if (bTerminator)
                *((WCHAR*)p) = 0;
        }
        break;

        case TextEncoding::UTF8:
            EcdWideToUtf8(rb, rsStr.Data(), rsStr.Size());
            if (!bTerminator)
                rb.PopBack(1);
            break;
        default:
            EckDbgBreak();
            break;
        }
    }

    static void CovertTextEncoding(
        CByteBuffer& rb,
        const std::vector<CStringW>& v,
        TextEncoding eEncoding,
        BOOL bTerminator = FALSE) noexcept
    {
        if (v.size() == 1u)
            return CovertTextEncoding(rb, v.front(), eEncoding, bTerminator);
        for (size_t i = 0; i < v.size() - 1; ++i)
            CovertTextEncoding(rb, v[i], eEncoding, TRUE);
        CovertTextEncoding(rb, v.back(), eEncoding, TRUE);
    }

    static void ConvertTextEncoding(
        CStringW& rsResult,
        CMemoryReader& w,
        int cb,
        int iTextEncoding)
    {
        EckAssert(iTextEncoding >= 0 && iTextEncoding <= 3);

        rsResult.Clear();
        const BOOL bTerminator = (cb < 0);

        switch (iTextEncoding)
        {
        case 0:// ISO-8859-1
            if (bTerminator)
                cb = w.CountStringLength<char>();
            if (!cb)
                break;
            EcdMultiByteToWide(rsResult, (PCCH)w.Data(), cb, CP_ACP);
            break;

        case 1:// UTF-16LE
        {
            if (bTerminator)
                cb = w.CountStringLength<WCHAR>() * sizeof(WCHAR);
            if (!cb)
                break;

            if (memcmp(w.Data(), BOM_UTF16LE, 2) == 0)
                w += sizeof(WCHAR), cb -= sizeof(WCHAR);
            else if (memcmp(w.Data(), BOM_UTF16BE, 2) == 0)// For ID3v2.3
            {
                w += sizeof(WCHAR), cb -= sizeof(WCHAR);
                goto Utf16BE;
            }

            const auto cch = int(cb / sizeof(WCHAR));
            rsResult.ReSize(cch);
            wmemcpy(rsResult.Data(), (PCWCH)w.Data(), cch);
        }
        break;

        case 2:// UTF-16BE
        {
            if (bTerminator)
                cb = w.CountStringLength<WCHAR>() * sizeof(WCHAR);
            if (!cb)
                break;

            if (memcmp(w.Data(), BOM_UTF16BE, 2) == 0)
                w += sizeof(WCHAR), cb -= sizeof(WCHAR);
        Utf16BE:
            const auto cch = int(cb / sizeof(WCHAR));
            rsResult.ReSize(cch);
            const auto pSrc = (PCWCH)w.Data();
            for (int i = 0; i < cch; ++i)
                rsResult[i] = _byteswap_ushort(pSrc[i]);
        }
        break;

        case 3:// UTF-8
            if (bTerminator)
                cb = w.CountStringLength<char>();
            if (!cb)
                break;
            EcdUtf8ToWide(rsResult, (PCCH)w.Data(), cb);
            break;
        default: ECK_UNREACHABLE;
        }

        w += cb;
        if (bTerminator)
            w += (iTextEncoding == 0 || iTextEncoding == 3) ? 1 : 2;
    }

    EckInline static void ConvertTextEncoding(
        CStringW& rsResult,
        CMemoryReader& w,
        int cb,
        TextEncoding iTextEncoding)
    {
        ConvertTextEncoding(rsResult, w, cb, (int)iTextEncoding);
    }

    static BOOL CheckLanguageId(_Inout_count_(3) PCHAR pszLangId) noexcept
    {
        for (size_t i = 0; i < 3; ++i)
        {
            if (pszLangId[i] < 'a' || pszLangId[i] > 'z')
            {
                memcpy(pszLangId, "XXX", 3);
                return FALSE;
            }
        }
        return TRUE;
    }

    static void StepOverTerminator(
        CMemoryReader& w,
        TextEncoding eEncoding) noexcept
    {
        switch (eEncoding)
        {
        case TextEncoding::Latin1:
        case TextEncoding::UTF8:
            w += 1;
            break;
        case TextEncoding::UTF16LE:
        case TextEncoding::UTF16BE:
            w += 2;
            break;
        default:
            EckDbgBreak();
            break;
        }
    }
};

struct UFID final : public FRAME
{
    CStringA rsEmail{};
    CByteBuffer rbOwnerData{};

    Result Serialize(CByteBuffer& rb, const SERIAL_CTX& Ctx) noexcept override
    {
        const size_t cbFrame = rsEmail.Size() + 1 + rbOwnerData.Size();
        auto w = PreSerialize(rb, Ctx, cbFrame);
        w << rsEmail << rbOwnerData;
        return PostSerialize(rb, Ctx, cbFrame);
    }

    Result Deserialize(CByteBuffer& rb, const SERIAL_CTX& Ctx) noexcept override try
    {
        auto w = PreDeserialize(rb, Ctx);
        w >> rsEmail;
        if (w.GetRemainingSize() > 64u)
            return Result::TooLargeData;
        else
            rbOwnerData.Assign(w.Data(), w.GetRemainingSize());
        return PostDeserialize(rb, Ctx);
    }
    catch (const CMemoryReader::Xpt&)
    {
        return Result::Broken;
    }

    ECK_DECL_ID3FRAME_METHOD(UFID)
};

struct TEXTFRAME : public FRAME
{
    TextEncoding eEncoding{ TextEncoding::Default };
    std::vector<CStringW> vText{};

    Result Serialize(CByteBuffer& rb, const SERIAL_CTX& Ctx) noexcept override
    {
        if (EqualId("TENC") || EqualId("TLEN"))
            bFileAlterDiscard = TRUE;

        Ctx.rbWork.Clear();
        CovertTextEncoding(Ctx.rbWork, vText, eEncoding);
        size_t cbFrame = 1 + Ctx.rbWork.Size();
        auto w = PreSerialize(rb, Ctx, cbFrame);
        w << eEncoding << Ctx.rbWork;
        return PostSerialize(rb, Ctx, cbFrame);
    }

    Result Deserialize(CByteBuffer& rb, const SERIAL_CTX& Ctx) noexcept override try
    {
        auto w = PreDeserialize(rb, Ctx);
        w >> eEncoding;
        if (eEncoding >= TextEncoding::Max)
            return Result::Enum_TextEncoding;
        vText.clear();
        Ctx.rbWork.Clear();

        int cb;
        while (!w.IsEnd())
        {
            if (eEncoding == TextEncoding::Latin1 ||
                eEncoding == TextEncoding::UTF8)
                cb = w.CountStringLengthSafe<char>();
            else
                cb = w.CountStringLengthSafe<WCHAR>() * sizeof(WCHAR);

            ConvertTextEncoding(vText.emplace_back(), w, cb, eEncoding);
            if (!w.IsEnd())
                StepOverTerminator(w, eEncoding);
        }
        return PostDeserialize(rb, Ctx);
    }
    catch (const CMemoryReader::Xpt&)
    {
        return Result::Broken;
    }

    ECK_DECL_ID3FRAME_METHOD_ID(TEXTFRAME)
};

struct TXXX : public FRAME
{
    TextEncoding eEncoding{ TextEncoding::Default };
    CStringW rsDesc{};
    CStringW rsText{};

    Result Serialize(CByteBuffer& rb, const SERIAL_CTX& Ctx) noexcept override
    {
        Ctx.rbWork.Clear();
        CovertTextEncoding(Ctx.rbWork, rsDesc, eEncoding, TRUE);
        CovertTextEncoding(Ctx.rbWork, rsText, eEncoding);
        const size_t cbFrame = 1 + Ctx.rbWork.Size();
        auto w = PreSerialize(rb, Ctx, cbFrame);
        w << eEncoding << Ctx.rbWork;
        return PostSerialize(rb, Ctx, cbFrame);
    }

    Result Deserialize(CByteBuffer& rb, const SERIAL_CTX& Ctx) noexcept override try
    {
        auto w = PreDeserialize(rb, Ctx);
        w >> eEncoding;
        if (eEncoding >= TextEncoding::Max)
            return Result::Enum_TextEncoding;
        ConvertTextEncoding(rsDesc, w, -1, eEncoding);
        ConvertTextEncoding(rsText, w, (int)w.GetRemainingSize(), eEncoding);
        return PostDeserialize(rb, Ctx);
    }
    catch (const CMemoryReader::Xpt&)
    {
        return Result::Broken;
    }

    ECK_DECL_ID3FRAME_METHOD(TXXX)
};

struct LINKFRAME : public FRAME
{
    CStringA rsUrl{};

    Result Serialize(CByteBuffer& rb, const SERIAL_CTX& Ctx) noexcept override
    {
        auto w = PreSerialize(rb, Ctx, rsUrl.Size());
        w.Write(rsUrl.Data(), rsUrl.Size());
        return PostSerialize(rb, Ctx, rsUrl.Size());
    }

    Result Deserialize(CByteBuffer& rb, const SERIAL_CTX& Ctx) noexcept override
    {
        auto w = PreDeserialize(rb, Ctx);
        rsUrl.Assign((PCCH)w.Data(), (int)w.GetRemainingSize());
        return PostDeserialize(rb, Ctx);
    }

    ECK_DECL_ID3FRAME_METHOD_ID(LINKFRAME)
};

struct WXXX : public FRAME
{
    TextEncoding eEncoding{ TextEncoding::Default };
    CStringW rsDesc{};
    CStringA rsUrl{};

    Result Serialize(CByteBuffer& rb, const SERIAL_CTX& Ctx) noexcept override
    {
        Ctx.rbWork.Clear();
        CovertTextEncoding(Ctx.rbWork, rsDesc, eEncoding, TRUE);
        const size_t cbFrame = 1 + Ctx.rbWork.Size() + rsUrl.Size();
        auto w = PreSerialize(rb, Ctx, cbFrame);
        w << eEncoding << Ctx.rbWork;
        w.Write(rsUrl.Data(), rsUrl.Size());
        return PostSerialize(rb, Ctx, cbFrame);
    }

    Result Deserialize(CByteBuffer& rb, const SERIAL_CTX& Ctx) noexcept override try
    {
        auto w = PreDeserialize(rb, Ctx);
        w >> eEncoding;
        if (eEncoding >= TextEncoding::Max)
            return Result::Enum_TextEncoding;
        ConvertTextEncoding(rsDesc, w, -1, eEncoding);
        rsUrl.Assign((PCCH)w.Data(), (int)w.GetRemainingSize());
        return PostDeserialize(rb, Ctx);
    }
    catch (const CMemoryReader::Xpt&)
    {
        return Result::Broken;
    }

    ECK_DECL_ID3FRAME_METHOD(WXXX)
};

struct MCID final : public FRAME
{
    CByteBuffer rbToc{};

    Result Serialize(CByteBuffer& rb, const SERIAL_CTX& Ctx) noexcept override
    {
        auto w = PreSerialize(rb, Ctx, rbToc.Size());
        w << rbToc;
        return PostSerialize(rb, Ctx, rbToc.Size());
    }

    Result Deserialize(CByteBuffer& rb, const SERIAL_CTX& Ctx) noexcept override
    {
        auto w = PreDeserialize(rb, Ctx);
        rbToc.Assign(w.Data(), w.GetRemainingSize());
        return PostDeserialize(rb, Ctx);
    }

    ECK_DECL_ID3FRAME_METHOD(MCID)
};

struct ETCO final : public FRAME
{
    struct EVENT
    {
        EventType eType;
        UINT uTimestamp;
    };

    TimestampFmt eTimestampFmt{};
    std::vector<EVENT> vEvent{};

    Result Serialize(CByteBuffer& rb, const SERIAL_CTX& Ctx) noexcept override
    {
        bFileAlterDiscard = TRUE;

        const size_t cbFrame = 1 + 5 * vEvent.size();
        auto w = PreSerialize(rb, Ctx, cbFrame);
        w << eTimestampFmt;
        for (const auto& e : vEvent)
            (w << e.eType).WriteReversed(e.uTimestamp);
        return PostSerialize(rb, Ctx, cbFrame);
    }

    Result Deserialize(CByteBuffer& rb, const SERIAL_CTX& Ctx) noexcept override try
    {
        auto w = PreDeserialize(rb, Ctx);
        w >> eTimestampFmt;
        if (eTimestampFmt >= TimestampFmt::Max)
            return Result::Enum;
        vEvent.clear();
        while (!w.IsEnd())
        {
            auto& e = vEvent.emplace_back();
            w >> e.eType;
            if (e.eType >= EventType::InvalidEnd)
                return Result::Enum;
            w.ReadReversed(e.uTimestamp);
        }
        return PostDeserialize(rb, Ctx);
    }
    catch (const CMemoryReader::Xpt&)
    {
        return Result::Broken;
    }

    ECK_DECL_ID3FRAME_METHOD(ETCO)
};

// MLLT未实现 

struct SYTC final : public FRAME
{
    struct TEMPO
    {
        USHORT usBpm{};
        UINT uTimestamp{};
    };
    TimestampFmt eTimestampFmt{};
    std::vector<TEMPO> vTempo{};

    Result Serialize(CByteBuffer& rb, const SERIAL_CTX& Ctx) noexcept override
    {
        size_t cbFrame = 1 + vTempo.size() * (4 + 1/*时间戳4B，BPM基础1B*/);
        for (const auto& e : vTempo)
        {
            if (e.usBpm >= 0xFF)// 超出0xFF需要两个字节
                ++cbFrame;
            // 有效范围2 - 510，0和1具有特殊含义
            // 0 = 无节拍时间段，1 = 一个单一节拍后的无节拍时间段
            else if (e.usBpm > 510)
                return Result::Value;
        }

        bFileAlterDiscard = TRUE;
        auto w = PreSerialize(rb, Ctx, cbFrame);
        w << eTimestampFmt;
        for (const auto& e : vTempo)
        {
            if (e.usBpm >= 0xFF)
                w << (BYTE)0xFF << (BYTE)(e.usBpm - 0xFF);
            else
                w << (BYTE)e.usBpm;
            w.WriteReversed(e.uTimestamp);
        }
        return PostSerialize(rb, Ctx, cbFrame);
    }

    Result Deserialize(CByteBuffer& rb, const SERIAL_CTX& Ctx) noexcept override try
    {
        auto w = PreDeserialize(rb, Ctx);
        w >> eTimestampFmt;
        if (eTimestampFmt >= TimestampFmt::Max)
            return Result::Enum;
        vTempo.clear();
        while (!w.IsEnd())
        {
            auto& e = vTempo.emplace_back();
            BYTE byBpm;
            w >> byBpm;
            if (byBpm == 0xFF)
            {
                w >> byBpm;
                e.usBpm = USHORT(byBpm + 0xFF);
            }
            else
                e.usBpm = byBpm;
            w.ReadReversed(e.uTimestamp);
        }
        return PostDeserialize(rb, Ctx);
    }
    catch (const CMemoryReader::Xpt&)
    {
        return Result::Broken;
    }

    ECK_DECL_ID3FRAME_METHOD(SYTC)
};

struct USLT final : public FRAME
{
    TextEncoding eEncoding{ TextEncoding::Default };
    CHAR byLang[3]{ 'X','X','X' };
    CStringW rsDesc{};
    CStringW rsLrc{};

    Result Serialize(CByteBuffer& rb, const SERIAL_CTX& Ctx) noexcept override
    {
        CheckLanguageId(byLang);

        Ctx.rbWork.Clear();
        CovertTextEncoding(Ctx.rbWork, rsDesc, eEncoding, TRUE);
        CovertTextEncoding(Ctx.rbWork, rsLrc, eEncoding);
        const size_t cbFrame = 4 + Ctx.rbWork.Size();
        auto w = PreSerialize(rb, Ctx, cbFrame);
        w << eEncoding << byLang << Ctx.rbWork;
        return PostSerialize(rb, Ctx, cbFrame);
    }

    Result Deserialize(CByteBuffer& rb, const SERIAL_CTX& Ctx) noexcept override try
    {
        auto w = PreDeserialize(rb, Ctx);
        w >> eEncoding;
        if (eEncoding >= TextEncoding::Max)
            return Result::Enum_TextEncoding;
        w.Read(byLang, 3);
        ConvertTextEncoding(rsDesc, w, -1, eEncoding);
        ConvertTextEncoding(rsLrc, w, (int)w.GetRemainingSize(), eEncoding);
        return PostDeserialize(rb, Ctx);
    }
    catch (const CMemoryReader::Xpt&)
    {
        return Result::Broken;
    }

    ECK_DECL_ID3FRAME_METHOD(USLT)
};

struct SYLT final : public FRAME
{
    struct SYNC
    {
        CStringW rsText{};
        UINT uTimestamp{};
    };

    TextEncoding eEncoding{ TextEncoding::Default };
    CHAR byLang[3]{ 'X','X','X' };
    TimestampFmt eTimestampFmt{};
    LrcContentType eContent{ LrcContentType::Lyrics };
    CStringW rsDesc{};
    std::vector<SYNC> vSync{};

    Result Serialize(CByteBuffer& rb, const SERIAL_CTX& Ctx) noexcept override
    {
        bFileAlterDiscard = TRUE;
        CheckLanguageId(byLang);

        std::vector<std::pair<size_t, size_t>> vRange(vSync.size() + 1);
        Ctx.rbWork.Clear();
        // 描述
        CovertTextEncoding(Ctx.rbWork, rsDesc, eEncoding, TRUE);
        vRange[0] = { 0u, Ctx.rbWork.Size() };
        // 歌词
        EckCounter(vSync.size(), i)
        {
            const auto pos = Ctx.rbWork.Size();
            CovertTextEncoding(Ctx.rbWork, vSync[i].rsText, eEncoding, TRUE);
            vRange[i + 1] = { pos, Ctx.rbWork.Size() };
        }

        const size_t cbFrame = 6 + vSync.size() * 4 + Ctx.rbWork.Size();
        auto w = PreSerialize(rb, Ctx, cbFrame);
        w << eEncoding << byLang << eTimestampFmt << eContent;
        w << Ctx.rbWork.SubSpan(vRange[0].first, vRange[0].second - vRange[0].first);
        EckCounter(vSync.size(), i)
        {
            const auto r = vRange[i + 1];
            w << Ctx.rbWork.SubSpan(r.first, r.second - r.first);
            w.WriteReversed(vSync[i].uTimestamp);
        }
        return PostSerialize(rb, Ctx, cbFrame);
    }

    Result Deserialize(CByteBuffer& rb, const SERIAL_CTX& Ctx) noexcept override try
    {
        auto w = PreDeserialize(rb, Ctx);
        w >> eEncoding;
        if (eEncoding >= TextEncoding::Max)
            return Result::Enum_TextEncoding;
        w.Read(byLang, 3);
        w >> eTimestampFmt;
        if (eTimestampFmt >= TimestampFmt::Max)
            return Result::Enum;
        w >> eContent;
        if (eContent >= LrcContentType::Max)
            return Result::Enum;
        ConvertTextEncoding(rsDesc, w, -1, eEncoding);
        vSync.clear();
        while (!w.IsEnd())
        {
            SYNC sync;
            ConvertTextEncoding(sync.rsText, w, -1, eEncoding);
            w.ReadReversed(sync.uTimestamp);
            vSync.push_back(std::move(sync));
        }
        return PostDeserialize(rb, Ctx);
    }
    catch (const CMemoryReader::Xpt&)
    {
        return Result::Broken;
    }

    ECK_DECL_ID3FRAME_METHOD(SYLT)
};

struct COMM final : public FRAME
{
    TextEncoding eEncoding{ TextEncoding::Default };
    CHAR byLang[3]{ 'X','X','X' };
    CStringW rsDesc{};
    CStringW rsText{};

    Result Serialize(CByteBuffer& rb, const SERIAL_CTX& Ctx) noexcept override
    {
        CheckLanguageId(byLang);

        Ctx.rbWork.Clear();
        CovertTextEncoding(Ctx.rbWork, rsDesc, eEncoding, TRUE);
        CovertTextEncoding(Ctx.rbWork, rsText, eEncoding);
        const size_t cbFrame = 4 + Ctx.rbWork.Size();
        auto w = PreSerialize(rb, Ctx, cbFrame);
        w << eEncoding << byLang << Ctx.rbWork;
        return PostSerialize(rb, Ctx, cbFrame);
    }

    Result Deserialize(CByteBuffer& rb, const SERIAL_CTX& Ctx) noexcept override try
    {
        auto w = PreDeserialize(rb, Ctx);
        w >> eEncoding;
        if (eEncoding >= TextEncoding::Max)
            return Result::Enum_TextEncoding;
        w.Read(byLang, 3);
        ConvertTextEncoding(rsDesc, w, -1, eEncoding);
        ConvertTextEncoding(rsText, w, (int)w.GetRemainingSize(), eEncoding);
        return PostDeserialize(rb, Ctx);
    }
    catch (const CMemoryReader::Xpt&)
    {
        return Result::Broken;
    }

    ECK_DECL_ID3FRAME_METHOD(COMM)
};

struct RVA2 final : public FRAME
{
    struct CHANNEL
    {
        ChannelType eChannel{};
        BYTE cPeekVolBit{};
        short shVol{};
        CBitSet<256> bsPeekVol{};
    };

    CStringA rsId{};
    std::vector<CHANNEL> vChannel{};

    Result Serialize(CByteBuffer& rb, const SERIAL_CTX& Ctx) noexcept override
    {
        bFileAlterDiscard = TRUE;

        size_t cbFrame = rsId.Size() + 1 + vChannel.size() * 4;
        for (const auto& e : vChannel)
            cbFrame += CeilDivide<UINT>(e.cPeekVolBit, 8);

        auto w = PreSerialize(rb, Ctx, cbFrame);
        w << rsId;
        for (const auto& e : vChannel)
        {
            w << e.eChannel;
            w.WriteReversed(e.shVol);
            w << e.cPeekVolBit;
            w.WriteReversed(e.bsPeekVol.Data(), CeilDivide<UINT>(e.cPeekVolBit, 8));
        }
        return PostSerialize(rb, Ctx, cbFrame);
    }

    Result Deserialize(CByteBuffer& rb, const SERIAL_CTX& Ctx) noexcept override try
    {
        auto w = PreDeserialize(rb, Ctx);
        w >> rsId;
        vChannel.clear();
        while (!w.IsEnd())
        {
            auto& e = vChannel.emplace_back();
            w >> e.eChannel;
            if (e.eChannel >= ChannelType::Max)
                return Result::Enum;
            w.ReadReversed(e.shVol);
            w >> e.cPeekVolBit;
            w.ReadReversed(e.bsPeekVol.Data(), CeilDivide<UINT>(e.cPeekVolBit, 8));
        }
        return PostDeserialize(rb, Ctx);
    }
    catch (const CMemoryReader::Xpt&)
    {
        return Result::Broken;
    }

    ECK_DECL_ID3FRAME_METHOD(RVA2)
};

struct EQU2 final : public FRAME
{
    struct POINT
    {
        USHORT uFreq{};	// 单位1/2Hz
        short shVol{};
    };

    Interpolation eInterpolation{};
    CStringA rsId{};
    std::vector<POINT> vPoint{};

    Result Serialize(CByteBuffer& rb, const SERIAL_CTX& Ctx) noexcept override
    {
        bFileAlterDiscard = TRUE;

        const size_t cbFrame = 1 + rsId.Size() + 1 + vPoint.size() * 4;
        auto w = PreSerialize(rb, Ctx, cbFrame);
        w << eInterpolation << rsId;
        for (auto e : vPoint)
            w.WriteReversed(e.uFreq).WriteReversed(e.shVol);
        return PostSerialize(rb, Ctx, cbFrame);
    }

    Result Deserialize(CByteBuffer& rb, const SERIAL_CTX& Ctx) noexcept override try
    {
        auto w = PreDeserialize(rb, Ctx);
        w >> eInterpolation;
        if (eInterpolation >= Interpolation::Max)
            return Result::Enum;
        w >> rsId;
        vPoint.clear();
        while (!w.IsEnd())
        {
            auto& e = vPoint.emplace_back();
            w.ReadReversed(e.uFreq);
            w.ReadReversed(e.shVol);
        }
        return PostDeserialize(rb, Ctx);
    }
    catch (const CMemoryReader::Xpt&)
    {
        return Result::Broken;
    }

    ECK_DECL_ID3FRAME_METHOD(EQU2)
};

struct RVRB final : public FRAME
{
    USHORT Left{};
    USHORT Right{};
    BYTE BouncesLeft{};
    BYTE BouncesRight{};
    BYTE FeedbackLeftToLeft{};
    BYTE FeedbackLeftToRight{};
    BYTE FeedbackRightToRight{};
    BYTE FeedbackRightToLeft{};
    BYTE PremixLeftToRight{};
    BYTE PremixRightToLeft{};

    Result Serialize(CByteBuffer& rb, const SERIAL_CTX& Ctx) noexcept override
    {
        constexpr size_t cbFrame = 12;
        auto w = PreSerialize(rb, Ctx, cbFrame);
        w.WriteReversed(Left).WriteReversed(Right).Write(&BouncesLeft, 8);
        return PostSerialize(rb, Ctx, cbFrame);
    }

    Result Deserialize(CByteBuffer& rb, const SERIAL_CTX& Ctx) noexcept override try
    {
        auto w = PreDeserialize(rb, Ctx);
        w.ReadReversed(Left).ReadReversed(Right).Read(&BouncesLeft, 8);
        return PostDeserialize(rb, Ctx);
    }
    catch (const CMemoryReader::Xpt&)
    {
        return Result::Broken;
    }

    ECK_DECL_ID3FRAME_METHOD(RVRB)
};

struct APIC final : public FRAME
{
    TextEncoding eEncoding{ TextEncoding::Default };
    PicType eType{ PicType::CoverFront };
    CStringA rsMime{};
    CStringW rsDesc{};
    CByteBuffer rbData{};

    Result Serialize(CByteBuffer& rb, const SERIAL_CTX& Ctx) noexcept override
    {
        Ctx.rbWork.Clear();
        CovertTextEncoding(Ctx.rbWork, rsDesc, eEncoding, TRUE);
        const size_t cbFrame = 2 + rsMime.Size() + 1 +
            Ctx.rbWork.Size() + rbData.Size();
        auto w = PreSerialize(rb, Ctx, cbFrame);
        w << eEncoding << rsMime << eType << Ctx.rbWork << rbData;
        return PostSerialize(rb, Ctx, cbFrame);
    }

    Result Deserialize(CByteBuffer& rb, const SERIAL_CTX& Ctx) noexcept override try
    {
        auto w = PreDeserialize(rb, Ctx);
        w >> eEncoding;
        if (eEncoding >= TextEncoding::Max)
            return Result::Enum_TextEncoding;
        w >> rsMime >> eType;
        ConvertTextEncoding(rsDesc, w, -1, eEncoding);
        rbData.Assign(w.Data(), w.GetRemainingSize());
        return PostDeserialize(rb, Ctx);
    }
    catch (const CMemoryReader::Xpt&)
    {
        return Result::Broken;
    }

    ECK_DECL_ID3FRAME_METHOD(APIC)
};

struct GEOB final : public FRAME
{
    TextEncoding eEncoding{ TextEncoding::Default };
    CStringA rsMime{};
    CStringW rsFile{};
    CStringW rsDesc{};
    CByteBuffer rbObj{};

    Result Serialize(CByteBuffer& rb, const SERIAL_CTX& Ctx) noexcept override
    {
        Ctx.rbWork.Clear();
        CovertTextEncoding(Ctx.rbWork, rsFile, eEncoding, TRUE);
        CovertTextEncoding(Ctx.rbWork, rsDesc, eEncoding, TRUE);
        const size_t cbFrame = 1 + rsMime.Size() + 1 +
            Ctx.rbWork.Size() + rbObj.Size();
        auto w = PreSerialize(rb, Ctx, cbFrame);
        w << eEncoding << rsMime << Ctx.rbWork << rbObj;
        return PostSerialize(rb, Ctx, cbFrame);
    }

    Result Deserialize(CByteBuffer& rb, const SERIAL_CTX& Ctx) noexcept override try
    {
        auto w = PreDeserialize(rb, Ctx);
        w >> eEncoding;
        if (eEncoding >= TextEncoding::Max)
            return Result::Enum_TextEncoding;
        w >> rsMime;
        ConvertTextEncoding(rsFile, w, -1, eEncoding);
        ConvertTextEncoding(rsDesc, w, -1, eEncoding);
        rbObj.Assign(w.Data(), w.GetRemainingSize());
        return PostDeserialize(rb, Ctx);
    }
    catch (const CMemoryReader::Xpt&)
    {
        return Result::Broken;
    }

    ECK_DECL_ID3FRAME_METHOD(GEOB)
};

struct PCNT final : public FRAME
{
    ULONGLONG cPlay{};

    Result Serialize(CByteBuffer& rb, const SERIAL_CTX& Ctx) noexcept override
    {
        size_t cbFrame = 4;
        for (int i = 7; i >= 4; --i)
        {
            if (GetIntegerByte(cPlay, i))
            {
                cbFrame = i + 1;
                break;
            }
        }
        auto w = PreSerialize(rb, Ctx, cbFrame);
        w.WriteReversed(&cPlay, cbFrame);
        return PostSerialize(rb, Ctx, cbFrame);
    }

    Result Deserialize(CByteBuffer& rb, const SERIAL_CTX& Ctx) noexcept override
    {
        auto w = PreDeserialize(rb, Ctx);
        cPlay = 0;
        w.ReadReversed(&cPlay, std::min(w.GetRemainingSize(), (size_t)8));
        return PostDeserialize(rb, Ctx);
    }

    ECK_DECL_ID3FRAME_METHOD(PCNT)
};

struct POPM final : public FRAME
{
    CStringA rsEmail{};
    BYTE byRating{};
    ULONGLONG cPlay{};

    Result Serialize(CByteBuffer& rb, const SERIAL_CTX& Ctx) noexcept override
    {
        size_t cbPlayCount = 4;
        for (int i = 7; i >= 4; --i)
        {
            if (GetIntegerByte(cPlay, i))
            {
                cbPlayCount = i + 1;
                break;
            }
        }
        const size_t cbFrame = rsEmail.Size() + 1 + 1 + cbPlayCount;
        auto w = PreSerialize(rb, Ctx, cbFrame);
        w << rsEmail << byRating;
        w.WriteReversed(&cPlay, cbPlayCount);
        return PostSerialize(rb, Ctx, cbFrame);
    }

    Result Deserialize(CByteBuffer& rb, const SERIAL_CTX& Ctx) noexcept override try
    {
        auto w = PreDeserialize(rb, Ctx);
        w >> rsEmail >> byRating;
        cPlay = 0;
        w.ReadReversed(&cPlay, std::min(w.GetRemainingSize(), (size_t)8));
        return PostDeserialize(rb, Ctx);
    }
    catch (const CMemoryReader::Xpt&)
    {
        return Result::Broken;
    }

    ECK_DECL_ID3FRAME_METHOD(POPM)
};

struct RBUF final : public FRAME
{
    UINT cbBuf{};
    BYTE b{};
    UINT ocbNextTag{};

    Result Serialize(CByteBuffer& rb, const SERIAL_CTX& Ctx) noexcept override
    {
        if (b != 0 && b != 1)
            return Result::ReservedData;
        constexpr size_t cbFrame = 8;
        auto w = PreSerialize(rb, Ctx, cbFrame);
        w.WriteReversed(&cbBuf, 3) << b;
        w.WriteReversed(ocbNextTag);
        return PostSerialize(rb, Ctx, cbFrame);
    }

    Result Deserialize(CByteBuffer& rb, const SERIAL_CTX& Ctx) noexcept override try
    {
        auto w = PreDeserialize(rb, Ctx);
        cbBuf = 0;
        w.ReadReversed(&cbBuf, 3) >> b;
        if (b != 0 && b != 1)
            return Result::ReservedData;
        w.ReadReversed(ocbNextTag);
        return PostDeserialize(rb, Ctx);
    }
    catch (const CMemoryReader::Xpt&)
    {
        return Result::Broken;
    }

    ECK_DECL_ID3FRAME_METHOD(RBUF)
};

struct AENC final : public FRAME
{
    CStringA rsOwnerId{};
    USHORT usPreviewBegin{};
    USHORT usPreviewLength{};
    CByteBuffer rbData{};

    Result Serialize(CByteBuffer& rb, const SERIAL_CTX& Ctx) noexcept override
    {
        bFileAlterDiscard = TRUE;

        const size_t cbFrame = rsOwnerId.Size() + 1 + 4 + rbData.Size();
        auto w = PreSerialize(rb, Ctx, cbFrame);
        w << rsOwnerId;
        w.WriteReversed(usPreviewBegin).WriteReversed(usPreviewLength);
        w << rbData;
        return PostSerialize(rb, Ctx, cbFrame);
    }

    Result Deserialize(CByteBuffer& rb, const SERIAL_CTX& Ctx) noexcept override try
    {
        auto w = PreDeserialize(rb, Ctx);
        w >> rsOwnerId;
        w.ReadReversed(usPreviewBegin).ReadReversed(usPreviewLength);
        rbData.Assign(w.Data(), w.GetRemainingSize());
        return PostDeserialize(rb, Ctx);
    }
    catch (const CMemoryReader::Xpt&)
    {
        return Result::Broken;
    }

    ECK_DECL_ID3FRAME_METHOD(AENC)
};

struct LINK final : public FRAME
{
    CHAR szIdTarget[4]{};
    CStringA rsUrl{};
    CStringA rsAdditional{};

    Result Serialize(CByteBuffer& rb, const SERIAL_CTX& Ctx) noexcept override
    {
        const size_t cbFrame = 4 + rsUrl.Size() + 1 + rsUrl.Size();
        auto w = PreSerialize(rb, Ctx, cbFrame);
        w << szIdTarget << rsUrl;
        w.Write(rsAdditional.Data(), rsAdditional.Size());
        return PostSerialize(rb, Ctx, cbFrame);
    }

    Result Deserialize(CByteBuffer& rb, const SERIAL_CTX& Ctx) noexcept override try
    {
        auto w = PreDeserialize(rb, Ctx);
        w.Read(szIdTarget, 4);
        w >> rsUrl;
        rsAdditional.Assign((PCCH)w.Data(), (int)w.GetRemainingSize());
        return PostDeserialize(rb, Ctx);
    }
    catch (const CMemoryReader::Xpt&)
    {
        return Result::Broken;
    }

    ECK_DECL_ID3FRAME_METHOD(LINK)
};

struct POSS final : public FRAME
{
    TimestampFmt eTimestamp{};
    UINT uTime{};

    Result Serialize(CByteBuffer& rb, const SERIAL_CTX& Ctx) noexcept override
    {
        bFileAlterDiscard = TRUE;

        constexpr size_t cbFrame = 5;
        auto w = PreSerialize(rb, Ctx, cbFrame);
        w << eTimestamp;
        w.WriteReversed(uTime);
        return PostSerialize(rb, Ctx, cbFrame);
    }

    Result Deserialize(CByteBuffer& rb, const SERIAL_CTX& Ctx) noexcept override try
    {
        auto w = PreDeserialize(rb, Ctx);
        w >> eTimestamp;
        if (eTimestamp >= TimestampFmt::Max)
            return Result::Enum;
        w.ReadReversed(uTime);
        return PostDeserialize(rb, Ctx);
    }
    catch (const CMemoryReader::Xpt&)
    {
        return Result::Broken;
    }

    ECK_DECL_ID3FRAME_METHOD(POSS)
};

struct USER final : public FRAME
{
    TextEncoding eEncoding{ TextEncoding::Default };
    CHAR byLang[3]{ 'X','X','X' };
    CStringW rsText{};

    Result Serialize(CByteBuffer& rb, const SERIAL_CTX& Ctx) noexcept override
    {
        Ctx.rbWork.Clear();
        CovertTextEncoding(Ctx.rbWork, rsText, eEncoding);
        const size_t cbFrame = 4 + Ctx.rbWork.Size();
        auto w = PreSerialize(rb, Ctx, cbFrame);
        w << eEncoding << byLang << Ctx.rbWork;
        return PostSerialize(rb, Ctx, cbFrame);
    }

    Result Deserialize(CByteBuffer& rb, const SERIAL_CTX& Ctx) noexcept override try
    {
        auto w = PreDeserialize(rb, Ctx);
        w >> eEncoding;
        if (eEncoding >= TextEncoding::Max)
            return Result::Enum_TextEncoding;
        w.Read(byLang, 3);
        ConvertTextEncoding(rsText, w, (int)w.GetRemainingSize(), eEncoding);
        return PostDeserialize(rb, Ctx);
    }
    catch (const CMemoryReader::Xpt&)
    {
        return Result::Broken;
    }

    ECK_DECL_ID3FRAME_METHOD(USER)
};

struct OWNE final : public FRAME
{
    TextEncoding eEncoding{ TextEncoding::Default };
    CStringA rsPrice{};
    CHAR szDate[8]{};
    CStringW rsSeller{};

    Result Serialize(CByteBuffer& rb, const SERIAL_CTX& Ctx) noexcept override
    {
        Ctx.rbWork.Clear();
        CovertTextEncoding(Ctx.rbWork, rsSeller, eEncoding);
        const size_t cbFrame = 10 + rsPrice.Size() + Ctx.rbWork.Size();
        auto w = PreSerialize(rb, Ctx, cbFrame);
        w << eEncoding << rsPrice << szDate << Ctx.rbWork;
        return PostSerialize(rb, Ctx, cbFrame);
    }

    Result Deserialize(CByteBuffer& rb, const SERIAL_CTX& Ctx) noexcept override try
    {
        auto w = PreDeserialize(rb, Ctx);
        w >> eEncoding;
        if (eEncoding >= TextEncoding::Max)
            return Result::Enum_TextEncoding;
        w >> rsPrice >> szDate;
        ConvertTextEncoding(rsSeller, w, (int)w.GetRemainingSize(), eEncoding);
        return PostDeserialize(rb, Ctx);
    }
    catch (const CMemoryReader::Xpt&)
    {
        return Result::Broken;
    }

    ECK_DECL_ID3FRAME_METHOD(OWNE)
};

struct COMR final : public FRAME
{
    TextEncoding eEncoding{ TextEncoding::Default };
    ReceivedWay eReceivedWay{};
    CStringA rsPrice{};
    CHAR szDate[8]{};
    CStringA rsUrl{};
    CStringW rsSeller{};
    CStringW rsDesc{};
    CStringA rsMime{};
    CByteBuffer rbLogo{};

    Result Serialize(CByteBuffer& rb, const SERIAL_CTX& Ctx) noexcept override
    {
        Ctx.rbWork.Clear();
        CovertTextEncoding(Ctx.rbWork, rsSeller, eEncoding, TRUE);
        CovertTextEncoding(Ctx.rbWork, rsDesc, eEncoding, TRUE);

        const size_t cbFrame = 1 + rsPrice.Size() + 1 + 8 +
            rsUrl.Size() + 1 + 1 + Ctx.rbWork.Size() +
            rsMime.Size() + 1 + rbLogo.Size();
        auto w = PreSerialize(rb, Ctx, cbFrame);
        w << eEncoding << rsPrice << szDate << rsUrl
            << eReceivedWay << Ctx.rbWork << rsMime << rbLogo;
        return PostSerialize(rb, Ctx, cbFrame);
    }

    Result Deserialize(CByteBuffer& rb, const SERIAL_CTX& Ctx) noexcept override try
    {
        auto w = PreDeserialize(rb, Ctx);
        w >> eEncoding;
        if (eEncoding >= TextEncoding::Max)
            return Result::Enum_TextEncoding;
        w >> rsPrice >> szDate >> rsUrl >> eReceivedWay;
        if (eReceivedWay >= ReceivedWay::Max)
            return Result::Enum;
        ConvertTextEncoding(rsSeller, w, -1, eEncoding);
        ConvertTextEncoding(rsDesc, w, -1, eEncoding);
        w >> rsMime;
        rbLogo.Assign(w.Data(), w.GetRemainingSize());
        return PostDeserialize(rb, Ctx);
    }
    catch (const CMemoryReader::Xpt&)
    {
        return Result::Broken;
    }

    ECK_DECL_ID3FRAME_METHOD(COMR)
};

struct ENCR final : public FRAME
{
    CStringA rsEmail{};
    BYTE byMethod{};
    CByteBuffer rbData{};

    Result Serialize(CByteBuffer& rb, const SERIAL_CTX& Ctx) noexcept override
    {
        const size_t cbFrame = 1 + rsEmail.Size() + 1 + rbData.Size();
        auto w = PreSerialize(rb, Ctx, cbFrame);
        w << rsEmail << byMethod << rbData;
        return PostSerialize(rb, Ctx, cbFrame);
    }

    Result Deserialize(CByteBuffer& rb, const SERIAL_CTX& Ctx) noexcept override try
    {
        auto w = PreDeserialize(rb, Ctx);
        w >> rsEmail >> byMethod;
        rbData.Assign(w.Data(), w.GetRemainingSize());
        return PostDeserialize(rb, Ctx);
    }
    catch (const CMemoryReader::Xpt&)
    {
        return Result::Broken;
    }

    ECK_DECL_ID3FRAME_METHOD(ENCR)
};

struct GRID final : public FRAME
{
    CStringA rsEmail{};
    BYTE byId{};
    CByteBuffer rbData{};

    Result Serialize(CByteBuffer& rb, const SERIAL_CTX& Ctx) noexcept override
    {
        const size_t cbFrame = 1 + rsEmail.Size() + 1 + rbData.Size();
        auto w = PreSerialize(rb, Ctx, cbFrame);
        w << rsEmail << byId << rbData;
        return PostSerialize(rb, Ctx, cbFrame);
    }

    Result Deserialize(CByteBuffer& rb, const SERIAL_CTX& Ctx) noexcept override try
    {
        auto w = PreDeserialize(rb, Ctx);
        w >> rsEmail >> byId;
        rbData.Assign(w.Data(), w.GetRemainingSize());
        return PostDeserialize(rb, Ctx);
    }
    catch (const CMemoryReader::Xpt&)
    {
        return Result::Broken;
    }

    ECK_DECL_ID3FRAME_METHOD(GRID)
};

struct PRIV final : public FRAME
{
    CStringA rsEmail{};
    CByteBuffer rbData{};

    Result Serialize(CByteBuffer& rb, const SERIAL_CTX& Ctx) noexcept override
    {
        const size_t cbFrame = rsEmail.Size() + 1 + rbData.Size();
        auto w = PreSerialize(rb, Ctx, cbFrame);
        w << rsEmail << rbData;
        return PostSerialize(rb, Ctx, cbFrame);
    }

    Result Deserialize(CByteBuffer& rb, const SERIAL_CTX& Ctx) noexcept override try
    {
        auto w = PreDeserialize(rb, Ctx);
        w >> rsEmail;
        rbData.Assign(w.Data(), w.GetRemainingSize());
        return PostDeserialize(rb, Ctx);
    }
    catch (const CMemoryReader::Xpt&)
    {
        return Result::Broken;
    }

    ECK_DECL_ID3FRAME_METHOD(PRIV)
};

struct SIGN final : public FRAME
{
    BYTE byGroupId{};
    CByteBuffer rbData{};

    Result Serialize(CByteBuffer& rb, const SERIAL_CTX& Ctx) noexcept override
    {
        const size_t cbFrame = 1 + rbData.Size();
        auto w = PreSerialize(rb, Ctx, cbFrame);
        w << byGroupId << rbData;
        return PostSerialize(rb, Ctx, cbFrame);
    }

    Result Deserialize(CByteBuffer& rb, const SERIAL_CTX& Ctx) noexcept override try
    {
        auto w = PreDeserialize(rb, Ctx);
        w >> byGroupId;
        rbData.Assign(w.Data(), w.GetRemainingSize());
        return PostDeserialize(rb, Ctx);
    }
    catch (const CMemoryReader::Xpt&)
    {
        return Result::Broken;
    }

    ECK_DECL_ID3FRAME_METHOD(SIGN)
};

struct SEEK final : public FRAME
{
    UINT ocbNextTag{};

    Result Serialize(CByteBuffer& rb, const SERIAL_CTX& Ctx) noexcept override
    {
        bFileAlterDiscard = TRUE;

        constexpr size_t cbFrame = 4;
        auto w = PreSerialize(rb, Ctx, cbFrame);
        w.WriteReversed(ocbNextTag);
        return PostSerialize(rb, Ctx, cbFrame);
    }

    Result Deserialize(CByteBuffer& rb, const SERIAL_CTX& Ctx) noexcept override try
    {
        auto w = PreDeserialize(rb, Ctx);
        w.ReadReversed(ocbNextTag);
        return PostDeserialize(rb, Ctx);
    }
    catch (const CMemoryReader::Xpt&)
    {
        return Result::Broken;
    }

    ECK_DECL_ID3FRAME_METHOD(SEEK)
};

struct ASPI final : public FRAME
{
    UINT S{};
    UINT L{};
    USHORT N{};
    BYTE b{};
    std::vector<USHORT> vIndex{};

    Result Serialize(CByteBuffer& rb, const SERIAL_CTX& Ctx) noexcept override
    {
        if (N != 8 && N != 16)
            return Result::Value;
        bFileAlterDiscard = TRUE;

        const size_t cbFrame = 11 + vIndex.size() * 2;
        auto w = PreSerialize(rb, Ctx, cbFrame);
        w.WriteReversed(S).WriteReversed(L).WriteReversed(N) << b;
        for (const auto e : vIndex)
            w.WriteReversed(&e, b / 8);
        return PostSerialize(rb, Ctx, cbFrame);
    }

    Result Deserialize(CByteBuffer& rb, const SERIAL_CTX& Ctx) noexcept override try
    {
        auto w = PreDeserialize(rb, Ctx);
        w.ReadReversed(S).ReadReversed(L).ReadReversed(N) >> b;
        if (N != 8 && N != 16)
            return Result::Value;
        vIndex.clear();
        while (!w.IsEnd())
        {
            USHORT us{};
            w.ReadReversed(&us, N / 8);
            vIndex.push_back(us);
        }
        return PostDeserialize(rb, Ctx);
    }
    catch (const CMemoryReader::Xpt&)
    {
        return Result::Broken;
    }

    ECK_DECL_ID3FRAME_METHOD(ASPI)
};

struct OTHERFRAME : public FRAME
{
    CByteBuffer rbData;

    Result Serialize(CByteBuffer& rb, const SERIAL_CTX& Ctx) noexcept override
    {
        const size_t cbFrame = rbData.Size();
        auto w = PreSerialize(rb, Ctx, cbFrame);
        w << rbData;
        return PostSerialize(rb, Ctx, cbFrame);
    }

    Result Deserialize(CByteBuffer& rb, const SERIAL_CTX& Ctx) noexcept override
    {
        auto w = PreDeserialize(rb, Ctx);
        rbData.Assign(w.Data(), w.GetRemainingSize());
        return PostDeserialize(rb, Ctx);
    }

    ECK_DECL_ID3FRAME_METHOD_ID(OTHERFRAME)
};

//---------v2.3---------

struct IPLS final :public FRAME
{
    struct MAP
    {
        CStringW rsPosition{};
        CStringW rsName{};
    };
    TextEncoding eEncoding{ TextEncoding::Default };
    std::vector<MAP> vMap{};

    Result Serialize(CByteBuffer& rb, const SERIAL_CTX& Ctx) noexcept override
    {
        if (vMap.empty())
            return Result::EmptyData;
        Ctx.rbWork.Clear();
        int cchOrg{};
        for (const auto& e : vMap)
        {
            cchOrg += (e.rsPosition.Size() + e.rsName.Size());
            CovertTextEncoding(Ctx.rbWork, e.rsPosition, eEncoding, TRUE);
            CovertTextEncoding(Ctx.rbWork, e.rsName, eEncoding, TRUE);
        }
        if (!cchOrg)
            return Result::EmptyData;
        if (!vMap.empty())
            Ctx.rbWork.PopBack();// 弹掉最后一个结尾NULL

        const size_t cbFrame = 1 + Ctx.rbWork.Size();
        auto w = PreSerialize(rb, Ctx, cbFrame);
        w << eEncoding << Ctx.rbWork;
        return PostSerialize(rb, Ctx, cbFrame);
    }

    Result Deserialize(CByteBuffer& rb, const SERIAL_CTX& Ctx) noexcept override try
    {
        auto w = PreDeserialize(rb, Ctx);
        w >> eEncoding;
        if (eEncoding >= TextEncoding::Max)
            return Result::Enum_TextEncoding;
        vMap.clear();
        int cb;
        while (!w.IsEnd())
        {
            if (eEncoding == TextEncoding::Latin1 ||
                eEncoding == TextEncoding::UTF8)
                cb = w.CountStringLengthSafe<char>();
            else
                cb = w.CountStringLengthSafe<WCHAR>() * sizeof(WCHAR);

            auto& e = vMap.emplace_back();
            ConvertTextEncoding(e.rsPosition, w, cb, eEncoding);
            StepOverTerminator(w, eEncoding);

            if (eEncoding == TextEncoding::Latin1 ||
                eEncoding == TextEncoding::UTF8)
                cb = w.CountStringLengthSafe<char>();
            else
                cb = w.CountStringLengthSafe<WCHAR>() * sizeof(WCHAR);
            if (!cb)
            {
                vMap.pop_back();
                break;
            }
            ConvertTextEncoding(e.rsName, w, cb, eEncoding);

            if (!w.IsEnd())
                StepOverTerminator(w, eEncoding);
        }
        return PostDeserialize(rb, Ctx);
    }
    catch (const CMemoryReader::Xpt&)
    {
        return Result::Broken;
    }

    ECK_DECL_ID3FRAME_METHOD(IPLS)
};

struct RVAD final : public FRAME
{
    BYTE byCtrl{};
    BYTE cBits{};
    BOOLEAN bHasBack{};
    BOOLEAN bHasCenter{};
    BOOLEAN bHasBass{};
    ULONGLONG VolRight{};       // 右调整
    ULONGLONG VolLeft{};        // 左调整
    ULONGLONG PeakRight{};      // 右峰值
    ULONGLONG PeakLeft{};       // 左峰值
    ULONGLONG VolRightBack{};   // 右后调整
    ULONGLONG VolLeftBack{};    // 左后调整
    ULONGLONG PeakRightBack{};  // 右后峰值
    ULONGLONG PeakLeftBack{};   // 左后峰值
    ULONGLONG VolCenter{};      // 中央调整
    ULONGLONG PeakCenter{};     // 中央峰值
    ULONGLONG VolBass{};        // 低音调整
    ULONGLONG PeakBass{};       // 低音峰值

    Result Serialize(CByteBuffer& rb, const SERIAL_CTX& Ctx) noexcept override
    {
        if (byCtrl & 0b1100'0000)
            return Result::ReservedData;
        if (cBits > 64)
            return Result::Value;
        bFileAlterDiscard = TRUE;

        const size_t cbField = CeilDivide<UINT>(cBits, 8);
        size_t cbFrame = 2 + (cbField * 4);
        if (bHasBack)
            cbFrame += (cbField * 4);
        if (bHasCenter)
            cbFrame += (cbField * 2);
        if (bHasBass)
            cbFrame += (cbField * 2);

        auto w = PreSerialize(rb, Ctx, cbFrame);
        w << byCtrl << cBits;
        w.WriteReversed(&VolRight, cbField);
        w.WriteReversed(&VolLeft, cbField);
        w.WriteReversed(&PeakRight, cbField);
        w.WriteReversed(&PeakLeft, cbField);
        if (bHasBack)
        {
            w.WriteReversed(&VolRightBack, cbField);
            w.WriteReversed(&VolLeftBack, cbField);
            w.WriteReversed(&PeakRightBack, cbField);
            w.WriteReversed(&PeakLeftBack, cbField);
        }
        if (bHasCenter)
        {
            w.WriteReversed(&VolCenter, cbField);
            w.WriteReversed(&PeakCenter, cbField);
        }
        if (bHasBass)
        {
            w.WriteReversed(&VolBass, cbField);
            w.WriteReversed(&PeakBass, cbField);
        }
        return PostSerialize(rb, Ctx, cbFrame);
    }

    Result Deserialize(CByteBuffer& rb, const SERIAL_CTX& Ctx) noexcept override try
    {
        auto w = PreDeserialize(rb, Ctx);
        w >> byCtrl >> cBits;
        if (byCtrl & 0b1100'0000)
            return Result::ReservedData;
        if (cBits > 64)
            return Result::Value;
        bHasBack = bHasCenter = bHasBass = FALSE;
        VolRight = VolLeft = PeakRight = PeakLeft = 0;
        VolRightBack = VolLeftBack = PeakRightBack = PeakLeftBack = 0;
        VolCenter = PeakCenter = VolBass = PeakBass = 0;

        const size_t cbField = CeilDivide<UINT>(cBits, 8);
        w.ReadReversed(&VolRight, cbField);
        w.ReadReversed(&VolLeft, cbField);
        if (!w.IsEnd())
        {
            w.ReadReversed(&PeakRight, cbField);
            w.ReadReversed(&PeakLeft, cbField);
        }
        if (!w.IsEnd())
        {
            bHasBack = TRUE;
            w.ReadReversed(&VolRightBack, cbField);
            w.ReadReversed(&VolLeftBack, cbField);
            w.ReadReversed(&PeakRightBack, cbField);
            w.ReadReversed(&PeakLeftBack, cbField);
        }
        if (!w.IsEnd())
        {
            bHasCenter = TRUE;
            w.ReadReversed(&VolCenter, cbField);
            w.ReadReversed(&PeakCenter, cbField);
        }
        if (!w.IsEnd())
        {
            bHasBass = TRUE;
            w.ReadReversed(&VolBass, cbField);
            w.ReadReversed(&PeakBass, cbField);
        }
        return PostDeserialize(rb, Ctx);
    }
    catch (const CMemoryReader::Xpt&)
    {
        return Result::Broken;
    }

    ECK_DECL_ID3FRAME_METHOD(RVAD)
};

struct EQUA final : public FRAME
{
    struct ADJ
    {
        USHORT usFreq{};
        LONGLONG llAdjust{};
    };
    BYTE cBits{};
    std::vector<ADJ> vAdjust{};

    Result Serialize(CByteBuffer& rb, const SERIAL_CTX& Ctx) noexcept override
    {
        if (!cBits || cBits > 63)
            return Result::Value;
        bFileAlterDiscard = TRUE;

        const size_t cbField = CeilDivide<UINT>(cBits, 8);
        const size_t cbFrame = 1 + vAdjust.size() * (2 + cbField);
        auto w = PreSerialize(rb, Ctx, cbFrame);
        w << cBits;
        for (const auto& e : vAdjust)
        {
            const auto us = (e.usFreq & 0b0111'1111) |
                ((e.llAdjust >= 0) ? 0b1000'0000 : 0);
            const auto ull = Abs(e.llAdjust);
            w.WriteReversed(&us, 2).WriteReversed(&ull, cbField);
        }
        return PostSerialize(rb, Ctx, cbFrame);
    }

    Result Deserialize(CByteBuffer& rb, const SERIAL_CTX& Ctx) noexcept override try
    {
        auto w = PreDeserialize(rb, Ctx);
        w >> cBits;
        if (!cBits || cBits > 63)
            return Result::Value;
        vAdjust.clear();
        const size_t cbField = CeilDivide<UINT>(cBits, 8);
        while (!w.IsEnd())
        {
            auto& e = vAdjust.emplace_back();
            USHORT us{};
            w.ReadReversed(&us, 2);
            w.ReadReversed(&e.llAdjust, cbField);
            e.usFreq = us & 0b0111'1111;
            if (us & 0b1000'0000)
                e.llAdjust = -e.llAdjust;
        }
        return PostDeserialize(rb, Ctx);
    }
    catch (const CMemoryReader::Xpt&)
    {
        return Result::Broken;
    }

    ECK_DECL_ID3FRAME_METHOD(EQUA)
};

//---------v2.2---------

struct CRM final : public FRAME
{
    CStringA rsEmail{};
    CStringA rsDesc{};
    CByteBuffer rbData{};

    Result Serialize(CByteBuffer& rb, const SERIAL_CTX& Ctx) noexcept override
    {
        const size_t cbFrame = 2 + rsEmail.Size() + rsDesc.Size() + rbData.Size();
        auto w = PreSerialize(rb, Ctx, cbFrame);
        w << rsEmail << rsDesc << rbData;
        return PostSerialize(rb, Ctx, cbFrame);
    }

    Result Deserialize(CByteBuffer& rb, const SERIAL_CTX& Ctx) noexcept override
    {
        return Result::NotSupport;
    }

    ECK_DECL_ID3FRAME_METHOD(CRM)
};

//---------非标---------

enum class ReplayGainType : BYTE
{
    TrackGain,
    AlbumGain,
    TrackPeak,
    AlbumPeak,
};

struct TXXX_ReplayGain : public TXXX
{
    ReplayGainType eType{};
    float fVal{};

    Result Serialize(CByteBuffer& rb, const SERIAL_CTX& Ctx) noexcept override
    {
        bFileAlterDiscard = TRUE;

        const auto rsVal = Format(
            (eType == ReplayGainType::TrackGain || eType == ReplayGainType::AlbumGain) ?
            "%.2f dB" : "%.6f", (double)fVal);
        // 验证'.'前面是否只有一个数字
        if (eType == ReplayGainType::TrackGain || eType == ReplayGainType::AlbumGain)
        {
            if (rsVal.Size() < 6)// 0.0 dB
                return Result::Value;
            if (rsVal.Front() == '-')
            {
                if (rsVal.FindChar('.') != 2)
                    return Result::Value;
            }
            else
            {
                if (rsVal.FindChar('.') != 1)
                    return Result::Value;
            }
        }
        else
        {
            if (rsVal.Size() < 3)// 0.0
                return Result::Value;
            if (rsVal.FindChar('.') != 1)
                return Result::Value;
        }
        const size_t cbFrame = 1 + 22 + rsVal.Size();
        auto w = PreSerialize(rb, Ctx, cbFrame);
        constexpr CHAR Key[4][22]
        {
            "REPLAYGAIN_TRACK_GAIN",
            "REPLAYGAIN_ALBUM_GAIN",
            "REPLAYGAIN_TRACK_PEAK",
            "REPLAYGAIN_ALBUM_PEAK",
        };

        w << TextEncoding::Latin1 << Key[(int)eType];
        w.Write(rsVal.Data(), rsVal.Size());
        return PostSerialize(rb, Ctx, cbFrame);
    }

    ECK_DECL_ID3FRAME_METHOD_CLONE_DEF_CONS(TXXX_ReplayGain);
};

struct RGAD final : public FRAME
{
    UINT ulPeak{};
    USHORT usTrackGain{};
    USHORT usAlbumGain{};

    Result Serialize(CByteBuffer& rb, const SERIAL_CTX& Ctx) noexcept override
    {
        bFileAlterDiscard = TRUE;

        const size_t cbFrame = 4 + 2 + 2;
        auto w = PreSerialize(rb, Ctx, cbFrame);
        w.WriteReversed(ulPeak).WriteReversed(usTrackGain).WriteReversed(usAlbumGain);
        return PostSerialize(rb, Ctx, cbFrame);
    }

    Result Deserialize(CByteBuffer& rb, const SERIAL_CTX& Ctx) noexcept override try
    {
        auto w = PreDeserialize(rb, Ctx);
        w.ReadReversed(ulPeak).ReadReversed(usTrackGain).ReadReversed(usAlbumGain);
        return PostDeserialize(rb, Ctx);
    }
    catch (const CMemoryReader::Xpt&)
    {
        return Result::Broken;
    }

    ECK_DECL_ID3FRAME_METHOD(RGAD)
};

#undef ECK_DECL_ID3FRAME_METHOD_CLONE
#undef ECK_DECL_ID3FRAME_METHOD_CLONE_DEF_CONS
#undef ECK_DECL_ID3FRAME_METHOD
#undef ECK_DECL_ID3FRAME_METHOD_ID

ECK_ID3V2_FRAME_NAMESPACE_END
ECK_MEDIATAG_NAMESPACE_END
ECK_NAMESPACE_END