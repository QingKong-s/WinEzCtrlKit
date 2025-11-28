#pragma once
#include "CRefStr.h"
#include "CRefBin.h"
#include "CStreamWalker.h"
#include "CMemWalker.h"
#include "CBitSet.h"
#include "Utility2.h"
#include "AutoPtrDef.h"
#include "ComPtr.h"
#include "StringConvert.h"

#include <variant>

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
	TAG_APE = 1u << 6,
};
// 元数据类型
enum MIMASKS :UINT
{
	MIM_NONE = 0u,
	MIM_TITLE = 1u << 0,	// 标题
	MIM_ARTIST = 1u << 1,	// 艺术家
	MIM_ALBUM = 1u << 2,	// 专辑
	MIM_COMMENT = 1u << 3,	// 备注
	MIM_LRC = 1u << 4,		// 歌词
	MIM_COVER = 1u << 5,	// 封面
	MIM_GENRE = 1u << 6,	// 风格
	MIM_DISC = 1u << 8,		// 碟片号
	MIM_TRACK = 1u << 9,	// 音轨号

	MIM_ALL = 0xFFFFFFFF
};
ECK_ENUM_BIT_FLAGS(MIMASKS);

// 标签读写标志
enum MIFLAGS :UINT
{
	//												| ID3v1	| ID3v2 | Flac	|  APE	|
	// 在Vorbis注释的TRACK或TRACKNUMBER中写入斜杠"/"加总音轨数
	MIF_WRITE_TRACK_TOTAL = 1u << 0,			// 	|	 	|	 	|	T	|	 	|
	// 在Vorbis注释的DISCNUMBER中写入总碟片数
	MIF_WRITE_DISC_TOTAL = 1u << 1,				// 	|	 	|	 	|	T	|	 	|
	// 允许保留空白填充
	MIF_ALLOW_PADDING = 1u << 2,				// 	|		|	T	|	T	|		|
	// 写入APE时，若图片类型为无效，则假定为"封面"
	MIF_APE_INVALID_COVER_AS_FRONT = 1u << 3,	// 	|		|		|	 	|	T	|
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
	// 写入图片到Vorbis注释而不是图片块
	MIF_WRITE_METADATA_BLOCK_PICTURE = 1u << 12,//	|		|		|	T	|		|
	// 创建ID3v2扩展头
	MIF_CREATE_ID3V2_EXT_HEADER = 1u << 13,		// 	|		|	T	|		|		|
};
ECK_ENUM_BIT_FLAGS(MIFLAGS);

// 标签单元写入选项
enum MIIWFLAG :BYTE
{
	MIIWF_PREPEND_ID3V2_4 = 0,			// 将该帧置于文件头部的标签，这是默认值
	MIIWF_APPEND_ID3V2_4 = 1u << 0,		// 将该帧置于文件尾部的标签
	MIIWF_WRITE_METADATA_BLOCK_PICTURE = 1u << 1,	// 将该图片写入为Vorbis注释中的METADATA_BLOCK_PICTURE
};
ECK_ENUM_BIT_FLAGS(MIIWFLAG);

// 错误码
enum class Result
{
	Ok,
	TagErr,				// 标签识别出错
	IllegalEnum_TextEncoding,	// 非法文本编码值
	TooLargeData,		// 某数据长度超出限制
	InvalidEnumVal,		// 枚举值无效
	LenErr,				// 长度字段无效（如过短）
	InvalidVal,			// 值无效
	IllegalRepeat,		// 非法重复
	EmptyData,			// 某数据为空
	ReservedDataErr,	// 保留部分或未定义部分填入错误信息
	NoTag,				// 文件中无标签或标签还未被读入
	MpegSyncFailed,		// MPEG同步失败
	NotSupport,			// 不支持请求的操作
	OutOfMemory,		// 内存不足
	FileAccessDenied,	// 无法访问文件
};
// 图片类型
enum class PicType :BYTE
{
	Invalid = 0xFF,     // 任何无效的值
	PrivBegin = 0,      // ！起始占位
	Other = PrivBegin,  // 其他
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
	PrivEnd             // ！终止占位
};

