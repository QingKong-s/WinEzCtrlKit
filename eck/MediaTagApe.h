#pragma once
#include "MediaTag.h"
#include "FileHelper.h"

ECK_NAMESPACE_BEGIN
ECK_MEDIATAG_NAMESPACE_BEGIN
class CApe : public CTag
{
private:
    enum class ItemType
    {
        String, // utf-8
        Binary,
        Link,
        Reserved
    };

    struct ITEM
    {
        ItemType eType;
        CRefStrA rsKey;
        std::variant<std::vector<CRefStrW>, CRefBin> Value;
        CRefStrA rsU8Cache;// 调用方可以随意修改此字段

        EckInlineCe auto& EnsureStrList(ItemType eType_ = ItemType::String) noexcept
        {
            eType = eType_;
            if (Value.index() != 0)
                return Value.emplace<0>();
            return std::get<0>(Value);
        }
        EckInlineCe auto& EnsureBin() noexcept
        {
            eType = ItemType::Binary;
            if (Value.index() != 1)
                return Value.emplace<1>();
            return std::get<1>(Value);
        }
        EckInlineNdCe auto& GetStrList() noexcept { return std::get<0>(Value); }
        EckInlineNdCe auto& GetStrList() const noexcept { return std::get<0>(Value); }
        EckInlineNdCe auto& GetBin() noexcept { return std::get<1>(Value); }
        EckInlineNdCe auto& GetBin() const noexcept { return std::get<1>(Value); }
    };

    struct PIC
    {
        PicType eType;
        eck::CRefStrA rsKey;
        eck::CRefStrW rsDesc;
        eck::CRefBin rbData;
        CRefStrA rsU8Cache;// 调用方可以随意修改此字段
    };

    size_t m_cbTag{};
    APE_HEADER m_Hdr{};

    std::vector<ITEM> m_vItem{};
    std::vector<PIC> m_vPic{};

    EckInlineNdCe static ItemType GetItemType(UINT uFlags) noexcept
    {
        return (ItemType)GetLowNBits(uFlags >> 1, 2);
    }

    static BOOL IsStringItem(const CRefStrA& rsKey) noexcept
    {
        return
            rsKey.CompareI("TITLE"sv) == 0 ||
            rsKey.CompareI("ARTIST"sv) == 0 ||
            rsKey.CompareI("ALBUM"sv) == 0 ||
            rsKey.CompareI("LYRICS"sv) == 0 ||
            rsKey.CompareI("COMMENT"sv) == 0 ||
            rsKey.CompareI("GENRE"sv) == 0 ||
            rsKey.CompareI("TRACKNUMBER"sv) == 0 ||
            rsKey.CompareI("TRACK"sv) == 0 ||
            rsKey.CompareI("TRACKTOTAL"sv) == 0 ||
            rsKey.CompareI("TOTALTRACKS"sv) == 0;
    }
public:
    CApe(CMediaFile& mf) noexcept : CTag{ mf } {}

