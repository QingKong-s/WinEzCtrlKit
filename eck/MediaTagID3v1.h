#pragma once
#include "MediaTag.h"

ECK_NAMESPACE_BEGIN
ECK_MEDIATAG_NAMESPACE_BEGIN
struct ID3_GENRE
{
    std::wstring_view ChsName;
    std::wstring_view EngName;
};
constexpr inline ID3_GENRE Id3Genre[]
{
    { L"蓝调"sv, L"Blues"sv },
    { L"经典摇滚"sv, L"Classic Rock"sv },
    { L"乡村"sv, L"Country"sv },
    { L"舞曲"sv, L"Dance"sv },
    { L"迪斯科"sv, L"Disco"sv },
    { L"放克"sv, L"Funk"sv },
    { L"垃圾摇滚"sv, L"Grunge"sv },
    { L"嘻哈"sv, L"Hip-Hop"sv },
    { L"爵士乐"sv, L"Jazz"sv },
    { L"金属乐"sv, L"Metal"sv },
    { L"新世纪"sv, L"New Age"sv },
    { L"老歌"sv, L"Oldies"sv },
    { L"其他"sv, L"Other"sv },
    { L"流行乐"sv, L"Pop"sv },
    { L"节奏蓝调"sv, L"R&B"sv },
    { L"说唱"sv, L"Rap"sv },
    { L"雷鬼"sv, L"Reggae"sv },
    { L"摇滚乐"sv, L"Rock"sv },
    { L"高科技舞曲"sv, L"Techno"sv },
    { L"工业音乐"sv, L"Industrial"sv },
    { L"另类音乐"sv, L"Alternative"sv },
    { L"斯卡"sv, L"Ska"sv },
    { L"死亡金属"sv, L"Death Metal"sv },
    { L"恶作剧"sv, L"Pranks"sv },
    { L"原声音乐"sv, L"Soundtrack"sv },
    { L"欧洲泰克诺"sv, L"Euro-Techno"sv },
    { L"氛围音乐"sv, L"Ambient"sv },
    { L"吹泡音乐"sv, L"Trip-Hop"sv },
    { L"人声"sv, L"Vocal"sv },
    { L"爵士+放克"sv, L"Jazz+Funk"sv },
    { L"融合爵士乐"sv, L"Fusion"sv },
    { L"出神音乐"sv, L"Trance"sv },
    { L"古典音乐"sv, L"Classical"sv },
    { L"器乐"sv, L"Instrumental"sv },
    { L"酸性音乐"sv, L"Acid"sv },
    { L"浩室音乐"sv, L"House"sv },
    { L"游戏"sv, L"Game"sv },
    { L"声音剪辑"sv, L"Sound Clip"sv },
    { L"福音音乐"sv, L"Gospel"sv },
    { L"噪音"sv, L"Noise"sv },
    { L"另类摇滚"sv, L"Alternative Rock"sv },
    { L"贝斯"sv, L"Bass"sv },
    { L"灵魂乐"sv, L"Soul"sv },
    { L"朋克"sv, L"Punk"sv },
    { L"空间音乐"sv, L"Space"sv },
    { L"冥想"sv, L"Meditative"sv },
    { L"流行器乐"sv, L"Instrumental Pop"sv },
    { L"器乐摇滚"sv, L"Instrumental Rock"sv },
    { L"民族特色音乐"sv, L"Ethnic"sv },
    { L"哥特音乐"sv, L"Gothic"sv },
    { L"暗波"sv, L"Darkwave"sv },
    { L"工业泰克诺"sv, L"Techno-Industrial"sv },
    { L"电子乐"sv, L"Electronic"sv },
    { L"流行民谣"sv, L"Pop-Folk"sv },
    { L"欧陆舞曲"sv, L"Eurodance"sv },
    { L"梦幻"sv, L"Dream"sv },
    { L"南方摇滚"sv, L"Southern Rock"sv },
    { L"喜剧"sv, L"Comedy"sv },
    { L"邪典"sv, L"Cult"sv },
    { L"帮派"sv, L"Gangsta"sv },
    { L"40强"sv, L"Top 40"sv },
    { L"基督教说唱"sv, L"Christian Rap"sv },
    { L"流行/放克"sv, L"Pop/Funk"sv },
    { L"丛林"sv, L"Jungle"sv },
    { L"美洲原住民音乐"sv, L"Native American"sv },
    { L"卡巴莱"sv, L"Cabaret"sv },
    { L"新浪潮"sv, L"New Wave"sv },
    { L"迷幻音乐"sv, L"Psychedelic"sv },
    { L"锐舞"sv, L"Rave"sv },
    { L"表演曲调"sv, L"Showtunes"sv },
    { L"预告片"sv, L"Trailer"sv },
    { L"低保真"sv, L"Lo-Fi"sv },
    { L"部落音乐"sv, L"Tribal"sv },
    { L"酸性朋克"sv, L"Acid Punk"sv },
    { L"酸性爵士"sv, L"Acid Jazz"sv },
    { L"波尔卡"sv, L"Polka"sv },
    { L"复古"sv, L"Retro"sv },
    { L"音乐剧"sv, L"Musical"sv },
    { L"摇滚"sv, L"Rock & Roll"sv },
    { L"硬式摇滚"sv, L"Hard Rock"sv },
    { L"民俗音乐"sv, L"Folk"sv },
    { L"民谣摇滚"sv, L"Folk-Rock"sv },
    { L"民俗音乐"sv, L"National Folk"sv },
    { L"摇摆乐"sv, L"Swing"sv },
    { L"快速融合乐"sv, L"Fast Fusion"sv },
    { L"比博普爵士"sv, L"Bebop"sv },
    { L"拉丁音乐"sv, L"Latin"sv },
    { L"复兴音乐"sv, L"Revival"sv },
    { L"凯尔特音乐"sv, L"Celtic"sv },
    { L"蓝草音乐"sv, L"Bluegrass"sv },
    { L"前卫音乐"sv, L"Avantgarde"sv },
    { L"哥特摇滚"sv, L"Gothic Rock"sv },
    { L"前卫摇滚"sv, L"Progressive Rock"sv },
    { L"迷幻摇滚"sv, L"Psychedelic Rock"sv },
    { L"交响摇滚"sv, L"Symphonic Rock"sv },
    { L"慢摇滚"sv, L"Slow Rock"sv },
    { L"大乐团"sv, L"Big Band"sv },
    { L"合唱"sv, L"Chorus"sv },
    { L"轻松音乐"sv, L"Easy Listening"sv },
    { L"原声音乐"sv, L"Acoustic"sv },
    { L"幽默"sv, L"Humour"sv },
    { L"朗诵"sv, L"Speech"sv },
    { L"香颂"sv, L"Chanson"sv },
    { L"歌剧"sv, L"Opera"sv },
    { L"室内乐"sv, L"Chamber Music"sv },
    { L"奏鸣曲"sv, L"Sonata"sv },
    { L"交响曲"sv, L"Symphony"sv },
    { L"电臀贝斯"sv, L"Booty Bass"sv },
    { L"Primus"sv, L"Primus"sv },
    { L"色情律动"sv, L"Porn Groove"sv },
    { L"讽刺音乐"sv, L"Satire"sv },
    { L"慢板音乐"sv, L"Slow Jam"sv },
    { L"俱乐部音乐"sv, L"Club"sv },
    { L"探戈"sv, L"Tango"sv },
    { L"桑巴"sv, L"Samba"sv },
    { L"民间传说音乐"sv, L"Folklore"sv },
    { L"谣曲"sv, L"Ballad"sv },
    { L"力量抒情歌"sv, L"Power Ballad"sv },
    { L"节奏灵魂乐"sv, L"Rhythmic Soul"sv },
    { L"即兴饶舌"sv, L"Freestyle"sv },
    { L"二重奏"sv, L"Duet"sv },
    { L"朋克摇滚"sv, L"Punk Rock"sv },
    { L"鼓独奏"sv, L"Drum Solo"sv },
    { L"无伴奏合唱"sv, L"A cappella"sv },
    { L"欧陆浩室"sv, L"Euro-House"sv },
    { L"舞厅音乐"sv, L"Dance Hall"sv },
    { L"果阿出神"sv, L"Goa"sv },
    { L"鼓打贝斯"sv, L"Drum & Bass"sv },
    { L"夜店浩室"sv, L"Club-House"sv },
    { L"硬核音乐"sv, L"Hardcore"sv },
    { L"恐核音乐"sv, L"Terror"sv },
    { L"独立音乐"sv, L"Indie"sv },
    { L"英伦摇滚"sv, L"BritPop"sv },
    { L"世界节拍"sv, L"Worldbeat"sv },
    { L"波兰朋克"sv, L"Polsk Punk"sv },
    { L"节拍音乐"sv, L"Beat"sv },
    { L"基督教帮派说唱"sv, L"Christian Gangsta Rap"sv },
    { L"重金属乐"sv, L"Heavy Metal"sv },
    { L"黑金属乐"sv, L"Black Metal"sv },
    { L"跨界音乐"sv, L"Crossover"sv },
    { L"当代基督教音乐"sv, L"Contemporary Christian"sv },
    { L"福音摇滚"sv, L"Christian Rock"sv },
    { L"梅伦格舞曲"sv, L"Merengue"sv },
    { L"萨尔萨音乐"sv, L"Salsa"sv },
    { L"敲击金属"sv, L"Thrash Metal"sv },
    { L"动画音乐"sv, L"Anime"sv },
    { L"日本流行"sv, L"JPop"sv },
    { L"合成器流行"sv, L"Synthpop"sv },
    { L"抽象音乐"sv, L"Abstract"sv },
    { L"艺术摇滚"sv, L"Art Rock"sv },
    { L"巴洛克音乐"sv, L"Baroque"sv },
    { L"巴恩格拉"sv, L"Bhangra"sv },
    { L"重拍"sv, L"Big Beat"sv },
    { L"碎拍"sv, L"Breakbeat"sv },
    { L"驰放"sv, L"Chillout"sv },
    { L"缓拍"sv, L"Downtempo"sv },
    { L"回响音乐"sv, L"Dub"sv },
    { L"电子身体舞曲"sv, L"EBM"sv },
    { L"混合风格"sv, L"Eclectic"sv },
    { L"电子放克"sv, L"Electro"sv },
    { L"电子冲击"sv, L"Electroclash"sv },
    { L"情绪摇滚"sv, L"Emo"sv },
    { L"实验音乐"sv, L"Experimental"sv },
    { L"车库浩室"sv, L"Garage"sv },
    { L"全球音乐"sv, L"Global"sv },
    { L"智能舞曲"sv, L"IDM"sv },
    { L"病气氛围"sv, L"Illbient"sv },
    { L"工业哥特"sv, L"Industro-Goth"sv },
    { L"即兴乐队"sv, L"Jam Band"sv },
    { L"酸菜摇滚"sv, L"Krautrock"sv },
    { L"左域音乐"sv, L"Leftfield"sv },
    { L"休闲音乐"sv, L"Lounge"sv },
    { L"数学摇滚"sv, L"Math Rock"sv },
    { L"新浪漫主义"sv, L"New Romantic"sv },
    { L"新派碎拍"sv, L"Nu-Breakz"sv },
    { L"后朋克"sv, L"Post-Punk"sv },
    { L"后摇滚"sv, L"Post-Rock"sv },
    { L"迷幻出神"sv, L"Psytrance"sv },
    { L"瞪鞋摇滚"sv, L"Shoegaze"sv },
    { L"太空摇滚"sv, L"Space Rock"sv },
    { L"热带摇滚"sv, L"Trop Rock"sv },
    { L"世界音乐"sv, L"World Music"sv },
    { L"新古典主义音乐"sv, L"Neoclassical"sv },
    { L"有声读物"sv, L"Audiobook"sv },
    { L"广播剧"sv, L"Audio Theatre"sv },
    { L"新德国浪潮"sv, L"Neue Deutsche Welle"sv },
    { L"播客"sv, L"Podcast"sv },
    { L"独立摇滚"sv, L"Indie Rock"sv },
    { L"帮派放克"sv, L"G-Funk"sv },
    { L"回响贝斯"sv, L"Dubstep"sv },
    { L"车库摇滚"sv, L"Garage Rock"sv },
    { L"迷幻驰放"sv, L"Psybient"sv },
};

