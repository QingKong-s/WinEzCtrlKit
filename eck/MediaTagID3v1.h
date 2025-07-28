#pragma once
#include "MediaTag.h"

ECK_NAMESPACE_BEGIN
ECK_MEDIATAG_NAMESPACE_BEGIN
struct ID3_GENRE
{
	PCWSTR ZhName;
	PCWSTR EnName;
};
constexpr inline ID3_GENRE Genre[]
{
	{L"蓝调",L"Blues"},
	{L"经典摇滚",L"Classic Rock"},
	{L"乡村",L"Country"},
	{L"舞曲",L"Dance"},
	{L"迪斯科",L"Disco"},
	{L"放克",L"Funk"},
	{L"油渍摇滚",L"Grunge"},
	{L"嘻哈",L"Hip-Hop"},
	{L"爵士乐",L"Jazz"},
	{L"金属乐",L"Metal"},
	{L"新世纪",L"New Age"},
	{L"老歌",L"Oldies"},
	{L"其他",L"Other"},
	{L"流行乐",L"Pop"},
	{L"节奏蓝调",L"R&B"},
	{L"说唱",L"Rap"},
	{L"雷鬼",L"Reggae"},
	{L"摇滚乐",L"Rock"},
	{L"高科技舞曲",L"Techno"},
	{L"工业摇滚",L"Industrial"},
	{L"另类音乐",L"Alternative"},
	{L"斯卡",L"Ska"},
	{L"死亡金属",L"Death Metal"},
	{L"恶作剧",L"Pranks"},
	{L"原声音乐",L"Soundtrack"},
	{L"欧洲泰克诺",L"Euro-Techno"},
	{L"氛围音乐",L"Ambient"},
	{L"痴哈",L"Trip-Hop"},
	{L"人声",L"Vocal"},
	{L"爵士+放克",L"Jazz+Funk"},
	{L"融合爵士乐",L"Fusion"},
	{L"出神音乐",L"Trance"},
	{L"古典音乐",L"Classical"},
	{L"器乐",L"Instrumental"},
	{L"酸性音乐",L"Acid"},
	{L"浩室音乐",L"House"},
	{L"游戏",L"Game"},
	{L"声音剪辑",L"Sound Clip"},
	{L"福音音乐",L"Gospel"},
	{L"噪音",L"Noise"},
	{L"另类摇滚",L"AlternRock"},
	{L"贝斯",L"Bass"},
	{L"灵魂乐",L"Soul"},
	{L"朋克",L"Punk"},
	{L"空间音乐",L"Space"},
	{L"冥想",L"Meditative"},
	{L"流行器乐",L"Instrumental Pop"},
	{L"器乐摇滚",L"Instrumental Rock"},
	{L"民族特色音乐",L"Ethnic"},
	{L"哥特音乐",L"Gothic"},
	{L"暗波",L"Darkwave"},
	{L"工业泰克诺",L"Techno-Industrial"},
	{L"电子乐",L"Electronic"},
	{L"流行民谣",L"Pop-Folk"},
	{L"欧陆舞曲",L"Eurodance"},
	{L"梦幻",L"Dream"},
	{L"南方摇滚",L"Southern Rock"},
	{L"喜剧",L"Comedy"},
	{L"邪典",L"Cult"},
	{L"帮派",L"Gangsta"},
	{L"40强",L"Top 40"},
	{L"基督教说唱",L"Christian Rap"},
	{L"流行/放克",L"Pop/Funk"},
	{L"丛林",L"Jungle"},
	{L"美洲原住民音乐",L"Native American"},
	{L"卡巴莱",L"Cabaret"},
	{L"新浪潮",L"New Wave"},
	{L"灵歌",L"Psychadelic"},
	{L"锐舞",L"Rave"},
	{L"表演曲调",L"Showtunes"},
	{L"预告片",L"Trailer"},
	{L"低保真",L"Lo-Fi"},
	{L"部落音乐",L"Tribal"},
	{L"酸性朋克",L"Acid Punk"},
	{L"酸性爵士",L"Acid Jazz"},
	{L"波尔卡",L"Polka"},
	{L"复古",L"Retro"},
	{L"音乐剧",L"Musical"},
	{L"摇滚",L"Rock & Roll"},
	{L"硬式摇滚",L"Hard Rock"},
	{L"民俗音乐",L"Folk"},
	{L"民谣摇滚",L"Folk-Rock"},
	{L"民俗音乐",L"National Folk"},
	{L"摇摆乐",L"Swing"},
	{L"快速融合乐",L"Fast Fusion"},
	{L"咆勃爵士乐",L"Bebob"},
	{L"拉丁音乐",L"Latin"},
	{L"复兴音乐",L"Revival"},
	{L"凯尔特音乐",L"Celtic"},
	{L"蓝草音乐",L"Bluegrass"},
	{L"前卫音乐",L"Avantgarde"},
	{L"哥特摇滚",L"Gothic Rock"},
	{L"前卫摇滚",L"Progressive Rock"},
	{L"迷幻摇滚",L"Psychedelic Rock"},
	{L"交响摇滚",L"Symphonic Rock"},
	{L"慢摇滚",L"Slow Rock"},
	{L"大乐团",L"Big Band"},
	{L"Chorus",L"Chorus"},
	{L"轻松音乐",L"Easy Listening"},
	{L"原音乐",L"Acoustic"},
	{L"Humour",L"Humour"},
	{L"Speech",L"Speech"},
	{L"香颂",L"Chanson"},
	{L"歌剧",L"Opera"},
	{L"室内乐",L"Chamber Music"},
	{L"奏鸣曲",L"Sonata"},
	{L"交响曲",L"Symphony"},
	{L"电臀贝斯",L"Booty Bass"},
	{L"Primus",L"Primus"},
	{L"色情律动",L"Porn Groove"},
	{L"讽刺音乐",L"Satire"},
	{L"慢板音乐",L"Slow Jam"},
	{L"俱乐部音乐",L"Club"},
	{L"探戈",L"Tango"},
	{L"桑巴",L"Samba"},
	{L"民间传说音乐",L"Folklore"},
	{L"谣曲",L"Ballad"},
	{L"力量抒情歌",L"Power Ballad"},
	{L"节奏灵魂乐",L"Rhythmic Soul"},
	{L"即兴饶舌",L"Freestyle"},
	{L"二重奏",L"Duet"},
	{L"朋克摇滚",L"Punk Rock"},
	{L"鼓独奏",L"Drum Solo"},
	{L"无伴奏合唱",L"A capella"},
	{L"欧陆浩室",L"Euro-House"},
	{L"舞厅音乐",L"Dance Hall"},
	{L"果阿出神",L"Goa"},
	{L"鼓打贝斯",L"Drum & Bass"},
	{L"夜店浩室",L"Club-House"},
	{L"硬核音乐",L"Hardcore"},
	{L"恐核音乐",L"Terror"},
	{L"独立音乐",L"Indie"},
	{L"英伦摇滚",L"BritPop"},
	{L"黑人朋克",L"Negerpunk"},
	{L"波兰朋克",L"Polsk Punk"},
	{L"节拍音乐",L"Beat"},
	{L"基督教帮派说唱",L"Christian Gangsta Rap"},
	{L"重金属乐",L"Heavy Metal"},
	{L"黑金属乐",L"Black Metal"},
	{L"跨界音乐",L"Crossover"},
	{L"当代基督教音乐",L"Contemporary Christian"},
	{L"福音摇滚",L"Christian Rock"},
	{L"梅伦格舞曲",L"Merengue"},
	{L"萨尔萨音乐",L"Salsa"},
	{L"敲击金属",L"Thrash Metal"},
	{L"动画音乐",L"Anime"},
	{L"日本流行",L"JPop"},
	{L"合成器流行",L"Synthpop"},
	{L"抽象音乐",L"Abstract"},
	{L"艺术摇滚",L"Art Rock"},
	{L"巴洛克音乐",L"Baroque"},
	{L"巴恩格拉",L"Bhangra"},
	{L"重拍",L"Big Beat"},
	{L"碎拍",L"Breakbeat"},
	{L"驰放",L"Chillout"},
	{L"缓拍",L"Downtempo"},
	{L"回响音乐",L"Dub"},
	{L"电子身体舞曲",L"EBM"},
	{L"Eclectic",L"Eclectic"},
	{L"电子放克",L"Electro"},
	{L"电子冲击",L"Electroclash"},
	{L"情绪摇滚",L"Emo"},
	{L"实验音乐",L"Experimental"},
	{L"车库浩室",L"Garage"},
	{L"全球音乐",L"Global"},
	{L"智能舞曲",L"IDM"},
	{L"Illbient",L"Illbient"},
	{L"Industro-Goth",L"Industro-Goth"},
	{L"即兴乐队",L"Jam Band"},
	{L"酸菜摇滚",L"Krautrock"},
	{L"左外野二人组",L"Leftfield"},
	{L"休闲音乐",L"Lounge"},
	{L"数学摇滚",L"Math Rock"},
	{L"新浪漫主义",L"New Romantic"},
	{L"新派碎拍",L"Nu-Breakz"},
	{L"后朋克",L"Post-Punk"},
	{L"后摇滚",L"Post-Rock"},
	{L"迷幻出神",L"Psytrance"},
	{L"瞪鞋摇滚",L"Shoegaze"},
	{L"太空摇滚",L"Space Rock"},
	{L"热带摇滚",L"Trop Rock"},
	{L"世界音乐",L"World Music"},
	{L"新古典主义音乐",L"Neoclassical"},
	{L"有声读物",L"Audiobook"},
	{L"Audio Theatre",L"Audio Theatre"},
	{L"新德国浪潮",L"Neue Deutsche Welle"},
	{L"播客",L"Podcast"},
	{L"独立摇滚",L"Indie Rock"},
	{L"帮派放克",L"G-Funk"},
	{L"回响贝斯",L"Dubstep"},
	{L"车库摇滚",L"Garage Rock"},
	{L"迷幻驰放",L"Psybient"},
};

