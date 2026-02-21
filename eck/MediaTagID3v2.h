#pragma once
#include "MediaTag.h"
#include "Compress.h"

ECK_NAMESPACE_BEGIN
ECK_MEDIATAG_NAMESPACE_BEGIN
EckInlineNd BOOL TagCheckId3FrameId(_In_reads_(4) PCCH Id) noexcept
{
    return isalnum(Id[0]) && isalnum(Id[1]) && isalnum(Id[2]) && isalnum(Id[3]) &&
        (memcmp(Id, "ID3", 3) != 0) &&
        (memcmp(Id, "3DI", 3) != 0) &&
        (memcmp(Id, "APE", 3) != 0) &&
        (memcmp(Id, "TAG", 3) != 0);
}

EckInlineNdCe UINT TagGetFrameLength(
    const ID3v2_HEADER* pTagHdr,
    const ID3v2_FRAME_HEADER* pFrameHdr) noexcept
{
    if (pTagHdr->Ver == 4)
        return TagSyncSafeIntToUInt(pFrameHdr->Size);
    else
        return ReverseInteger(*(UINT*)pFrameHdr->Size);
}

class CID3v2 final : public CTag
{
public:
    enum class TagSizeRestriction : BYTE
    {
        Max128Frames_1MB,
        Max64Frames_128KB,
        Max32Frames_40KB,
        Max32Frames_4KB,
    };
    enum class TextEncodingRestriction : BYTE
    {
        No,
        OnlyLatin1OrU8
    };
    enum class TextFieldSizeRestriction : BYTE
    {
        No,
        Max1024Char,
        Max128Char,
        Max30Char,
    };
    enum class ImageFormatRestriction : BYTE
    {
        No,
        OnlyPngOrJpeg
    };
    enum class ImageSizeRestriction : BYTE
    {
        No,
        Max256x256,
        Max64x64,
        Only64x64
    };

    struct EXTHDR_INFO
    {
        BOOLEAN bCrc;
        BOOLEAN bTagAlter;      // ID3v2.4 Only
        BOOLEAN bRestrictions;  // ID3v2.4 Only
        TagSizeRestriction eTagSize;            // ID3v2.4 Only
        TextEncodingRestriction eTextEncoding;  // ID3v2.4 Only
        TextFieldSizeRestriction eTextFieldSize;// ID3v2.4 Only
        ImageFormatRestriction eImageFormat;    // ID3v2.4 Only
        ImageSizeRestriction eImageSize;        // ID3v2.4 Only
        UINT uCrc;
        UINT cbPadding;         // ID3v2.3 Only
    };

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
                    w.MoveToEnd() += 1;// throw
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
                    w.MoveToEnd() += 1;// throw
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
                StrW2X(rb, rsStr.Data(), rsStr.Size(), CP_ACP);
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
                StrW2U8(rb, rsStr.Data(), rsStr.Size());
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
                StrX2W(rsResult, (PCCH)w.Data(), cb, CP_ACP);
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
                StrU82W(rsResult, (PCCH)w.Data(), cb);
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
                (w << e.eType).WriteRev(e.uTimestamp);
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
                w.ReadRev(e.uTimestamp);
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
                w.WriteRev(e.uTimestamp);
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
                w.ReadRev(e.uTimestamp);
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
                w.WriteRev(vSync[i].uTimestamp);
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
                w.ReadRev(sync.uTimestamp);
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
                cbFrame += DivUpper(e.cPeekVolBit, 8);

            auto w = PreSerialize(rb, Ctx, cbFrame);
            w << rsId;
            for (const auto& e : vChannel)
            {
                w << e.eChannel;
                w.WriteRev(e.shVol);
                w << e.cPeekVolBit;
                w.WriteRev(e.bsPeekVol.Data(), DivUpper(e.cPeekVolBit, 8));
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
                w.ReadRev(e.shVol);
                w >> e.cPeekVolBit;
                w.ReadRev(e.bsPeekVol.Data(), DivUpper(e.cPeekVolBit, 8));
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
                w.WriteRev(e.uFreq).WriteRev(e.shVol);
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
                w.ReadRev(e.uFreq);
                w.ReadRev(e.shVol);
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
            w.WriteRev(Left).WriteRev(Right).Write(&BouncesLeft, 8);
            return PostSerialize(rb, Ctx, cbFrame);
        }

        Result Deserialize(CByteBuffer& rb, const SERIAL_CTX& Ctx) noexcept override try
        {
            auto w = PreDeserialize(rb, Ctx);
            w.ReadRev(Left).ReadRev(Right).Read(&BouncesLeft, 8);
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
            w.WriteRev(&cPlay, cbFrame);
            return PostSerialize(rb, Ctx, cbFrame);
        }

        Result Deserialize(CByteBuffer& rb, const SERIAL_CTX& Ctx) noexcept override
        {
            auto w = PreDeserialize(rb, Ctx);
            cPlay = 0;
            w.ReadRev(&cPlay, std::min(w.GetRemainingSize(), (size_t)8));
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
            w.WriteRev(&cPlay, cbPlayCount);
            return PostSerialize(rb, Ctx, cbFrame);
        }

        Result Deserialize(CByteBuffer& rb, const SERIAL_CTX& Ctx) noexcept override try
        {
            auto w = PreDeserialize(rb, Ctx);
            w >> rsEmail >> byRating;
            cPlay = 0;
            w.ReadRev(&cPlay, std::min(w.GetRemainingSize(), (size_t)8));
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
            w.WriteRev(&cbBuf, 3) << b;
            w.WriteRev(ocbNextTag);
            return PostSerialize(rb, Ctx, cbFrame);
        }

        Result Deserialize(CByteBuffer& rb, const SERIAL_CTX& Ctx) noexcept override try
        {
            auto w = PreDeserialize(rb, Ctx);
            cbBuf = 0;
            w.ReadRev(&cbBuf, 3) >> b;
            if (b != 0 && b != 1)
                return Result::ReservedData;
            w.ReadRev(ocbNextTag);
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
            w.WriteRev(usPreviewBegin).WriteRev(usPreviewLength);
            w << rbData;
            return PostSerialize(rb, Ctx, cbFrame);
        }

        Result Deserialize(CByteBuffer& rb, const SERIAL_CTX& Ctx) noexcept override try
        {
            auto w = PreDeserialize(rb, Ctx);
            w >> rsOwnerId;
            w.ReadRev(usPreviewBegin).ReadRev(usPreviewLength);
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
            w.WriteRev(uTime);
            return PostSerialize(rb, Ctx, cbFrame);
        }