class CID3v1 : public CTag
{
public:
    enum class Speed : BYTE
    {
        None,
        Slow,
        Middle,
        Fast,
        VeryFast
    };
private:
    struct INFO
    {
        CStringW rsTitle{};
        CStringW rsArtist{};
        CStringW rsAlbum{};
        CStringW rsComment{};
        USHORT usYear{};
        BYTE byTrack{};
        BYTE byGenre{ 0xFF };

        // 以下为ID3v1.2信息

        Speed eSpeed{};
        CStringW rsGenre{};
        UINT uBeginSec{};
        UINT uEndSec{};
    };

    INFO m_Info{};
    BOOL m_bEmpty{ TRUE };

    static void TagpZeroTail(CStringA& rs, int cchMax) noexcept
    {
        EckAssert(rs.Capacity() >= cchMax);
        if (rs.Size() < cchMax)
            ZeroMemory(rs.Data() + rs.Size(), cchMax - rs.Size());
    }

    void TagpWriteString(const CStringW& rs, int cchMax, CStringA& rsWork) noexcept
    {
        rsWork.Clear();
        StrW2X(rsWork, rs.Data(), std::min(rs.Size(), cchMax + 8));
        TagpZeroTail(rsWork, cchMax);
        m_Stream.Write(rsWork.Data(), cchMax);
    }