constexpr inline std::string_view ApePicType[]
{
	"Other"sv,
	"Icon"sv,
	"Other File Icon"sv,
	"Front"sv,
	"Back"sv,
	"Leaflet"sv,
	"Media"sv,
	"Lead Artist"sv,
	"Artist"sv,
	"Conductor"sv,
	"Band"sv,
	"Composer"sv,
	"Lyricist"sv,
	"Recording Location"sv,
	"During Recording"sv,
	"During Performance"sv,
	"Video Capture"sv,
	"A Bright Colored Fish"sv,
	"Illustration"sv,
	"Band Logotype"sv,
	"Publisher Logotype"sv,
	"Invalid"sv,
};

constexpr inline  std::wstring_view ZhPicType[]
{
	L"其他"sv,
	L"32×32文件图标"sv,
	L"其他图标"sv,
	L"封面"sv,
	L"封底"sv,
	L"宣传图"sv,
	L"实体媒介照片"sv,
	L"艺术家照片"sv,
	L"演唱者照片"sv,
	L"指挥者照片"sv,
	L"乐队/剧团照片"sv,
	L"作曲家照片"sv,
	L"作词者照片"sv,
	L"录音场地照片"sv,
	L"录音过程照片"sv,
	L"表演过程照片"sv,
	L"视频截图"sv,
	L"艳鱼图"sv,
	L"插画"sv,
	L"艺术家/艺术团队Logo"sv,
	L"发行商/工作室Logo"sv,
	L"无效"sv,
};

static_assert(ARRAYSIZE(ApePicType) == ARRAYSIZE(ZhPicType));
static_assert(ARRAYSIZE(ApePicType) == (size_t)PicType::PrivEnd + 1);

EckInlineNdCe auto PicTypeToString(PicType e)
{
	if (e >= PicType::PrivEnd)
		return ZhPicType[ARRAYSIZE(ZhPicType) - 1];
	return ZhPicType[(size_t)e];
}
EckInlineNdCe auto PicTypeToApeString(PicType e)
{
	if (e >= PicType::PrivEnd)
		return ApePicType[ARRAYSIZE(ApePicType) - 1];
	return ApePicType[(size_t)e];
}


struct StrList
{
	struct Iterator
	{
		PCWSTR p;

		Iterator operator++(int)
		{
			const auto r{ *this };
			p += (*p + 2);
			if (!*p) p = nullptr;
			return r;
		}

		Iterator& operator++()
		{
			p += (*p + 2);
			if (!*p) p = nullptr;
			return *this;
		}

		std::wstring_view operator*() const
		{
			return { p + 1,(size_t)*p };
		}

		bool operator==(const Iterator& x) const { return p == x.p; }
	};

	eck::CRefStrW Str{};

	EckInline void Clear() { Str.Clear(); }

	EckInlineNd PCWSTR FrontData() const
	{
		if (Str.IsEmpty()) return nullptr;
		return Str.Data() + 1;
	}
	EckInlineNd PWSTR FrontData()
	{
		if (Str.IsEmpty()) return nullptr;
		return Str.Data() + 1;
	}

	void PushBackStringView(std::wstring_view svText, std::wstring_view svDiv)
	{
		EckAssert(svText.size() <= 0xFFFF);
		WCHAR cchText;
		if (svText.empty())
			return;
		cchText = (WCHAR)svText.size();
		if (svDiv.empty())
		{
			Str.PushBackChar(cchText);
			Str.PushBack(svText.data(), cchText);
			Str.PushBackChar(L'\0');
		}
		else
		{
			if (Str.IsEmpty())
				Str.PushBackChar(L'\0');// 长度
			else
				Str.PushBack(svDiv);
			Str.PushBack(svText.data(), cchText);
			Str[0] = WCHAR(Str.Size() - 1);
		}
	}

