/*
* WinEzCtrlKit Library
*
* MediaTag.h ： 媒体元数据解析
*
* Copyright(C) 2024 QingKong
*/
#pragma once
#include "CRefStr.h"
#include "CRefBin.h"
#include "CStreamWalker.h"
#include "CMemWalker.h"

#include <variant>
#include <bitset>

#define ECK_MEDIATAG_NAMESPACE_BEGIN namespace MediaTag {
#define ECK_MEDIATAG_NAMESPACE_END }

ECK_NAMESPACE_BEGIN
ECK_MEDIATAG_NAMESPACE_BEGIN
// 标签类型
enum :UINT
{
	TAG_INVALID = 0u,
	TAG_NONE = 1u << 0,
	TAG_ID3V1 = 1u << 1,
	TAG_ID3V2_3 = 1u << 2,
	TAG_ID3V2_4 = 1u << 3,
	TAG_FLAC = 1u << 4,
};
// 元数据类型
enum MediaInfoFlags :UINT
{
	MIM_TITLE = 1u << 0,
	MIM_ARTIST = 1u << 1,
	MIM_ALBUM = 1u << 2,
	MIM_COMMENT = 1u << 3,
	MIM_LRC = 1u << 4,
	MIM_COVER = 1u << 5,
	MIM_GENRE = 1u << 6,
	MIM_DATE = 1u << 7,

	MIM_ALL = 0xFFFFFFFF
};
// 标签读写标志
enum :UINT
{
	MIF_JOIN_ARTIST = 1u << 0,	// 用指定的分隔符连接多个艺术家
	MIF_JOIN_COMMENT = 1u << 1,	// 用指定的分隔符连接多个备注
	MIF_ALLOW_PADDING = 1u << 2,// 允许保留空白填充以防止重写整个文件
	MIF_DATE_STRING = 1u << 3,	// 用当前本地设置格式化日期为字符串
	MIF_SPLIT_ARTIST_IN_ID3V2_3 = 1u << 4,	// 用ID3v2.3规定的斜杠("/")分割艺术家列表
	MIF_SCAN_ALL_FRAME = 1u << 5,			// 扫描并存储标签中的所有记录
	MIF_APPEND_TAG = 1u << 6,	// 在文件尾部追加标签（如果标记系统允许）
};
// ID3帧写入选项
enum :BYTE
{
	MTID3F_PREPEND = 1u << 0,	// 将该帧置于文件头部的标签
	MTID3F_APPEND = 1u << 1,	// 将该帧置于文件尾部的标签
	MTID3F_DIRTY = 1u << 2,		// 数据已被修改
};

// 解析结果错误码
enum class Result
{
	Ok,
	TagErr,
	TextEncodingErr,
	TooLargeData,
	InvalidEnumVal,
	LenErr,
	InvalidVal,
};
// 图片类型
enum class PicType
{
	Invalid = -1,       // 任何无效的值
	Begin___ = 0,       // ！起始占位
	Other = Begin___,   // 其他
	FileIcon32x32,      // 32×32大小文件图标（仅PNG）
	OtherFileIcon,      // 其他图标
	CoverFront,         // 封面
	CoverBack,          // 封底
	LeafletPage,        // 宣传图
	Media,              // 实体媒介照片
	LeadArtist,         // 艺术家照片
	Artist,             // 演唱者照片
	Conductor,          // 指挥者照片
	Band,               // 乐队/剧团照片
	Composer,           // 作曲家照片
	Lyricist,           // 作词者照片
	RecordingLocation,  // 录音场地照片
	DuringRecording,    // 录音过程照片
	DuringPerformance,  // 表演过程照片
	MovieCapture,       // 视频截图
	ABrightColouredFish,// 艳鱼图
	Illustration,       // 插画
	BandLogotype,       // 艺术家/艺术团队Logo
	PublisherLogotype,  // 发行商/工作室Logo
	End___              // ！终止占位
};

constexpr inline PCWSTR c_pszPicType[]
{
	L"无效图片类型",
	L"其他",
	L"32×32文件图标",
	L"其他图标",
	L"封面",
	L"封底",
	L"宣传图",
	L"实体媒介照片",
	L"艺术家照片",
	L"演唱者照片",
	L"指挥者照片",
	L"乐队/剧团照片",
	L"作曲家照片",
	L"作词者照片",
	L"录音场地照片",
	L"录音过程照片",
	L"表演过程照片",
	L"视频截图",
	L"艳鱼图",
	L"插画",
	L"艺术家/艺术团队Logo",
	L"发行商/工作室Logo",
};

EckInline constexpr PCWSTR PicTypeToString(PicType e)
{
	return c_pszPicType[(int)e + 1];
}

struct MUSICPIC
{
	PicType eType;
	BOOL bLink;
	CRefStrW rsDesc;
	CRefStrW rsMime;
	std::variant<CRefBin, CRefStrW> varPic;
};

struct MUSICINFO
{
	UINT uMask{ MIM_ALL };		// 指定欲读取信息类型的掩码
	UINT uMaskRead{};			// 函数返回后设置已读取的信息
	PCWSTR pszArtistDiv = L"、";	// 若设置了MIF_JOIN_ARTIST，则此字段指示分隔符
	PCWSTR pszCommDiv = L"\n";	// 若设置了MIF_JOIN_COMMENT，则此字段指示分隔符
	UINT uFlag{};		// 操作标志

	CRefStrW rsTitle{}; // 标题
	std::variant<std::vector<CRefStrW>, CRefStrW>
		Artist{};		// 艺术家
	CRefStrW rsAlbum{}; // 专辑
	std::variant<std::vector<CRefStrW>, CRefStrW>
		Comment{};		// 备注
	CRefStrW rsLrc{};	// 歌词
	CRefStrW rsGenre{};	// 流派
	std::variant<SYSTEMTIME, CRefStrW>
		Date{};			// 录制日期
	std::vector<MUSICPIC> vImage{};// 图片

	// 取主封面。函数遍历图片列表，然后按照 封面 > 封底 > 第一幅图片 的优先级顺序返回指定的图片，若失败则返回NULL
	const MUSICPIC* GetMainCover() const
	{
		if (vImage.empty())
			return NULL;
		auto it = std::find_if(vImage.begin(), vImage.end(),
			[](const MUSICPIC& e) { return e.eType == PicType::CoverFront; });
		if (it == vImage.end())
			it = std::find_if(vImage.begin(), vImage.end(),
				[](const MUSICPIC& e) { return e.eType == PicType::CoverBack; });
		if (it == vImage.end())
			return vImage.data();
		else
			return &*it;
	}

	CRefStrW& GetArtistStr()
	{
		EckAssert(Artist.index() == 1u);
		return std::get<1>(Artist);
	}

	CRefStrW& GetCommentStr()
	{
		EckAssert(Comment.index() == 1u);
		return std::get<1>(Comment);
	}

	void Clear()
	{
		uMaskRead = 0u;
		rsTitle.Clear();
		Artist.emplace<0>();
		rsAlbum.Clear();
		Comment.emplace<0>();
		rsLrc.Clear();
		rsGenre.Clear();
		Date.emplace<0>();
		vImage.clear();
	}

	void AppendArtist(CRefStrW&& rs_)
	{
		if (uFlag & MIF_JOIN_ARTIST)
		{
			if (Artist.index() == 0)
				Artist.emplace<1>();
			auto& rs = std::get<1>(Artist);
			if (rs.IsEmpty())
				rs = std::move(rs_);
			else
			{
				rs.PushBack(pszArtistDiv);
				rs.PushBack(rs_);
			}
		}
		else
		{
			if (Artist.index() == 1)
				Artist.emplace<0>();
			std::get<0>(Artist).emplace_back(std::move(rs_));
		}
	}

	void AppendComment(CRefStrW&& rs_)
	{
		if (uFlag & MIF_JOIN_COMMENT)
		{
			if (Comment.index() == 0)
				Comment.emplace<1>();
			auto& rs = std::get<1>(Comment);
			if (rs.IsEmpty())
				rs = std::move(rs_);
			else
			{
				rs.PushBack(pszCommDiv);
				rs.PushBack(rs_);
			}
		}
		else
		{
			if (Comment.index() == 1)
				Comment.emplace<0>();
			std::get<0>(Comment).emplace_back(std::move(rs_));
		}
	}
};


struct ID3v2_Header		// ID3v2标签头
{
	CHAR Header[3];		// "ID3"
	BYTE Ver;			// 版本号
	BYTE Revision;		// 副版本号
	BYTE Flags;			// 标志
	BYTE Size[4];		// 标签大小，28位数据，每个字节最高位不使用，包括标签头的10个字节和所有的标签帧
};

struct ID3v2_ExtHeader  // ID3v2扩展头
{
	BYTE ExtHeaderSize[4];  // 扩展头大小
	BYTE Flags[2];          // 标志
	BYTE PaddingSize[4];    // 空白大小
};

struct ID3v2_FrameHeader// ID3v2帧头
{
	CHAR ID[4];			// 帧标识
	BYTE Size[4];		// 帧内容的大小，32位数据，不包括帧头
	BYTE Flags[2];		// 存放标志
};

struct FLAC_BlockHeader      // Flac头
{
	BYTE by;
	BYTE bySize[3];
};

static_assert(alignof(ID3v2_Header) == 1);
static_assert(alignof(ID3v2_ExtHeader) == 1);
static_assert(alignof(ID3v2_FrameHeader) == 1);
static_assert(alignof(FLAC_BlockHeader) == 1);


// ID3v2头标志
enum :UINT
{
	ID3V2HF_UNSYNCHRONIZATION = 1u << 7,            // 不同步
	ID3V2HF_EXTENDED_HEADER = 1u << 6,              // 含扩展头
	ID3V2HF_EXPERIMENTAL = 1u << 5,                 // 实验性标签
	// ----Only ID3v2.4----
	ID3V2HF_FOOTER = 1u << 4,                       // 含页脚
};

// ID3v2扩展头标志
enum :UINT
{
	// ----Only ID3v2.3----
	ID3V23EH_CRC_DATA_PRESENT = 1u << 7,            // 含CRC数据
	// ----Only ID3v2.4----
	ID3V24EH_UPDATE = 1u << 6,                      // 更新标志
	ID3V24EH_CRC_DATA_PRESENT = 1u << 5,            // 含CRC数据
	ID3V24EH_RESTRICTIONS = 1u << 4,                // 限制标签尺寸
};

// ID3v2帧标志
enum :UINT
{
	// ----状态----
	ID3V24FF_TAG_ALTER_PRESERVATION = 1u << 6,      // 标签修改后应丢弃
	ID3V24FF_FILE_ALTER_PRESERVATION = 1u << 5,     // 文件修改后应丢弃
	ID3V24FF_READ_ONLY = 1u << 4,                   // 只读
	// ----格式----
	ID3V24FF_HAS_GROUP_IDENTITY = 1u << 6,           // 含组标志（1B）
	ID3V24FF_COMPRESSION = 1u << 3,                  // 已压缩（zlib）
	ID3V24FF_ENCRYPTION = 1u << 2,                   // 已加密（1B，指示加密方式）
	ID3V24FF_UNSYNCHRONIZATION = 1u << 1,            // 不同步
	ID3V24FF_HAS_DATA_LENGTH_INDICATOR = 1u << 0,    // 含长度指示（4B，同步安全整数）

