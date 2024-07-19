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
#include "CBitSet.h"
#include "CException.h"

#include <variant>

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
	TAG_ID3V1Ext = 1u << 2,
	TAG_ID3V2_3 = 1u << 3,
	TAG_ID3V2_4 = 1u << 4,
	TAG_FLAC = 1u << 5,
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
	//												| ID3v1	| ID3v2 | Flac	|  APE	|
	// 用指定的分隔符连接多个艺术家
	MIF_JOIN_ARTIST = 1u << 0,					//	|		|	T	|	T	|	T	|
	// 用指定的分隔符连接多个备注
	MIF_JOIN_COMMENT = 1u << 1,					// 	|		|	T	|	T	|	T	|
	// 允许保留空白填充
	MIF_ALLOW_PADDING = 1u << 2,				// 	|		|	T	|	T	|		|
	// 用当前本地设置格式化日期为字符串
	MIF_DATE_STRING = 1u << 3,					// 	|	T	|	T	|	T	|	T	|
	// 用ID3v2.3规定的斜杠("/")分割艺术家列表
	MIF_SPLIT_ARTIST_IN_ID3V2_3 = 1u << 4,		// 	|		|	T	|		|		|
	// 移除其他标记系统
	MIF_REMOVE_OTHER_TAG = 1u << 5,				// 	|	T	|	T	|	T	|	T	|
	// 在文件尾部追加标签
	MIF_APPEND_TAG = 1u << 6,					// 	|		|	T	|		|	T	|
	// 当存在其他标签时同步其信息
	MIF_SYNC_OTHER_TAG = 1u << 7,				// 	|	T	|	T	|	T	|	T	|
	// 创建ID3v1扩展信息
	MIF_CREATE_ID3V1_EXT = 1u << 8,				// 	|	T	|		|		|		|
	// 创建ID3v2.3标签
	MIF_CREATE_ID3V2_3 = 1u << 9,				// 	|		|	T	|		|		|
	// 创建ID3v2.4标签
	MIF_CREATE_ID3V2_4 = 1u << 10,				// 	|		|	T	|		|		|
	// 移除空白填充
	MIF_REMOVE_PADDING = 1u << 11,				// 	|		|	T	|	T	|		|
	// 写入METADATA_BLOCK_PICTURE而不是图片块
	MIF_WRITE_METADATA_BLOCK_PICTURE = 1u << 12,//	|		|		|	T	|		|
};
// ID3帧写入选项
enum :BYTE
{
	MTID3F_PREPEND = 0,		// 将该帧置于文件头部的标签，这是默认值
	MTID3F_APPEND = 1u << 0,// 将该帧置于文件尾部的标签
};

// 错误码
enum class Result
{
	Ok,
	TagErr,				// 标签识别出错
	IllegalEnum_TextEncoding,
	TooLargeData,
	InvalidEnumVal,
	LenErr,
	InvalidVal,
	IllegalRepeat,
	EmptyData,			// 某数据为空
	ReservedDataErr,	// 保留部分或未定义部分填入错误信息
	NoTag,				// 文件中无标签或标签还未被读入
	MpegSyncFailed,		// MPEG同步失败
};
// 图片类型
enum class PicType :BYTE
{
	Invalid = 0xFF,     // 任何无效的值
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
	CRefStrA rsMime;
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