	void PushBackString(const CRefStrW& rs, std::wstring_view svDiv)
	{
		PushBackStringView(rs.ToStringView(), svDiv);
	}

	Iterator begin() const { return { Str.Data() }; }
	Iterator end() const { return {}; }
};

struct MUSICPIC
{
	PicType eType;
	BOOL bLink;
	CRefStrW rsDesc;
	CRefStrA rsMime;
	std::variant<CRefBin, CRefStrW> varPic;

	EckInlineNdCe auto& GetPicPath() { return std::get<CRefStrW>(varPic); }
	EckInlineNdCe auto& GetPicPath() const { return std::get<CRefStrW>(varPic); }
	EckInlineNdCe auto& GetPicData() { return std::get<CRefBin>(varPic); }
	EckInlineNdCe auto& GetPicData() const { return std::get<CRefBin>(varPic); }

	HRESULT CreateStream(_Out_ IStream*& pStream) const
	{
		if (bLink)
		{
			return SHCreateStreamOnFileEx(GetPicPath().Data(),
				STGM_READ, 0, FALSE, nullptr, &pStream);
		}
		else
		{
			pStream = new eck::CStreamView{ GetPicData() };
			return S_OK;
		}
	}
};

struct MUSICINFO
{
	MIMASKS uMask{ MIM_ALL };	// 指定欲读取信息类型的掩码
	MIMASKS uMaskChecked{};		// 函数返回后设置已读取的信息
	MIFLAGS uFlag{};			// 控制读写操作的标志

	CRefStrW rsTitle{}; // 标题
	StrList slArtist{};	// 艺术家
	CRefStrW rsAlbum{}; // 专辑
	StrList slComment{};// 备注
	CRefStrW rsLrc{};	// 歌词
	CRefStrW rsGenre{};	// 流派
	std::vector<MUSICPIC> vImage{};// 图片
	int nTrack{};		// 音轨号
	int cTotalTrack{};// 总音轨数
	int nDisc{};		// 碟片号
	int cTotalDisc{};	// 总碟片数

	// 取主封面。
	// 函数遍历图片列表，然后按照 封面 > 封底 > 第一幅图片
	// 的优先级顺序返回指定的图片，若失败则返回NULL
	const MUSICPIC* GetMainCover() const
	{
		if (vImage.empty())
			return nullptr;
		const MUSICPIC* pFront{}, * pBack{};
		for (const auto& e : vImage)
		{
			if (e.eType == PicType::CoverFront)
				pFront = &e;
			else if (e.eType == PicType::CoverBack)
				pBack = &e;
		}
		if (pFront) return pFront;
		if (pBack) return pBack;
		return vImage.data();
	}

	void Clear()
	{
		uMaskChecked = MIM_NONE;
		rsTitle.Clear();
		slArtist.Clear();
		rsAlbum.Clear();
		slComment.Clear();
		rsLrc.Clear();
		rsGenre.Clear();
		vImage.clear();
	}
};

enum : UINT
{
	SMOF_NONE = 0,
	SMOF_MOVE = 1 << 0,	// 允许使用移动操作避免复制
	SMOF_SET = 1 << 1,	// 将MUSICINFO数据设置到CTag中，若无此标志则相反
};

struct SIMPLE_OPT
{
	UINT uFlags{};
	// 艺术家连接符，若为空则不连接
	std::wstring_view svArtistDiv{ L"、"sv };
	// 备注连接符，若为空则不连接
	std::wstring_view svCommDiv{ L"\n"sv };
};


struct ID3v2_Header		// ID3v2标签头
{
	CHAR Header[3];		// "ID3"
	BYTE Ver;			// 版本号
	BYTE Revision;		// 副版本号
	BYTE Flags;			// 标志
	BYTE Size[4];		// 标签大小
};

struct ID3v2_FrameHeader// ID3v2帧头
{
	CHAR ID[4];			// 帧标识
	BYTE Size[4];		// 帧内容大小
	BYTE Flags[2];		// 标志
};

struct FLAC_BlockHeader // Flac头
{
	BYTE eType;
	BYTE bySize[3];
};