	// ----状态----
	ID3V23FF_TAG_ALTER_PRESERVATION = 1u << 7,      // 标签修改后应丢弃
	ID3V23FF_FILE_ALTER_PRESERVATION = 1u << 6,     // 文件修改后应丢弃
	ID3V23FF_READ_ONLY = 1u << 5,                   // 只读
	// ----格式----
	ID3V23FF_HAS_GROUP_IDENTITY = 1u << 7,           // 含组标志（1B）
	ID3V23FF_COMPRESSION = 1u << 6,                  // 已压缩（zlib）
	ID3V23FF_ENCRYPTION = 1u << 5,                   // 已加密（1B，指示加密方式）
};

/// <summary>
/// 同步安全整数到32位小端整数
/// </summary>
/// <param name="p">输入字节流</param>
/// <returns>转换结果</returns>
EckInline constexpr static DWORD SynchSafeIntToDWORD(PCBYTE p)
{
	return ((p[0] & 0x7F) << 21) | ((p[1] & 0x7F) << 14) | ((p[2] & 0x7F) << 7) | (p[3] & 0x7F);
}

EckInline constexpr static void DwordToSynchSafeInt(BYTE* p, DWORD dw)
{
	p[3] = (dw) & 0b0111'1111;
	p[2] = (dw >> 7) & 0b0111'1111;
	p[1] = (dw >> 14) & 0b0111'1111;
	p[0] = (dw >> 21) & 0b0111'1111;
}

class CMediaFile
{
	friend class CID3v2;
private:
	IStream* m_pStream{};
	UINT m_uTagType{};

	UINT DetectID3v2()
	{
		CStreamWalker w(m_pStream);
		w.MoveToBegin();
		ID3v2_Header hdr;
		w >> hdr;
		if (w.GetLastRWSize() != sizeof(hdr))
			return TAG_INVALID;
		if (memcmp(hdr.Header, "ID3", 3u) == 0 ||
			SynchSafeIntToDWORD(hdr.Size) != 0)
		{
			if (hdr.Ver == 3)
				return TAG_ID3V2_3;
			if (hdr.Ver == 4)
				return TAG_ID3V2_4;
		}
		else
		{
			// TODO:查找标签尾
		}
		return TAG_INVALID;
	}

	BOOL DetectFlac()
	{
		CStreamWalker w(m_pStream);
		w.MoveToBegin();
		BYTE by[4];
		w >> by;
		return memcmp(by, "fLaC", 4) == 0;
	}
public:
	ECK_DISABLE_COPY_MOVE(CMediaFile)
public:
	CMediaFile(IStream* pStream) : m_pStream{ pStream }
	{
		m_pStream->AddRef();
	}

	CMediaFile(PCWSTR pszFile, DWORD grfMode = STGM_READWRITE,
		DWORD dwAttr = FILE_ATTRIBUTE_NORMAL, BOOL bCreate = FALSE)
	{
		SHCreateStreamOnFileEx(pszFile, grfMode, dwAttr, bCreate, NULL, &m_pStream);
	}

	~CMediaFile() { m_pStream->Release(); }

	IStream* GetStream() const { return m_pStream; }

	UINT GetTagType() const { return m_uTagType; }

	UINT DetectTag()
	{
		m_uTagType = 0u;
		m_uTagType |= DetectID3v2();
		if (DetectFlac())
			m_uTagType |= TAG_FLAC;
		return m_uTagType;
	}
};

class CID3v1
{
private:
	CMediaFile& m_File;
	CStreamWalker m_Stream{};
};

class CID3v2
{
private:
	CMediaFile& m_File;
	CStreamWalker m_Stream{};

	ID3v2_Header m_Header{};
	ID3v2_ExtHeader m_ExtHdr{};
	DWORD m_cbTag{};

	class CFrame
	{
	private:
		CStreamWalker& m_w;
		const ID3v2_FrameHeader& m_Header{};
		CRefBin m_rbFrame{};
		DWORD m_cbFrame{};
		BOOL m_bUnsync = FALSE;
	public:
		CFrame(CID3v2& id3, const ID3v2_FrameHeader& Header, DWORD cbFrame) :
			m_w(id3.m_Stream), m_Header(Header), m_cbFrame(cbFrame),
			m_bUnsync(id3.m_Header.Flags& ID3V2HF_UNSYNCHRONIZATION),
			m_rbFrame(cbFrame)
		{
			if (!m_bUnsync && id3.m_Header.Ver == 4)
				m_bUnsync = !!(m_Header.Flags[1] & ID3V24FF_UNSYNCHRONIZATION);
		}

		std::pair<CMemWalker, DWORD> Begin()
		{
			m_w.Read(m_rbFrame.Data(), m_cbFrame);
			if (m_bUnsync)
				m_rbFrame.ReplaceSubBin({ 0xFF, 0x00 }, { 0xFF });// 恢复不同步处理
			return
			{
				CMemWalker(m_rbFrame.Data(), m_rbFrame.Size()),
				(DWORD)m_rbFrame.Size()
			};
		}
	};

	/// <summary>
	/// 按指定编码处理文本
	/// </summary>
	/// <param name="pStream">字节流指针；未指定iTextEncoding时指向整个文本帧，指定iTextEncoding时指向字符串</param>
	/// <param name="iLength">长度；未指定iTextEncoding时表示整个文本帧长度（包括1B的编码标记，不含结尾NULL），指定iTextEncoding时表示字符串长度（不含结尾NULL）</param>
	/// <param name="iTextEncoding">自定义文本编码；-1（缺省）指示处理的是文本帧</param>
	/// <returns>返回处理完毕的文本</returns>
	static CRefStrW GetID3v2_ProcString(CMemWalker& w, int cb, int iTextEncoding = -1)
	{
		const BOOL bHasTNull = (cb < 0);
		SIZE_T cbTNull{};
		int iType = iTextEncoding, cchBuf;
		if (iTextEncoding == -1)
		{
			iType = *w.Data();
			w += 1;// 跳过文本编码标志
			--cb;
		}

		CRefStrW rsResult{};

		switch (iType)
		{
		case 0:// ISO-8859-1
			if (bHasTNull)
			{
				cb = (int)strlen((PCSTR)w.Data());
				cbTNull = 1u;
			}

			if (cchBuf = MultiByteToWideChar(CP_ACP/*keep ANSI encoding tag happy*/, 0,
				(PCCH)w.Data(), cb, NULL, 0))
			{
				rsResult.ReSize(cchBuf);
				MultiByteToWideChar(CP_ACP, 0, (PCCH)w.Data(), cb, rsResult.Data(), cchBuf);
			}
			break;

		case 1:// UTF-16LE
			if (bHasTNull)
			{
				cb = (int)wcslen((PCWSTR)w.Data()) * sizeof(WCHAR);
				cbTNull = 2u;
			}

			if (*(PWSTR)w.Data() == L'\xFEFF')// 跳BOM
			{
				w += sizeof(WCHAR);
				cb -= sizeof(WCHAR);
			}
			else if (*(PWSTR)w.Data() == L'\xFFFE')// for ID3v2.3
			{
				w += sizeof(WCHAR);
				cb -= sizeof(WCHAR);
				goto Utf16BE;
			}

			if (cb)
			{
				cchBuf = cb / sizeof(WCHAR);
				rsResult.ReSize(cchBuf);
				wcsncpy(rsResult.Data(), (PWSTR)w.Data(), cchBuf);
			}
			break;

		case 2:// UTF-16BE
			if (bHasTNull)
			{
				cb = (int)wcslen((PCWSTR)w.Data()) * sizeof(WCHAR);
				cbTNull = 2u;
			}

			if (*(PWSTR)w.Data() == L'\xFFFE')// 跳BOM
			{
				w += sizeof(WCHAR);
				cb -= sizeof(WCHAR);
			}
		Utf16BE:
			if (cb)
			{
				cchBuf = cb / sizeof(WCHAR);
				rsResult.ReSize(cchBuf);
				LCMapStringEx(LOCALE_NAME_USER_DEFAULT, LCMAP_BYTEREV,
					(PCWSTR)w.Data(), cchBuf, rsResult.Data(), cchBuf, NULL, NULL, 0);
			}
			break;

		case 3:// UTF-8
			if (bHasTNull)
			{
				cb = (int)strlen((PCSTR)w.Data());
				cbTNull = 1u;
			}

			if (cchBuf = MultiByteToWideChar(CP_UTF8, 0, (PCCH)w.Data(), cb, NULL, 0))
			{
				rsResult.ReSize(cchBuf);
				MultiByteToWideChar(CP_UTF8, 0, (PCCH)w.Data(), cb, rsResult.Data(), cchBuf);
			}
			break;
		default:
			EckDbgBreak();
			break;
		}

		w += cb;
		if (bHasTNull)
			w += cbTNull;
		return rsResult;
	}
public:
	enum class EventType :BYTE
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
		Profanity,				// 脏话 **(仅ID3v2.4)**
		ProfanityEnd,			// 脏话结束 **(仅ID3v2.4)**

		InvalidBeginV2_4,		// ！无效区开始 **(仅ID3v2.4)**
		InvalidEnd = 0xFC,		// ！无效区结束

		AudioEnd = 0xFD,		// 音频结束(静音开始)
		AudioFileEnds,			// 音频文件结束

		InvalidBeginV2_3 = Profanity,// 无效区开始 **(仅ID3v2.3)**
	};

	enum class TimestampFmt :BYTE
	{
		MpegFrame,
		Milliseconds,
		Max
	};

	enum class TextEncoding :BYTE
	{
		Latin1,
		UTF16V2_3,
		UTF16LE = UTF16V2_3,
		MaxV2_3,
		UTF16BE = MaxV2_3,
		UTF8,
		Max
	};

	enum class LrcContentType :BYTE
	{
		Other,				// 其他
		Lyrics,				// 歌词
		TextTranscription,	// 文字转录
		Movement,			// 乐章/乐段名称(如"Adagio")。
		Events,				// 事件(如"堂吉诃德登场")
		Chord,				// 和弦(如"Bb F Fsus")
		Trivia,				// 小节/弹出式信息
		WebpageUrl,			// 网页URL
		ImageUrl,			// 图片URL
		Max
	};

	enum class ChannelType :BYTE
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

	enum class Interpolation :BYTE
	{
		Band,	// 不进行插值。从一个调整级别到另一个调整级别的跳变发生在两个调整点之间的中间位置。
		Linear,	// 调整点之间的插值是线性的。
		Max
	};

	enum class ReceivedWay :BYTE
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