    void TagpWriteSecond(UINT uSecTotal, CStringA& rsWork) noexcept
    {
        constexpr BYTE ZeroBytes[6]{};

        const UINT uMin = uSecTotal / 60;
        const UINT uSec = uSecTotal % 60;
        if (uMin > 999)
            m_Stream.Write(ZeroBytes, 6);
        else
        {
            EckAssert(rsWork.Capacity() > 6);
            if (sprintf_s(rsWork.Data(), rsWork.Capacity(), "%03u:%02u", uMin, uSec) == 6)
                m_Stream.Write(rsWork.Data(), 6);
            else
                m_Stream.Write(ZeroBytes, 6);
        }
    }

    void TagpReadString(
        _Out_writes_(cchMax) PCH pszWork,
        int cchMax,
        _Out_ int& cchNew,
        CStringW& rs) noexcept
    {
        m_Stream.Read(pszWork, cchMax);
        if (pszWork[cchMax - 1])
            cchNew = cchMax;
        else
            cchNew = (int)TcsLen(pszWork);
        rs.Clear();
        StrX2W(rs, pszWork, cchNew);
    }
public:
    CID3v1(CMediaFile& File) noexcept : CTag{ File } {}

    Result SimpleGet(MUSICINFO& mi, const SIMPLE_OPT& Opt) noexcept override
    {
        mi.Clear();
        if (m_File.GetTagLocation().posV1 == CMediaFile::NPos)
            return Result::NoTag;
        const auto bMove = Opt.uFlags & SMOF_MOVE;

#undef ECKTEMP_GET_VAL
#define ECKTEMP_GET_VAL(Field)       \
        if (bMove)                   \
            mi.Field = std::move(m_Info.Field); \
        else                         \
            mi.Field = m_Info.Field;

        if (mi.uMask & MIM_TITLE)
        {
            ECKTEMP_GET_VAL(rsTitle);
            mi.uMaskChecked |= MIM_TITLE;
        }
        if (mi.uMask & MIM_ARTIST)
        {
            mi.slArtist.PushBackString(m_Info.rsArtist, Opt.svArtistDiv);
            mi.uMaskChecked |= MIM_ARTIST;
        }
        if (mi.uMask & MIM_ALBUM)
        {
            ECKTEMP_GET_VAL(rsAlbum);
            mi.uMaskChecked |= MIM_ALBUM;
        }
        if (mi.uMask & MIM_COMMENT)
        {
            mi.slComment.PushBackString(m_Info.rsComment, Opt.svCommDiv);
            mi.uMaskChecked |= MIM_COMMENT;
        }
        if (mi.uMask & MIM_GENRE)
        {
            if (m_Info.rsGenre.IsEmpty())
            {
                if (m_Info.byGenre < ARRAYSIZE(Id3Genre))
                {
                    mi.rsGenre = Id3Genre[m_Info.byGenre].EngName;
                    mi.uMaskChecked |= MIM_GENRE;
                }
            }
            else
            {
                ECKTEMP_GET_VAL(rsGenre);
                mi.uMaskChecked |= MIM_GENRE;
            }
        }
        if (mi.uMask & MIM_TRACK)
        {
            mi.nTrack = m_Info.byTrack;
            mi.cTotalTrack = 0;
            mi.uMaskChecked |= MIM_TRACK;
        }
#undef ECKTEMP_GET_VAL
        return Result::Ok;
    }