struct APE_Header
{
	CHAR byPreamble[8];
	DWORD dwVer;
	DWORD cbBody;
	DWORD cItems;
	DWORD dwFlags;
	CHAR byReserved[8];
};

static_assert(alignof(ID3v2_Header) == 1);
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

enum :UINT
{
	APE_READ_ONLY = 1u << 0,
	APE_HEADER = 1u << 29,
	APE_HAS_FOOTER = 1u << 30,
	APE_HAS_HEADER = 1u << 31,
};

// 从p处读取4字节并转换为32位小端整数
EckInlineNdCe DWORD ThSyncSafeIntToDWORD(_In_reads_bytes_(4) PCBYTE p)
{
	return ((p[0] & 0x7F) << 21) | ((p[1] & 0x7F) << 14) | ((p[2] & 0x7F) << 7) | (p[3] & 0x7F);
}

// 将32位小端整数dw转为同步安全整数，并写入p处
EckInlineNdCe void ThDwordToSyncSafeInt(_Out_writes_bytes_(4) BYTE* p, DWORD dw)
{
	p[3] = (dw) & 0b0111'1111;
	p[2] = (dw >> 7) & 0b0111'1111;
	p[1] = (dw >> 14) & 0b0111'1111;
	p[0] = (dw >> 21) & 0b0111'1111;
}

[[nodiscard]] inline constexpr BOOL ThIsLegalID3v2Header(const ID3v2_Header& hdr)
{
	return hdr.Ver < 0xFF && hdr.Revision < 0xFF &&
		(hdr.Flags & 0b1111) == 0 &&
		hdr.Size[0] < 0x80 && hdr.Size[1] < 0x80 && hdr.Size[2] < 0x80 && hdr.Size[3] < 0x80 &&
		ThSyncSafeIntToDWORD(hdr.Size) != 0;
}