#define ECK_DECL_ID3FRAME_CLONE(x) \
	FRAME* Clone() override { return new x{ *this }; }

	struct FRAME
	{
		CHAR Id[4]{};
		BYTE uFlags[2]{};
		BYTE byAddtFlags{};// MTID3F_常量

		virtual ~FRAME() {}

		virtual Result SerializeData(CRefBin& rb, ID3v2_Header* phdr) = 0;

		virtual FRAME* Clone() = 0;
	protected:
		CMemWalker PreSerialize(CRefBin& rb, ID3v2_Header* phdr, size_t cbFrame)
		{
			auto w{ CMemWalker(rb.PushBack(
				sizeof(ID3v2_FrameHeader) + cbFrame) + sizeof(ID3v2_FrameHeader), cbFrame) };
			ID3v2_FrameHeader* pfhdr;
			w.SkipPointer(pfhdr);
			memcpy(pfhdr->ID, Id, 4);
			memcpy(pfhdr->Flags, &uFlags, 2);
			if (phdr->Ver == 4)
				DwordToSynchSafeInt(pfhdr->Size, (DWORD)cbFrame);
			else
				*(DWORD*)pfhdr->Size = ReverseInteger((DWORD)cbFrame);
			return w;
		}

		void PostSerialize(CRefBin& rb, ID3v2_Header* phdr, size_t cbFrame)
		{
			EckAssert(rb.Size() >= sizeof(ID3v2_Header) + cbFrame);
			if (phdr->Ver == 4)
			{
				if (uFlags[1] & ID3V24FF_COMPRESSION)
				{
					// TODO:压缩
				}

				if (phdr->Flags & ID3V2HF_UNSYNCHRONIZATION || uFlags[1] & ID3V24FF_UNSYNCHRONIZATION)
				{
					for (size_t i = rb.Size() - cbFrame; i < rb.Size() - 1; ++i)
					{
						if (rb[i] == 0xFF &&
							(rb[i + 1] == 0 || IsBitSet(rb[i + 1], 0b1110'0000)))
							rb.Insert(i + 1, 0);
					}
				}
			}
			else
			{
				if (uFlags[1] & ID3V24FF_COMPRESSION)
				{
					// TODO:压缩
				}

				if (phdr->Flags & ID3V2HF_UNSYNCHRONIZATION)
				{
					for (size_t i = rb.Size() - cbFrame; i < rb.Size() - 1; ++i)
					{
						if (rb[i] == 0xFF &&
							(rb[i + 1] == 0 || IsBitSet(rb[i + 1], 0b1110'0000)))
							rb.Insert(i + 1, 0);
					}
				}
			}
		}

		static CRefBin CovertTextEncoding(const CRefStrW& rsStr,
			TextEncoding eEncoding, BOOL bAddTerNull = FALSE)
		{
			CRefBin rb{};
			if (rsStr.IsEmpty())
			{
				if (bAddTerNull)
					switch (eEncoding)
					{
					case TextEncoding::Latin1:
					case TextEncoding::UTF8:
						rb.ReSize(1);
						rb.Front() = 0;
						break;
					case TextEncoding::UTF16LE:
					case TextEncoding::UTF16BE:
						rb.ReSize(2);
						rb[0] = 0;
						rb[1] = 0;
						break;
					}
				return rb;
			}

			switch (eEncoding)
			{
			case TextEncoding::Latin1:
				if (int cchBuf; cchBuf = WideCharToMultiByte(CP_ACP, 0,
					rsStr.Data(), rsStr.Size(), NULL, 0, NULL, NULL))
				{
					rb.ReSize(bAddTerNull ? cchBuf + 1 : cchBuf);
					WideCharToMultiByte(CP_ACP, 0,
						rsStr.Data(), rsStr.Size(), (PSTR)rb.Data(), cchBuf, NULL, NULL);
					if (bAddTerNull)
						rb.Back() = 0;
				}
				break;

			case TextEncoding::UTF16LE:
				rb.ReSize(rsStr.ByteSize() + 2 - (bAddTerNull ? 0 : sizeof(WCHAR)));
				memcpy(rb.Data(), BOM_UTF16LE, 2);
				memcpy(rb.Data() + 2, rsStr.Data(), rsStr.ByteSize() - (bAddTerNull ? 0 : sizeof(WCHAR)));
				break;

			case TextEncoding::UTF16BE:
				rb.ReSize(rsStr.ByteSize() - (bAddTerNull ? 0 : sizeof(WCHAR)));
				LCMapStringEx(LOCALE_NAME_USER_DEFAULT, LCMAP_BYTEREV,
					rsStr.Data(), rsStr.Size() + (bAddTerNull ? 1 : 0),
					(PWSTR)rb.Data(), rsStr.Size() + (bAddTerNull ? 1 : 0), NULL, NULL, 0);
				break;

			case TextEncoding::UTF8:
				if (int cchBuf; cchBuf = WideCharToMultiByte(CP_UTF8, 0,
					rsStr.Data(), rsStr.Size(), NULL, 0, NULL, NULL))
				{
					rb.ReSize(bAddTerNull ? cchBuf + 1 : cchBuf);
					WideCharToMultiByte(CP_UTF8, 0,
						rsStr.Data(), rsStr.Size(), (PSTR)rb.Data(), cchBuf, NULL, NULL);
					if (bAddTerNull)
						rb.Back() = 0;
				}
				break;
			default:
				EckDbgBreak();
				break;
			}
			return rb;
		}

		static CRefBin CovertTextEncoding(const std::vector<CRefStrW>& v,
			TextEncoding eEncoding, BOOL bAddTerNull = FALSE)
		{
			if (v.size() == 1u)
				return CovertTextEncoding(v.front(), eEncoding, bAddTerNull);
			CRefStrW rs{};
			int cch{};
			for (const auto& e : v)
				cch += (e.Size() + 1);
			rs.ReSize(bAddTerNull ? cch : cch - 1);
			for (PWSTR psz = rs.Data(); const auto & e : v)
			{
				wmemcpy(psz, e.Data(), e.Size() + 1);
				psz += (e.Size() + 1);
			}
			return CovertTextEncoding(rs, eEncoding, bAddTerNull);
		}
	};

	struct UFID :public FRAME
	{
		CRefStrA rsEmail{};
		CRefBin rbOwnerData{};

		Result SerializeData(CRefBin& rb, ID3v2_Header* phdr) override
		{
			const size_t cbFrame = rsEmail.Size() + 1 + rbOwnerData.Size();
			auto w = PreSerialize(rb, phdr, cbFrame);
			w << rsEmail << rbOwnerData;
			PostSerialize(rb, phdr, cbFrame);
			return Result::Ok;
		}

		ECK_DECL_ID3FRAME_CLONE(UFID)
	};

	struct TEXTFRAME :public FRAME
	{
		TextEncoding eEncoding{};
		std::vector<CRefStrW> vText{};

		Result SerializeData(CRefBin& rb, ID3v2_Header* phdr) override
		{
			const auto rbText = CovertTextEncoding(vText, eEncoding);
			size_t cbFrame = 1 + rbText.Size();
			auto w = PreSerialize(rb, phdr, cbFrame);
			w << eEncoding << rbText;
			PostSerialize(rb, phdr, cbFrame);
			return Result::Ok;
		}

		ECK_DECL_ID3FRAME_CLONE(TEXTFRAME)
	};

	struct TXXX :public FRAME
	{
		TextEncoding eEncoding{};
		CRefStrW rsDesc{};
		CRefStrW rsText{};

		Result SerializeData(CRefBin& rb, ID3v2_Header* phdr) override
		{
			CRefBin rbDesc = CovertTextEncoding(rsDesc, eEncoding, TRUE);
			CRefBin rbText = CovertTextEncoding(rsText, eEncoding);
			const size_t cbFrame = 1 + rbDesc.Size() + rbText.Size();
			auto w = PreSerialize(rb, phdr, cbFrame);
			w << eEncoding << rbDesc << rbText;
			PostSerialize(rb, phdr, cbFrame);
			return Result::Ok;
		}

		ECK_DECL_ID3FRAME_CLONE(TXXX)
	};

	struct LINKFRAME :public FRAME
	{
		CRefStrA rsUrl{};

		Result SerializeData(CRefBin& rb, ID3v2_Header* phdr) override
		{
			auto w = PreSerialize(rb, phdr, rsUrl.Size());
			w.Write(rsUrl.Data(), rsUrl.Size());
			PostSerialize(rb, phdr, rsUrl.Size());
			return Result::Ok;
		}

		ECK_DECL_ID3FRAME_CLONE(LINKFRAME)
	};

	struct WXXX :public FRAME
	{
		TextEncoding eEncoding{};
		CRefStrW rsDesc{};
		CRefStrA rsUrl{};

		Result SerializeData(CRefBin& rb, ID3v2_Header* phdr) override
		{
			auto rbDesc = CovertTextEncoding(rsDesc, eEncoding, TRUE);
			const size_t cbFrame = 1 + rbDesc.Size() + rsUrl.Size();
			auto w = PreSerialize(rb, phdr, cbFrame);
			w << eEncoding << rbDesc;
			w.Write(rsUrl.Data(), rsUrl.Size());
			PostSerialize(rb, phdr, cbFrame);
			return Result::Ok;
		}

		ECK_DECL_ID3FRAME_CLONE(WXXX)
	};

	struct MCID :public FRAME
	{
		CRefBin rbToc{};

		Result SerializeData(CRefBin& rb, ID3v2_Header* phdr) override
		{
			auto w = PreSerialize(rb, phdr, rbToc.Size());
			w << rbToc;
			PostSerialize(rb, phdr, rbToc.Size());
			return Result::Ok;
		}

		ECK_DECL_ID3FRAME_CLONE(MCID)
	};

	struct ETCO :public FRAME
	{
		struct EVENT
		{
			EventType eType;
			UINT uTimestamp;
		};

		TimestampFmt eTimestampFmt{};
		std::vector<EVENT> vEvent{};

		Result SerializeData(CRefBin& rb, ID3v2_Header* phdr) override
		{
			const size_t cbFrame = 1 + 5 * vEvent.size();
			auto w = PreSerialize(rb, phdr, cbFrame);
			w << eTimestampFmt;
			for (const auto& e : vEvent)
				(w << e.eType).WriteAndRevByte(e.uTimestamp);
			PostSerialize(rb, phdr, cbFrame);
			return Result::Ok;
		}

		ECK_DECL_ID3FRAME_CLONE(ETCO)
	};

	struct MLLT :public FRAME
	{
		struct REF
		{
			std::bitset<256> ByteOffset;
			std::bitset<256> MillisecondsOffset;
		};
		USHORT cMpegFrame{};
		UINT cByte{};
		UINT cMilliseconds{};
		BYTE cByteOffsetValBit{};
		BYTE cMillisecondsOffsetValBit{};
		std::vector<REF> vRef{};

		Result SerializeData(CRefBin& rb, ID3v2_Header* phdr) override
		{
			const size_t cbFrame = 0;
			auto w = PreSerialize(rb, phdr, cbFrame);
			// TODO:位
			PostSerialize(rb, phdr, cbFrame);
			return Result::Ok;
		}

		ECK_DECL_ID3FRAME_CLONE(MLLT)
	};

	struct SYTC :public FRAME
	{
		struct TEMPO
		{
			USHORT bpm{};
			UINT uTimestamp{};
		};
		TimestampFmt eTimestampFmt{};
		std::vector<TEMPO> vTempo{};

		Result SerializeData(CRefBin& rb, ID3v2_Header* phdr) override
		{
			size_t cbFrame = 1 + vTempo.size() * 5;
			for (const auto& e : vTempo)
				if (e.bpm >= 0xFF)
					++cbFrame;
				else if (e.bpm > 510)
					return Result::InvalidVal;

			auto w = PreSerialize(rb, phdr, cbFrame);
			w << eTimestampFmt;
			for (const auto& e : vTempo)
			{
				if (e.bpm >= 0xFF)
					w << (BYTE)0xFF << (BYTE)(e.bpm - 0xFF);
				else
					w << (BYTE)e.bpm;
				w.WriteAndRevByte(e.uTimestamp);
			}
			PostSerialize(rb, phdr, cbFrame);
			return Result::Ok;
		}

		ECK_DECL_ID3FRAME_CLONE(SYTC)
	};

	struct USLT :public FRAME
	{
		TextEncoding eEncoding{};
		CHAR byLang[3]{};
		CRefStrW rsDesc{};
		CRefStrW rsLrc{};

		Result SerializeData(CRefBin& rb, ID3v2_Header* phdr) override
		{
			const auto rbDesc = CovertTextEncoding(rsDesc, eEncoding, TRUE);
			const auto rbLrc = CovertTextEncoding(rsLrc, eEncoding);
			const size_t cbFrame = 4 + rbDesc.Size() + rbLrc.Size();
			auto w = PreSerialize(rb, phdr, cbFrame);
			w << eEncoding << byLang << rbDesc << rbLrc;
			PostSerialize(rb, phdr, cbFrame);
			return Result::Ok;
		}

		ECK_DECL_ID3FRAME_CLONE(USLT)
	};

	struct SYLT :public FRAME
	{
		struct SYNC
		{
			CRefStrW rsText{};
			UINT uTimestamp{};
		};

		TextEncoding eEncoding{};
		CHAR byLang[3]{};
		TimestampFmt eTimestampFmt{};
		LrcContentType eContent{};
		CRefStrW rsDesc{};
		std::vector<SYNC> vSync{};

		Result SerializeData(CRefBin& rb, ID3v2_Header* phdr) override
		{
			const auto rbDesc = CovertTextEncoding(rsDesc, eEncoding, TRUE);
			size_t cbFrame = 6 + rbDesc.Size() + vSync.size() * 4;

			std::vector<CRefBin> vLrc(vSync.size());
			EckCounter(vSync.size(), i)
			{
				vLrc[i] = CovertTextEncoding(vSync[i].rsText, eEncoding, TRUE);
				cbFrame += vLrc[i].Size();
			}

			auto w = PreSerialize(rb, phdr, cbFrame);
			w << eEncoding << byLang << eTimestampFmt << rbDesc;
			EckCounter(vSync.size(), i)
				(w << vLrc[i]).WriteAndRevByte(vSync[i].uTimestamp);
			PostSerialize(rb, phdr, cbFrame);
			return Result::Ok;
		}

		ECK_DECL_ID3FRAME_CLONE(SYLT)
	};

	struct COMM :public FRAME
	{
		TextEncoding eEncoding{};
		CHAR byLang[3]{};
		CRefStrW rsDesc{};
		CRefStrW rsText{};

		Result SerializeData(CRefBin& rb, ID3v2_Header* phdr) override
		{
			const auto rbDesc = CovertTextEncoding(rsDesc, eEncoding, TRUE);
			const auto rbText = CovertTextEncoding(rsText, eEncoding);
			const size_t cbFrame = 4 + rbDesc.Size() + rbText.Size();
			auto w = PreSerialize(rb, phdr, cbFrame);
			w << eEncoding << byLang << rbDesc << rbText;
			PostSerialize(rb, phdr, cbFrame);
			return Result::Ok;
		}

		ECK_DECL_ID3FRAME_CLONE(COMM)
	};

	struct RVA2 :public FRAME
	{
		struct CHANNEL
		{
			ChannelType eChannel{};
			BYTE cPeekVolBit{};
			short shVol{};
			std::bitset<256> PeekVol{};
		};
		CRefStrA rsId{};
		std::vector<CHANNEL> vChannel{};

		Result SerializeData(CRefBin& rb, ID3v2_Header* phdr) override
		{
			const size_t cbFrame = 0;
			auto w = PreSerialize(rb, phdr, cbFrame);
			// TODO:位
			PostSerialize(rb, phdr, cbFrame);
			return Result::Ok;
		}

		ECK_DECL_ID3FRAME_CLONE(RVA2)
	};

	struct EQU2 :public FRAME
	{
		struct POINT
		{
			USHORT uFreq{};	// 单位1/2Hz
			short shVol{};
		};
		Interpolation eInterpolation{};
		CRefStrA rsId{};
		std::vector<POINT> vPoint{};

		Result SerializeData(CRefBin& rb, ID3v2_Header* phdr) override
		{
			const size_t cbFrame = 2 + rsId.Size() + vPoint.size() * 4;
			auto w = PreSerialize(rb, phdr, cbFrame);
			w << eInterpolation << rsId;
			for (auto e : vPoint)
				w.WriteAndRevByte(e.uFreq).WriteAndRevByte(e.shVol);
			PostSerialize(rb, phdr, cbFrame);
			return Result::Ok;
		}

		ECK_DECL_ID3FRAME_CLONE(EQU2)
	};

	struct RVRB :public FRAME
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

		Result SerializeData(CRefBin& rb, ID3v2_Header* phdr) override
		{
			constexpr size_t cbFrame = 12;
			auto w = PreSerialize(rb, phdr, cbFrame);
			w.WriteAndRevByte(Left).WriteAndRevByte(Right).Write(&BouncesLeft, 8);
			PostSerialize(rb, phdr, cbFrame);
			return Result::Ok;
		}

		ECK_DECL_ID3FRAME_CLONE(RVRB)
	};

	struct APIC :public FRAME
	{
		TextEncoding eTextEncoding{};
		PicType eType{};
		CRefStrA rsMime{};
		CRefStrW rsDesc{};
		CRefBin rbData{};

		Result SerializeData(CRefBin& rb, ID3v2_Header* phdr) override
		{
			const auto rbDesc = CovertTextEncoding(rsDesc, eTextEncoding, TRUE);
			const size_t cbFrame = 3 + rsMime.Size() + rbDesc.Size() + rbData.Size();
			auto w = PreSerialize(rb, phdr, cbFrame);
			w << eTextEncoding << eType << rsMime << rbDesc << rbData;
			PostSerialize(rb, phdr, cbFrame);
			return Result::Ok;
		}

		ECK_DECL_ID3FRAME_CLONE(APIC)
	};

	struct GEOB :public FRAME
	{
		TextEncoding eTextEncoding{};
		CRefStrA rsMime{};
		CRefStrW rsFile{};
		CRefStrW rsDesc{};
		CRefBin rbObj{};

		Result SerializeData(CRefBin& rb, ID3v2_Header* phdr) override
		{
			const auto rbFile = CovertTextEncoding(rsFile, eTextEncoding, TRUE);
			const auto rbDesc = CovertTextEncoding(rsDesc, eTextEncoding, TRUE);
			const size_t cbFrame = 2 + rsMime.Size() + rbFile.Size() + rbDesc.Size() + rbObj.Size();
			auto w = PreSerialize(rb, phdr, cbFrame);
			w << eTextEncoding << rsMime << rbFile << rbDesc << rbObj;
			PostSerialize(rb, phdr, cbFrame);
			return Result::Ok;
		}

		ECK_DECL_ID3FRAME_CLONE(GEOB)
	};

	struct PCNT :public FRAME
	{
		ULONGLONG cPlay{};

		Result SerializeData(CRefBin& rb, ID3v2_Header* phdr) override
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
			auto w = PreSerialize(rb, phdr, cbFrame);
			w.WriteAndRevByte(&cPlay, cbFrame);
			PostSerialize(rb, phdr, cbFrame);
			return Result::Ok;
		}

		ECK_DECL_ID3FRAME_CLONE(PCNT)
	};

	struct POPM :public FRAME
	{
		CRefStrA rsEmail{};
		BYTE byRating{};
		ULONGLONG cPlay{};

		Result SerializeData(CRefBin& rb, ID3v2_Header* phdr) override
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
			const size_t cbFrame = 2 + rsEmail.Size() + cbPlayCount;
			auto w = PreSerialize(rb, phdr, cbFrame);
			w << rsEmail << byRating;
			w.WriteAndRevByte(&cPlay, cbPlayCount);
			PostSerialize(rb, phdr, cbFrame);
			return Result::Ok;
		}

		ECK_DECL_ID3FRAME_CLONE(POPM)
	};

	struct RBUF :public FRAME
	{
		UINT cbBuf{};
		BYTE b{};
		UINT ocbNextTag{};

		Result SerializeData(CRefBin& rb, ID3v2_Header* phdr) override
		{
			constexpr size_t cbFrame = 8;
			auto w = PreSerialize(rb, phdr, cbFrame);
			w.WriteAndRevByte(&cbBuf, 3) << b;
			w.WriteAndRevByte(ocbNextTag);
			PostSerialize(rb, phdr, cbFrame);
			return Result::Ok;
		}

		ECK_DECL_ID3FRAME_CLONE(RBUF)
	};

	struct AENC :public FRAME
	{
		CRefStrA rsOwnerId{};
		USHORT usPreviewBegin{};
		USHORT usPreviewLength{};
		CRefBin rbData{};

		Result SerializeData(CRefBin& rb, ID3v2_Header* phdr) override
		{
			const size_t cbFrame = 5 + rsOwnerId.Size() + rbData.Size();
			auto w = PreSerialize(rb, phdr, cbFrame);
			w << rsOwnerId;
			w.WriteAndRevByte(usPreviewBegin).WriteAndRevByte(usPreviewLength);
			w << rbData;
			PostSerialize(rb, phdr, cbFrame);
			return Result::Ok;
		}

		ECK_DECL_ID3FRAME_CLONE(AENC)
	};

	struct LINK :public FRAME
	{
		CHAR IdTarget[4]{};
		CRefStrA rsUrl{};
		CRefStrA rsAdditional{};

		Result SerializeData(CRefBin& rb, ID3v2_Header* phdr) override
		{
			const size_t cbFrame = 5 + rsUrl.Size() + rsUrl.Size();
			auto w = PreSerialize(rb, phdr, cbFrame);
			w << IdTarget << rsUrl << rsAdditional;
			PostSerialize(rb, phdr, cbFrame);
			return Result::Ok;
		}

		ECK_DECL_ID3FRAME_CLONE(LINK)
	};

	struct POSS :public FRAME
	{
		TimestampFmt eTimestamp{};
		UINT uTime{};

		Result SerializeData(CRefBin& rb, ID3v2_Header* phdr) override
		{
			const size_t cbFrame = 5;
			auto w = PreSerialize(rb, phdr, cbFrame);
			(w << eTimestamp).WriteAndRevByte(uTime);
			PostSerialize(rb, phdr, cbFrame);
			return Result::Ok;
		}

		ECK_DECL_ID3FRAME_CLONE(POSS)
	};

	struct USER :public FRAME
	{
		TextEncoding eEncoding{};
		CHAR byLang[3]{};
		CRefStrW rsText{};

		Result SerializeData(CRefBin& rb, ID3v2_Header* phdr) override
		{
			const auto rbText = CovertTextEncoding(rsText, eEncoding);
			const size_t cbFrame = 4 + rbText.Size();
			auto w = PreSerialize(rb, phdr, cbFrame);
			w << eEncoding << byLang << rbText;
			PostSerialize(rb, phdr, cbFrame);
			return Result::Ok;
		}

		ECK_DECL_ID3FRAME_CLONE(USER)
	};

	struct OWNE :public FRAME
	{
		TextEncoding eEncoding{};
		CRefStrA rsPrice{};
		CHAR szDate[8]{};
		CRefStrW rsSeller{};

		Result SerializeData(CRefBin& rb, ID3v2_Header* phdr) override
		{
			const auto rbSeller = CovertTextEncoding(rsSeller, eEncoding);
			const size_t cbFrame = 10 + rsPrice.Size() + rbSeller.Size();
			auto w = PreSerialize(rb, phdr, cbFrame);
			w << eEncoding << rsPrice << szDate << rbSeller;
			PostSerialize(rb, phdr, cbFrame);
			return Result::Ok;
		}

		ECK_DECL_ID3FRAME_CLONE(OWNE)
	};

	struct COMR :public FRAME
	{
		TextEncoding eEncoding{};
		ReceivedWay eReceivedWay{};
		CRefStrA rsPrice{};
		CHAR szDate[8]{};
		CRefStrA rsUrl{};
		CRefStrW rsSeller{};
		CRefStrW rsDesc{};
		CRefStrA rsMime{};
		CRefBin rbLogo{};

		Result SerializeData(CRefBin& rb, ID3v2_Header* phdr) override
		{
			const auto rbSeller = CovertTextEncoding(rsSeller, eEncoding, TRUE);
			const auto rbDesc = CovertTextEncoding(rsDesc, eEncoding, TRUE);

			const size_t cbFrame = 13 + rsPrice.Size() + rsUrl.Size() +
				rbSeller.Size() + rbDesc.Size() + rsMime.Size() + rbLogo.Size();
			auto w = PreSerialize(rb, phdr, cbFrame);
			w << eEncoding << eReceivedWay << rsPrice << szDate << rsUrl <<
				rbSeller << rbDesc << rsMime << rbLogo;
			PostSerialize(rb, phdr, cbFrame);
			return Result::Ok;
		}

		ECK_DECL_ID3FRAME_CLONE(COMR)
	};

	struct ENCR :public FRAME
	{
		CRefStrA rsEmail{};
		BYTE byMethod{};
		CRefBin rbData{};

		Result SerializeData(CRefBin& rb, ID3v2_Header* phdr) override
		{
			const size_t cbFrame = 2 + rsEmail.Size() + rbData.Size();
			auto w = PreSerialize(rb, phdr, cbFrame);
			w << rsEmail << byMethod << rbData;
			PostSerialize(rb, phdr, cbFrame);
			return Result::Ok;
		}

		ECK_DECL_ID3FRAME_CLONE(ENCR)
	};

	struct GRID :public FRAME
	{
		CRefStrA rsEmail{};
		BYTE byId{};
		CRefBin rbData{};

		Result SerializeData(CRefBin& rb, ID3v2_Header* phdr) override
		{
			const size_t cbFrame = 2 + rsEmail.Size() + rbData.Size();
			auto w = PreSerialize(rb, phdr, cbFrame);
			w << rsEmail << byId << rbData;
			PostSerialize(rb, phdr, cbFrame);
			return Result::Ok;
		}

		ECK_DECL_ID3FRAME_CLONE(GRID)
	};

	struct PRIV :public FRAME
	{
		CRefStrA rsEmail{};
		CRefBin rbData{};

		Result SerializeData(CRefBin& rb, ID3v2_Header* phdr) override
		{
			const size_t cbFrame = 1 + rsEmail.Size() + rbData.Size();
			auto w = PreSerialize(rb, phdr, cbFrame);
			w << rsEmail << rbData;
			PostSerialize(rb, phdr, cbFrame);
			return Result::Ok;
		}

		ECK_DECL_ID3FRAME_CLONE(PRIV)
	};

	struct SIGN :public FRAME
	{
		BYTE byGroupId{};
		CRefBin rbData{};

		Result SerializeData(CRefBin& rb, ID3v2_Header* phdr) override
		{
			const size_t cbFrame = 1 + rbData.Size();
			auto w = PreSerialize(rb, phdr, cbFrame);
			w << byGroupId << rbData;
			PostSerialize(rb, phdr, cbFrame);
			return Result::Ok;
		}

		ECK_DECL_ID3FRAME_CLONE(SIGN)
	};

	struct SEEK :public FRAME
	{
		UINT ocbNextTag{};

		Result SerializeData(CRefBin& rb, ID3v2_Header* phdr) override
		{
			constexpr size_t cbFrame = 4;
			auto w = PreSerialize(rb, phdr, cbFrame);
			w.WriteAndRevByte(ocbNextTag);
			PostSerialize(rb, phdr, cbFrame);
			return Result::Ok;
		}

		ECK_DECL_ID3FRAME_CLONE(SEEK)
	};
	// TODO:ASPI
private:
	std::vector<FRAME*> m_vFrame{};

	struct CFrame2
	{
		CStreamWalker& m_w;
		const ID3v2_FrameHeader& m_Header{};
		CRefBin m_rbFrame{};
		DWORD m_cbFrame{};
		BOOL m_bUnsync = FALSE;

		CFrame2(CID3v2& id3, const ID3v2_FrameHeader& Header, DWORD cbFrame, FRAME* pFrame) :
			m_w(id3.m_Stream), m_Header(Header), m_cbFrame(cbFrame),
			m_bUnsync(id3.m_Header.Flags& ID3V2HF_UNSYNCHRONIZATION),
			m_rbFrame(cbFrame)
		{
			memcpy(pFrame->Id, Header.ID, 4);
			memcpy(&pFrame->uFlags, Header.Flags, 2);
			if (!m_bUnsync && id3.m_Header.Ver == 4)
				m_bUnsync = !!(m_Header.Flags[1] & ID3V24FF_UNSYNCHRONIZATION);
		}

		std::pair<CMemWalker, DWORD> Begin()
		{
			m_w.Read(m_rbFrame.Data(), m_cbFrame);
			if (m_bUnsync)
				m_rbFrame.ReplaceSubBin({ 0xFF, 0x00 }, { 0xFF });// 恢复不同步处理
			return
			{
				CMemWalker(m_rbFrame.Data(), m_rbFrame.Size()),
				(DWORD)m_rbFrame.Size()
			};
		}
	};

	EckInline static CRefStrW GetID3v2_ProcString(CMemWalker& w, int cb, TextEncoding iTextEncoding)
	{
		return GetID3v2_ProcString(w, cb, (int)iTextEncoding);
	}
public:
	ECK_DISABLE_COPY_MOVE(CID3v2)
public:
	CID3v2(CMediaFile& File) :m_File{ File }, m_Stream(File.GetStream())
	{
		m_Stream.GetStream()->AddRef();
		m_Stream.MoveToBegin();
		m_Stream >> m_Header;
		m_cbTag = SynchSafeIntToDWORD(m_Header.Size);
		if (m_Header.Ver == 3)// 2.3
		{
			if (m_Header.Flags & 0x20)// 有扩展头
			{
				m_Stream >> m_ExtHdr;
				const int cb = ReverseInteger(*(DWORD*)m_ExtHdr.ExtHeaderSize) - 6;
				if (cb < 0)
					m_cbTag = 0u;
				else
					m_Stream += cb;
			}
		}
		else if (m_Header.Ver == 4)// 2.4
		{
			if (m_Header.Flags & 0x20)// 有扩展头
			{
				m_Stream >> m_ExtHdr;
				const int cb = SynchSafeIntToDWORD(m_ExtHdr.ExtHeaderSize) - 10;
				if (cb < 0)
					m_cbTag = 0u;
				else
					m_Stream += cb;
			}
			// 2.4里变成了同步安全整数，而且这个尺寸包含了记录尺寸的四个字节
		}
		else
		{
			m_cbTag = 0u;
			EckDbgPrintWithPos(L"未识别的ID3版本");
			EckDbgBreak();
		}
	}

	~CID3v2() { m_Stream.GetStream()->Release(); }

	BOOL ReadTag(MUSICINFO& mi)
	{
		mi.Clear();
		if (!m_cbTag)
			return FALSE;

		DWORD cbUnit;
		ID3v2_FrameHeader FrameHdr;
		DWORD cbExtra;

		ULARGE_INTEGER uliSize = m_Stream.GetSize();
		auto pos = m_Stream.GetPos();
		while (m_Stream.GetPos().QuadPart < m_cbTag)
		{
			m_Stream >> FrameHdr;

			cbExtra = 0;
			if (m_Header.Ver == 3)
			{
				cbUnit = ReverseInteger(*(DWORD*)FrameHdr.Size);// v2.3：32位数据，不包括帧头（偏4字节）
				if (FrameHdr.Flags[1] & ID3V23FF_HAS_GROUP_IDENTITY)
					cbExtra += 1;// 跳过组标识符
				if (FrameHdr.Flags[1] & ID3V23FF_ENCRYPTION)
					cbExtra += 1;// 跳过加密类型标识符
				if (FrameHdr.Flags[1] & ID3V23FF_COMPRESSION)
					cbExtra += 4;// 跳过数据长度指示器
			}
			else/* if (m_Header.Ver == 4)*/
			{
				if (FrameHdr.Flags[1] & ID3V24FF_HAS_GROUP_IDENTITY)
					cbExtra += 1;// 跳过组标识符
				if (FrameHdr.Flags[1] & ID3V24FF_ENCRYPTION)
					cbExtra += 1;// 跳过加密类型标识符
				if (FrameHdr.Flags[1] & ID3V24FF_HAS_DATA_LENGTH_INDICATOR)
					cbExtra += 4;// 跳过数据长度指示器
				cbUnit = SynchSafeIntToDWORD(FrameHdr.Size);// v2.4：28位数据（同步安全整数）
			}
			cbUnit -= cbExtra;
			m_Stream += cbExtra;
			// TODO:处理压缩帧

			if ((mi.uMask & MIM_TITLE) && memcmp(FrameHdr.ID, "TIT2", 4) == 0)// 标题
			{
				CFrame f(*this, FrameHdr, cbUnit);
				auto [w, cb] = f.Begin();
				mi.rsTitle = GetID3v2_ProcString(w, cb);
				mi.uMaskRead |= MIM_TITLE;
			}
			else if ((mi.uMask & MIM_ARTIST) && memcmp(FrameHdr.ID, "TPE1", 4) == 0)// 作者
			{
				CFrame f(*this, FrameHdr, cbUnit);
				auto [w, cb] = f.Begin();
				mi.AppendArtist(GetID3v2_ProcString(w, cb));
				mi.uMaskRead |= MIM_ARTIST;
			}
			else if ((mi.uMask & MIM_ALBUM) && memcmp(FrameHdr.ID, "TALB", 4) == 0)// 专辑
			{
				CFrame f(*this, FrameHdr, cbUnit);
				auto [w, cb] = f.Begin();
				mi.rsAlbum = GetID3v2_ProcString(w, cb);
				mi.uMaskRead |= MIM_ALBUM;
			}
			else if ((mi.uMask & MIM_LRC) && memcmp(FrameHdr.ID, "USLT", 4) == 0)// 不同步歌词
			{
				/*
				<帧头>（帧标识为USLT）
				文本编码						$xx
				自然语言代码					$xx xx xx
				内容描述						<字符串> $00 (00)
				歌词							<字符串>
				*/
				CFrame f(*this, FrameHdr, cbUnit);
				auto [w, cb] = f.Begin();

				BYTE byEncodeType;
				w >> byEncodeType;// 读文本编码

				CHAR byLangCode[3];
				w >> byLangCode;// 读自然语言代码

				UINT t;
				if (byEncodeType == 0 || byEncodeType == 3)// ISO-8859-1或UTF-8
					t = (int)strlen((PCSTR)w.Data()) + 1;
				else// UTF-16LE或UTF-16BE
					t = ((int)wcslen((PCWSTR)w.Data()) + 1) * sizeof(WCHAR);
				w += t;// 跳过内容描述

				cb -= (t + 4);

				mi.rsLrc = GetID3v2_ProcString(w, cb, byEncodeType);
				mi.uMaskRead |= MIM_LRC;
			}
			else if ((mi.uMask & MIM_COMMENT) && memcmp(FrameHdr.ID, "COMM", 4) == 0)// 备注
			{
				/*
				<帧头>（帧标识为COMM）
				文本编码						$xx
				自然语言代码					$xx xx xx
				备注摘要						<字符串> $00 (00)
				备注							<字符串>
				*/
				CFrame f(*this, FrameHdr, cbUnit);
				auto [w, cb] = f.Begin();

				BYTE byEncodeType;
				w >> byEncodeType;// 读文本编码

				CHAR byLangCode[3];
				w >> byLangCode;// 读自然语言代码

				UINT t;
				if (byEncodeType == 0 || byEncodeType == 3)// ISO-8859-1或UTF-8
					t = (int)strlen((PCSTR)w.Data()) + 1;
				else// UTF-16LE或UTF-16BE
					t = ((int)wcslen((PCWSTR)w.Data()) + 1) * sizeof(WCHAR);
				w += t;// 跳过备注摘要

				cb -= (t + 4);

				mi.AppendComment(GetID3v2_ProcString(w, cb, byEncodeType));
				mi.uMaskRead |= MIM_COMMENT;
			}
			else if ((mi.uMask & MIM_COVER) && memcmp(FrameHdr.ID, "APIC", 4) == 0)// 图片
			{
				/*
				<帧头>（帧标识为APIC）
				文本编码                        $xx
				MIME 类型                       <ASCII字符串>$00（如'image/bmp'）
				图片类型                        $xx
				描述                            <字符串>$00(00)
				<图片数据>
				*/
				CFrame f(*this, FrameHdr, cbUnit);
				auto [w, cb] = f.Begin();

				MUSICPIC Pic{};

				BYTE byEncodeType;
				w >> byEncodeType;// 读文本编码

				BYTE byType;

				UINT t;
				t = (int)strlen((PCSTR)w.Data());
				CRefStrA rsMime((PCSTR)w.Data(), t);
				Pic.rsMime = StrX2W(rsMime.Data(), rsMime.Size());
				w += (t + 1);// 跳过MIME类型字符串

				w >> byType;// 图片类型
				if (byType < (BYTE)PicType::Begin___ || byType >= (BYTE)PicType::End___)
					Pic.eType = PicType::Invalid;
				else
					Pic.eType = (PicType)byType;

				Pic.rsDesc = GetID3v2_ProcString(w, -1, byEncodeType);
				Pic.bLink = (Pic.rsDesc == L"-->");

				if (Pic.bLink)
					Pic.varPic = GetID3v2_ProcString(w, (int)w.GetLeaveSize(), byEncodeType);
				else
					Pic.varPic = CRefBin(w.Data(), cb);
				mi.vImage.push_back(std::move(Pic));
				mi.uMaskRead |= MIM_COVER;
			}
			else if ((mi.uMask & MIM_GENRE) && memcmp(FrameHdr.ID, "TCON", 4) == 0)// 流派
			{
				CFrame f(*this, FrameHdr, cbUnit);
				auto [w, cb] = f.Begin();
				mi.rsGenre = GetID3v2_ProcString(w, cb);
				mi.uMaskRead |= MIM_GENRE;
			}
			else if ((mi.uMask & MIM_DATE) && memcmp(FrameHdr.ID, "TYER", 4) == 0 && m_Header.Ver == 3)// 年代
			{
				// TODO:本地格式化
				CFrame f(*this, FrameHdr, cbUnit);
				auto [w, cb] = f.Begin();
				auto rsDate = GetID3v2_ProcString(w, cb);
				if (rsDate.Size() == 4)
				{
					if (mi.uFlag & MIF_DATE_STRING)
						mi.Date = std::move(rsDate);
					else
						mi.Date = SYSTEMTIME{ .wYear = (WORD)_wtoi(rsDate.Data()) };
					mi.uMaskRead |= MIM_DATE;
				}
			}
			else if ((mi.uMask & MIM_DATE) && memcmp(FrameHdr.ID, "TDRC", 4) == 0 && m_Header.Ver == 4)// 年代
			{
				CFrame f(*this, FrameHdr, cbUnit);
				auto [w, cb] = f.Begin();
				auto rsDate = GetID3v2_ProcString(w, cb);
				if (rsDate.Size() >= 4)
				{
					if (mi.uFlag & MIF_DATE_STRING)
					{
						mi.Date = std::move(rsDate);
						mi.uMaskRead |= MIM_DATE;
					}
					else
					{
						SYSTEMTIME st{};
						if (swscanf(rsDate.Data(), L"%hd-%hd-%hdT%hd:%hd:%hd",
							&st.wYear, &st.wMonth, &st.wDay,
							&st.wHour, &st.wMinute, &st.wSecond) > 0)
						{
							mi.Date = st;
							mi.uMaskRead |= MIM_DATE;
						}
					}
				}
			}
			else
				m_Stream += cbUnit;
		}
		return TRUE;
	}

	Result ReadTag(UINT uFlags)
	{
		for (auto e : m_vFrame)
			delete e;
		m_vFrame.clear();
		if (!m_cbTag)
			return Result::TagErr;

		DWORD cbUnit;
		ID3v2_FrameHeader FrameHdr;
		DWORD cbExtra;

		ULARGE_INTEGER uliSize = m_Stream.GetSize();
		auto pos = m_Stream.GetPos();
		while (m_Stream.GetPos().QuadPart < m_cbTag)
		{
			m_Stream >> FrameHdr;

			cbExtra = 0;
			if (m_Header.Ver == 3)
			{
				cbUnit = ReverseInteger(*(DWORD*)FrameHdr.Size);// v2.3：32位数据，不包括帧头（偏4字节）
				if (FrameHdr.Flags[1] & ID3V23FF_HAS_GROUP_IDENTITY)
					cbExtra += 1;// 跳过组标识符
				if (FrameHdr.Flags[1] & ID3V23FF_ENCRYPTION)
					cbExtra += 1;// 跳过加密类型标识符
				if (FrameHdr.Flags[1] & ID3V23FF_COMPRESSION)
					cbExtra += 4;// 跳过数据长度指示器
			}
			else/* if (m_Header.Ver == 4)*/
			{
				if (FrameHdr.Flags[1] & ID3V24FF_HAS_GROUP_IDENTITY)
					cbExtra += 1;// 跳过组标识符
				if (FrameHdr.Flags[1] & ID3V24FF_ENCRYPTION)
					cbExtra += 1;// 跳过加密类型标识符
				if (FrameHdr.Flags[1] & ID3V24FF_HAS_DATA_LENGTH_INDICATOR)
					cbExtra += 4;// 跳过数据长度指示器
				cbUnit = SynchSafeIntToDWORD(FrameHdr.Size);// v2.4：28位数据（同步安全整数）
			}
			cbUnit -= cbExtra;
			m_Stream += cbExtra;
			// TODO:处理压缩帧

#define ECK_HIT_ID3FRAME(x) (memcmp(FrameHdr.ID, #x, 4) == 0)
			if (ECK_HIT_ID3FRAME(UFID))
			{
				const auto p = new UFID{};
				CFrame2 f(*this, FrameHdr, cbUnit, p);
				auto [w, cb] = f.Begin();

				w >> p->rsEmail;
				if (w.GetLeaveSize() > 64u)
				{
					delete p;
					return Result::TooLargeData;
				}
				else
				{
					p->rbOwnerData.DupStream(w.Data(), w.GetLeaveSize());
					m_vFrame.push_back(p);
				}
			}
			else if (ECK_HIT_ID3FRAME(TXXX))
			{
				const auto p = new TXXX{};
				CFrame2 f(*this, FrameHdr, cbUnit, p);
				auto [w, cb] = f.Begin();

				w >> p->eEncoding;
				if (p->eEncoding >= TextEncoding::Max)
				{
					delete p;
					return Result::TextEncodingErr;
				}
				p->rsDesc = GetID3v2_ProcString(w, -1, p->eEncoding);
				p->rsText = GetID3v2_ProcString(w, (int)w.GetLeaveSize(), p->eEncoding);
				m_vFrame.push_back(p);
			}
			else if (ECK_HIT_ID3FRAME(MCID))
			{
				const auto p = new MCID{};
				CFrame2 f(*this, FrameHdr, cbUnit, p);
				auto [w, cb] = f.Begin();
				p->rbToc = std::move(f.m_rbFrame);
				m_vFrame.push_back(p);
			}
			else if (ECK_HIT_ID3FRAME(WXXX))
			{
				const auto p = new WXXX{};
				CFrame2 f(*this, FrameHdr, cbUnit, p);
				auto [w, cb] = f.Begin();

				w >> p->eEncoding;
				if (p->eEncoding >= TextEncoding::Max)
				{
					delete p;
					return Result::TextEncodingErr;
				}
				p->rsDesc = GetID3v2_ProcString(w, -1, p->eEncoding);
				p->rsUrl.DupString((PCSTR)w.Data(), (int)w.GetLeaveSize());
				m_vFrame.push_back(p);
			}
			else if (ECK_HIT_ID3FRAME(ETCO))
			{
				const auto p = new ETCO{};
				CFrame2 f(*this, FrameHdr, cbUnit, p);
				auto [w, cb] = f.Begin();

				w >> p->eTimestampFmt;
				if (p->eTimestampFmt >= TimestampFmt::Max)
				{
					delete p;
					return Result::InvalidEnumVal;
				}
				if ((cb - 1) % 5)
				{
					delete p;
					return Result::LenErr;
				}
				EckCounterNV((cb - 1) / 5)
				{
					auto& e = p->vEvent.emplace_back();
					w >> e.eType >> e.uTimestamp;
				}

				m_vFrame.push_back(p);
			}
			else if (ECK_HIT_ID3FRAME(MLLT))
			{
				const auto p = new MLLT{};
				CFrame2 f(*this, FrameHdr, cbUnit, p);
				auto [w, cb] = f.Begin();

				(w >> p->cMpegFrame)
					.Read(&p->cByte, 3)
					.Read(&p->cMilliseconds, 3)
					>> p->cByteOffsetValBit
					>> p->cMillisecondsOffsetValBit;
				auto& e = p->vRef.emplace_back();
				// TODO:位
				m_vFrame.push_back(p);
			}
			else if (ECK_HIT_ID3FRAME(SYTC))
			{
				const auto p = new SYTC{};
				CFrame2 f(*this, FrameHdr, cbUnit, p);
				auto [w, cb] = f.Begin();
				w >> p->eTimestampFmt;
				BYTE by;
				Result result{ Result::Ok };
				while (!w.IsEnd())
				{
					auto& e = p->vTempo.emplace_back();
					w >> by;
					if (by == 0xFF)
					{
						if (w.IsEnd())
						{
							result = Result::LenErr;
							goto Failed;
						}
						w >> by;
						e.bpm = 255 + by;
					}
					else
						e.bpm = by;
					if (w.IsEnd())
					{
						result = Result::LenErr;
						goto Failed;
					}
					w >> e.uTimestamp;
				}
				m_vFrame.push_back(p);
				goto Ok;
			Failed:
				delete p;
				return result;
			Ok:;
			}
			else if (ECK_HIT_ID3FRAME(USLT))
			{
				const auto p = new USLT{};
				CFrame2 f(*this, FrameHdr, cbUnit, p);
				auto [w, cb] = f.Begin();
				if (cb < 4)
				{
					delete p;
					return Result::LenErr;
				}

				w >> p->eEncoding;
				if (p->eEncoding >= TextEncoding::Max)
				{
					delete p;
					return Result::TextEncodingErr;
				}
				w >> p->byLang;
				p->rsDesc = GetID3v2_ProcString(w, -1, p->eEncoding);
				p->rsLrc = GetID3v2_ProcString(w, (int)w.GetLeaveSize(), p->eEncoding);
				m_vFrame.push_back(p);
			}
			else if (ECK_HIT_ID3FRAME(SYLT))
			{
				const auto p = new SYLT{};
				CFrame2 f(*this, FrameHdr, cbUnit, p);
				auto [w, cb] = f.Begin();
				if (cb < 7)
				{
					delete p;
					return Result::LenErr;
				}

				w >> p->eEncoding;
				if (p->eEncoding >= TextEncoding::Max)
				{
					delete p;
					return Result::TextEncodingErr;
				}
				w >> p->byLang >> p->eTimestampFmt >> p->eContent;
				p->rsDesc = GetID3v2_ProcString(w, -1, p->eEncoding);
				while (!w.IsEnd())
				{
					auto& e = p->vSync.emplace_back();
					e.rsText = GetID3v2_ProcString(w, -1, p->eEncoding);
					if (w.IsEnd())
					{
						delete p;
						return Result::LenErr;
					}
					w >> e.uTimestamp;
				}
				m_vFrame.push_back(p);
			}
			else if (ECK_HIT_ID3FRAME(COMM))
			{
				const auto p = new COMM{};
				CFrame2 f(*this, FrameHdr, cbUnit, p);
				auto [w, cb] = f.Begin();
				if (cb < 5)
				{
					delete p;
					return Result::LenErr;
				}
				w >> p->eEncoding;
				if (p->eEncoding >= TextEncoding::Max)
				{
					delete p;
					return Result::TextEncodingErr;
				}
				w >> p->byLang;
				p->rsDesc = GetID3v2_ProcString(w, -1, p->eEncoding);
				p->rsText = GetID3v2_ProcString(w, (int)w.GetLeaveSize(), p->eEncoding);
				m_vFrame.push_back(p);
			}
			else if (ECK_HIT_ID3FRAME(RVA2))
			{
				const auto p = new RVA2{};
				CFrame2 f(*this, FrameHdr, cbUnit, p);
				auto [w, cb] = f.Begin();
				if (cb < 5)
				{
					delete p;
					return Result::LenErr;
				}

				w >> p->rsId;
				while (!w.IsEnd())
				{
					auto& e = p->vChannel.emplace_back();
					w >> e.eChannel >> e.shVol >> e.cPeekVolBit;
					if (e.cPeekVolBit)
					{
						// TODO:位
					}
				}

				m_vFrame.push_back(p);
			}
			else if (ECK_HIT_ID3FRAME(EQU2))
			{
				const auto p = new EQU2{};
				CFrame2 f(*this, FrameHdr, cbUnit, p);
				auto [w, cb] = f.Begin();
				if (cb < 6)
				{
					delete p;
					return Result::LenErr;
				}

				w >> p->eInterpolation >> p->rsId;
				if (w.GetLeaveSize() % 4)
				{
					delete p;
					return Result::LenErr;
				}
				const auto cPoint = w.GetLeaveSize() / 4;
				EckCounterNV(cPoint)
				{
					auto& e = p->vPoint.emplace_back();
					w >> e;
				}
				m_vFrame.push_back(p);
			}
			else if (ECK_HIT_ID3FRAME(RVRB))
			{
				const auto p = new RVRB{};
				CFrame2 f(*this, FrameHdr, cbUnit, p);
				auto [w, cb] = f.Begin();
				if (cb != 12)
				{
					delete p;
					return Result::LenErr;
				}
				w.Read(&p->Left, 12);
				m_vFrame.push_back(p);
			}
			else if (ECK_HIT_ID3FRAME(APIC))
			{
				const auto p = new APIC{};
				CFrame2 f(*this, FrameHdr, cbUnit, p);
				auto [w, cb] = f.Begin();
				if (cb < 5)
				{
					delete p;
					return Result::LenErr;
				}

				w >> p->eTextEncoding >> p->rsMime >> p->eType;
				p->rsDesc = GetID3v2_ProcString(w, -1, p->eTextEncoding);
				p->rbData.DupStream(w.Data(), w.GetLeaveSize());

				m_vFrame.push_back(p);
			}
			else if (ECK_HIT_ID3FRAME(GEOB))
			{
				const auto p = new GEOB{};
				CFrame2 f(*this, FrameHdr, cbUnit, p);
				auto [w, cb] = f.Begin();
				if (cb < 4)
				{
					delete p;
					return Result::LenErr;
				}
				w >> p->eTextEncoding >> p->rsMime;
				p->rsFile = GetID3v2_ProcString(w, -1, p->eTextEncoding);
				p->rsDesc = GetID3v2_ProcString(w, -1, p->eTextEncoding);
				p->rbObj.DupStream(w.Data(), w.GetLeaveSize());
				m_vFrame.push_back(p);
			}
			else if (ECK_HIT_ID3FRAME(PCNT))
			{
				const auto p = new PCNT{};
				CFrame2 f(*this, FrameHdr, cbUnit, p);
				auto [w, cb] = f.Begin();
				if (cb < 4)
				{
					delete p;
					return Result::LenErr;
				}
				w.Read(&p->cPlay, std::min(cb, (DWORD)8u));// 截断到8字节

				m_vFrame.push_back(p);
			}
			else if (ECK_HIT_ID3FRAME(POPM))
			{
				const auto p = new POPM{};
				CFrame2 f(*this, FrameHdr, cbUnit, p);
				auto [w, cb] = f.Begin();
				if (cb < 6)
				{
					delete p;
					return Result::LenErr;
				}
				w >> p->rsEmail >> p->byRating;
				w.Read(&p->cPlay, std::min(w.GetLeaveSize(), (SIZE_T)8u));// 截断到8字节
				m_vFrame.push_back(p);
			}
			else if (ECK_HIT_ID3FRAME(RBUF))
			{
				const auto p = new RBUF{};
				CFrame2 f(*this, FrameHdr, cbUnit, p);
				auto [w, cb] = f.Begin();
				if (cb != 8)
				{
					delete p;
					return Result::LenErr;
				}
				w.Read(&p->cbBuf, 3) >> p->b >> p->ocbNextTag;

				m_vFrame.push_back(p);
			}
			else if (ECK_HIT_ID3FRAME(AENC))
			{
				const auto p = new AENC{};
				CFrame2 f(*this, FrameHdr, cbUnit, p);
				auto [w, cb] = f.Begin();
				if (cb < 5)
				{
					delete p;
					return Result::LenErr;
				}
				w >> p->rsOwnerId >> p->usPreviewBegin >> p->usPreviewLength;
				p->rbData.DupStream(w.Data(), w.GetLeaveSize());
				m_vFrame.push_back(p);
			}
			else if (ECK_HIT_ID3FRAME(LINK))
			{
				const auto p = new LINK{};
				CFrame2 f(*this, FrameHdr, cbUnit, p);
				auto [w, cb] = f.Begin();
				if (cb < 5)
				{
					delete p;
					return Result::LenErr;
				}
				w >> p->IdTarget >> p->rsUrl;
				p->rsAdditional.DupString((PCSTR)w.Data(), (int)w.GetLeaveSize());

				m_vFrame.push_back(p);
			}
			else if (ECK_HIT_ID3FRAME(POSS))
			{
				const auto p = new POSS{};
				CFrame2 f(*this, FrameHdr, cbUnit, p);
				auto [w, cb] = f.Begin();
				if (cb != 5)
				{
					delete p;
					return Result::LenErr;
				}
				w >> p->eTimestamp >> p->uTime;
				m_vFrame.push_back(p);
			}
			else if (ECK_HIT_ID3FRAME(USER))
			{
				const auto p = new USER{};
				CFrame2 f(*this, FrameHdr, cbUnit, p);
				auto [w, cb] = f.Begin();
				if (cb < 4)
				{
					delete p;
					return Result::LenErr;
				}
				w >> p->eEncoding >> p->byLang;
				p->rsText = GetID3v2_ProcString(w, (int)w.GetLeaveSize(), p->eEncoding);

				m_vFrame.push_back(p);
			}
			else if (ECK_HIT_ID3FRAME(OWNE))
			{
				const auto p = new OWNE{};
				CFrame2 f(*this, FrameHdr, cbUnit, p);
				auto [w, cb] = f.Begin();
				if (cb < 10)
				{
					delete p;
					return Result::LenErr;
				}
				w >> p->eEncoding >> p->rsPrice >> p->szDate;
				p->rsSeller = GetID3v2_ProcString(w, (int)w.GetLeaveSize(), p->eEncoding);
				m_vFrame.push_back(p);
			}
			else if (ECK_HIT_ID3FRAME(COMR))
			{
				const auto p = new COMR{};
				CFrame2 f(*this, FrameHdr, cbUnit, p);
				auto [w, cb] = f.Begin();
				if (cb < 15)
				{
					delete p;
					return Result::LenErr;
				}

				w >> p->eEncoding >> p->rsPrice >> p->szDate >> p->rsUrl >> p->eReceivedWay;
				p->rsSeller = GetID3v2_ProcString(w, -1, p->eEncoding);
				p->rsDesc = GetID3v2_ProcString(w, -1, p->eEncoding);
				w >> p->rsMime;
				p->rbLogo.DupStream(w.Data(), w.GetLeaveSize());

				m_vFrame.push_back(p);
			}
			else if (ECK_HIT_ID3FRAME(ENCR))
			{
				const auto p = new ENCR{};
				CFrame2 f(*this, FrameHdr, cbUnit, p);
				auto [w, cb] = f.Begin();
				if (cb < 2)
				{
					delete p;
					return Result::LenErr;
				}
				w >> p->rsEmail >> p->byMethod;
				p->rbData.DupStream(w.Data(), w.GetLeaveSize());
				m_vFrame.push_back(p);
			}
			else if (ECK_HIT_ID3FRAME(GRID))
			{
				const auto p = new GRID{};
				CFrame2 f(*this, FrameHdr, cbUnit, p);
				auto [w, cb] = f.Begin();
				if (cb < 2)
				{
					delete p;
					return Result::LenErr;
				}
				w >> p->rsEmail >> p->byId;
				p->rbData.DupStream(w.Data(), w.GetLeaveSize());
				m_vFrame.push_back(p);
			}
			else if (ECK_HIT_ID3FRAME(PRIV))
			{
				const auto p = new PRIV{};
				CFrame2 f(*this, FrameHdr, cbUnit, p);
				auto [w, cb] = f.Begin();
				if (cb < 1)
				{
					delete p;
					return Result::LenErr;
				}
				w >> p->rsEmail;

				m_vFrame.push_back(p);
			}
			else if (ECK_HIT_ID3FRAME(SIGN))
			{
				const auto p = new SIGN{};
				CFrame2 f(*this, FrameHdr, cbUnit, p);
				auto [w, cb] = f.Begin();
				if (cb < 1)
				{
					delete p;
					return Result::LenErr;
				}
				w >> p->byGroupId;
				p->rbData.DupStream(w.Data(), w.GetLeaveSize());
				m_vFrame.push_back(p);
			}
			else if (ECK_HIT_ID3FRAME(SEEK))
			{
				const auto p = new SEEK{};
				CFrame2 f(*this, FrameHdr, cbUnit, p);
				auto [w, cb] = f.Begin();
				if (cb != 4)
				{
					delete p;
					return Result::LenErr;
				}
				w >> p->ocbNextTag;
				m_vFrame.push_back(p);
			}
			//else if (ECK_HIT_ID3FRAME(ASPI))
			//{
			//	const auto p = new ASPI{};
			//	CFrame2 f(*this, FrameHdr, cbUnit, p);
			//	auto [w, cb] = f.Begin();


			//	m_vFrame.push_back(p);
			//}
			else if (FrameHdr.ID[0] == 'T')
			{
				const auto p = new TEXTFRAME{};
				CFrame2 f(*this, FrameHdr, cbUnit, p);
				auto [w, cb] = f.Begin();
				w >> p->eEncoding;
				auto rs = GetID3v2_ProcString(w, cb - 1, p->eEncoding);
				SplitBin(rs.Data(), rs.ByteSize() - sizeof(WCHAR), L"\0", sizeof(WCHAR), 0,
					[pFrame = p](PCVOID p, SIZE_T cb)
					{
						pFrame->vText.emplace_back((PCWSTR)p, (int)(cb / sizeof(WCHAR)));
					});
				m_vFrame.push_back(p);
			}
			else if (FrameHdr.ID[0] == 'W')
			{
				const auto p = new LINKFRAME{};
				CFrame2 f(*this, FrameHdr, cbUnit, p);
				auto [w, cb] = f.Begin();
				p->rsUrl.DupString((PCSTR)w.Data(), (int)cb);
				m_vFrame.push_back(p);
			}
		}
		return Result::Ok;
	}

	Result WriteTag(UINT uFlags)
	{

	}

	EckInline auto& GetFrameList() { return m_vFrame; }
};

class CFlac
{
private:
	CMediaFile& m_File;
	CStreamWalker m_Stream{};
public:
	ECK_DISABLE_COPY_MOVE(CFlac)
public:
	CFlac(CMediaFile& File) :m_File{ File }, m_Stream(File.GetStream())
	{
		m_Stream.GetStream()->AddRef();
		m_Stream.MoveToBegin() += 4;
	}

	~CFlac() { m_Stream.GetStream()->Release(); }

	BOOL ReadTag(MUSICINFO& mi)
	{
		FLAC_BlockHeader Header;
		DWORD cbBlock;
		UINT t;
		do
		{
			m_Stream >> Header;
			cbBlock = Header.bySize[2] | Header.bySize[1] << 8 | Header.bySize[0] << 16;
			if (cbBlock <= 0)
				return FALSE;
			switch (Header.by & 0x7F)
			{
			case 4:// 标签信息，注意：这一部分是小端序
			{
				m_Stream >> t;// 编码器信息大小
				m_Stream += t;// 跳过编码器信息

				UINT uCount;
				m_Stream >> uCount;// 标签数量

				EckCounterNV(uCount)
				{
					m_Stream >> t;// 标签大小

					CRefStrA u8Label(t);
					m_Stream.Read(u8Label.Data(), t);// 读标签
					int iPos = u8Label.Find("=");// 找等号
					if (iPos == StrNPos)
						continue;
					++iPos;
					const int cchActual = u8Label.Size() - iPos;
					if ((mi.uMask & MIM_TITLE) && u8Label.IsStartOf("TITLE"))
					{
						mi.rsTitle = StrX2W(u8Label.Data() + iPos, cchActual, CP_UTF8);
						mi.uMaskRead |= MIM_TITLE;
					}
					else if ((mi.uMask & MIM_ARTIST) && u8Label.IsStartOf("ARTIST"))
					{
						mi.AppendArtist(StrX2W(u8Label.Data() + iPos, cchActual, CP_UTF8));
						mi.uMaskRead |= MIM_ARTIST;
					}
					else if ((mi.uMask & MIM_ALBUM) && u8Label.IsStartOf("ALBUM"))
					{
						mi.rsAlbum = StrX2W(u8Label.Data() + iPos, cchActual, CP_UTF8);
						mi.uMaskRead |= MIM_ALBUM;
					}
					else if ((mi.uMask & MIM_LRC) && u8Label.IsStartOf("LYRICS"))
					{
						mi.rsLrc = StrX2W(u8Label.Data() + iPos, cchActual, CP_UTF8);
						mi.uMaskRead |= MIM_LRC;
					}
					else if ((mi.uMask & MIM_COMMENT) && u8Label.IsStartOf("DESCRIPTION"))
					{
						mi.AppendComment(StrX2W(u8Label.Data() + iPos, cchActual, CP_UTF8));
						mi.uMaskRead |= MIM_COMMENT;
					}
					else if ((mi.uMask & MIM_GENRE) && u8Label.IsStartOf("GENRE"))
					{
						mi.rsGenre = StrX2W(u8Label.Data() + iPos, cchActual, CP_UTF8);
						mi.uMaskRead |= MIM_GENRE;
					}
					else if ((mi.uMask & MIM_DATE) && u8Label.IsStartOf("DATE"))
					{
						WORD y, m{}, d{};
						// TODO:日期处理
						if (sscanf(u8Label.Data() + iPos, "%hd-%hd-%hd", &y, &m, &d) >= 1)
							mi.Date = SYSTEMTIME{ .wYear = y,.wMonth = m,.wDay = d };
						mi.uMaskRead |= MIM_DATE;
					}
					else if ((mi.uMask & MIM_COVER) && u8Label.IsStartOf("METADATA_BLOCK_PICTURE"))
					{
						auto rb = Base64Decode(u8Label.Data() + iPos, cchActual);
						CMemReader r(rb.Data(), rb.Size());
						MUSICPIC Pic{};

						DWORD dwType;
						r >> dwType;// 图片类型
						if (dwType < (BYTE)PicType::Begin___ || dwType >= (BYTE)PicType::End___)
							Pic.eType = PicType::Invalid;
						else
							Pic.eType = (PicType)dwType;

						r >> t;// 长度
						t = ReverseInteger(t);
						CRefStrA rsMime(t);
						r.Read(rsMime.Data(), t);// MIME类型字符串
						Pic.rsMime = StrX2W(rsMime.Data(), rsMime.Size());

						r >> t;// 描述字符串长度
						t = ReverseInteger(t);
						CRefStrA u8Desc(t);
						r.Read(u8Desc.Data(), t);// MIME类型字符串
						Pic.rsDesc = StrX2W(u8Desc.Data(), u8Desc.Size(), CP_UTF8);

						r += 16;// 跳过宽度、高度、色深、索引图颜色数

						r >> t;// 图片数据长度
						t = ReverseInteger(t);// 图片数据长度

						Pic.bLink = (Pic.rsMime == L"-->");

						if (Pic.bLink)
						{
							CRefStrA u8(t);
							r.Read(u8.Data(), t);
							Pic.varPic = StrX2W(u8.Data(), u8.Size(), CP_UTF8);
						}
						else
						{
							rb.Erase(0, r.Data() - rb.Data());
							Pic.varPic = std::move(rb);
						}
						mi.vImage.push_back(std::move(Pic));
						mi.uMaskRead |= MIM_COVER;
					}
				}
			}
			break;
			case 6:// 图片（大端序）
			{
				if (mi.uMask & MIM_COVER)
				{
					MUSICPIC Pic{};

					DWORD dwType;
					m_Stream >> dwType;// 图片类型
					if (dwType < (BYTE)PicType::Begin___ || dwType >= (BYTE)PicType::End___)
						Pic.eType = PicType::Invalid;
					else
						Pic.eType = (PicType)dwType;

					m_Stream >> t;// 长度
					t = ReverseInteger(t);
					CRefStrA rsMime(t);
					m_Stream.Read(rsMime.Data(), t);// MIME类型字符串
					Pic.rsMime = StrX2W(rsMime.Data(), rsMime.Size());

					m_Stream >> t;// 描述字符串长度
					t = ReverseInteger(t);
					CRefStrA u8Desc(t);
					m_Stream.Read(u8Desc.Data(), t);// MIME类型字符串
					Pic.rsDesc = StrX2W(u8Desc.Data(), u8Desc.Size(), CP_UTF8);

					m_Stream += 16;// 跳过宽度、高度、色深、索引图颜色数

					m_Stream >> t;// 图片数据长度
					t = ReverseInteger(t);// 图片数据长度

					Pic.bLink = (Pic.rsMime == L"-->");

					if (Pic.bLink)
					{
						CRefStrA u8(t);
						m_Stream.Read(u8.Data(), t);
						Pic.varPic = StrX2W(u8.Data(), u8.Size(), CP_UTF8);
					}
					else
						Pic.varPic = m_Stream.ReadBin(t);
					mi.vImage.push_back(std::move(Pic));
					mi.uMaskRead |= MIM_COVER;
				}
				else
					m_Stream += cbBlock;
			}
			break;
			default:
				m_Stream += cbBlock;// 跳过块
			}

		} while (!(Header.by & 0x80));// 检查最高位，判断是不是最后一个块
		return TRUE;
	}
};
ECK_MEDIATAG_NAMESPACE_END
ECK_NAMESPACE_END