    Result SimpleGet(MUSICINFO& mi, const SIMPLE_OPT& Opt) noexcept override
    {
#undef ECKTEMP_GET_VAL
#define ECKTEMP_GET_VAL(MiField)  \
        if (e.GetStrList().empty()) \
            MiField.Clear();        \
        else                        \
            if (bMove)              \
                MiField = std::move(e.GetStrList()[0]); \
            else                    \
                MiField = e.GetStrList()[0]; \

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
                for (const auto& f : e.GetStrList())
                    mi.slArtist.PushBackString(f, Opt.svArtistDiv);
                mi.uMaskChecked |= MIM_ARTIST;
            }
            else if ((mi.uMask & MIM_ALBUM) &&
                e.rsKey.CompareI("ALBUM"sv) == 0)
            {
                ECKTEMP_GET_VAL(mi.rsAlbum);
                mi.uMaskChecked |= MIM_ALBUM;
            }
            else if (!!(mi.uMask & MIM_LRC) && (
                // 以下三个都有使用
                e.rsKey.CompareI("LYRICS"sv) == 0 ||
                e.rsKey.CompareI("UNSYNCED LYRICS"sv) == 0 ||
                e.rsKey.CompareI("UNSYNCEDLYRICS"sv) == 0))
            {
                ECKTEMP_GET_VAL(mi.rsLrc);
                mi.uMaskChecked |= MIM_LRC;
            }
            else if ((mi.uMask & MIM_COMMENT) &&
                e.rsKey.CompareI("COMMENT"sv) == 0)
            {
                for (const auto& f : e.GetStrList())
                    mi.slComment.PushBackString(f, Opt.svCommDiv);
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
                if (e.eType != ItemType::String)
                    continue;
                if (e.GetStrList().empty() || e.GetStrList()[0].IsEmpty())
                {
                    mi.uMaskChecked |= MIM_TRACK;
                    continue;
                }
                const auto bTrackNum = (e.rsKey.CompareI("TRACKNUMBER"sv) == 0);
                if (bTrackNum || e.rsKey.CompareI("TRACK"sv) == 0)
                {
                    const auto n = mi.cTotalTrack;
                    if (TagGetNumberAndTotal(e.GetStrList()[0].ToStringView(),
                        mi.nTrack, mi.cTotalTrack))
                    {
                        mi.uMaskChecked |= MIM_TRACK;
                        // 防止覆盖总轨为0
                        if (bTrackNum && mi.cTotalTrack <= 0)
                            mi.cTotalTrack = n;
                    }
                }
                // 应使用TOTALTRACKS
                else if (e.rsKey.CompareI("TOTALTRACKS"sv) == 0 ||
                    e.rsKey.CompareI("TRACKTOTAL"sv) == 0)
                {
                    const auto& rs = e.GetStrList()[0];
                    if (TcsToInt(rs.Data(), rs.Size(), mi.cTotalTrack) == TcsCvtErr::Ok)
                        mi.uMaskChecked |= MIM_TRACK;
                }
            }
        }
