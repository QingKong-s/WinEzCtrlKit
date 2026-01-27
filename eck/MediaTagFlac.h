#pragma once
#include "MediaTag.h"
#include "Utility2.h"
#include "BaseN.h"

ECK_NAMESPACE_BEGIN
ECK_MEDIATAG_NAMESPACE_BEGIN
class CFlac : public CTag
{
public:
    struct ITEM
    {
        CRefStrA rsKey;
        CRefStrW rsValue;
    };

    enum class BlockType : BYTE
    {
        StreamInfo = 0,
        Padding = 1,
        Application = 2,
        SeekTable = 3,
        VorbisComment = 4,
        CueSheet = 5,
        Picture = 6,
        Invalid = 127,
    };

    struct BLOCK
    {
        BlockType eType;
        CRefBin rbData;
    };

    struct STREAMINFO
    {
        USHORT cBlockSampleMin;
        USHORT cBlockSampleMax;
        UINT cbMinFrame;
        UINT cbMaxFrame;
        UINT cSampleRate;
        BYTE cChannels;
        BYTE cBitsPerSample;
        ULONGLONG ullTotalSamples;
        BYTE Md5[16];
    };

    struct PIC
    {
        PicType eType{};
        CRefStrA rsMime{};
        CRefStrW rsDesc{};
        CRefBin rbData{};
        UINT cx{};
        UINT cy{};
        UINT bpp{};
        UINT cColor{};
        MIITEMFLAGS byAddtFlags{};
    };
private:
    std::vector<ITEM> m_vItem{};	// 所有Vorbis注释
    std::vector<PIC> m_vPic{};	    // 所有图片
    STREAMINFO m_si{};				// 流信息
    std::vector<BLOCK> m_vBlock{};	// 其他块

    CRefStrW m_rsVendor{};

    size_t m_posStreamInfoEnd{ CMediaFile::NPos };
    size_t m_posFlacTagEnd{ CMediaFile::NPos };

    // WARNING 函数返回后rb已被移动
    static Result ParseImageBlock(CRefBin& rb, PIC& Pic) noexcept
    {
        CMemWalker r(rb.Data(), rb.Size());
        UINT t;
        UINT uType;
        r.ReadRev(uType);// 图片类型
        if (uType < (BYTE)PicType::PrivBegin || uType >= (BYTE)PicType::PrivEnd)
            Pic.eType = PicType::Invalid;
        else
            Pic.eType = (PicType)uType;

        r.ReadRev(t);// 长度
        Pic.rsMime.ReSize(t);
        r.Read(Pic.rsMime.Data(), t);// MIME类型字符串

        r.ReadRev(t);// 描述字符串长度
        Pic.rsDesc = StrX2W((PCCH)r.Data(), (int)t, CP_UTF8);
        r += t;

        r.ReadRev(Pic.cx).ReadRev(Pic.cy).ReadRev(Pic.bpp).ReadRev(Pic.cColor);

        r.ReadRev(t);// 图片数据长度

        rb.Erase(0, r.Data() - rb.Data());
        Pic.rbData = std::move(rb);
        return Result::Ok;
    }