    Result SimpleSet(MUSICINFO& mi, const SIMPLE_OPT& Opt) noexcept override
    {
        const auto bMove = Opt.uFlags & SMOF_MOVE;
#undef ECKTEMP_SET_VAL
#define ECKTEMP_SET_VAL(Field)       \
        if (bMove)                   \
            m_Info.Field = std::move(mi.Field); \
        else                         \
            m_Info.Field = mi.Field;

#undef ECKTEMP_SET_VAL_LIST
#define ECKTEMP_SET_VAL_LIST(MiField, Field, Div, DefDiv) \
        {                                         \
            m_Info.Field.Clear();                 \
            const auto svDiv = Opt.Div.empty() ? DefDiv : Opt.Div; \
            for (const auto e : mi.MiField)       \
            {                                     \
                if (!m_Info.Field.IsEmpty())      \
                    m_Info.Field.PushBack(svDiv); \
                    m_Info.Field.PushBack(e);     \
            }                                     \
        }

        if (mi.uMask & MIM_TITLE)
            ECKTEMP_SET_VAL(rsTitle);
        if (mi.uMask & MIM_ARTIST)
            ECKTEMP_SET_VAL_LIST(slArtist, rsArtist, svArtistDiv, L"/"sv);
        if (mi.uMask & MIM_ALBUM)
            ECKTEMP_SET_VAL(rsAlbum);
        if (mi.uMask & MIM_COMMENT)
            ECKTEMP_SET_VAL_LIST(slComment, rsComment, svCommDiv, L"\n"sv);
        if (mi.uMask & MIM_GENRE)
            ECKTEMP_SET_VAL(rsGenre);
        if (mi.uMask & MIM_TRACK)
            m_Info.byTrack = (BYTE)mi.nTrack;
#undef ECKTEMP_SET_VAL
#undef ECKTEMP_SET_VAL_LIST
        return Result::Ok;
    }