class CID3v1 :public CTag
{
private:
	enum class Speed :BYTE
	{
		None,
		Slow,
		Middle,
		Fast,
		VeryFast
	};

	struct INFO
	{
		CRefStrA rsTitle{};
		CRefStrA rsArtist{};
		CRefStrA rsAlbum{};
		CRefStrA rsComment{};
		USHORT usYear{};
		BYTE byTrack{};
		BYTE byGenre{};
		//------以下字段为ID3v1.2信息------
		Speed eSpeed{};
		CRefStrA rsGenre{};
		UINT uBeginSec{};
		UINT uEndSec{};
	};

	INFO m_Info{};
public:
	CID3v1(CMediaFile& File) :CTag(File) {}

	Result SimpleExtract(MUSICINFO& mi) override
	{
		mi.Clear();
		if (m_File.m_Loc.posV1 == SIZETMax)
			return Result::NoTag;
		if (mi.uMask & MIM_TITLE)
		{
			mi.rsTitle = StrX2W(m_Info.rsTitle);
			mi.uMaskRead |= MIM_TITLE;
		}
		if (mi.uMask & MIM_ARTIST)
		{
			mi.AppendArtist(StrX2W(m_Info.rsArtist));
			mi.uMaskRead |= MIM_ARTIST;
		}
		if (mi.uMask & MIM_ALBUM)
		{
			mi.rsAlbum = StrX2W(m_Info.rsAlbum);
			mi.uMaskRead |= MIM_ALBUM;
		}
		if (mi.uMask & MIM_COMMENT)
		{
			mi.AppendComment(StrX2W(m_Info.rsComment));
			mi.uMaskRead |= MIM_COMMENT;
		}
		if (mi.uMask & MIM_GENRE)
		{
			if (m_Info.rsGenre.IsEmpty())
			{
				if (m_Info.byGenre < ARRAYSIZE(Genre))
				{
					mi.rsGenre = Genre[m_Info.byGenre].EnName;
					mi.uMaskRead |= MIM_GENRE;
				}
			}
			else
			{
				mi.rsGenre = StrX2W(m_Info.rsGenre);
				mi.uMaskRead |= MIM_GENRE;
			}
		}
		if (mi.uMask & MIM_DATE)
		{
			if (m_Info.usYear)
			{
				if (mi.uFlag & MIF_DATE_STRING)
					mi.Date = Format(L"%04d", m_Info.usYear);
				else
					mi.Date = SYSTEMTIME{ .wYear = m_Info.usYear };
				mi.uMaskRead |= MIM_DATE;
			}
		}
		if (mi.uMask & MIM_TRACK)
		{
			mi.nTrack = m_Info.byTrack;
			mi.cTotalTrack = 0;
			mi.uMaskRead |= MIM_TRACK;
		}
		return Result::Ok;
	}