#undef ECKTEMP_GET_VAL
        if (mi.uMask & MIM_COVER)
        {
            if (!m_vPic.empty())
                mi.uMaskChecked |= MIM_COVER;
            for (auto& e : m_vPic)
            {
                if (bMove)
                    mi.vPic.emplace_back(
                        e.eType,
                        FALSE,
                        std::move(e.rsDesc),
                        CRefStrA{},
                        std::move(e.rbData));
                else
                {
                    auto& f = mi.vPic.emplace_back(
                        e.eType,
                        FALSE,
                        e.rsDesc,
                        CRefStrA{},
                        CRefBin(e.rbData.Size()));
                    memcpy(f.GetPictureData().Data(),
                        e.rbData.Data(), e.rbData.Size());
                }
            }
        }
        return Result::Ok;
    }

    Result SimpleSet(MUSICINFO& mi, const SIMPLE_OPT& Opt) noexcept override
    {
        for (auto it = m_vItem.begin(); it != m_vItem.end(); )
        {
            const auto& e = *it;
            if ((mi.uMask & MIM_TITLE) &&
                e.rsKey.CompareI("TITLE"sv) == 0)
                it = m_vItem.erase(it);
            else if ((mi.uMask & MIM_ARTIST) &&
                e.rsKey.CompareI("ARTIST"sv) == 0)
                it = m_vItem.erase(it);
            else if ((mi.uMask & MIM_ALBUM) &&
                e.rsKey.CompareI("ALBUM"sv) == 0)
                it = m_vItem.erase(it);
            else if ((mi.uMask & MIM_LRC) && (
                e.rsKey.CompareI("LYRICS"sv) == 0 ||
                e.rsKey.CompareI("UNSYNCED LYRICS"sv) == 0 ||
                e.rsKey.CompareI("UNSYNCEDLYRICS"sv) == 0))
                it = m_vItem.erase(it);
            else if ((mi.uMask & MIM_COMMENT) &&
                e.rsKey.CompareI("COMMENT"sv) == 0)
                it = m_vItem.erase(it);
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
                e.EnsureStrList().emplace_back(std::move(MiField)); \
            else                         \
                e.EnsureStrList().emplace_back(MiField); \
            m_vItem.emplace_back(std::move(e));          \
        }

#undef ECKTEMP_SET_VAL_LIST
#define ECKTEMP_SET_VAL_LIST(MiField, KeyStr)   \
        {                                       \
            ITEM e{};                           \
            e.rsKey.Assign(KeyStr);          \
            auto& v = e.EnsureStrList();        \
            for (const auto f : MiField)        \
                v.emplace_back(f);              \
            m_vItem.emplace_back(std::move(e)); \
        }

        const auto bMove = Opt.uFlags & SMOF_MOVE;
        if (mi.uMask & MIM_TITLE)
            ECKTEMP_SET_VAL(mi.rsTitle, "TITLE"sv);
        if (mi.uMask & MIM_ARTIST)
            ECKTEMP_SET_VAL_LIST(mi.slArtist, "ARTIST"sv);
        if (mi.uMask & MIM_ALBUM)
            ECKTEMP_SET_VAL(mi.rsAlbum, "ALBUM"sv);
        if (mi.uMask & MIM_COMMENT)
            ECKTEMP_SET_VAL_LIST(mi.slComment, "COMMENT"sv);
        if (mi.uMask & MIM_LRC)
            ECKTEMP_SET_VAL(mi.rsLrc, "UNSYNCED LYRICS"sv);
        if (mi.uMask & MIM_GENRE)
            ECKTEMP_SET_VAL(mi.rsGenre, "GENRE"sv);
        if (mi.uMask & MIM_TRACK)
        {
            WCHAR szBuf[TcsCvtCalcBufferSize<UINT>() * 2 + 1];
            PWCH p{};
            std::string_view svKey{};
            if (mi.nTrack >= 0)// 尝试写入音轨/总轨
            {
                svKey = "TRACK"sv;
                TcsFromInt(szBuf, ARRAYSIZE(szBuf), mi.nTrack, 10, TRUE, &p);
                if (mi.cTotalTrack)
                {
                    *p++ = L'/';
                    TcsFromInt(p, size_t(szBuf + ARRAYSIZE(szBuf) - p),
                        mi.cTotalTrack, 10, TRUE, &p);
                }
            }
            else if (mi.cTotalTrack > 0)// 尝试写入总轨
            {
                svKey = "TOTALTRACKS"sv;
                TcsFromInt(szBuf, ARRAYSIZE(szBuf), mi.cTotalTrack, 10, TRUE, &p);
            }

            if (!svKey.empty() && p != szBuf)
            {
                ITEM e{};
                e.rsKey.Assign(svKey);
                e.EnsureStrList().emplace_back(CRefStrW{ szBuf, int(p - szBuf) });
                m_vItem.emplace_back(std::move(e));
            }
        }
#undef ECKTEMP_SET_VAL
#undef ECKTEMP_SET_VAL_LIST

        if (mi.uMask & MIM_COVER)
        {
            m_vPic.clear();
            for (auto& e : mi.vPic)
            {
                const auto eType = (
                    e.eType == PicType::Invalid &&
                    (mi.uFlag & MIF_APE_INVALID_COVER_AS_FRONT) ?
                    PicType::CoverFront : e.eType);
                CRefBin rbData{};
                if (e.bLink)
                    rbData = ReadInFile(e.GetPicturePath().Data());
                else
                {
                    if (bMove)
                        rbData = std::move(e.GetPictureData());
                    else
                    {
                        const auto& rb = e.GetPictureData();
                        rbData.ReSize(rb.Size());
                        memcpy(rbData.Data(), rb.Data(), rb.Size());
                    }
                }
                auto& f = m_vPic.emplace_back(
                    eType,
                    CRefStrA{},
                    e.rsDesc,
                    std::move(rbData));
                if (eType == PicType::Invalid)
                    f.rsKey.Assign(EckStrAndLen("Cover Art"));
                else
                {
                    f.rsKey.Assign(EckStrAndLen("Cover Art ("));
                    f.rsKey.PushBack(TagPictureTypeToApeString(eType));
                    f.rsKey.PushBackChar(')');
                }
            }
        }
        return Result::Ok;
    }

    Result ReadTag(UINT uFlags = 0u) noexcept override try
    {
        const auto& Loc = m_File.GetTagLocation();
        if (Loc.posApeHdr == SIZETMax)
            return Result::NoTag;
        m_Stream.MoveTo(Loc.posApeHdr) >> m_Hdr;
        if (!TagCheckApeHeader(m_Hdr))
            return Result::Tag;
        BYTE* const pBuf = (BYTE*)VAlloc(m_Hdr.cbBody + 4);
        if (!pBuf)
            return Result::OutOfMemory;
        const UniquePtr<DelVA<BYTE>> _{ pBuf };
        m_Stream.MoveTo(Loc.posApe).Read(pBuf, m_Hdr.cbBody);

        CMemReader w{ pBuf, m_Hdr.cbBody };
        UINT cbVal, uItemFlags;

        EckCounterNV(m_Hdr.cItems)
        {
            ITEM e{};
            w >> cbVal >> uItemFlags;
            if (cbVal > w.GetLeaveSize())
                return Result::Length;
            e.eType = GetItemType(uItemFlags);
            w >> e.rsKey;
            if (e.rsKey.IsStartWithI(EckStrAndLen("Cover Art")))
            {
                // 解析图片类型
                PicType eType = PicType::Invalid;
                const auto posBracket0 = e.rsKey.FindChar('(');
                if (posBracket0 >= 0)
                {
                    const auto cchType =
                        e.rsKey.FindChar(')', posBracket0 + 1) - posBracket0 - 1;
                    if (cchType > 0)
                    {
                        EckCounter(ARRAYSIZE(ApePicType), i)
                        {
                            const auto& sv = ApePicType[i];
                            if (TcsEqualLen2I(sv.data(), sv.size(),
                                e.rsKey.Data() + posBracket0 + 1,
                                cchType))
                            {
                                eType = (PicType)i;
                                break;
                            }
                        }
                    }
                }
                // 备注
                const auto pBaseU8 = (PCCH)w.Data();
                const auto cchDesc = (int)TcsLen(pBaseU8);
                if (cchDesc + 1 > (int)cbVal)
                    return Result::Length;
                //
                const auto cbData = size_t(cbVal - cchDesc - 1);
                auto& f = m_vPic.emplace_back(
                    eType,
                    std::move(e.rsKey),
                    StrU82W(pBaseU8, cchDesc),
                    CRefBin(cbData));
                memcpy(f.rbData.Data(), w.Data() + cchDesc + 1, cbData);
                w += cbVal;
            }
            else if (e.eType == ItemType::Link ||
                e.eType == ItemType::String ||
                IsStringItem(e.rsKey))
            {
                const auto pBaseU8 = (PCCH)w.Data();
                w += cbVal;
                int pos0{}, pos1{};
                for (; pos1 < (int)cbVal; ++pos1)
                {
                    if (pBaseU8[pos1] == '\0')
                    {
                        if (pos1 > pos0)
                            e.GetStrList().emplace_back(
                                StrU82W(pBaseU8 + pos0, pos1 - pos0));
                        pos0 = pos1 + 1;
                    }
                }
                if (pos1 > pos0)
                    e.GetStrList().emplace_back(
                        StrU82W(pBaseU8 + pos0, pos1 - pos0));
                m_vItem.emplace_back(std::move(e));
            }
            else
            {
                e.EnsureBin();
                e.GetBin().ReSize(cbVal);
                memcpy(e.GetBin().Data(), w.Data(), cbVal);
                w += cbVal;
                m_vItem.emplace_back(std::move(e));
            }
        }
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
    catch (const CMemReader::Xpt&)
    {
        return Result::Length;
    }

    Result WriteTag(UINT uFlags = 0u) noexcept override try
    {
        // 提前计算固定8字节+1字节键结束符大小
        size_t cbBody = (m_vItem.size() + m_vPic.size()) * (4 + 4 + 1);
        for (auto& e : m_vItem)
        {
            cbBody += e.rsKey.Size();
            switch (e.eType)
            {
            case ItemType::Link:
            case ItemType::String:
            {
                e.rsU8Cache.Clear();
                BOOL bAppendNull{};
                for (auto& s : e.GetStrList())
                {
                    if (bAppendNull)
                        e.rsU8Cache.PushBackChar('\0');
                    StrW2U8(e.rsU8Cache, s.Data(), s.Size());
                    bAppendNull = TRUE;
                }
                cbBody += e.rsU8Cache.Size();
            }
            break;
            case ItemType::Reserved:
            case ItemType::Binary:
                cbBody += e.GetBin().Size();
                break;
            default:
                ECK_UNREACHABLE;
            }
        }
        for (auto& e : m_vPic)
        {
            e.rsU8Cache.Clear();
            StrW2U8(e.rsU8Cache, e.rsDesc.Data(), e.rsDesc.Size());
            cbBody += (e.rsKey.Size() + e.rsU8Cache.Size() + 1 + e.rbData.Size());
        }

        APE_HEADER Hdr;
        memcpy(Hdr.byPreamble, "APETAGEX", 8);
        Hdr.dwVer = 2000;
        Hdr.cbBody = UINT(cbBody + 32);
        Hdr.cItems = UINT(m_vItem.size() + m_vPic.size());
        Hdr.dwFlags = APE_HAS_HEADER | APE_HAS_FOOTER;
        ZeroMemory(Hdr.byReserved, sizeof(Hdr.byReserved));

        const size_t cbTotal = Hdr.cbBody + 32;

        size_t cbPadding{};
        const auto& Loc = m_File.GetTagLocation();
        if (Loc.posApeTag != SIZETMax)
            if (Loc.bPrependApe != !!(uFlags & MIF_PREPEND_TAG))
            {
                m_Stream.Erase(Loc.posApeTag, Loc.cbApeTag);
                goto NewTag;
            }
            else
            {
                if (Loc.cbApeTag < cbTotal)
                    m_Stream.Insert(Loc.posApeTag + Loc.cbApeTag,
                        cbTotal - Loc.cbApeTag);
                else
                {
                    cbPadding = Loc.cbApeTag - cbTotal;
                    if ((uFlags & MIF_ALLOW_PADDING) &&
                        cbPadding > 4 + 4 + 5/*Dummy*/ + 1 &&
                        cbPadding <= 1024)
                        cbPadding -= (4 + 4 + 5/*Dummy*/ + 1);
                    else
                    {
                        const auto pos = Loc.posApeTag +
                            Loc.cbApeTag - cbPadding;
                        m_Stream.Erase(pos, cbPadding);
                        cbPadding = 0;
                    }
                }
                m_Stream.MoveTo(Loc.posApeTag);
            }
        else
        {
        NewTag:
            if (uFlags & MIF_PREPEND_TAG)
            {
                m_Stream.Insert(0, cbTotal);
                m_Stream.MoveToBegin();
            }
            else
            {
                const auto& Loc = m_File.GetTagLocation();
                const auto cbFile = m_Stream.GetSize();
                size_t pos;
                if (Loc.posV1Ext != CMediaFile::NPos)
                    pos = Loc.posV1Ext;
                else if (Loc.posV1 != CMediaFile::NPos)
                    pos = Loc.posV1;
                else
                    pos = cbFile;
                if (pos > cbFile)
                    return Result::Stream;
                m_Stream.Insert(pos, cbTotal);
                m_Stream.MoveTo(pos);
            }
        }

        Hdr.dwFlags |= APE_IS_HEADER;
        m_Stream << Hdr;
        Hdr.dwFlags &= ~APE_IS_HEADER;

        for (const auto& e : m_vItem)
        {
            const auto uItemFlags = (UINT)e.eType << 1;
            switch (e.eType)
            {
            case ItemType::Link:
            case ItemType::String:
            {
                m_Stream << (UINT)e.rsU8Cache.Size() << uItemFlags << e.rsKey;
                m_Stream.Write(e.rsU8Cache.Data(), e.rsU8Cache.Size());
            }
            break;
            case ItemType::Reserved:
            case ItemType::Binary:
                m_Stream << (UINT)e.GetBin().Size() << uItemFlags << e.rsKey << e.GetBin();
                break;
            }
        }
        for (const auto& e : m_vPic)
        {
            const auto dwFlag = (UINT)ItemType::Binary << 1;
            m_Stream << UINT(e.rbData.Size() + e.rsU8Cache.Size() + 1)
                << dwFlag << e.rsKey
                << e.rsU8Cache << e.rbData;
        }

        if (cbPadding)
        {
            const UniquePtr<DelVA<void>> p{ VAlloc(cbPadding) };
            m_Stream << (UINT)cbPadding
                << ((UINT)ItemType::Binary << 1)
                << "Dummy"sv;
            m_Stream.Write(p.get(), cbPadding);
        }

        m_Stream << Hdr;
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

    void Reset() noexcept override { ItmClear(); }
    BOOL IsEmpty() noexcept override { return m_vItem.empty() && m_vPic.empty(); }

    EckInlineNdCe auto& GetItemList() noexcept { return m_vItem; }
    EckInlineNdCe auto& GetItemList() const noexcept { return m_vItem; }
    EckInlineNdCe auto& GetPictureList() noexcept { return m_vPic; }
    EckInlineNdCe auto& GetPictureList() const noexcept { return m_vPic; }

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