    Result InitForWriteTag() noexcept
    {
        const auto& Loc = m_File.GetTagLocation();
        if (Loc.posFlac == CMediaFile::NPos)
            return Result::NoTag;
        m_Stream.MoveTo(Loc.posFlac) += 4;
        FLAC_BLOCK_HEADER Header;
        UINT cbBlock;
        do
        {
            m_Stream >> Header;
            cbBlock = Header.bySize[2] | Header.bySize[1] << 8 | Header.bySize[0] << 16;
            switch (Header.eType & 0b0111'1111)
            {
            case 0:// 流信息
                m_Stream += cbBlock;
                m_posStreamInfoEnd = m_Stream.GetPosition();
                break;
            default:
                m_Stream += cbBlock;
                break;
            }
        } while (!(Header.eType & 0b1000'0000));
        m_posFlacTagEnd = m_Stream.GetPosition();
        return Result::Ok;
    }

    // 尾插到rbImage
    void SerializePicture(const PIC& e, CRefBin& rbImage) noexcept
    {
        const auto cbCurr = rbImage.Size();
        rbImage.PushBack(4u);// 悬而未决
        rbImage
            << ReverseInteger((UINT)e.eType)
            << ReverseInteger(e.rsMime.Size());
        rbImage.PushBack(e.rsMime.Data(), e.rsMime.Size());

        const int cchDesc = WideCharToMultiByte(CP_UTF8, 0,
            e.rsDesc.Data(), e.rsDesc.Size(),
            nullptr, 0, nullptr, nullptr);
        rbImage << ReverseInteger(cchDesc);
        WideCharToMultiByte(CP_UTF8, 0,
            e.rsDesc.Data(), e.rsDesc.Size(),
            (CHAR*)rbImage.PushBack(cchDesc), cchDesc, nullptr, nullptr);

        rbImage
            << ReverseInteger(e.cx)
            << ReverseInteger(e.cy)
            << ReverseInteger(e.bpp)
            << ReverseInteger(e.cColor)
            << ReverseInteger((UINT)e.rbData.Size())
            << e.rbData;

        const auto cbData = UINT(rbImage.Size() - cbCurr - 4);
        const auto phdr = (FLAC_BLOCK_HEADER*)(rbImage.Data() + cbCurr);
        phdr->bySize[0] = GetIntegerByte<2>(cbData);
        phdr->bySize[1] = GetIntegerByte<1>(cbData);
        phdr->bySize[2] = GetIntegerByte<0>(cbData);
        phdr->eType = (BYTE)BlockType::Picture;
    }
public:
    ECK_DISABLE_COPY_MOVE(CFlac)
public:
    CFlac(CMediaFile& File) :CTag(File) {}

    Result SimpleGet(MUSICINFO& mi, const SIMPLE_OPT& Opt) noexcept override
    {
#undef ECKTEMP_GET_VAL
#define ECKTEMP_GET_VAL(MiField) \
            if (bMove)           \
                MiField = std::move(e.rsValue);	\
            else                 \
                MiField = e.rsValue;

        mi.Clear();
        const auto bMove = Opt.uFlags & SMOF_MOVE;

        for (auto& e : m_vItem)
        {
            if ((mi.uMask & MIM_TITLE) &&
                e.rsKey.CompareI("TITLE"sv) == 0)
            {
                ECKTEMP_GET_VAL(mi.rsTitle);
                mi.uMaskChecked |= MIM_TITLE;
            }
            else if ((mi.uMask & MIM_ARTIST) &&
                e.rsKey.CompareI("ARTIST"sv) == 0)
            {
                mi.slArtist.PushBackString(e.rsValue, Opt.svArtistDiv);
                mi.uMaskChecked |= MIM_ARTIST;
            }
            else if ((mi.uMask & MIM_ALBUM) &&
                e.rsKey.CompareI("ALBUM"sv) == 0)
            {
                ECKTEMP_GET_VAL(mi.rsAlbum);
                mi.uMaskChecked |= MIM_ALBUM;
            }
            else if (!!(mi.uMask & MIM_LRC) && (
                // LYRICS在Vorbis注释中出现较少，应使用后两个
                e.rsKey.CompareI("LYRICS"sv) == 0 ||
                e.rsKey.CompareI("UNSYNCED LYRICS"sv) == 0 ||
                e.rsKey.CompareI("UNSYNCEDLYRICS"sv) == 0))
            {
                ECKTEMP_GET_VAL(mi.rsLrc);
                mi.uMaskChecked |= MIM_LRC;
            }
            // Vorbis注释建议使用DESCRIPTION
            else if ((mi.uMask & MIM_COMMENT) && (
                e.rsKey.CompareI("DESCRIPTION"sv) == 0 ||
                e.rsKey.CompareI("COMMENT"sv) == 0))
            {
                mi.slComment.PushBackString(e.rsValue, Opt.svCommDiv);
                mi.uMaskChecked |= MIM_COMMENT;
            }
            else if ((mi.uMask & MIM_GENRE) &&
                e.rsKey.CompareI("GENRE"sv) == 0)
            {
                ECKTEMP_GET_VAL(mi.rsGenre);
                mi.uMaskChecked |= MIM_GENRE;
            }
            else if (mi.uMask & MIM_TRACK)
            {
                if (e.rsValue.IsEmpty())
                {
                    mi.uMaskChecked |= MIM_TRACK;
                    continue;
                }
                // 应使用TRACKNUMBER
                const auto bTrackNum = (e.rsKey.CompareI("TRACKNUMBER"sv) == 0);
                if (bTrackNum || e.rsKey.CompareI("TRACK"sv) == 0)
                {
                    const auto n = mi.cTotalTrack;
                    if (TagGetNumberAndTotal(e.rsValue.ToStringView(),
                        mi.nTrack, mi.cTotalTrack))
                    {
                        mi.uMaskChecked |= MIM_TRACK;
                        // 防止覆盖总轨为0
                        if (bTrackNum && mi.cTotalTrack <= 0)
                            mi.cTotalTrack = n;
                    }
                }
                // 应使用TRACKTOTAL
                else if (e.rsKey.CompareI("TOTALTRACKS"sv) == 0 ||
                    e.rsKey.CompareI("TRACKTOTAL"sv) == 0)
                {
                    if (TcsToInt(e.rsValue.Data(), e.rsValue.Size(),
                        mi.cTotalTrack) == TcsCvtErr::Ok)
                        mi.uMaskChecked |= MIM_TRACK;
                }
            }
        }

        if (mi.uMask & MIM_COVER)
        {
            if (!m_vPic.empty())
                mi.uMaskChecked |= MIM_COVER;
            for (auto& e : m_vPic)
            {
                if (e.rsMime.Compare("-->"sv) == 0)
                    mi.vPic.emplace_back(
                        e.eType,
                        TRUE,
                        e.rsDesc,
                        e.rsMime,
                        StrU82W(e.rbData));
                else
                    if (bMove)
                        mi.vPic.emplace_back(
                            e.eType,
                            FALSE,
                            e.rsDesc,
                            e.rsMime,
                            std::move(e.rbData));
                    else
                    {
                        auto& f = mi.vPic.emplace_back(
                            e.eType,
                            FALSE,
                            e.rsDesc,
                            e.rsMime,
                            CRefBin(e.rbData.Size()));
                        memcpy(f.GetPictureData().Data(), e.rbData.Data(), e.rbData.Size());
                    }
            }
        }
        return Result::Ok;
    }

    Result SimpleSet(MUSICINFO& mi, const SIMPLE_OPT& Opt) noexcept override
    {
        const auto bMove = Opt.uFlags & SMOF_MOVE;
        StrList::Iterator itArt{}, itArtEnd{}, itComm{}, itCommEnd{};
        if (mi.uMask & MIM_ARTIST)
        {
            itArt = mi.slArtist.begin();
            itArtEnd = mi.slArtist.end();
        }
        if (mi.uMask & MIM_COMMENT)
        {
            itComm = mi.slComment.begin();
            itCommEnd = mi.slComment.end();
        }

        for (auto it = m_vItem.begin(); it != m_vItem.end();)
        {
            auto& e = *it;
            if ((mi.uMask & MIM_TITLE) &&
                e.rsKey.CompareI("TITLE"sv) == 0)
                it = m_vItem.erase(it);
            else if ((mi.uMask & MIM_ARTIST) &&
                e.rsKey.CompareI("ARTIST"sv) == 0)
            {
                if (itArt == itArtEnd)
                    it = m_vItem.erase(it);
                else
                {
                    if (bMove)
                        e.rsValue = std::move(*itArt++);
                    else
                        e.rsValue = *itArt++;
                    ++it;
                }
            }
            else if ((mi.uMask & MIM_ALBUM) &&
                e.rsKey.CompareI("ALBUM"sv) == 0)
                it = m_vItem.erase(it);
            else if ((mi.uMask & MIM_LRC) && (
                e.rsKey.CompareI("LYRICS"sv) == 0 ||
                e.rsKey.CompareI("UNSYNCED LYRICS"sv) == 0 ||
                e.rsKey.CompareI("UNSYNCEDLYRICS"sv) == 0))
                it = m_vItem.erase(it);
            else if ((mi.uMask & MIM_COMMENT) && (
                e.rsKey.CompareI("DESCRIPTION"sv) == 0 ||
                e.rsKey.CompareI("COMMENT"sv) == 0))
            {
                if (itComm == itCommEnd)
                    it = m_vItem.erase(it);
                else
                {
                    if (bMove)
                        e.rsValue = std::move(*itComm++);
                    else
                        e.rsValue = *itComm++;
                    ++it;
                }
            }
            else if ((mi.uMask & MIM_GENRE) &&
                e.rsKey.CompareI("GENRE"sv) == 0)
                it = m_vItem.erase(it);
            else if ((mi.uMask & MIM_TRACK) && (
                e.rsKey.CompareI("TRACKNUMBER"sv) == 0 ||
                e.rsKey.CompareI("TRACK"sv) == 0 ||
                e.rsKey.CompareI("TOTALTRACKS"sv) == 0 ||
                e.rsKey.CompareI("TRACKTOTAL"sv) == 0))
                it = m_vItem.erase(it);
            else
                ++it;
        }

#undef ECKTEMP_SET_VAL
#define ECKTEMP_SET_VAL(MiField, KeyStr) \
        {                                \
            ITEM e{};                    \
            e.rsKey.Assign(KeyStr);   \
            if (bMove)                   \
                e.rsValue = std::move(MiField); \
            else                         \
                e.rsValue = MiField;     \
            m_vItem.emplace_back(std::move(e)); \
        }

        if (mi.uMask & MIM_TITLE)
            ECKTEMP_SET_VAL(mi.rsTitle, "TITLE"sv);
        if (mi.uMask & MIM_ARTIST)
        {
            while (itArt != itArtEnd)
                m_vItem.emplace_back("ARTIST"sv, *itArt++);
        }
        if (mi.uMask & MIM_ALBUM)
            ECKTEMP_SET_VAL(mi.rsAlbum, "ALBUM"sv);
        if (mi.uMask & MIM_COMMENT)
        {
            while (itComm != itCommEnd)
                m_vItem.emplace_back("DESCRIPTION"sv, *itComm++);
        }
        if (mi.uMask & MIM_LRC)
            ECKTEMP_SET_VAL(mi.rsLrc, "UNSYNCED LYRICS"sv);
        if (mi.uMask & MIM_GENRE)
            ECKTEMP_SET_VAL(mi.rsGenre, "GENRE"sv);
        if (mi.uMask & MIM_TRACK)
        {
            WCHAR szBuf[TcsCvtCalcBufferSize<UINT>() * 2 + 1];
            PWCH p;
            if (mi.nTrack >= 0)
            {
                TcsFromInt(szBuf, ARRAYSIZE(szBuf), mi.nTrack, 10, TRUE, &p);
                if (p != szBuf)
                    m_vItem.emplace_back("TRACKNUMBER"sv, CRefStrW{ szBuf, int(p - szBuf) });
            }
            if (mi.cTotalTrack > 0)
            {
                TcsFromInt(szBuf, ARRAYSIZE(szBuf), mi.cTotalTrack, 10, TRUE, &p);
                if (p != szBuf)
                    m_vItem.emplace_back("TRACKTOTAL"sv, CRefStrW{ szBuf, int(p - szBuf) });
            }
        }
#undef ECKTEMP_SET_VAL

        m_vPic.clear();
        for (auto& e : mi.vPic)
        {
            auto& f = m_vPic.emplace_back();
            f.eType = e.eType;
            if (bMove)
            {
                f.rsMime = std::move(e.rsMime);
                f.rsDesc = std::move(e.rsDesc);
            }
            else
            {
                f.rsMime = e.rsMime;
                f.rsDesc = e.rsDesc;
            }

            if (e.bLink)
            {
                const auto& rs = e.GetPicturePath();
                StrW2U8(f.rbData, rs.Data(), rs.Size());
            }
            else
                if (bMove)
                    f.rbData = std::move(e.GetPictureData());
                else
                    f.rbData = e.GetPictureData();
        }
        return Result::Ok;
    }

    Result ReadTag(UINT uFlags = 0u) noexcept override try
    {
        const auto& Loc = m_File.GetTagLocation();
        if (Loc.posFlac == CMediaFile::NPos)
            return Result::NoTag;
        m_Stream.MoveTo(Loc.posFlac) += 4;
        FLAC_BLOCK_HEADER Header;
        UINT cbBlock;
        UINT t;
        do
        {
            m_Stream >> Header;
            cbBlock = Header.bySize[2] | Header.bySize[1] << 8 | Header.bySize[0] << 16;
            if (!cbBlock)
                return Result::Length;
            switch (Header.eType & 0b0111'1111)
            {
            case 0:// 流信息
            {
                m_Stream
                    .ReadRev(&m_si.cBlockSampleMin, 2)
                    .ReadRev(&m_si.cBlockSampleMax, 2)
                    .ReadRev(&m_si.cbMinFrame, 3)
                    .Read(&m_si.cbMaxFrame, 3);
                ULONGLONG ull;
                m_Stream >> ull;
                m_si.cSampleRate = (UINT)GetLowNBits(ull, 20);
                m_si.cChannels = (BYTE)GetLowNBits(ull >> 20, 3) + 1;
                m_si.cBitsPerSample = (BYTE)GetLowNBits(ull >> 23, 5) + 1;
                m_si.ullTotalSamples = GetHighNBits(ull, 36);
                m_Stream.Read(m_si.Md5, 16);
                m_posStreamInfoEnd = m_Stream.GetPosition();
            }
            break;
            case 4:// Vorbis注释（小端）
            {
                CRefStrA rsTemp{};

                m_Stream >> t;// 编码器信息大小
                rsTemp.ReSize((int)t);
                m_Stream.Read(rsTemp.Data(), t);
                m_rsVendor.Clear();
                StrU82W(m_rsVendor, rsTemp.Data(), rsTemp.Size());

                UINT cItem;
                m_Stream >> cItem;// 标签数量
                EckCounterNV(cItem)
                {
                    m_Stream >> t;// 标签大小
                    rsTemp.ReSize(t);
                    m_Stream.Read(rsTemp.Data(), t);
                    // 现在rsTemp是"键=值"的格式
                    int pos = rsTemp.FindChar('=');
                    if (pos <= 0 || pos == rsTemp.Size() - 1)
                        continue;
                    ++pos;// Eat "="
                    const int cchActual = rsTemp.Size() - pos;
                    if (rsTemp.IsStartWithI("METADATA_BLOCK_PICTURE"sv))
                    {
                        auto rb = Base64Decode(rsTemp.Data() + pos, cchActual);
                        ParseImageBlock(rb, m_vPic.emplace_back());
                    }
                    else [[likely]]
                    {
                        m_vItem.emplace_back(
                            rsTemp.SubString(0, pos - 1),
                            StrU82W(rsTemp.Data() + pos, cchActual));
                    }
                }
            }
            break;
            case 6:// 图片
            {
                CRefBin rb(cbBlock);
                m_Stream.Read(rb.Data(), cbBlock);
                ParseImageBlock(rb, m_vPic.emplace_back());
            }
            break;
            default:
            {
                if ((Header.eType & 0x7F) == (BYTE)BlockType::Padding)
                    m_Stream += cbBlock;
                else
                {
                    CRefBin rb(cbBlock);
                    m_Stream.Read(rb.Data(), cbBlock);
                    m_vBlock.emplace_back((BlockType)(Header.eType & 0x7F), std::move(rb));
                }
            }
            break;
            }
        } while (!(Header.eType & 0x80));
        m_posFlacTagEnd = m_Stream.GetPosition();
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

    Result WriteTag(UINT uFlags = 0u) noexcept override try
    {
        if (m_File.GetTagLocation().posFlac == CMediaFile::NPos)
            return Result::NoTag;
        if (m_posFlacTagEnd == CMediaFile::NPos ||
            m_posStreamInfoEnd == CMediaFile::NPos)
            InitForWriteTag();
        if (m_posFlacTagEnd == CMediaFile::NPos ||
            m_posStreamInfoEnd == CMediaFile::NPos)
            return Result::Tag;
        CRefBin rbVorbis{}, rbPic{};
        //
        // 序列化Vorbis注释
        //

        rbVorbis.PushBack(4u);// PENDING 块头
        if (m_rsVendor.IsEmpty())
            rbVorbis << 0u;
        else
        {
            const int cchVendor = WideCharToMultiByte(CP_UTF8, 0,
                m_rsVendor.Data(), m_rsVendor.Size(),
                nullptr, 0, nullptr, nullptr);
            rbVorbis << cchVendor;
            WideCharToMultiByte(CP_UTF8, 0,
                m_rsVendor.Data(), m_rsVendor.Size(),
                (CHAR*)rbVorbis.PushBack(cchVendor), cchVendor, nullptr, nullptr);
        }
        size_t posCommCount = rbVorbis.Size();
        UINT cComm = (UINT)m_vItem.size();
        rbVorbis << 0u;// PENDING 注释项目数
        for (const auto& e : m_vItem)
        {
            if (e.rsKey.IsEmpty())
                continue;
            const int cchValue = WideCharToMultiByte(CP_UTF8, 0,
                e.rsValue.Data(), e.rsValue.Size(),
                nullptr, 0, nullptr, nullptr);
            rbVorbis << UINT(cchValue + 1 + e.rsKey.Size())
                << e.rsKey;
            rbVorbis.Back() = '=';
            WideCharToMultiByte(CP_UTF8, 0,
                e.rsValue.Data(), e.rsValue.Size(),
                (CHAR*)rbVorbis.PushBack(cchValue), cchValue, nullptr, nullptr);
        }

        //
        // 序列化图片
        //

        // 预估大小
        size_t cbImageGuess{};
        for (const auto& e : m_vPic)
            cbImageGuess += (e.rsMime.Size() + e.rsDesc.Size() * 2 + e.rbData.Size() + 40);
        rbPic.Reserve(cbImageGuess);

        CRefBin rbTemp{};
        constexpr CHAR Key_MetaDataBlockPicture[]{ "METADATA_BLOCK_PICTURE" };
        for (const auto& e : m_vPic)
        {
            if ((e.byAddtFlags & MIIF_METADATA_BLOCK_PICTURE) ||
                (uFlags & MIF_WRITE_METADATA_BLOCK_PICTURE))
            {
                rbTemp.Clear();
                rbTemp.Reserve(e.rsMime.Size() + e.rsDesc.Size() * 2 + e.rbData.Size() + 40);
                SerializePicture(e, rbTemp);
                // +4 = 跳过开头的长度字段
                auto rs = Base64Encode(rbTemp.Data() + 4, rbTemp.Size() - 4);
                // sizeof(Key)已经比实际长度多1，恰好用于'='
                rbVorbis << UINT(rs.Size() + sizeof(Key_MetaDataBlockPicture));
                rbVorbis << Key_MetaDataBlockPicture;
                rbVorbis.Back() = '=';// 覆盖'\0'
                rbVorbis << rs;
                ++cComm;
            }
            else
                SerializePicture(e, rbPic);
        }
        // BACKFILL 注释项目数
        *(UINT*)(rbVorbis.Data() + posCommCount) = cComm;
        // BACKFILL 块头
        const auto cbData = (UINT)rbVorbis.Size() - 4;
        auto phdrVorbisComm = (FLAC_BLOCK_HEADER*)rbVorbis.Data();
        phdrVorbisComm->bySize[0] = GetIntegerByte<2>(cbData);
        phdrVorbisComm->bySize[1] = GetIntegerByte<1>(cbData);
        phdrVorbisComm->bySize[2] = GetIntegerByte<0>(cbData);
        phdrVorbisComm->eType = (BYTE)BlockType::VorbisComment;

        auto cbNewVorbisAndPic = rbVorbis.Size() + rbPic.Size();
        for (const auto& e : m_vBlock)
            cbNewVorbisAndPic += (e.rbData.Size() + 4);
        // 写入
        const auto cbAvailable = m_posFlacTagEnd - m_posStreamInfoEnd;
        BOOL bHasEnd{};// 末尾块是否已确定
        if (cbAvailable > cbNewVorbisAndPic)
        {
            UINT cbPadding = (UINT)(cbAvailable - cbNewVorbisAndPic);
            if ((uFlags & MIF_REMOVE_PADDING) || cbPadding > 1024)
                m_Stream.Erase(m_posStreamInfoEnd + cbNewVorbisAndPic, cbPadding);
            else
            {
                bHasEnd = TRUE;
                cbPadding -= 4;
                m_Stream.MoveTo(m_posStreamInfoEnd + cbNewVorbisAndPic)
                    // 这种情况下填充肯定为最后一个块
                    << BYTE(0b1000'0000 | (BYTE)BlockType::Padding)
                    << GetIntegerByte<2>(cbPadding)
                    << GetIntegerByte<1>(cbPadding)
                    << GetIntegerByte<0>(cbPadding);
                const auto p = VAlloc(cbPadding);
                EckCheckMem(p);
                m_Stream.Write(p, cbPadding);
                VFree(p);
            }
        }
        else if (cbAvailable < cbNewVorbisAndPic)
            m_Stream.Insert(m_posFlacTagEnd, cbNewVorbisAndPic - cbAvailable);
        // 确定末尾块
        if (m_vBlock.empty() && !bHasEnd)
        {
            phdrVorbisComm->eType |= 0b1000'0000;
            bHasEnd = TRUE;
        }

        m_Stream.MoveTo(m_posStreamInfoEnd);
        m_Stream << rbPic << rbVorbis;
        EckCounter(m_vBlock.size(), i)
        {
            const auto& e = m_vBlock[i];
            if (!bHasEnd && i == m_vBlock.size() - 1)
                m_Stream << BYTE(0b1000'0000_by | (BYTE)e.eType);
            else
                m_Stream << (BYTE)e.eType;
            const auto cbData = (UINT)e.rbData.Size();
            m_Stream << GetIntegerByte<2>(cbData)
                << GetIntegerByte<1>(cbData)
                << GetIntegerByte<0>(cbData)
                << e.rbData;
        }
        m_Stream.GetStream()->Commit(STGC_DEFAULT);
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
        m_vBlock.clear();
        m_si = {};
        m_rsVendor.Clear();
        m_posFlacTagEnd = CMediaFile::NPos;
        m_posStreamInfoEnd = CMediaFile::NPos;
    }

    BOOL IsEmpty() noexcept override { return m_vItem.empty() && m_vPic.empty(); }

    EckInlineNdCe auto& GetItemList() noexcept { return m_vItem; }
    EckInlineNdCe auto& GetItemList() const noexcept { return m_vItem; }
    EckInlineNdCe auto& GetPictureList() noexcept { return m_vPic; }
    EckInlineNdCe auto& GetPictureList() const noexcept { return m_vPic; }
    EckInlineNdCe auto& GetBlockList() noexcept { return m_vBlock; }
    EckInlineNdCe auto& GetBlockList() const noexcept { return m_vBlock; }
    EckInlineNdCe auto& GetStreamInfo() const noexcept { return m_si; }
    EckInlineNdCe auto& GetVendor() noexcept { return m_rsVendor; }
    EckInlineNdCe auto& GetVendor() const noexcept { return m_rsVendor; }

    auto ItmAt(std::string_view svKey) noexcept
    {
        return std::find_if(m_vItem.begin(), m_vItem.end(),
            [&](const ITEM& e) { return e.rsKey.CompareI(svKey) == 0; });
    }
    auto ItmAt(std::string_view svKey) const noexcept
    {
        return std::find_if(m_vItem.begin(), m_vItem.end(),
            [&](const ITEM& e) { return e.rsKey.CompareI(svKey) == 0; });
    }

    auto ItmEndIterator() noexcept { return m_vItem.end(); }
    auto ItmEndIterator() const noexcept { return m_vItem.end(); }

    auto& ItmEnsure(std::string_view svKey) noexcept
    {
        if (const auto it = ItmAt(svKey); it == ItmEndIterator())
            return *it;
        ITEM e{};
        e.rsKey.Assign(svKey);
        return m_vItem.emplace_back(std::move(e));
    }

    void ItmClear() noexcept
    {
        m_vItem.clear();
        m_vPic.clear();
    }
};
ECK_MEDIATAG_NAMESPACE_END
ECK_NAMESPACE_END