[[nodiscard]] inline BOOL ThIsLegalApeHeader(const APE_Header& hdr)
{
	return memcmp(hdr.byPreamble, "APETAGEX", 8) == 0 &&
		(hdr.dwVer == 1000u || hdr.dwVer == 2000u) &&
		(hdr.dwFlags & 0b0001'1111'1111'1111'1111'1111'1111'1000u) == 0 &&
		*(ULONGLONG*)hdr.byReserved == 0ull;
}

inline void ThGetSetNumAndTotalNum(BOOL bSet, BOOL bWriteSlash,
	int& nNum, int& nTotal, CRefStrW& rsValue)
{
	if (bSet)
		if (nTotal && bWriteSlash)
			rsValue.Format(L"%d/%d", nNum, nTotal);
		else
			rsValue.Format(L"%d", nNum);
	else
	{
		PWCH pEnd;
		TcsToInt(rsValue.Data(), rsValue.Size(), nNum, 10, &pEnd);
		const int posSlash = rsValue.FindChar(L'/', int(pEnd - rsValue.Data()));
		if (posSlash != StrNPos)
			TcsToInt(rsValue.Data() + (posSlash + 1),
				rsValue.Size() - (posSlash + 1), nTotal);
		else
			nTotal = 0;
	}
}


class CMediaFile final
{
	friend class CID3v1;
	friend class CID3v2;
	friend class CFlac;
	friend class CMpegInfo;
	friend class CApe;
private:
	ComPtr<IStream> m_pStream{};
	UINT m_uTagType{};

	struct TAG_LOCATION
	{
		SIZE_T posV2{ SIZETMax };		// ID3v2标签头的位置
		SIZE_T posV2Footer{ SIZETMax };		// ID3v2标签尾内容的位置
		SIZE_T posV2FooterHdr{ SIZETMax };	// ID3v2标签尾结构位置
		SIZE_T cbID3v2{};				// 若无标签头则为0，若有标签头则与头中的Size字段相同，不处理预置/追加组合的判长

		SIZE_T posV1{ SIZETMax };		// ID3v1位置
		SIZE_T posV1Ext{ SIZETMax };	// 扩展ID3v1位置

		SIZE_T posApeHdr{ SIZETMax };	// APE标签头/尾结构的位置
		SIZE_T posApe{ SIZETMax };		// APE标签内容开始位置
		SIZE_T posApeTag{ SIZETMax };	// APE标签开始位置
		SIZE_T cbApeTag{};				// 整个标签的大小
		BOOL bPrependApe{};				// 是否为预置APE标签

		SIZE_T posFlac{ SIZETMax };		// Flac标签头位置
	}  m_Loc{};

	UINT DetectID3_APE()
	{
		m_Loc = {};

		UINT uRet{};
		BYTE by[16];

		CStreamWalker w{ m_pStream.Get() };
		const auto cbSize = w.GetSize();
		// 查找ID3v1
		if (cbSize > 128u)
		{
			w->Seek(ToLi(-128), STREAM_SEEK_END, nullptr);
			w.Read(by, 3);
			if (memcmp(by, "TAG", 3) == 0)
			{
				m_Loc.posV1 = w.GetPos() - 3u;
				uRet |= TAG_ID3V1;
			}
			if (cbSize > 128u + 227u)
			{
				w->Seek(ToLi(-(128 + 227)), STREAM_SEEK_END, nullptr);
				w.Read(by, 4);
				if (memcmp(by, "TAG+", 4) == 0)
				{
					m_Loc.posV1Ext = w.GetPos() - 4u;
					uRet |= TAG_ID3V1Ext;
				}
			}
		}
		// 查找APE
		SIZE_T cbID3v1{};
		if (m_Loc.posV1Ext != SIZETMax)
			cbID3v1 = 227u + 128u;
		else if (m_Loc.posV1 != SIZETMax)
			cbID3v1 = 128u;
		if (cbSize > cbID3v1 + 32u)
		{
			w->Seek(ToLi(-SSIZE_T(cbID3v1 + 32u)), STREAM_SEEK_END, nullptr);
			APE_Header Hdr;
			w >> Hdr;
			if (ThIsLegalApeHeader(Hdr) && !(Hdr.dwFlags & APE_HEADER))
			{
				m_Loc.posApeHdr = cbSize - (cbID3v1 + 32u);
				m_Loc.posApe = m_Loc.posApeHdr + 32u - Hdr.cbBody;
				m_Loc.cbApeTag = Hdr.cbBody;
				if (Hdr.dwFlags & APE_HAS_HEADER)
				{
					m_Loc.posApeTag = m_Loc.posApe - 32u;
					m_Loc.cbApeTag += 32u;
				}
				else
					m_Loc.posApeTag = m_Loc.posApe;
				uRet |= TAG_APE;
				m_Loc.bPrependApe = FALSE;
			}
			else
			{
				w.MoveToBegin() >> Hdr;
				if (ThIsLegalApeHeader(Hdr) && (Hdr.dwFlags & APE_HEADER))
				{
					m_Loc.posApeHdr = 0u;
					m_Loc.posApe = 32u;
					m_Loc.posApeTag = m_Loc.posApeHdr;
					m_Loc.cbApeTag = Hdr.cbBody + 32u;
					uRet |= TAG_APE;
					m_Loc.bPrependApe = TRUE;
				}
			}
		}

		// 查找ID3v2
		if (cbSize > 10u)
		{
			ID3v2_Header hdr;
			w.MoveToBegin() >> hdr;
			if (memcmp(hdr.Header, "ID3", 3u) == 0 && ThIsLegalID3v2Header(hdr))
			{
				// 若已找到标签头，则使用其内部的SEEK帧来寻找尾部标签，因此此处不需要继续查找标签尾
				m_Loc.cbID3v2 = ThSyncSafeIntToDWORD(hdr.Size);
				m_Loc.posV2 = 0u;
				if (hdr.Ver == 3)
					uRet |= TAG_ID3V2_3;
				else if (hdr.Ver == 4)
					uRet |= TAG_ID3V2_4;
			}
			else
			{
				// TODO:检查ape前后
				{
					SIZE_T cbFrames{};
					// 若未找到标签头，则应从尾部扫描，检查是否有追加标签
					if (m_Loc.posV1Ext != SIZETMax)
					{
						if (cbSize > 128u + 227u + 10u)
						{
							w.MoveTo(m_Loc.posV1Ext - 10) >> hdr;
							if (memcmp(hdr.Header, "3DI", 3u) == 0 && ThIsLegalID3v2Header(hdr))
							{
								cbFrames = ThSyncSafeIntToDWORD(hdr.Size);
								if (cbSize >= 128u + 227u + 10u + cbFrames)
								{
									m_Loc.posV2FooterHdr = (SIZE_T)w.GetPos() - 10u;
									m_Loc.posV2Footer = m_Loc.posV2FooterHdr - cbFrames;
									if (hdr.Ver == 3)
										uRet |= TAG_ID3V2_3;
									else if (hdr.Ver == 4)
										uRet |= TAG_ID3V2_4;
								}
							}
						}
					}
					else if (m_Loc.posV1 != SIZETMax)
					{
						if (cbSize > 128u + 10u)
						{
							w.MoveTo(m_Loc.posV1 - 10) >> hdr;
							if (memcmp(hdr.Header, "3DI", 3u) == 0 && ThIsLegalID3v2Header(hdr))
							{
								cbFrames = ThSyncSafeIntToDWORD(hdr.Size);
								if (cbSize >= 128u + 10u + cbFrames)
								{
									m_Loc.posV2FooterHdr = (SIZE_T)w.GetPos() - 10u;
									m_Loc.posV2Footer = m_Loc.posV2FooterHdr - cbFrames;
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
						w->Seek(ToLi(-10), STREAM_SEEK_END, nullptr);
						w >> hdr;
						if (memcmp(hdr.Header, "3DI", 3u) == 0 && ThIsLegalID3v2Header(hdr))
						{
							cbFrames = ThSyncSafeIntToDWORD(hdr.Size);
							if (cbSize >= 10u + cbFrames)
							{
								m_Loc.posV2FooterHdr = (SIZE_T)w.GetPos() - 10u;
								m_Loc.posV2Footer = m_Loc.posV2FooterHdr - cbFrames;
								if (hdr.Ver == 3)
									uRet |= TAG_ID3V2_3;
								else if (hdr.Ver == 4)
									uRet |= TAG_ID3V2_4;
							}
						}
					}

					m_Loc.cbID3v2 = cbFrames;
				}
			}

			if (m_Loc.cbID3v2 && m_Loc.posApe == SIZETMax)
			{
				APE_Header Hdr{};
				if (m_Loc.posV2 != SIZETMax)// 检查预置ID3v2后面
				{
					w.MoveTo(m_Loc.posV2 + m_Loc.cbID3v2 + 10u) >> Hdr;
					if (ThIsLegalApeHeader(Hdr) && (Hdr.dwFlags & APE_HEADER))
					{
						m_Loc.posApeHdr = m_Loc.posV2 + m_Loc.cbID3v2 + 10u;
						m_Loc.posApe = m_Loc.posApe + 32u;
						m_Loc.posApeTag = m_Loc.posApeHdr;
						m_Loc.cbApeTag = Hdr.cbBody + 32u;
						uRet |= TAG_APE;
						m_Loc.bPrependApe = TRUE;
					}
				}
				else if (m_Loc.posV2Footer != SIZETMax &&
					m_Loc.posV2Footer >= m_Loc.cbID3v2 + 32u)// 检查追加ID3v2前面
				{
					w.MoveTo(m_Loc.posV2Footer - m_Loc.cbID3v2 - 32u) >> Hdr;
					if (ThIsLegalApeHeader(Hdr) && !(Hdr.dwFlags & APE_HEADER))
					{
						m_Loc.posApeHdr = m_Loc.posV2Footer - m_Loc.cbID3v2 - 32u;
						m_Loc.posApe = m_Loc.posApeHdr + 32u - Hdr.cbBody;
						m_Loc.cbApeTag = Hdr.cbBody;
						if (Hdr.dwFlags & APE_HAS_HEADER)
						{
							m_Loc.posApeTag = m_Loc.posApe - 32u;
							m_Loc.cbApeTag += 32u;
						}
						else
							m_Loc.posApeTag = m_Loc.posApe;
						uRet |= TAG_APE;
						m_Loc.bPrependApe = FALSE;
					}
					else
					{
						ID3v2_Header Id3Hdr{};
						w.MoveTo(m_Loc.posV2Footer - m_Loc.cbID3v2 - 10u) >> Id3Hdr;
						if (ThIsLegalID3v2Header(Id3Hdr))
						{
							w.MoveTo(m_Loc.posV2Footer - m_Loc.cbID3v2 - 32u - 10u) >> Hdr;
							if (ThIsLegalApeHeader(Hdr) && !(Hdr.dwFlags & APE_HEADER))
							{
								m_Loc.posApeHdr = m_Loc.posV2Footer - m_Loc.cbID3v2 - 32u - 10u;
								m_Loc.posApe = m_Loc.posApeHdr + 32u - Hdr.cbBody;
								m_Loc.cbApeTag = Hdr.cbBody;
								if (Hdr.dwFlags & APE_HAS_HEADER)
								{
									m_Loc.posApeTag = m_Loc.posApe - 32u;
									m_Loc.cbApeTag += 32u;
								}
								else
									m_Loc.posApeTag = m_Loc.posApe;
								uRet |= TAG_APE;
								m_Loc.bPrependApe = FALSE;
							}
						}
					}
				}
			}
		}
		return uRet;
	}

	BOOL DetectFlac()
	{
		CStreamWalker w{ m_pStream.Get() };
		if (m_Loc.posV2 == SIZETMax)
			w.MoveToBegin();
		else
			w.MoveTo(m_Loc.posV2 + m_Loc.cbID3v2 + 10);
		BYTE by[4];
		w >> by;
		// XXX: 若失配则向后查找
		if (memcmp(by, "fLaC", 4) == 0)
		{
			m_Loc.posFlac = w.GetPos() - 4u;
			return TRUE;
		}
		else
		{
			m_Loc.posFlac = SIZETMax;
			return FALSE;
		}
	}
public:
	CMediaFile() = default;
	CMediaFile(IStream* pStream) : m_pStream{ pStream } { DetectTag(); }

	CMediaFile(PCWSTR pszFile, DWORD grfMode = STGM_READ,
		DWORD dwAttr = FILE_ATTRIBUTE_NORMAL, BOOL bCreate = FALSE)
	{
		SHCreateStreamOnFileEx(pszFile, grfMode, dwAttr,
			bCreate, nullptr, &m_pStream);
		DetectTag();
	}

	EckInline void SetStream(IStream* pStream)
	{
		m_pStream = pStream;
		DetectTag();
	}
	EckInlineNdCe IStream* GetStream() const { return m_pStream.Get(); }

	EckInlineNdCe UINT GetTagType() const { return m_uTagType; }
	EckInlineNdCe BOOL IsValid() const { return !!GetStream(); }

	UINT DetectTag()
	{
		m_Loc = {};
		m_uTagType = 0u;
		if (!IsValid())
			return 0u;
		m_uTagType |= DetectID3_APE();
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
	ECK_DISABLE_COPY_MOVE(CTag);
	CTag(CMediaFile& mf) :m_File{ mf }, m_Stream{ mf.GetStream() } { m_Stream->AddRef(); }
	virtual ~CTag() { m_Stream->Release(); }

	virtual Result SimpleGetSet(MUSICINFO& mi, const SIMPLE_OPT& Opt) = 0;

	virtual Result ReadTag(UINT uFlags) = 0;
	virtual Result WriteTag(UINT uFlags) = 0;
	virtual void Reset() = 0;
};
ECK_MEDIATAG_NAMESPACE_END
ECK_NAMESPACE_END