    Result ReadTag(UINT uFlags = 0u) noexcept override try
    {
        const auto& Loc = m_File.GetTagLocation();
        if (Loc.posV1 == CMediaFile::NPos)
        {
            m_bEmpty = TRUE;
            return Result::NoTag;
        }
        m_bEmpty = FALSE;
        char szTemp[61];
        szTemp[60] = '\0';// Guard
        int cchTemp;

        if (Loc.posV1Ext == CMediaFile::NPos)
        {
            m_Stream.MoveTo(Loc.posV1 + 3);
            TagpReadString(szTemp, 30, cchTemp, m_Info.rsTitle);
            TagpReadString(szTemp, 30, cchTemp, m_Info.rsArtist);
            TagpReadString(szTemp, 30, cchTemp, m_Info.rsAlbum);
        }
        else
        {
            m_Stream.MoveTo(Loc.posV1Ext + 4);
            TagpReadString(szTemp, 60, cchTemp, m_Info.rsTitle);
            TagpReadString(szTemp, 60, cchTemp, m_Info.rsArtist);
            TagpReadString(szTemp, 60, cchTemp, m_Info.rsAlbum);
            m_Stream >> m_Info.eSpeed;
            TagpReadString(szTemp, 30, cchTemp, m_Info.rsGenre);

            szTemp[6] = '\0';
            USHORT us0, us1;

            m_Stream.Read(szTemp, 6);
            us0 = us1 = 0;
            if (sscanf_s(szTemp, "%hu:%hu", &us0, &us1) == 2)
                m_Info.uBeginSec = us0 * 60 + us1;
            else
                m_Info.uBeginSec = 0;

            m_Stream.Read(szTemp, 6);
            us0 = us1 = 0;
            if (sscanf_s(szTemp, "%hu:%hu", &us0, &us1) == 2)
                m_Info.uEndSec = us0 * 60 + us1;
            else
                m_Info.uEndSec = 0;
            // 移动到ID3v1位置继续解析，跳过"TAG"标记3字节，
            // 标题、艺术家、专辑共90字节
            if (Loc.posV1 == CMediaFile::NPos)
            {
                m_Info.usYear = 0;
                m_Info.rsComment.Clear();
                m_Info.byTrack = 0;
                m_Info.byGenre = 0xFF;
                return Result::Ok;
            }
            else
                m_Stream.MoveTo(Loc.posV1 + 93);
        }
#undef ECKTEMP_READ_STR

        szTemp[4] = '\0';
        m_Stream.Read(szTemp, 4);
        TcsToInt(szTemp, 4, m_Info.usYear, 10);

        m_Info.rsComment.ReSize(30);
        m_Stream.Read(szTemp, 30);
        if (!szTemp[28])
        {
            m_Info.byTrack = szTemp[29];
            cchTemp = std::min((int)TcsLen(szTemp), 30);
        }
        else
        {
            m_Info.byTrack = 0;
            if (szTemp[29])
                cchTemp = 30;
            else
                cchTemp = std::min((int)TcsLen(szTemp), 30);
        }
        m_Info.rsComment.Clear();
        StrX2W(m_Info.rsComment, szTemp, cchTemp);

        m_Stream >> m_Info.byGenre;
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

    Result WriteTag(UINT uFlags) noexcept override try
    {
        CStringA rsTemp{};
        rsTemp.Reserve(60);

        const auto& Loc = m_File.GetTagLocation();
        if (Loc.posV1Ext == CMediaFile::NPos)
            if (uFlags & MIF_CREATE_ID3V1_EXT)
                if (Loc.posV1 == CMediaFile::NPos)
                    m_Stream.MoveToEnd();
                else
                    m_Stream.MoveTo(Loc.posV1);
            else
            {
                m_Stream.MoveTo(Loc.posV1);
                goto SkipID3v1Ext;
            }
        else
            m_Stream.MoveTo(Loc.posV1Ext);
        // 写入1.2
        m_Stream.Write("TAG+", 4);

        TagpWriteString(m_Info.rsTitle, 60, rsTemp);
        TagpWriteString(m_Info.rsArtist, 60, rsTemp);
        TagpWriteString(m_Info.rsAlbum, 60, rsTemp);
        m_Stream << m_Info.eSpeed;
        TagpWriteString(m_Info.rsGenre, 30, rsTemp);
        TagpWriteSecond(m_Info.uBeginSec, rsTemp);
        TagpWriteSecond(m_Info.uEndSec, rsTemp);
    SkipID3v1Ext:
        // 写入1.1
        m_Stream.Write("TAG", 3);

        TagpWriteString(m_Info.rsTitle, 30, rsTemp);
        TagpWriteString(m_Info.rsArtist, 30, rsTemp);
        TagpWriteString(m_Info.rsAlbum, 30, rsTemp);

        if (sprintf_s(rsTemp.Data(), rsTemp.Capacity(), "%04hu", m_Info.usYear) == 4)
            m_Stream.Write(rsTemp.Data(), 4);
        else
        {
            constexpr BYTE ZeroBytes[4]{};
            m_Stream.Write(ZeroBytes, 4);
        }

        rsTemp.Clear();
        StrW2X(rsTemp, m_Info.rsComment.Data(),
            std::min(m_Info.rsComment.Size(), 30 + 8));
        TagpZeroTail(rsTemp, 30);
        if (rsTemp.Size() <= 28)
        {
            rsTemp.ReSize(30);
            rsTemp.Back() = (char)m_Info.byTrack;
        }
        else
            rsTemp.ReSize(30);
        rsTemp.PushBackChar((char)m_Info.byGenre);
        m_Stream.Write(rsTemp.Data(), 31);
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
    BOOL IsEmpty() noexcept override { return m_bEmpty; }

    auto& ItmGet() noexcept { return m_Info; }
    auto& ItmGet() const noexcept { return m_Info; }

    void ItmClear() noexcept
    {
        m_Info = {};
        m_bEmpty = TRUE;
    }
};
ECK_MEDIATAG_NAMESPACE_END
ECK_NAMESPACE_END