	Result ReadTag(UINT uFlags) override
	{
		if (m_File.m_Loc.posV1 == SIZETMax)
			return Result::NoTag;
		if (m_File.m_Loc.posV1Ext == SIZETMax)
		{
			m_Stream.MoveTo(m_File.m_Loc.posV1 + 3);
			m_Info.rsTitle.ReSize(30);
			m_Stream.Read(m_Info.rsTitle.Data(), 30);
			if (!m_Info.rsTitle[29])
				m_Info.rsTitle.ReCalcLen();

			m_Info.rsArtist.ReSize(30);
			m_Stream.Read(m_Info.rsArtist.Data(), 30);
			if (!m_Info.rsArtist[29])
				m_Info.rsArtist.ReCalcLen();

			m_Info.rsAlbum.ReSize(30);
			m_Stream.Read(m_Info.rsAlbum.Data(), 30);
			if (!m_Info.rsAlbum[29])
				m_Info.rsAlbum.ReCalcLen();
		}
		else
		{
			m_Stream.MoveTo(m_File.m_Loc.posV1Ext + 4);
			m_Info.rsTitle.ReSize(60);
			m_Stream.Read(m_Info.rsTitle.Data(), 60);
			if (!m_Info.rsTitle[59])
				m_Info.rsTitle.ReCalcLen();

			m_Info.rsArtist.ReSize(60);
			m_Stream.Read(m_Info.rsArtist.Data(), 60);
			if (!m_Info.rsArtist[59])
				m_Info.rsArtist.ReCalcLen();

			m_Info.rsAlbum.ReSize(60);
			m_Stream.Read(m_Info.rsAlbum.Data(), 60);
			if (!m_Info.rsAlbum[59])
				m_Info.rsAlbum.ReCalcLen();

			m_Stream >> m_Info.eSpeed;

			m_Info.rsGenre.ReSize(30);
			m_Stream.Read(m_Info.rsGenre.Data(), 30);
			if (!m_Info.rsGenre[29])
				m_Info.rsGenre.ReCalcLen();

			CHAR ch[7];
			ch[6] = '\0';
			m_Stream.Read(ch, 6);
			USHORT us0{}, us1{};
			(void)sscanf(ch, "%hu:%hu", &us0, &us1);
			m_Info.uBeginSec = us0 * 60 + us1;

			m_Stream.Read(ch, 6);
			us0 = us1 = 0;
			(void)sscanf(ch, "%hu:%hu", &us0, &us1);
			m_Stream.MoveTo(m_File.m_Loc.posV1 + 93);
			m_Info.uEndSec = us0 * 60 + us1;
		}

		CHAR ch[5];
		m_Stream.Read(ch, 4);
		ch[4] = '\0';
		m_Info.usYear = (USHORT)atoi(ch);

		m_Info.rsComment.ReSize(30);
		m_Stream.Read(m_Info.rsComment.Data(), 30);
		if (!m_Info.rsComment[28])
		{
			m_Info.rsComment.ReCalcLen();
			m_Info.byTrack = m_Info.rsComment[29];
		}
		else
		{
			m_Info.byTrack = 0;
			if (!m_Info.rsComment[29])
				m_Info.rsComment.ReSize(29);
		}
		m_Stream >> m_Info.byGenre;
		return Result::Ok;
	}