        Result Deserialize(CByteBuffer& rb, const SERIAL_CTX& Ctx) noexcept override try
        {
            auto w = PreDeserialize(rb, Ctx);
            w >> eTimestamp;
            if (eTimestamp >= TimestampFmt::Max)
                return Result::Enum;
            w.ReadRev(uTime);
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
            w.WriteRev(ocbNextTag);
            return PostSerialize(rb, Ctx, cbFrame);
        }

        Result Deserialize(CByteBuffer& rb, const SERIAL_CTX& Ctx) noexcept override try
        {
            auto w = PreDeserialize(rb, Ctx);
            w.ReadRev(ocbNextTag);
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
            w.WriteRev(S).WriteRev(L).WriteRev(N) << b;
            for (const auto e : vIndex)
                w.WriteRev(&e, b / 8);
            return PostSerialize(rb, Ctx, cbFrame);
        }

        Result Deserialize(CByteBuffer& rb, const SERIAL_CTX& Ctx) noexcept override try
        {
            auto w = PreDeserialize(rb, Ctx);
            w.ReadRev(S).ReadRev(L).ReadRev(N) >> b;
            if (N != 8 && N != 16)
                return Result::Value;
            vIndex.clear();
            while (!w.IsEnd())
            {
                USHORT us{};
                w.ReadRev(&us, N / 8);
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

            const size_t cbField = DivUpper(cBits, 8);
            size_t cbFrame = 2 + (cbField * 4);
            if (bHasBack)
                cbFrame += (cbField * 4);
            if (bHasCenter)
                cbFrame += (cbField * 2);
            if (bHasBass)
                cbFrame += (cbField * 2);

            auto w = PreSerialize(rb, Ctx, cbFrame);
            w << byCtrl << cBits;
            w.WriteRev(&VolRight, cbField);
            w.WriteRev(&VolLeft, cbField);
            w.WriteRev(&PeakRight, cbField);
            w.WriteRev(&PeakLeft, cbField);
            if (bHasBack)
            {
                w.WriteRev(&VolRightBack, cbField);
                w.WriteRev(&VolLeftBack, cbField);
                w.WriteRev(&PeakRightBack, cbField);
                w.WriteRev(&PeakLeftBack, cbField);
            }
            if (bHasCenter)
            {
                w.WriteRev(&VolCenter, cbField);
                w.WriteRev(&PeakCenter, cbField);
            }
            if (bHasBass)
            {
                w.WriteRev(&VolBass, cbField);
                w.WriteRev(&PeakBass, cbField);
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

            const size_t cbField = DivUpper(cBits, 8);
            w.ReadRev(&VolRight, cbField);
            w.ReadRev(&VolLeft, cbField);
            if (!w.IsEnd())
            {
                w.ReadRev(&PeakRight, cbField);
                w.ReadRev(&PeakLeft, cbField);
            }
            if (!w.IsEnd())
            {
                bHasBack = TRUE;
                w.ReadRev(&VolRightBack, cbField);
                w.ReadRev(&VolLeftBack, cbField);
                w.ReadRev(&PeakRightBack, cbField);
                w.ReadRev(&PeakLeftBack, cbField);
            }
            if (!w.IsEnd())
            {
                bHasCenter = TRUE;
                w.ReadRev(&VolCenter, cbField);
                w.ReadRev(&PeakCenter, cbField);
            }
            if (!w.IsEnd())
            {
                bHasBass = TRUE;
                w.ReadRev(&VolBass, cbField);
                w.ReadRev(&PeakBass, cbField);
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

            const size_t cbField = DivUpper(cBits, 8);
            const size_t cbFrame = 1 + vAdjust.size() * (2 + cbField);
            auto w = PreSerialize(rb, Ctx, cbFrame);
            w << cBits;
            for (const auto& e : vAdjust)
            {
                const auto us = (e.usFreq & 0b0111'1111) |
                    ((e.llAdjust >= 0) ? 0b1000'0000 : 0);
                const auto ull = Abs(e.llAdjust);
                w.WriteRev(&us, 2).WriteRev(&ull, cbField);
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
            const size_t cbField = DivUpper(cBits, 8);
            while (!w.IsEnd())
            {
                auto& e = vAdjust.emplace_back();
                USHORT us{};
                w.ReadRev(&us, 2);
                w.ReadRev(&e.llAdjust, cbField);
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
            w.WriteRev(ulPeak).WriteRev(usTrackGain).WriteRev(usAlbumGain);
            return PostSerialize(rb, Ctx, cbFrame);
        }

        Result Deserialize(CByteBuffer& rb, const SERIAL_CTX& Ctx) noexcept override try
        {
            auto w = PreDeserialize(rb, Ctx);
            w.ReadRev(ulPeak).ReadRev(usTrackGain).ReadRev(usAlbumGain);
            return PostDeserialize(rb, Ctx);
        }
        catch (const CMemoryReader::Xpt&)
        {
            return Result::Broken;
        }

        ECK_DECL_ID3FRAME_METHOD(RGAD)
    };
private:
    ID3v2_HEADER m_Header{};
    EXTHDR_INFO m_ExtHdrInfo{};
    std::vector<std::unique_ptr<FRAME>> m_vItem{};

    CByteBuffer m_rbWork;

    size_t m_cbTag{};       // 标签长度
    size_t m_cbPrependTag{};// 前置标签长度
    size_t m_SeekVal{ CMediaFile::NPos };// 根据SEEK帧查找到的后置标签位置


    Result TagpParseFrameBody(size_t posEnd,
        _Out_opt_ size_t* pposActualEnd = nullptr) noexcept
    {
        if (pposActualEnd)
            *pposActualEnd = CMediaFile::NPos;

        posEnd = std::min(posEnd, m_Stream.GetSize());
        ID3v2_FRAME_HEADER FrameHdr;
        Result r;
        CByteBuffer rbFrame;
        const FRAME::SERIAL_CTX SerialCtx
        {
            .rbWork = m_rbWork,
            .pTagHdr = &m_Header,
            .pFrameHdr = &FrameHdr,
            .bFrameHdr = FALSE
        };
        while (m_Stream.GetPosition() < posEnd)
        {
            m_Stream >> FrameHdr;
            const auto cbUnit = TagGetFrameLength(&m_Header, &FrameHdr);
            rbFrame.ReSize(cbUnit);
            m_Stream.Read(rbFrame.Data(), cbUnit);

#define ECK_HIT_ID3FRAME(x) (memcmp(FrameHdr.Id, #x, 4) == 0)

#define ECKTEMP_PARSE_ID3FRAME(x) \
            else if (memcmp(FrameHdr.Id, #x, 4) == 0) \
            { \
                auto e = std::make_unique<x>(); \
                r = e->Deserialize(rbFrame, SerialCtx); \
                if (r != Result::Ok) \
                    return r; \
                m_vItem.emplace_back(std::move(e)); \
            }

            if (0) {}
            /**/ECKTEMP_PARSE_ID3FRAME(TXXX)
            else if (FrameHdr.Id[0] == 'T')
            {
                auto e = std::make_unique<TEXTFRAME>();
                r = e->Deserialize(rbFrame, SerialCtx);
                if (r != Result::Ok)
                    return r;
                m_vItem.emplace_back(std::move(e));
            }
            /**/ECKTEMP_PARSE_ID3FRAME(WXXX)
            else if (FrameHdr.Id[0] == 'W')
            {
                auto e = std::make_unique<LINKFRAME>();
                r = e->Deserialize(rbFrame, SerialCtx);
                if (r != Result::Ok)
                    return r;
                m_vItem.emplace_back(std::move(e));
            }
            /**/ECKTEMP_PARSE_ID3FRAME(UFID)
                ECKTEMP_PARSE_ID3FRAME(MCID)
                ECKTEMP_PARSE_ID3FRAME(ETCO)
                ECKTEMP_PARSE_ID3FRAME(SYTC)
                ECKTEMP_PARSE_ID3FRAME(USLT)
                ECKTEMP_PARSE_ID3FRAME(SYLT)
                ECKTEMP_PARSE_ID3FRAME(COMM)
                ECKTEMP_PARSE_ID3FRAME(RVA2)
                ECKTEMP_PARSE_ID3FRAME(EQU2)
                ECKTEMP_PARSE_ID3FRAME(RVRB)
                ECKTEMP_PARSE_ID3FRAME(APIC)
                ECKTEMP_PARSE_ID3FRAME(GEOB)
                ECKTEMP_PARSE_ID3FRAME(PCNT)
                ECKTEMP_PARSE_ID3FRAME(POPM)
                ECKTEMP_PARSE_ID3FRAME(RBUF)
                ECKTEMP_PARSE_ID3FRAME(AENC)
                ECKTEMP_PARSE_ID3FRAME(LINK)
                ECKTEMP_PARSE_ID3FRAME(POSS)
                ECKTEMP_PARSE_ID3FRAME(USER)
                ECKTEMP_PARSE_ID3FRAME(OWNE)
                ECKTEMP_PARSE_ID3FRAME(COMR)
                ECKTEMP_PARSE_ID3FRAME(ENCR)
                ECKTEMP_PARSE_ID3FRAME(GRID)
                ECKTEMP_PARSE_ID3FRAME(PRIV)
                ECKTEMP_PARSE_ID3FRAME(SIGN)
                ECKTEMP_PARSE_ID3FRAME(SEEK)
                ECKTEMP_PARSE_ID3FRAME(ASPI)
            else if (TagCheckId3FrameId(FrameHdr.Id))
            {
                auto e = std::make_unique<OTHERFRAME>();
                r = e->Deserialize(rbFrame, SerialCtx);
                if (r != Result::Ok)
                    return r;
                m_vItem.emplace_back(std::move(e));
            }
            else// 此帧错误，停止
            {
                if (pposActualEnd)
                    *pposActualEnd = m_Stream.GetPosition() - 10;
                return Result::Ok;
            }
        }
        if (pposActualEnd)
            *pposActualEnd = posEnd;
        return Result::Ok;
    }

    // 初始化m_cbTag、m_cbPrependTag、m_Header、m_ExtHdrInfo
    Result PreReadWrite() noexcept
    {
        const auto& Loc = m_File.GetTagLocation();
        if (Loc.posV2 != CMediaFile::NPos)
            m_Stream.MoveTo(Loc.posV2);
        else if (Loc.posV2Footer != CMediaFile::NPos)
            m_Stream.MoveTo(Loc.posV2FooterHdr);
        else
        {
            m_cbPrependTag = m_cbTag = 0u;
            return Result::NoTag;
        }
        m_Stream >> m_Header;
        m_cbPrependTag = m_cbTag = TagSyncSafeIntToUInt(m_Header.Size);
        m_ExtHdrInfo = {};
        if (m_Header.Ver == 3)
        {
            if (m_Header.Flags & ID3V2HF_EXTENDED_HEADER)
            {
                UINT cb;
                BYTE byFlags[2];
                UINT cbPadding;
                m_Stream >> cb >> byFlags >> cbPadding;
                cb = ReverseInteger(cb);
                if (cb != 6 && cb != 10)
                    return Result::Tag;
                if (byFlags[0] & ID3V23EH_CRC_DATA)
                {
                    UINT uCrc;
                    m_Stream >> uCrc;
                    m_ExtHdrInfo.bCrc = TRUE;
                    m_ExtHdrInfo.uCrc = ReverseInteger(uCrc);
                }
            }
            return Result::Ok;
        }
        else if (m_Header.Ver == 4)
        {
            if (m_Header.Flags & ID3V2HF_EXTENDED_HEADER)
            {
                BYTE bySize[4];
                UINT cb;
                BYTE by;
                m_Stream >> bySize;
                cb = TagSyncSafeIntToUInt(bySize);
                if (cb < 6)
                    return Result::Length;
                m_Stream >> by;
                if (by != 1)
                    return Result::Length;
                m_Stream >> by;

                m_ExtHdrInfo.bTagAlter = !!(by & ID3V24EH_UPDATE);
                if (by & ID3V24EH_CRC_DATA)
                {
                    if (cb < 11)
                        return Result::Length;
                    m_ExtHdrInfo.bCrc = TRUE;
                    BYTE t[5];
                    m_Stream >> t;
                    m_ExtHdrInfo.uCrc = ((t[0] & 0x7F) << 28) | ((t[1] & 0x7F) << 21) |
                        ((t[2] & 0x7F) << 14) | ((t[3] & 0x7F) << 7) | (t[4] & 0x7F);
                }
                if (by & ID3V24EH_RESTRICTIONS)
                {
                    if (cb < UINT(7 + (m_ExtHdrInfo.bCrc ? 5 : 0)))
                        return Result::Length;
                    m_ExtHdrInfo.bRestrictions = TRUE;
                    m_Stream >> by;
                    m_ExtHdrInfo.eTagSize = TagSizeRestriction((by >> 6) & 0b11);
                    m_ExtHdrInfo.eTextEncoding = TextEncodingRestriction((by >> 5) & 1);
                    m_ExtHdrInfo.eTextFieldSize = TextFieldSizeRestriction((by >> 3) & 0b11);
                    m_ExtHdrInfo.eImageFormat = ImageFormatRestriction((by >> 2) & 1);
                    m_ExtHdrInfo.eImageSize = ImageSizeRestriction(by & 0b11);
                }
            }
            return Result::Ok;
        }
        else
        {
            m_cbTag = 0u;
            return Result::Tag;
        }
    }

    // ID3规定，每种语言只能有一个注释帧
    // 若无连接符，默认使用"\n"
    void TagpSetComment(COMM* pFrame, const StrList& slComment,
        const SIMPLE_OPT& Opt) noexcept
    {
        const auto svDiv = Opt.svCommDiv.empty() ? L"\n"sv : Opt.svCommDiv;
        pFrame->rsText.Clear();
        for (const auto e : slComment)
        {
            if (!pFrame->rsText.IsEmpty())
                pFrame->rsText.PushBack(svDiv);
            pFrame->rsText.PushBack(e);
        }
    }

    void TagpSetTrack(TEXTFRAME* pFrame, int nTrack,
        int cTotalTrack, const SIMPLE_OPT& Opt) noexcept
    {
        pFrame->vText.resize(1);
        if (cTotalTrack > 0)
            pFrame->vText[0].Format(L"%d/%d", nTrack, cTotalTrack);
        else
            pFrame->vText[0].Format(L"%d", nTrack);
    }

    // 注意此函数不修改文本编码
    void TagpSetPicture(APIC* pFrame, MUSICPIC& Pic,
        const SIMPLE_OPT& Opt, BOOL bMove) noexcept
    {
        pFrame->eType = Pic.eType;
        if (bMove)
        {
            pFrame->rsMime = std::move(Pic.rsMime);
            pFrame->rsDesc = std::move(Pic.rsDesc);
        }
        else
        {
            pFrame->rsMime = Pic.rsMime;
            pFrame->rsDesc = Pic.rsDesc;
        }

        if (Pic.bLink)
        {
            pFrame->rbData.Clear();
            StrW2U8(pFrame->rbData, Pic.GetPicturePath().Data(), Pic.GetPicturePath().Size());
        }
        else
            if (bMove)
                pFrame->rbData = std::move(Pic.GetPictureData());
            else
                pFrame->rbData = Pic.GetPictureData();
    }

    struct EXTHDR_SERIAL_BUF
    {
        BYTE by[16];
    };

    void TagpSerializeExtendedHeader(_Out_ EXTHDR_SERIAL_BUF& Buf,
        _Out_ size_t& cb, _Out_ UINT*& pcbPadding) noexcept
    {
        EckAssert(m_ExtHdrInfo.bCrc == FALSE);// TODO: 支持CRC
        pcbPadding = nullptr;
        if (m_Header.Ver == 3)// 至多14字节
        {
            cb = 4 + 6;
            CMemoryWalker w{ Buf.by, cb };
            w << ReverseInteger(6u) << 0_us;
            w.SkipPointer(pcbPadding);// PENDEING 填充大小
        }
        else// 至多12字节
        {
            cb = 6;
            BYTE byFlags{};
            BYTE byRestrictions{};

            if (m_ExtHdrInfo.bTagAlter)
                byFlags |= ID3V24EH_UPDATE;
            if (m_ExtHdrInfo.bRestrictions)
            {
                byFlags |= ID3V24EH_RESTRICTIONS;
                ++cb;
                byRestrictions |= ((BYTE)m_ExtHdrInfo.eTagSize << 6);
                byRestrictions |= ((BYTE)m_ExtHdrInfo.eTextEncoding << 5);
                byRestrictions |= ((BYTE)m_ExtHdrInfo.eTextFieldSize << 3);
                byRestrictions |= ((BYTE)m_ExtHdrInfo.eImageFormat << 2);
                byRestrictions |= ((BYTE)m_ExtHdrInfo.eImageSize);
            }

            CMemoryWalker w{ Buf.by, cb };
            TagUIntToSyncSafeInt((BYTE*)w.Data(), (UINT)cb);
            w += 4;
            w << 1_by/*标志字节长度*/ << byFlags;
            if (m_ExtHdrInfo.bRestrictions)
                w << byRestrictions;
        }
    }

    void TagpSkipExtendedHeader()
    {
        BYTE bySize[4];
        m_Stream >> bySize;
        if (m_Header.Ver == 3)
            m_Stream += ReverseInteger(*(UINT*)bySize);
        else
            m_Stream += (TagSyncSafeIntToUInt(bySize) - 4);
    }
public:
    using CTag::CTag;

    Result SimpleGet(MUSICINFO& mi, const SIMPLE_OPT& Opt) noexcept override
    {
        mi.Clear();
        const auto bMove = (Opt.uFlags & SMOF_MOVE);

        for (const auto& e : m_vItem)
        {
            if ((mi.uMask & MIM_TITLE) && e->EqualId("TIT2"))
            {
                const auto p = DbgDynamicCast<TEXTFRAME*>(e.get());
                if (!p->vText.empty())
                {
                    if (bMove)
                        mi.rsTitle = std::move(p->vText[0]);
                    else
                        mi.rsTitle = p->vText[0];
                    mi.uMaskChecked |= MIM_TITLE;
                }
            }
            else if ((mi.uMask & MIM_ARTIST) && e->EqualId("TPE1"))
            {
                const auto p = DbgDynamicCast<TEXTFRAME*>(e.get());
                if (!p->vText.empty())
                    mi.uMaskChecked |= MIM_ARTIST;
                for (const auto& f : p->vText)
                    mi.slArtist.PushBackString(f, Opt.svArtistDiv);
            }
            else if ((mi.uMask & MIM_ALBUM) && e->EqualId("TALB"))
            {
                const auto p = DbgDynamicCast<TEXTFRAME*>(e.get());
                if (!p->vText.empty())
                {
                    if (bMove)
                        mi.rsAlbum = std::move(p->vText[0]);
                    else
                        mi.rsAlbum = p->vText[0];
                    mi.uMaskChecked |= MIM_ALBUM;
                }
            }
            else if ((mi.uMask & MIM_LRC) && e->EqualId("USLT"))
            {
                const auto p = DbgDynamicCast<USLT*>(e.get());
                if (bMove)
                    mi.rsLrc = std::move(p->rsLrc);
                else
                    mi.rsLrc = p->rsLrc;
                mi.uMaskChecked |= MIM_LRC;
            }
            else if ((mi.uMask & MIM_COMMENT) && e->EqualId("COMM"))
            {
                const auto p = DbgDynamicCast<COMM*>(e.get());
                mi.slComment.PushBackString(p->rsText, Opt.svCommDiv);
                mi.uMaskChecked |= MIM_COMMENT;
            }
            else if ((mi.uMask & MIM_COVER) && e->EqualId("APIC"))
            {
                const auto p = DbgDynamicCast<APIC*>(e.get());
                auto& Pic = mi.vPic.emplace_back();
                Pic.eType = p->eType;
                if (bMove)
                {
                    Pic.rsDesc = std::move(p->rsDesc);
                    Pic.rsMime = std::move(p->rsMime);
                }
                else
                {
                    Pic.rsDesc = p->rsDesc;
                    Pic.rsMime = p->rsMime;
                }

                Pic.bLink = (p->rsMime == "-->");
                if (Pic.bLink)
                    Pic.varPic = StrX2W((PCSTR)p->rbData.Data(), (int)p->rbData.Size());
                else
                    if (bMove)
                        Pic.varPic = std::move(p->rbData);
                    else
                    {
                        Pic.varPic = CByteBuffer(p->rbData.Size());
                        memcpy(Pic.GetPictureData().Data(), p->rbData.Data(), p->rbData.Size());
                    }
                mi.uMaskChecked |= MIM_COVER;
            }
            else if ((mi.uMask & MIM_GENRE) && e->EqualId("TCON"))
            {
                const auto p = DbgDynamicCast<TEXTFRAME*>(e.get());
                if (!p->vText.empty())
                {
                    if (bMove)
                        mi.rsGenre = std::move(p->vText[0]);
                    else
                        mi.rsGenre = p->vText[0];
                    mi.uMaskChecked |= MIM_GENRE;
                }
            }
            else if ((mi.uMask & MIM_TRACK) && e->EqualId("TRCK"))
            {
                const auto p = DbgDynamicCast<TEXTFRAME*>(e.get());
                if (!p->vText.empty())
                {
                    if (TagGetNumberAndTotal(p->vText[0].ToStringView(),
                        mi.nTrack, mi.cTotalTrack))
                        mi.uMaskChecked |= MIM_TRACK;
                }
            }
        }
        return Result::Ok;
    }

    Result SimpleSet(MUSICINFO& mi, const SIMPLE_OPT& Opt) noexcept override
    {
        mi.uMaskChecked = MIM_NONE;

        const auto bMove = Opt.uFlags & SMOF_MOVE;
        StrList::Iterator itArt{}, itArtEnd{};
        if (mi.uMask & MIM_ARTIST)
        {
            itArt = mi.slArtist.begin();
            itArtEnd = mi.slArtist.end();
        }
        decltype(mi.vPic.begin()) itPic{}, itPicEnd{};
        if (mi.uMask & MIM_COVER)
        {
            itPic = mi.vPic.begin();
            itPicEnd = mi.vPic.end();
        }

#undef ECKTEMP_SET_VAL
#define ECKTEMP_SET_VAL(MiField, Mask)   \
        {                                           \
            if (mi.uMaskChecked & Mask)             \
                it = m_vItem.erase(it);             \
            else {                                  \
                const auto p = DbgDynamicCast<TEXTFRAME*>(e.get());  \
                p->vText.resize(1);                     \
                if (bMove)                              \
                    __pragma(warning(suppress: 26800))/*对象已移动*/    \
                    p->vText[0] = std::move(MiField);   \
                else                                \
                    p->vText[0] = MiField;          \
                mi.uMaskChecked |= Mask;            \
                ++it;                               \
            }                                       \
        }

        for (auto it = m_vItem.begin(); it != m_vItem.end();)
        {
            const auto& e = *it;
            if ((mi.uMask & MIM_TITLE) && e->EqualId("TIT2"))
                ECKTEMP_SET_VAL(mi.rsTitle, MIM_TITLE)
            else if ((mi.uMask & MIM_ARTIST) && e->EqualId("TPE1"))
            {
                if (itArt == itArtEnd)
                {
                    it = m_vItem.erase(it);
                    continue;
                }
                const auto p = DbgDynamicCast<TEXTFRAME*>(e.get());
                p->vText.resize(1);
                p->vText[0] = *itArt++;
                ++it;
                mi.uMaskChecked |= MIM_ARTIST;
            }
            else if ((mi.uMask & MIM_ALBUM) && e->EqualId("TALB"))
                ECKTEMP_SET_VAL(mi.rsAlbum, MIM_ALBUM)
            else if ((mi.uMask & MIM_LRC) && e->EqualId("USLT"))
            {
                if (mi.uMaskChecked & MIM_LRC)
                {
                    it = m_vItem.erase(it);
                    continue;
                }
                const auto p = DbgDynamicCast<USLT*>(e.get());
                if (bMove)
                    p->rsLrc = std::move(mi.rsLrc);
                else
                    p->rsLrc = mi.rsLrc;
                ++it;
                mi.uMaskChecked |= MIM_LRC;
            }
            else if ((mi.uMask & MIM_COMMENT) && e->EqualId("COMM"))
            {
                if (mi.uMaskChecked & MIM_COMMENT)
                {
                    it = m_vItem.erase(it);
                    continue;
                }
                const auto p = DbgDynamicCast<COMM*>(e.get());
                TagpSetComment(p, mi.slComment, Opt);
                ++it;
                mi.uMaskChecked |= MIM_COMMENT;
            }
            else if ((mi.uMask & MIM_GENRE) && e->EqualId("TCON"))
                ECKTEMP_SET_VAL(mi.rsGenre, MIM_GENRE)
            else if ((mi.uMask & MIM_TRACK) && e->EqualId("TRCK"))
            {
                if (mi.uMaskChecked & MIM_TRACK)
                {
                    it = m_vItem.erase(it);
                    continue;
                }
                const auto p = DbgDynamicCast<TEXTFRAME*>(e.get());
                TagpSetTrack(p, mi.nTrack, mi.cTotalTrack, Opt);
                ++it;
                mi.uMaskChecked |= MIM_TRACK;
            }
            else if ((mi.uMask & MIM_COVER) && e->EqualId("APIC"))
            {
                if (itPic == itPicEnd)
                {
                    it = m_vItem.erase(it);
                    continue;
                }
                const auto p = DbgDynamicCast<APIC*>(e.get());
                TagpSetPicture(p, *itPic++, Opt, bMove);
                ++it;
                mi.uMaskChecked |= MIM_COVER;
            }
            else
                ++it;
        }
#undef ECKTEMP_SET_VAL

#undef ECKTEMP_NEW_VAL
#define ECKTEMP_NEW_VAL(MiField, KeyStr) \
        { \
            auto e = std::make_unique<TEXTFRAME>(KeyStr); \
            if (bMove)          \
                __pragma(warning(suppress: 26800))/*对象已移动*/    \
                e->vText.emplace_back(std::move(MiField)); \
            else \
                e->vText.emplace_back(MiField); \
            m_vItem.emplace_back(std::move(e)); \
        }

        if (!(mi.uMaskChecked & MIM_TITLE))
            ECKTEMP_NEW_VAL(mi.rsTitle, "TIT2");
        if (!(mi.uMaskChecked & MIM_ARTIST))
        {
            auto e = std::make_unique<TEXTFRAME>("TPE1");
            while (itArt != itArtEnd)
                e->vText.emplace_back(*itArt++);
            m_vItem.emplace_back(std::move(e));
        }
        if (!(mi.uMaskChecked & MIM_ALBUM))
            ECKTEMP_NEW_VAL(mi.rsAlbum, "TALB");
        if (!(mi.uMaskChecked & MIM_LRC))
        {
            auto e = std::make_unique<USLT>();
            e->eEncoding = TextEncoding::UTF8;
            if (bMove)
                e->rsLrc = std::move(mi.rsLrc);
            else
                e->rsLrc = mi.rsLrc;
            m_vItem.emplace_back(std::move(e));
        }
        if (!(mi.uMaskChecked & MIM_COMMENT))
        {
            auto e = std::make_unique<COMM>();
            e->eEncoding = TextEncoding::UTF8;
            TagpSetComment(e.get(), mi.slComment, Opt);
            m_vItem.emplace_back(std::move(e));
        }
        if (!(mi.uMaskChecked & MIM_GENRE))
            ECKTEMP_NEW_VAL(mi.rsGenre, "TCON");
        if (!(mi.uMaskChecked & MIM_TRACK))
        {
            auto e = std::make_unique<TEXTFRAME>("TRCK");
            TagpSetTrack(e.get(), mi.nTrack, mi.cTotalTrack, Opt);
            m_vItem.emplace_back(std::move(e));
        }
        if (!(mi.uMaskChecked & MIM_COVER))
        {
            while (itPic != itPicEnd)
            {
                auto e = std::make_unique<APIC>();
                TagpSetPicture(e.get(), *itPic++, Opt, bMove);
                m_vItem.emplace_back(std::move(e));
            }
        }
#undef ECKTEMP_SET_VAL
        return Result::Ok;
    }

    Result ReadTag(UINT uFlags = 0u) noexcept override try
    {
        ItmClear();
        m_SeekVal = CMediaFile::NPos;
        m_cbPrependTag = 0u;

        Result r;
        if (!m_cbTag && ((r = PreReadWrite()) != Result::Ok))
            return r;

        const auto& Loc = m_File.GetTagLocation();
        size_t posActualEnd;
        if (Loc.posV2 != CMediaFile::NPos)
        {
            m_Stream.MoveTo(Loc.posV2 + sizeof(ID3v2_HEADER));
            if (m_Header.Flags & ID3V2HF_EXTENDED_HEADER)
                TagpSkipExtendedHeader();

            r = TagpParseFrameBody(Loc.posV2 + m_cbTag, &posActualEnd);
            if (r != Result::Ok)
                return r;
            if (posActualEnd < Loc.posV2 + sizeof(ID3v2_FRAME_HEADER))
                return Result::Length;

            if (posActualEnd < Loc.posV2 + m_cbTag)// 可能有填充或后置标签
                m_cbPrependTag = size_t(posActualEnd - Loc.posV2);
            // 查找到的前置标签末尾超出标签头指示的长度
            if (m_cbPrependTag > m_cbTag)
                return Result::Length;
            // 若找到了SEEK帧，则移至其指示的位置继续解析，此时不可能含有空白填充
            if (m_SeekVal != CMediaFile::NPos)
            {
                m_SeekVal += (m_cbPrependTag + Loc.posV2);
                // 追加标签末尾超出文件长度
                if (m_SeekVal >= m_Stream.GetSize() - sizeof(ID3v2_HEADER))
                    return Result::Length;

                m_Stream.MoveTo(m_SeekVal);
                r = TagpParseFrameBody(m_SeekVal + m_cbTag);
            }
        }
        else if (Loc.posV2Footer != CMediaFile::NPos)
        {
            m_Stream.MoveTo(Loc.posV2Footer);
            r = TagpParseFrameBody(Loc.posV2Footer + m_cbTag);
        }
        else
            return Result::Tag;
        return r;
    }
    catch (const CStreamWalker::XptHResult& e)
    {
        m_hrLast = e.hr;
        return Result::Stream;
    }
    catch (const CStreamWalker::Xpt&)
    {
        return Result::Length;
    }

    Result WriteTag(UINT uFlags = 0u) noexcept override try
    {
        Result r;
        if (!m_cbTag)
        {
            r = PreReadWrite();
            if (r != Result::NoTag && r != Result::Ok)
                return r;
        }

        const auto& Loc = m_File.GetTagLocation();

        const BOOL bOnlyAppend =
            Loc.posV2Footer != CMediaFile::NPos &&
            Loc.posV2 == CMediaFile::NPos;
        const BOOL bShouldAppend = (uFlags & MIF_APPEND_TAG);

        ID3v2_HEADER Hdr{ m_Header };
        if (uFlags & MIF_CREATE_ID3V2_3)
            Hdr.Ver = 3;
        else if (uFlags & MIF_CREATE_ID3V2_4)
            Hdr.Ver = 4;
        else if (Hdr.Ver != 3 && Hdr.Ver != 4)
            Hdr.Ver = 3;

        if (uFlags & MIF_CREATE_ID3V2_EXT_HEADER)
            Hdr.Flags |= ID3V2HF_EXTENDED_HEADER;

        const FRAME::SERIAL_CTX SerialCtx
        {
            .rbWork = m_rbWork,
            .pTagHdr = &m_Header,
            .bFrameHdr = TRUE,
        };

        CByteBuffer rbPrepend{}, rbAppend{};
        BOOL bSeekFrameFound{};
        for (const auto& e : m_vItem)
        {
            if (!bSeekFrameFound && e->EqualId("SEEK"))
                bSeekFrameFound = TRUE;
            if (bShouldAppend || (e->byFlags & MIIF_ID3V2_4_APPEND))
            {
                e->Serialize(rbAppend, SerialCtx);
                Hdr.Ver = 4;
                Hdr.Flags |= ID3V2HF_FOOTER;
            }
            else
                e->Serialize(rbPrepend, SerialCtx);
        }

        // 前置后置均存在时，不允许使用填充
        const BOOL bAllowPadding = (uFlags & MIF_ALLOW_PADDING) &&
            !(!rbPrepend.IsEmpty() && !rbAppend.IsEmpty());

        size_t ocbNextTag{ CMediaFile::NPos };
        size_t dHdrFooterToEnd{ CMediaFile::NPos };
        //
        // 写入后置标签
        //
        if (!rbAppend.IsEmpty())
        {
            if (Loc.posV2Footer != CMediaFile::NPos)// 后置标签本身存在
            {
                if (m_cbTag < rbAppend.Size())
                {
                    m_Stream.Insert(Loc.posV2Footer + m_cbTag,
                        rbAppend.Size() - m_cbTag);
                }
                else
                {
                    const auto cbPadding = m_cbTag - rbAppend.Size();
                    if (cbPadding)// 无论如何都不对后置标签使用填充
                    {
                        m_Stream.Erase(Loc.posV2Footer + rbAppend.Size(),
                            cbPadding);
                    }
                }
                m_Stream.MoveTo(Loc.posV2Footer);
            }
            else if (m_SeekVal != CMediaFile::NPos)// 后置标签已通过SEEK帧定位
            {
                EckAssert(m_cbTag != CMediaFile::NPos);
                const auto cbOldAppend = m_cbTag - m_cbPrependTag;
                if (cbOldAppend < rbAppend.Size())
                {
                    m_Stream.Insert(m_SeekVal + cbOldAppend,
                        rbAppend.Size() - cbOldAppend);
                }
                else
                {
                    const auto cbPadding = cbOldAppend - rbAppend.Size();
                    if (cbPadding)// 无论如何都不对后置标签使用填充
                    {
                        m_Stream.Erase(m_SeekVal + rbAppend.Size(),
                            cbPadding);
                    }
                }
                m_Stream.MoveTo(m_SeekVal);
            }
            else// 后置标签不存在，确定插入位置
            {
                size_t posInsert;
                // 插入到ID3v1前，若无则插入到文件末尾
                if (Loc.posV1Ext != CMediaFile::NPos)
                    posInsert = Loc.posV1Ext;
                else if (Loc.posV1 != CMediaFile::NPos)
                    posInsert = Loc.posV1;
                else
                    posInsert = m_Stream.GetSize();
                m_Stream.Insert(posInsert, rbAppend.Size() + sizeof(ID3v2_HEADER));
                m_Stream.MoveTo(posInsert);
            }

            if (Loc.posV2 == CMediaFile::NPos)
                ocbNextTag = m_Stream.GetPosition();
            else
                ocbNextTag = m_Stream.GetPosition() - m_cbPrependTag - Loc.posV2 - 10;
            m_Stream << rbAppend;
            if (rbPrepend.IsEmpty())// 前置标签为空，立即写入标签尾
            {
                TagUIntToSyncSafeInt(Hdr.Size, (UINT)rbPrepend.Size());
                memcpy(Hdr.Header, "3DI", 3);
                m_Stream << Hdr;
            }
            else
                dHdrFooterToEnd = m_Stream.GetSize() - m_Stream.GetPosition();
        }
        else if (Loc.posV2Footer != CMediaFile::NPos)// 删除先前的后置标签
        {
            m_Stream.Erase(Loc.posV2Footer,
                m_cbTag - m_cbPrependTag + sizeof(ID3v2_HEADER));
        }
        //
        // 写入前置标签
        //

        // 前置部分的长度，包含**扩展头**，不含填充和标签头
        size_t cbPrependTotal = rbPrepend.Size();
        if (cbPrependTotal)
        {
            if (!bSeekFrameFound && ocbNextTag != CMediaFile::NPos)// 补下SEEK帧
            {
                SEEK Seek{};
                Seek.bFileAlterDiscard = TRUE;
                Seek.ocbNextTag = (UINT)ocbNextTag;
                Seek.Serialize(rbPrepend, SerialCtx);
                cbPrependTotal += (sizeof(ID3v2_FRAME_HEADER) + 4);
            }

            // 序列化扩展头
            EXTHDR_SERIAL_BUF ExtHdrBuf{};
            size_t cbExtHdr{};
            UINT* pcbPaddingExtHdrV23{};
            if (m_Header.Flags & ID3V2HF_EXTENDED_HEADER)
            {
                // PENDING 扩展头填充大小
                TagpSerializeExtendedHeader(ExtHdrBuf, cbExtHdr,
                    pcbPaddingExtHdrV23);
                cbPrependTotal += cbExtHdr;
            }
            //
            size_t cbPadding{};
            if (Loc.posV2 != CMediaFile::NPos)
            {
                const auto cbPrependOld = m_SeekVal == CMediaFile::NPos ?
                    m_cbTag : m_cbPrependTag;
                if (cbPrependOld < cbPrependTotal)
                {
                    m_Stream.Insert(Loc.posV2 + sizeof(ID3v2_HEADER) + m_cbPrependTag,
                        cbPrependTotal - m_cbPrependTag);
                }
                else if (cbPadding = cbPrependOld - cbPrependTotal)
                {
                    if (bAllowPadding && cbPadding <= 1024 && rbAppend.IsEmpty())
                    {
                        m_Stream.MoveTo(Loc.posV2 +
                            sizeof(ID3v2_HEADER) + cbPrependTotal);
                        void* p = VAlloc(cbPadding);
                        EckCheckMem(p);
                        m_Stream.Write(p, (ULONG)cbPadding);
                        VFree(p);
                    }
                    else
                    {
                        m_Stream.Erase(Loc.posV2 +
                            sizeof(ID3v2_HEADER) + cbPrependTotal, cbPadding);
                        cbPadding = 0u;
                    }
                }
                m_Stream.MoveTo(Loc.posV2);
            }
            else
            {
                m_Stream.Insert(0u, cbPrependTotal + sizeof(ID3v2_HEADER));
                m_Stream.MoveToBegin();
            }
            if (pcbPaddingExtHdrV23)// BACKFILL 扩展头填充大小
                *pcbPaddingExtHdrV23 = ReverseInteger((UINT)cbPadding);
            // 准备头
            memcpy(Hdr.Header, "ID3", 3);
            TagUIntToSyncSafeInt(Hdr.Size,
                UINT(cbPrependTotal + rbAppend.Size() + cbPadding));
            // 写入
            m_Stream << Hdr;
            if (cbExtHdr)
                m_Stream.Write(&ExtHdrBuf, cbExtHdr);
            m_Stream << rbPrepend;
            // BACKFILL 若标签尾写入挂起，完成之
            if (dHdrFooterToEnd != CMediaFile::NPos)
            {
                m_Stream.Seek(dHdrFooterToEnd, STREAM_SEEK_END);
                memcpy(Hdr.Header, "3DI", 3);
                m_Stream << Hdr;
            }
        }
        else if (Loc.posV2 != CMediaFile::NPos)// 删除先前的前置标签
            m_Stream.Erase(Loc.posV2, m_cbPrependTag + sizeof(ID3v2_HEADER));
        m_Stream.Commit();
        return Result::Ok;
    }
    catch (const CStreamWalker::XptHResult& e)
    {
        m_hrLast = e.hr;
        return Result::Stream;
    }
    catch (const CStreamWalker::Xpt&)
    {
        return Result::Length;
    }

    void Reset() noexcept override
    {
        ItmClear();
        m_Header = {};
        m_ExtHdrInfo = {};
        m_cbTag = 0u;
        m_cbPrependTag = 0u;
        m_SeekVal = CMediaFile::NPos;
    }

    BOOL IsEmpty() noexcept override { return m_vItem.empty(); }

    EckInlineNdCe auto& GetItemList() noexcept { return m_vItem; }
    EckInlineNdCe auto& GetItemList() const noexcept { return m_vItem; }
    EckInlineNdCe auto& GetHeader() const noexcept { return m_Header; }
    EckInlineNdCe auto& GetExtendedHeader() noexcept { return m_ExtHdrInfo; }
    EckInlineNdCe auto& GetExtendedHeader() const noexcept { return m_ExtHdrInfo; }

    auto ItmAt(_In_reads_(4) PCCH Id) noexcept
    {
        return std::find_if(m_vItem.begin(), m_vItem.end(),
            [=](const std::unique_ptr<FRAME>& e) { return e->EqualId(Id); });
    }
    auto ItmAt(_In_reads_(4) PCCH Id) const noexcept
    {
        return std::find_if(m_vItem.begin(), m_vItem.end(),
            [=](const std::unique_ptr<FRAME>& e) { return e->EqualId(Id); });
    }

    auto ItmEndIterator() noexcept { return m_vItem.end(); }
    auto ItmEndIterator() const noexcept { return m_vItem.end(); }

    FRAME* ItmCreate(_In_reads_(4) PCCH Id) noexcept
    {
        FRAME* p;
        if (memcmp(Id, "UFID", 4) == 0)
            p = new UFID{};
        else if (memcmp(Id, "TXXX", 4) == 0)
            p = new TXXX{};
        else if (memcmp(Id, "WXXX", 4) == 0)
            p = new WXXX{};
        else if (memcmp(Id, "MCID", 4) == 0)
            p = new MCID{};
        else if (memcmp(Id, "ETCO", 4) == 0)
            p = new ETCO{};
        else if (memcmp(Id, "SYTC", 4) == 0)
            p = new SYTC{};
        else if (memcmp(Id, "USLT", 4) == 0)
            p = new USLT{};
        else if (memcmp(Id, "SYLT", 4) == 0)
            p = new SYLT{};
        else if (memcmp(Id, "COMM", 4) == 0)
            p = new COMM{};
        else if (memcmp(Id, "RVA2", 4) == 0)
            p = new RVA2{};
        else if (memcmp(Id, "EQU2", 4) == 0)
            p = new EQU2{};
        else if (memcmp(Id, "RVRB", 4) == 0)
            p = new RVRB{};
        else if (memcmp(Id, "APIC", 4) == 0)
            p = new APIC{};
        else if (memcmp(Id, "GEOB", 4) == 0)
            p = new GEOB{};
        else if (memcmp(Id, "PCNT", 4) == 0)
            p = new PCNT{};
        else if (memcmp(Id, "POPM", 4) == 0)
            p = new POPM{};
        else if (memcmp(Id, "RBUF", 4) == 0)
            p = new RBUF{};
        else if (memcmp(Id, "AENC", 4) == 0)
            p = new AENC{};
        else if (memcmp(Id, "LINK", 4) == 0)
            p = new LINK{};
        else if (memcmp(Id, "POSS", 4) == 0)
            p = new POSS{};
        else if (memcmp(Id, "USER", 4) == 0)
            p = new USER{};
        else if (memcmp(Id, "OWNE", 4) == 0)
            p = new OWNE{};
        else if (memcmp(Id, "COMR", 4) == 0)
            p = new COMR{};
        else if (memcmp(Id, "ENCR", 4) == 0)
            p = new ENCR{};
        else if (memcmp(Id, "GRID", 4) == 0)
            p = new GRID{};
        else if (memcmp(Id, "PRIV", 4) == 0)
            p = new PRIV{};
        else if (memcmp(Id, "SIGN", 4) == 0)
            p = new SIGN{};
        else if (memcmp(Id, "SEEK", 4) == 0)
            p = new SEEK{};
        else if (memcmp(Id, "ASPI", 4) == 0)
            p = new ASPI{};
        else if (memcmp(Id, "IPLS", 4) == 0)
            p = new IPLS{};
        else if (memcmp(Id, "RVAD", 4) == 0)
            p = new RVAD{};
        else if (memcmp(Id, "EQUA", 4) == 0)
            p = new EQUA{};
        else if (memcmp(Id, "RGAD", 4) == 0)
            p = new RGAD{};
        else if (memcmp(Id, "XRVA", 4) == 0)
        {
            p = new RVA2{};
            memcpy(p->Id, "XRVA", 4);
        }
        else if (Id[0] == 'T')
            p = new TEXTFRAME(Id);
        else if (Id[0] == 'W')
            p = new LINKFRAME(Id);
        else
            p = new OTHERFRAME(Id);
        m_vItem.emplace_back(p);
        return p;
    }

    EckInline FRAME* ItmEnsure(_In_reads_(4) PCCH Id) noexcept
    {
        if (const auto it = ItmAt(Id); it == ItmEndIterator())
            return (*it).get();
        return ItmCreate(Id);
    }

    void ItmClear() noexcept { m_vItem.clear(); }
};
ECK_MEDIATAG_NAMESPACE_END
ECK_NAMESPACE_END