	void AppendArtist(const CRefStrW& rs_)
	{
		if (uFlag & MIF_JOIN_ARTIST)
		{
			if (Artist.index() == 0)
				Artist.emplace<1>();
			auto& rs = std::get<1>(Artist);
			if (rs.IsEmpty())
				rs = rs_;
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
			std::get<0>(Artist).emplace_back(rs_);
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

	void AppendComment(const CRefStrW& rs_)
	{
		if (uFlag & MIF_JOIN_COMMENT)
		{
			if (Comment.index() == 0)
				Comment.emplace<1>();
			auto& rs = std::get<1>(Comment);
			if (rs.IsEmpty())
				rs = rs_;
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
			std::get<0>(Comment).emplace_back(rs_);
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

struct APE_Header
{
	CHAR byPreamble[8];
	DWORD dwVer;
	DWORD cItems;
	DWORD dwFlags;
	CHAR byReserved[8];
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
	ID3V24FF_TAG_ALTER_DISCARD = 1u << 6,      // 标签修改后应丢弃
	ID3V24FF_FILE_ALTER_DISCARD = 1u << 5,     // 文件修改后应丢弃
	ID3V24FF_READ_ONLY = 1u << 4,              // 只读
	// ----格式----
	ID3V24FF_HAS_GROUP_IDENTITY = 1u << 6,     // 含组标志（1B）
	ID3V24FF_COMPRESSION = 1u << 3,            // 已压缩（zlib）
	ID3V24FF_ENCRYPTION = 1u << 2,             // 已加密（1B，指示加密方式）
	ID3V24FF_UNSYNCHRONIZATION = 1u << 1,      // 不同步
	ID3V24FF_HAS_DATA_LENGTH_INDICATOR = 1u << 0,    // 含长度指示（4B，同步安全整数）

	// ----状态----
	ID3V23FF_TAG_ALTER_DISCARD = 1u << 7,      // 标签修改后应丢弃
	ID3V23FF_FILE_ALTER_DISCARD = 1u << 6,     // 文件修改后应丢弃
	ID3V23FF_READ_ONLY = 1u << 5,              // 只读
	// ----格式----
	ID3V23FF_HAS_GROUP_IDENTITY = 1u << 7,     // 含组标志（1B）
	ID3V23FF_COMPRESSION = 1u << 6,            // 已压缩（zlib）
	ID3V23FF_ENCRYPTION = 1u << 5,             // 已加密（1B，指示加密方式）
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

struct ID3LOCATION
{
	SIZE_T posV2{ SIZETMax };
	SIZE_T posV2Footer{ SIZETMax };
	SIZE_T posV2FooterHdr{ SIZETMax };
	SIZE_T posV1{ SIZETMax };
	SIZE_T posV1Ext{ SIZETMax };
	SIZE_T posApe{ SIZETMax };
	SIZE_T posFlac{ SIZETMax };
};

class CMediaFile
{
	friend class CID3v1;
	friend class CID3v2;
	friend class CFlac;
	friend class CMpegInfo;
private:
	IStream* m_pStream{};
	UINT m_uTagType{};

	ID3LOCATION m_Id3Loc{};

	UINT DetectID3(ID3LOCATION& Id3Loc)
	{
		UINT uRet{};
		BYTE by[16];
		auto fnIsLegalHdr = [](const ID3v2_Header& hdr)->BOOL
			{
				return hdr.Ver < 0xFF && hdr.Revision < 0xFF &&
					(hdr.Flags & 0b1111) == 0 &&
					hdr.Size[0] < 0x80 && hdr.Size[1] < 0x80 && hdr.Size[2] < 0x80 && hdr.Size[3] < 0x80 &&
					SynchSafeIntToDWORD(hdr.Size) != 0;
			};
		CStreamWalker w(m_pStream);
		const auto cbSize = w.GetSize();
		// 查找ID3v1
		if (cbSize > 128u)
		{
			w.GetStream()->Seek(ToLi(-128), STREAM_SEEK_END, NULL);
			w.Read(by, 3);
			if (memcmp(by, "TAG", 3) == 0)
			{
				Id3Loc.posV1 = w.GetPos() - 3u;
				uRet |= TAG_ID3V1;
			}
			if (cbSize > 128u + 227u)
			{
				w.GetStream()->Seek(ToLi(-(128 + 227)), STREAM_SEEK_END, NULL);
				w.Read(by, 4);
				if (memcmp(by, "TAG+", 4) == 0)
				{
					Id3Loc.posV1Ext = w.GetPos() - 4u;
					uRet |= TAG_ID3V1Ext;
				}
			}
		}
		// 查找APE
		SIZE_T cbID3v1{};
		if (Id3Loc.posV1Ext != SIZETMax)
			cbID3v1 = 227u + 128u;
		else if (Id3Loc.posV1 != SIZETMax)
			cbID3v1 = 128u;
		if (cbSize > cbID3v1 + 32u)
		{
			w.GetStream()->Seek(ToLi(-SSIZE_T(cbID3v1 + 32u)), STREAM_SEEK_END, NULL);
			APE_Header Hdr;
			w >> Hdr;
			if (memcmp(Hdr.byPreamble, "APETAGEX", 8) == 0 &&
				(Hdr.dwVer == 1000u || Hdr.dwVer == 2000u) &&
				(Hdr.dwFlags & 0b0001'1111'1111'1111'1111'1111'1111'1000u) == 0 &&
				*(ULONGLONG*)Hdr.byReserved == 0ull)
			{
				Id3Loc.posApe = cbSize - (cbID3v1 + 32u);
			}
		}
		// 查找ID3v2
		ID3v2_Header hdr;
		if (cbSize > 10u)
		{
			w.MoveToBegin() >> hdr;
			if (memcmp(hdr.Header, "ID3", 3u) == 0 &&
				fnIsLegalHdr(hdr))
			{
				// 若已找到标签头，则使用其内部的SEEK帧来寻找尾部标签，因此此处不需要继续查找标签尾
				Id3Loc.posV2 = 0u;
				if (hdr.Ver == 3)
					uRet |= TAG_ID3V2_3;
				else if (hdr.Ver == 4)
					uRet |= TAG_ID3V2_4;
			}
			else
			{
				// 若未找到标签头，则应从尾部扫描，检查是否有追加标签
				if (Id3Loc.posV1Ext != SIZETMax)
				{
					if (cbSize > 128u + 227u + 10u)
					{
						w.MoveTo(Id3Loc.posV1Ext - 10) >> hdr;
						if (memcmp(hdr.Header, "3DI", 3u) == 0 &&
							fnIsLegalHdr(hdr))
						{
							SIZE_T cbFrames = SynchSafeIntToDWORD(hdr.Size);
							if (cbSize >= 128u + 227u + 10u + cbFrames)
							{
								Id3Loc.posV2FooterHdr = (SIZE_T)w.GetPos() - 10u;
								Id3Loc.posV2Footer = Id3Loc.posV2FooterHdr - cbFrames;
								if (hdr.Ver == 3)
									uRet |= TAG_ID3V2_3;
								else if (hdr.Ver == 4)
									uRet |= TAG_ID3V2_4;
							}
						}
					}
				}
				else if (Id3Loc.posV1 != SIZETMax)
				{
					if (cbSize > 128u + 10u)
					{
						w.MoveTo(Id3Loc.posV1 - 10) >> hdr;
						if (memcmp(hdr.Header, "3DI", 3u) == 0 &&
							fnIsLegalHdr(hdr))
						{
							SIZE_T cbFrames = SynchSafeIntToDWORD(hdr.Size);
							if (cbSize >= 128u + 10u + cbFrames)
							{
								Id3Loc.posV2FooterHdr = (SIZE_T)w.GetPos() - 10u;
								Id3Loc.posV2Footer = Id3Loc.posV2FooterHdr - cbFrames;
								if (hdr.Ver == 3)
									uRet |= TAG_ID3V2_3;
								else if (hdr.Ver == 4)
									uRet |= TAG_ID3V2_4;
							}
						}
					}
				}
				else
				{
					w.GetStream()->Seek(ToLi(-10), STREAM_SEEK_END, NULL);
					w >> hdr;
					if (memcmp(hdr.Header, "3DI", 3u) == 0 &&
						fnIsLegalHdr(hdr))
					{
						SIZE_T cbFrames = SynchSafeIntToDWORD(hdr.Size);
						if (cbSize >= 10u + cbFrames)
						{
							Id3Loc.posV2FooterHdr = (SIZE_T)w.GetPos() - 10u;
							Id3Loc.posV2Footer = Id3Loc.posV2FooterHdr - cbFrames;
							if (hdr.Ver == 3)
								uRet |= TAG_ID3V2_3;
							else if (hdr.Ver == 4)
								uRet |= TAG_ID3V2_4;
						}
					}
				}
			}
		}
		return uRet;
	}

	BOOL DetectFlac()
	{
		CStreamWalker w(m_pStream);
		if (m_Id3Loc.posV2 == SIZETMax)
			w.MoveToBegin();
		else
			w.MoveTo(m_Id3Loc.posV2);
		BYTE by[4];
		w >> by;
		if (memcmp(by, "fLaC", 4) == 0)
		{
			m_Id3Loc.posFlac = w.GetPos() - 4u;
			return TRUE;
		}
		else
		{
			m_Id3Loc.posFlac = SIZETMax;
			return FALSE;
		}
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
		m_uTagType |= DetectID3(m_Id3Loc);
		if (DetectFlac())
			m_uTagType |= TAG_FLAC;
		return m_uTagType;
	}
};

class CTag
{
protected:
	CMediaFile& m_File;
	CStreamWalker m_Stream{};
public:
	CTag(CMediaFile& File) :m_File{ File }, m_Stream(File.GetStream())
	{
		m_Stream.GetStream()->AddRef();
	}

	~CTag()
	{
		m_Stream.GetStream()->Release();
	}

	ECK_DISABLE_COPY_MOVE(CTag)

	virtual Result SimpleExtract(MUSICINFO& mi) = 0;
	virtual Result SimpleExtractMove(MUSICINFO& mi)
	{
		return SimpleExtract(mi);
	}
	virtual Result ReadTag(UINT uFlags) = 0;
	virtual Result WriteTag(UINT uFlags) = 0;
};
ECK_MEDIATAG_NAMESPACE_END
ECK_NAMESPACE_END