	Result WriteTag(UINT uFlags) override
	{
		BYTE byDummy[60]{};
		size_t cb;
		CHAR szBuf[CchI32ToStrBufNoRadix2 * 2 + 1];

		if (m_File.m_Loc.posV1Ext == SIZETMax)
			if (uFlags & MIF_CREATE_ID3V1_EXT)
			{
				if (m_File.m_Loc.posV1 == SIZETMax)
					m_Stream.MoveToEnd();
				else
				{
					m_Stream.Insert(ToUli(m_File.m_Loc.posV1), ToUli(227));
					m_Stream.GetStream()->Seek(ToLi(-(128 + 227)), STREAM_SEEK_END, nullptr);
				}
				m_Stream.Write("TAG+", 4);
			}
			else
				goto SkipID3v1Ext;
		else
			m_Stream.MoveTo(m_File.m_Loc.posV1Ext + 4);

		cb = std::min(m_Info.rsTitle.ByteSize() - 1, (size_t)60);
		if (cb)
			m_Stream.Write(m_Info.rsTitle.Data(), cb);
		if (cb != 60)
			m_Stream.Write(byDummy, 60 - cb);

		cb = std::min(m_Info.rsArtist.ByteSize() - 1, (size_t)60);
		if (cb)
			m_Stream.Write(m_Info.rsArtist.Data(), cb);
		if (cb != 60)
			m_Stream.Write(byDummy, 60 - cb);

		cb = std::min(m_Info.rsAlbum.ByteSize() - 1, (size_t)60);
		if (cb)
			m_Stream.Write(m_Info.rsAlbum.Data(), cb);
		if (cb != 60)
			m_Stream.Write(byDummy, 60 - cb);

		m_Stream << m_Info.eSpeed;

		cb = std::min(m_Info.rsGenre.ByteSize() - 1, (size_t)30);
		if (cb)
			m_Stream.Write(m_Info.rsGenre.Data(), cb);
		if (cb != 30)
			m_Stream.Write(byDummy, 30 - cb);

		if (m_Info.uBeginSec / 60 > 999)
			m_Stream.Write(byDummy, 6);
		else
		{
			sprintf(szBuf, "%03d:%02d",
				m_Info.uBeginSec / 60, m_Info.uBeginSec % 60);
			m_Stream.Write(szBuf, 6);
		}

		if (m_Info.uEndSec / 60 > 999)
			m_Stream.Write(byDummy, 6);
		else
		{
			sprintf(szBuf, "%03d:%02d",
				m_Info.uEndSec / 60, m_Info.uEndSec % 60);
			m_Stream.Write(szBuf, 6);
		}
	SkipID3v1Ext:
		if (m_File.m_Loc.posV1 == SIZETMax)
		{
			m_Stream.MoveToEnd();
			m_Stream.Write("TAG", 3);
		}
		else
			m_Stream.MoveTo(m_File.m_Loc.posV1 + 3);

		cb = std::min(m_Info.rsTitle.ByteSize() - 1, (size_t)30);
		if (cb)
			m_Stream.Write(m_Info.rsTitle.Data(), cb);
		if (cb != 30)
			m_Stream.Write(byDummy, 30 - cb);

		cb = std::min(m_Info.rsArtist.ByteSize() - 1, (size_t)30);
		if (cb)
			m_Stream.Write(m_Info.rsArtist.Data(), cb);
		if (cb != 30)
			m_Stream.Write(byDummy, 30 - cb);

		cb = std::min(m_Info.rsAlbum.ByteSize() - 1, (size_t)30);
		if (cb)
			m_Stream.Write(m_Info.rsAlbum.Data(), cb);
		if (cb != 30)
			m_Stream.Write(byDummy, 30 - cb);

		sprintf(szBuf, "%04hu", m_Info.usYear);

		cb = std::min(m_Info.rsComment.ByteSize() - 1, (size_t)30);
		if (cb)
			m_Stream.Write(m_Info.rsComment.Data(), cb);
		if (cb <= 28)
			m_Stream.Write(byDummy, 29 - cb) << m_Info.byTrack;
		else if (cb != 30)
			m_Stream.Write(byDummy, 30 - cb);

		m_Stream << m_Info.byGenre;
		return Result::Ok;
	}

	void Reset() override
	{
		m_Info = {};
	}

	INFO& GetInfo() { return m_Info; }

	const INFO& GetInfo() const { return m_Info; }
};
ECK_MEDIATAG_NAMESPACE_END
ECK_NAMESPACE_END