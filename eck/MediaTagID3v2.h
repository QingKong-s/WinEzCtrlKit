#pragma once
#include "MediaTagID3v2Frame.h"

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
private:
    ID3v2_HEADER m_Header{};
    EXTHDR_INFO m_ExtHdrInfo{};
    std::vector<std::unique_ptr<ID3v2::FRAME>> m_vItem{};

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
        const ID3v2::FRAME::SERIAL_CTX SerialCtx
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
                auto e = std::make_unique<ID3v2::x>(); \
                r = e->Deserialize(rbFrame, SerialCtx); \
                if (r != Result::Ok) \
                    return r; \
                m_vItem.emplace_back(std::move(e)); \
            }

            if (0) {}
            /**/ECKTEMP_PARSE_ID3FRAME(TXXX)
            else if (FrameHdr.Id[0] == 'T')
            {
                auto e = std::make_unique<ID3v2::TEXTFRAME>();
                r = e->Deserialize(rbFrame, SerialCtx);
                if (r != Result::Ok)
                    return r;
                m_vItem.emplace_back(std::move(e));
            }
            /**/ECKTEMP_PARSE_ID3FRAME(WXXX)
            else if (FrameHdr.Id[0] == 'W')
            {
                auto e = std::make_unique<ID3v2::LINKFRAME>();
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
                auto e = std::make_unique<ID3v2::OTHERFRAME>();
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
    void TagpSetComment(ID3v2::COMM* pFrame, const StrList& slComment,
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

    void TagpSetTrack(ID3v2::TEXTFRAME* pFrame, int nTrack,
        int cTotalTrack, const SIMPLE_OPT& Opt) noexcept
    {
        pFrame->vText.resize(1);
        if (cTotalTrack > 0)
            pFrame->vText[0].Format(L"%d/%d", nTrack, cTotalTrack);
        else
            pFrame->vText[0].Format(L"%d", nTrack);
    }

    // 注意此函数不修改文本编码
    void TagpSetPicture(ID3v2::APIC* pFrame, MUSICPIC& Pic,
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
            EcdWideToUtf8(pFrame->rbData, Pic.GetPicturePath().Data(), Pic.GetPicturePath().Size());
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
                const auto p = DbgDynamicCast<ID3v2::TEXTFRAME*>(e.get());
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
                const auto p = DbgDynamicCast<ID3v2::TEXTFRAME*>(e.get());
                if (!p->vText.empty())
                    mi.uMaskChecked |= MIM_ARTIST;
                for (const auto& f : p->vText)
                    mi.slArtist.PushBackString(f, Opt.svArtistDiv);
            }
            else if ((mi.uMask & MIM_ALBUM) && e->EqualId("TALB"))
            {
                const auto p = DbgDynamicCast<ID3v2::TEXTFRAME*>(e.get());
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
                const auto p = DbgDynamicCast<ID3v2::USLT*>(e.get());
                if (bMove)
                    mi.rsLrc = std::move(p->rsLrc);
                else
                    mi.rsLrc = p->rsLrc;
                mi.uMaskChecked |= MIM_LRC;
            }
            else if ((mi.uMask & MIM_COMMENT) && e->EqualId("COMM"))
            {
                const auto p = DbgDynamicCast<ID3v2::COMM*>(e.get());
                mi.slComment.PushBackString(p->rsText, Opt.svCommDiv);
                mi.uMaskChecked |= MIM_COMMENT;
            }
            else if ((mi.uMask & MIM_COVER) && e->EqualId("APIC"))
            {
                const auto p = DbgDynamicCast<ID3v2::APIC*>(e.get());
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
                    Pic.varPic = EcdMultiByteToWide((PCSTR)p->rbData.Data(), (int)p->rbData.Size());
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
                const auto p = DbgDynamicCast<ID3v2::TEXTFRAME*>(e.get());
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
                const auto p = DbgDynamicCast<ID3v2::TEXTFRAME*>(e.get());
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
                const auto p = DbgDynamicCast<ID3v2::TEXTFRAME*>(e.get());  \
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
                const auto p = DbgDynamicCast<ID3v2::TEXTFRAME*>(e.get());
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
                const auto p = DbgDynamicCast<ID3v2::USLT*>(e.get());
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
                const auto p = DbgDynamicCast<ID3v2::COMM*>(e.get());
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
                const auto p = DbgDynamicCast<ID3v2::TEXTFRAME*>(e.get());
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
                const auto p = DbgDynamicCast<ID3v2::APIC*>(e.get());
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
            auto e = std::make_unique<ID3v2::TEXTFRAME>(KeyStr); \
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
            auto e = std::make_unique<ID3v2::TEXTFRAME>("TPE1");
            while (itArt != itArtEnd)
                e->vText.emplace_back(*itArt++);
            m_vItem.emplace_back(std::move(e));
        }
        if (!(mi.uMaskChecked & MIM_ALBUM))
            ECKTEMP_NEW_VAL(mi.rsAlbum, "TALB");
        if (!(mi.uMaskChecked & MIM_LRC))
        {
            auto e = std::make_unique<ID3v2::USLT>();
            e->eEncoding = ID3v2::TextEncoding::UTF8;
            if (bMove)
                e->rsLrc = std::move(mi.rsLrc);
            else
                e->rsLrc = mi.rsLrc;
            m_vItem.emplace_back(std::move(e));
        }
        if (!(mi.uMaskChecked & MIM_COMMENT))
        {
            auto e = std::make_unique<ID3v2::COMM>();
            e->eEncoding = ID3v2::TextEncoding::UTF8;
            TagpSetComment(e.get(), mi.slComment, Opt);
            m_vItem.emplace_back(std::move(e));
        }
        if (!(mi.uMaskChecked & MIM_GENRE))
            ECKTEMP_NEW_VAL(mi.rsGenre, "TCON");
        if (!(mi.uMaskChecked & MIM_TRACK))
        {
            auto e = std::make_unique<ID3v2::TEXTFRAME>("TRCK");
            TagpSetTrack(e.get(), mi.nTrack, mi.cTotalTrack, Opt);
            m_vItem.emplace_back(std::move(e));
        }
        if (!(mi.uMaskChecked & MIM_COVER))
        {
            while (itPic != itPicEnd)
            {
                auto e = std::make_unique<ID3v2::APIC>();
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

        const ID3v2::FRAME::SERIAL_CTX SerialCtx
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
                ID3v2::SEEK Seek{};
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
                        EckCheckMemory(p);
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
            [=](const std::unique_ptr<ID3v2::FRAME>& e) { return e->EqualId(Id); });
    }
    auto ItmAt(_In_reads_(4) PCCH Id) const noexcept
    {
        return std::find_if(m_vItem.begin(), m_vItem.end(),
            [=](const std::unique_ptr<ID3v2::FRAME>& e) { return e->EqualId(Id); });
    }

    auto ItmEndIterator() noexcept { return m_vItem.end(); }
    auto ItmEndIterator() const noexcept { return m_vItem.end(); }

    ID3v2::FRAME* ItmCreate(_In_reads_(4) PCCH Id) noexcept
    {
        ID3v2::FRAME* p;
        if (memcmp(Id, "UFID", 4) == 0)
            p = new ID3v2::UFID{};
        else if (memcmp(Id, "TXXX", 4) == 0)
            p = new ID3v2::TXXX{};
        else if (memcmp(Id, "WXXX", 4) == 0)
            p = new ID3v2::WXXX{};
        else if (memcmp(Id, "MCID", 4) == 0)
            p = new ID3v2::MCID{};
        else if (memcmp(Id, "ETCO", 4) == 0)
            p = new ID3v2::ETCO{};
        else if (memcmp(Id, "SYTC", 4) == 0)
            p = new ID3v2::SYTC{};
        else if (memcmp(Id, "USLT", 4) == 0)
            p = new ID3v2::USLT{};
        else if (memcmp(Id, "SYLT", 4) == 0)
            p = new ID3v2::SYLT{};
        else if (memcmp(Id, "COMM", 4) == 0)
            p = new ID3v2::COMM{};
        else if (memcmp(Id, "RVA2", 4) == 0)
            p = new ID3v2::RVA2{};
        else if (memcmp(Id, "EQU2", 4) == 0)
            p = new ID3v2::EQU2{};
        else if (memcmp(Id, "RVRB", 4) == 0)
            p = new ID3v2::RVRB{};
        else if (memcmp(Id, "APIC", 4) == 0)
            p = new ID3v2::APIC{};
        else if (memcmp(Id, "GEOB", 4) == 0)
            p = new ID3v2::GEOB{};
        else if (memcmp(Id, "PCNT", 4) == 0)
            p = new ID3v2::PCNT{};
        else if (memcmp(Id, "POPM", 4) == 0)
            p = new ID3v2::POPM{};
        else if (memcmp(Id, "RBUF", 4) == 0)
            p = new ID3v2::RBUF{};
        else if (memcmp(Id, "AENC", 4) == 0)
            p = new ID3v2::AENC{};
        else if (memcmp(Id, "LINK", 4) == 0)
            p = new ID3v2::LINK{};
        else if (memcmp(Id, "POSS", 4) == 0)
            p = new ID3v2::POSS{};
        else if (memcmp(Id, "USER", 4) == 0)
            p = new ID3v2::USER{};
        else if (memcmp(Id, "OWNE", 4) == 0)
            p = new ID3v2::OWNE{};
        else if (memcmp(Id, "COMR", 4) == 0)
            p = new ID3v2::COMR{};
        else if (memcmp(Id, "ENCR", 4) == 0)
            p = new ID3v2::ENCR{};
        else if (memcmp(Id, "GRID", 4) == 0)
            p = new ID3v2::GRID{};
        else if (memcmp(Id, "PRIV", 4) == 0)
            p = new ID3v2::PRIV{};
        else if (memcmp(Id, "SIGN", 4) == 0)
            p = new ID3v2::SIGN{};
        else if (memcmp(Id, "SEEK", 4) == 0)
            p = new ID3v2::SEEK{};
        else if (memcmp(Id, "ASPI", 4) == 0)
            p = new ID3v2::ASPI{};
        else if (memcmp(Id, "IPLS", 4) == 0)
            p = new ID3v2::IPLS{};
        else if (memcmp(Id, "RVAD", 4) == 0)
            p = new ID3v2::RVAD{};
        else if (memcmp(Id, "EQUA", 4) == 0)
            p = new ID3v2::EQUA{};
        else if (memcmp(Id, "RGAD", 4) == 0)
            p = new ID3v2::RGAD{};
        else if (memcmp(Id, "XRVA", 4) == 0)
        {
            p = new ID3v2::RVA2{};
            memcpy(p->Id, "XRVA", 4);
        }
        else if (Id[0] == 'T')
            p = new ID3v2::TEXTFRAME(Id);
        else if (Id[0] == 'W')
            p = new ID3v2::LINKFRAME(Id);
        else
            p = new ID3v2::OTHERFRAME(Id);
        m_vItem.emplace_back(p);
        return p;
    }

    EckInline ID3v2::FRAME* ItmEnsure(_In_reads_(4) PCCH Id) noexcept
    {
        if (const auto it = ItmAt(Id); it == ItmEndIterator())
            return (*it).get();
        return ItmCreate(Id);
    }

    void ItmClear() noexcept { m_vItem.clear(); }
};
ECK_MEDIATAG_NAMESPACE_END
ECK_NAMESPACE_END