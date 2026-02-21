#pragma once
#include "CString.h"
#include "CByteBuffer.h"
#include "CStreamWalker.h"
#include "MemWalker.h"
#include "CBitSet.h"
#include "Utility2.h"
#include "AutoPtrDef.h"
#include "ComPtr.h"
#include "StringConvert.h"
#include "CStreamView.h"

#include <variant>

ECK_NAMESPACE_BEGIN
ECK_MEDIATAG_NAMESPACE_BEGIN
// 标签类型
enum : UINT
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
    MIM_GENRE = 1u << 6,	// 流派
    MIM_TRACK = 1u << 7,	// 音轨号

    MIM_ALL = 0xFFFFFFFF

    /*
    NOTE 251223
    碟片号解析比较复杂，同时这也是个使用频率比较低的字段，
    因此暂时移除碟片号的快读写支持。
    APE     DISC/DISCNUMBER/MEDIA，其中MEDIA还包含了媒体来源
    ID3v2   TPOS，也有使用TXXX{MEDIA}的情况
    Vorbis  DISC/DISCNUMBER/DICSTOTAL/TOTALDISCS
    */
};
ECK_ENUM_BIT_FLAGS(MIMASKS);

// 标签读写标志
enum MIFLAGS : UINT
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
    MIF_PREPEND_TAG = 1u << 14,					// 	|		|	T	|		|	T	|
    MIM_DEFPOS_TAG = 1u << 15,					// 	|		|	T	|		|	T	|
};
ECK_ENUM_BIT_FLAGS(MIFLAGS);

// 标签单元写入选项
enum MIITEMFLAGS : BYTE
{
    MIIF_ID3V2_4_APPEND = 1u << 0,		// 将该帧置于文件尾部的标签
    // 将该图片写入为Vorbis注释中的METADATA_BLOCK_PICTURE
    MIIF_METADATA_BLOCK_PICTURE = 1u << 1,
};
ECK_ENUM_BIT_FLAGS(MIITEMFLAGS);

// 错误码
enum class Result
{
    Ok,
    Tag,				// 标签识别出错，如标签头错误等
    Enum,		        // 枚举值无效
    Enum_TextEncoding,	// 非法文本编码值
    TooLargeData,		// 某数据长度超出限制
    Length,				// 长度字段无效（过长或过短）
    Value,			    // 值无效
    IllegalRepeat,		// 非法重复
    EmptyData,			// 某数据为空
    ReservedData,	    // 保留部分或未定义部分填入错误信息
    NoTag,				// 文件中无标签或标签还未被读入
    MpegSyncFailed,		// MPEG同步失败
    NotSupport,			// 不支持请求的操作
    OutOfMemory,		// 内存不足
    FileAccessDenied,	// 无法访问文件
    Stream,             // 流错误，调用方检查流的错误码和当前位置来确定具体错误
    Broken,             // 文件已损坏
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
    ColouredFish,       // 艳鱼图
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

EckInlineNdCe auto TagPictureTypeToString(PicType e) noexcept
{
    if (e >= PicType::PrivEnd)
        return ZhPicType[ARRAYSIZE(ZhPicType) - 1];
    return ZhPicType[(size_t)e];
}
EckInlineNdCe auto TagPictureTypeToApeString(PicType e) noexcept
{
    if (e >= PicType::PrivEnd)
        return ApePicType[ARRAYSIZE(ApePicType) - 1];
    return ApePicType[(size_t)e];
}

// 长度1(2B) | 字符串1 | 长度2(2B) | 字符串2 | ... | 0(2B)
struct StrList
{
    struct Iterator
    {
        PCWSTR p;

        EckInlineCe Iterator operator++(int) noexcept
        {
            const auto r{ *this };
            p += (*p + 2/*结尾NULL + 长度*/);
            if (!*p) p = nullptr;
            return r;
        }

        EckInlineCe Iterator& operator++() noexcept
        {
            p += (*p + 2);
            if (!*p) p = nullptr;
            return *this;
        }

        EckInlineNdCe std::wstring_view operator*() const noexcept
        {
            return { p + 1,(size_t)*p };
        }

        EckInlineNdCe bool operator==(const Iterator& x) const noexcept { return p == x.p; }
    };

    eck::CStringW Str{};

    EckInlineCe void Clear() noexcept { Str.Clear(); }

    EckInlineNdCe PCWSTR FrontData() const noexcept
    {
        if (Str.IsEmpty()) return nullptr;
        return Str.Data() + 1;
    }
    EckInlineNdCe PWSTR FrontData() noexcept
    {
        if (Str.IsEmpty()) return nullptr;
        return Str.Data() + 1;
    }

    void PushBackStringView(std::wstring_view svText,
        std::wstring_view svDiv) noexcept
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
        else// 如果提供了分隔符，则视为只有一个字符串
        {
            if (Str.IsEmpty())
                Str.PushBackChar(L'\0');// 长度
            else
            {
                Str.PopBack();
                Str.PushBack(svDiv);
            }
            Str.PushBack(svText.data(), cchText);
            Str[0] = WCHAR(Str.Size() - 1);
            Str.PushBackChar(L'\0');
        }
    }

    void PushBackString(const CStringW& rs, std::wstring_view svDiv) noexcept
    {
        PushBackStringView(rs.ToStringView(), svDiv);
    }

    EckInlineNdCe Iterator begin() const noexcept { return { Str.Data() }; }
    EckInlineNdCe Iterator end() const noexcept { return {}; }
};

struct MUSICPIC
{
    PicType eType{ PicType::CoverFront };
    BOOL bLink{};
    CStringW rsDesc{};
    CStringA rsMime{};
    std::variant<CByteBuffer, CStringW> varPic{};

    EckInlineNdCe auto& GetPicturePath() noexcept { return std::get<CStringW>(varPic); }
    EckInlineNdCe auto& GetPicturePath() const noexcept { return std::get<CStringW>(varPic); }
    EckInlineNdCe auto& GetPictureData() noexcept { return std::get<CByteBuffer>(varPic); }
    EckInlineNdCe auto& GetPictureData() const noexcept { return std::get<CByteBuffer>(varPic); }

    // 从当前内容创建图片数据字节流
    // 若bLink为假，调用方必须保证在当前实例析构前销毁返回的流
    HRESULT CreateStream(_Out_ IStream*& pStream) const noexcept
    {
        if (bLink)
        {
            return SHCreateStreamOnFileEx(GetPicturePath().Data(),
                STGM_READ, 0, FALSE, nullptr, &pStream);
        }
        else
        {
            pStream = new eck::CStreamView{ GetPictureData() };
            return S_OK;
        }
    }
};

struct MUSICINFO
{
    MIMASKS uMask{ MIM_ALL };	// 欲操作字段的掩码
    MIMASKS uMaskChecked{};		// 读操作返回后，设置已读取的信息；写操作时忽略
    MIFLAGS uFlag{};

    CStringW rsTitle{};
    StrList slArtist{};
    CStringW rsAlbum{};
    StrList slComment{};
    CStringW rsLrc{};
    CStringW rsGenre{};
    std::vector<MUSICPIC> vPic{};
    int nTrack{};
    int cTotalTrack{};

    // 取主封面。
    // 函数遍历图片列表，然后按照 封面 > 封底 > 第一幅图片
    // 的优先级顺序返回指定的图片，若失败则返回NULL
    const MUSICPIC* GetMainCover() const
    {
        if (vPic.empty())
            return nullptr;
        const MUSICPIC* pFront{}, * pBack{};
        for (const auto& e : vPic)
        {
            if (e.eType == PicType::CoverFront)
                pFront = &e;
            else if (e.eType == PicType::CoverBack)
                pBack = &e;
        }
        if (pFront) return pFront;
        if (pBack) return pBack;
        return vPic.data();
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
        vPic.clear();
    }
};

enum : UINT
{
    SMOF_NONE = 0,
    SMOF_MOVE = 1 << 0,	// 允许使用移动操作避免复制
};

struct SIMPLE_OPT
{
    UINT uFlags{};// SMOF_
    // 艺术家连接符，若为空则不连接
    std::wstring_view svArtistDiv{ L"、"sv };
    // 备注连接符，若为空则不连接
    std::wstring_view svCommDiv{ L"\n"sv };
};


struct ID3v2_HEADER
{
    CHAR Header[3];
    BYTE Ver;
    BYTE Revision;
    BYTE Flags;
    BYTE Size[4];
};
static_assert(alignof(ID3v2_HEADER) == 1);
static_assert(sizeof(ID3v2_HEADER) == 10);

struct ID3v2_FRAME_HEADER
{
    CHAR Id[4];
    BYTE Size[4];
    BYTE Flags[2];
};
static_assert(alignof(ID3v2_FRAME_HEADER) == 1);
static_assert(sizeof(ID3v2_HEADER) == 10);

// ID3v2头标志
enum : UINT
{
    ID3V2HF_UNSYNCHRONIZATION = 1u << 7,            // 不同步
    ID3V2HF_EXTENDED_HEADER = 1u << 6,              // 含扩展头
    ID3V2HF_EXPERIMENTAL = 1u << 5,                 // 实验性标签
    // ----Only ID3v2.4----
    ID3V2HF_FOOTER = 1u << 4,                       // 含页脚
};

// ID3v2扩展头标志
enum : BYTE
{
    // ----Only ID3v2.3----
    ID3V23EH_CRC_DATA = 1 << 7,     // 含CRC数据
    // ----Only ID3v2.4----
    ID3V24EH_UPDATE = 1 << 6,       // 更新标志
    ID3V24EH_CRC_DATA = 1 << 5,     // 含CRC数据
    ID3V24EH_RESTRICTIONS = 1 << 4, // 限制标签尺寸
};

// ID3v2帧标志
enum : BYTE
{
    // ----状态----
    ID3V24FF_TAG_ALTER_DISCARD = 1 << 6,    // 标签修改后应丢弃
    ID3V24FF_FILE_ALTER_DISCARD = 1 << 5,   // 文件修改后应丢弃
    ID3V24FF_READ_ONLY = 1 << 4,            // 只读
    // ----格式----
    ID3V24FF_HAS_GROUP_IDENTITY = 1 << 6,   // 含组标志（1B）
    ID3V24FF_COMPRESSION = 1 << 3,          // 已压缩（zlib）
    ID3V24FF_ENCRYPTION = 1 << 2,           // 已加密（1B，指示加密方式）
    ID3V24FF_UNSYNCHRONIZATION = 1 << 1,    // 不同步
    ID3V24FF_HAS_DATA_LENGTH_INDICATOR = 1 << 0,// 含长度指示（4B，同步安全整数）

    // ----状态----
    ID3V23FF_TAG_ALTER_DISCARD = 1u << 7,   // 标签修改后应丢弃
    ID3V23FF_FILE_ALTER_DISCARD = 1u << 6,  // 文件修改后应丢弃
    ID3V23FF_READ_ONLY = 1u << 5,           // 只读
    // ----格式----
    ID3V23FF_ENCRYPTION = 1u << 7,          // 已加密（1B，指示加密方式）
    ID3V23FF_COMPRESSION = 1u << 6,         // 已压缩（zlib）
    ID3V23FF_HAS_GROUP_IDENTITY = 1u << 5,  // 含组标志（1B）
};

// 从p处读取4字节并转换为32位小端整数
EckInlineNdCe UINT TagSyncSafeIntToUInt(_In_reads_bytes_(4) PCBYTE p) noexcept
{
    return ((p[0] & 0x7F) << 21) | ((p[1] & 0x7F) << 14) |
        ((p[2] & 0x7F) << 7) | (p[3] & 0x7F);
}
// 将32位小端整数dw转为同步安全整数，并写入p处
EckInlineNdCe void TagUIntToSyncSafeInt(_Out_writes_bytes_(4) BYTE* p, UINT dw) noexcept
{
    p[3] = (dw) & 0b0111'1111;
    p[2] = (dw >> 7) & 0b0111'1111;
    p[1] = (dw >> 14) & 0b0111'1111;
    p[0] = (dw >> 21) & 0b0111'1111;
}
EckNfInlineNdCe BOOL TagCheckID3v2Header(const ID3v2_HEADER& hdr,
    BOOL bHeaderOrFooter = TRUE) noexcept
{
    return (bHeaderOrFooter ?
            memcmp(hdr.Header, "ID3", 3) == 0 :
            memcmp(hdr.Header, "3DI", 3) == 0) &&
        hdr.Ver < 0xFF && hdr.Revision < 0xFF &&
        (hdr.Flags & 0b1111) == 0 &&
        hdr.Size[0] < 0x80 && hdr.Size[1] < 0x80 &&
        hdr.Size[2] < 0x80 && hdr.Size[3] < 0x80 &&
        TagSyncSafeIntToUInt(hdr.Size) != 0;
}

struct FLAC_BLOCK_HEADER
{
    BYTE eType;
    BYTE bySize[3];
};
static_assert(sizeof(FLAC_BLOCK_HEADER) == 4);

struct APE_HEADER
{
    CHAR byPreamble[8];
    UINT dwVer;
    UINT cbBody;
    UINT cItems;
    UINT dwFlags;
    CHAR byReserved[8];
};
static_assert(sizeof(APE_HEADER) == 32);

// APE头标志
enum :UINT
{
    APE_READ_ONLY = 1u << 0,
    APE_IS_HEADER = 1u << 29,
    APE_HAS_FOOTER = 1u << 30,
    APE_HAS_HEADER = 1u << 31,
};

EckNfInlineNd BOOL TagCheckApeHeader(const APE_HEADER& hdr) noexcept
{
    return memcmp(hdr.byPreamble, "APETAGEX", 8) == 0 &&
        (hdr.dwVer == 1000u || hdr.dwVer == 2000u) &&
        (hdr.dwFlags & 0b0001'1111'1111'1111'1111'1111'1111'1000u) == 0 &&
        hdr.cItems <= 65536 &&
        hdr.cbBody >= sizeof(APE_HEADER) &&
        *(ULONGLONG*)hdr.byReserved == 0ull;
}

inline BOOL TagGetNumberAndTotal(std::wstring_view sv,
    _Out_ int& nNum, _Out_ int& nTotal) noexcept
{
    PCWCH pEnd;
    if (TcsToInt(sv.data(), sv.size(), nNum, 10, &pEnd) != TcsCvtErr::Ok)
    {
        nTotal = 0;
        return FALSE;
    }
    const auto posSlash = sv.find(L'/', size_t(pEnd - sv.data()));
    if (posSlash != std::wstring_view::npos)
    {
        if (TcsToInt(sv.data() + posSlash + 1,
            sv.size() - (posSlash + 1), nTotal) != TcsCvtErr::Ok)
            return FALSE;
    }
    else
        nTotal = 0;
    return TRUE;
}

class CMediaFile final
{
public:
    constexpr static size_t NPos = SizeTMax;
private:
    ComPtr<IStream> m_pStream{};
    UINT m_uTagType{};

    struct TAG_LOCATION
    {
        size_t posV2{ NPos };           // ID3v2标签头的位置
        size_t posV2Footer{ NPos };     // ID3v2标签尾内容的位置
        size_t posV2FooterHdr{ NPos };  // ID3v2标签尾结构位置
        size_t cbID3v2{};               // 若无标签头则为0，若有标签头则与头中的Size字段相同，不处理预置/追加组合的判长

        size_t posV1{ NPos };       // ID3v1位置
        size_t posV1Ext{ NPos };    // 扩展ID3v1位置

        size_t posApeHdr{ NPos };   // APE标签头/尾结构的位置
        size_t posApe{ NPos };      // APE标签内容开始位置
        size_t posApeTag{ NPos };   // APE标签开始位置
        size_t cbApeTag{};          // 整个标签的大小
        BOOL bPrependApe{};         // 是否为预置APE标签

        size_t posFlac{ NPos };     // Flac标签头位置
    }  m_Loc{};

    // 如果APE未被识别，尝试根据ID3v2位置识别APE标签
    // 注意cbId3v2从文件中读取得到，是不可信的
    // 此函数依赖cbId3v2的值，因此当流抛出时放弃当前查找
    void DetectApeById3v2(CStreamWalker& w) noexcept try
    {
        if (m_Loc.cbID3v2 && m_Loc.posApe == NPos)
        {
            APE_HEADER Hdr{};
            if (m_Loc.posV2 != NPos)// 检查前置ID3v2后面
            {
                w.MoveTo(m_Loc.posV2 + m_Loc.cbID3v2) >> Hdr;
                if (TagCheckApeHeader(Hdr) && (Hdr.dwFlags & APE_IS_HEADER))
                {
                    m_Loc.posApeHdr = m_Loc.posV2 + m_Loc.cbID3v2;
                    m_Loc.posApe = m_Loc.posApe + 32u;
                    m_Loc.posApeTag = m_Loc.posApeHdr;
                    m_Loc.cbApeTag = Hdr.cbBody + 32u;
                    m_uTagType |= TAG_APE;
                    m_Loc.bPrependApe = TRUE;
                }
            }
            else if (m_Loc.posV2Footer != NPos &&
                m_Loc.posV2Footer >= m_Loc.cbID3v2 + 32u)// 检查追加ID3v2前面
            {
                w.MoveTo(m_Loc.posV2Footer - m_Loc.cbID3v2 - 32u) >> Hdr;
                if (TagCheckApeHeader(Hdr) && !(Hdr.dwFlags & APE_IS_HEADER))
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
                    m_uTagType |= TAG_APE;
                    m_Loc.bPrependApe = FALSE;
                }
                else
                {
                    ID3v2_HEADER Id3Hdr{};
                    w.MoveTo(m_Loc.posV2Footer - m_Loc.cbID3v2 - 10u) >> Id3Hdr;
                    if (TagCheckID3v2Header(Id3Hdr))
                    {
                        w.MoveTo(m_Loc.posV2Footer - m_Loc.cbID3v2 - 32u - 10u) >> Hdr;
                        if (TagCheckApeHeader(Hdr) && !(Hdr.dwFlags & APE_IS_HEADER))
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
                            m_uTagType |= TAG_APE;
                            m_Loc.bPrependApe = FALSE;
                        }
                    }
                }
            }
        }
    }
    catch (const CStreamWalker::Xpt&)
    {
    }

    void DetectId3Ape(CStreamWalker& w) noexcept try
    {
        m_Loc = {};

        BYTE by[16];

        const auto cbSize = w.GetSize();
        // 查找ID3v1
        if (cbSize > 128u)
        {
            w.Seek(-128, STREAM_SEEK_END);
            w.Read(by, 3);
            if (memcmp(by, "TAG", 3) == 0)
            {
                m_Loc.posV1 = w.GetPosition() - 3u;
                m_uTagType |= TAG_ID3V1;
            }
            if (cbSize > 128 + 227)
            {
                w.Seek(-(128 + 227), STREAM_SEEK_END);
                w.Read(by, 4);
                if (memcmp(by, "TAG+", 4) == 0)
                {
                    m_Loc.posV1Ext = w.GetPosition() - 4u;
                    m_uTagType |= TAG_ID3V1Ext;
                }
            }
        }
        // 查找APE
        size_t cbID3v1{};
        if (m_Loc.posV1Ext != NPos)
            cbID3v1 = 227u + 128u;
        else if (m_Loc.posV1 != NPos)
            cbID3v1 = 128u;
        if (cbSize > cbID3v1 + sizeof(APE_HEADER))
        {
            w.Seek(-SSIZE_T(cbID3v1 + sizeof(APE_HEADER)), STREAM_SEEK_END);
            APE_HEADER Hdr;
            w >> Hdr;
            if (TagCheckApeHeader(Hdr) && !(Hdr.dwFlags & APE_IS_HEADER))
            {
                m_Loc.posApeHdr = cbSize - (cbID3v1 + sizeof(APE_HEADER));
                m_Loc.posApe = m_Loc.posApeHdr + sizeof(APE_HEADER) - Hdr.cbBody;
                m_Loc.cbApeTag = Hdr.cbBody;
                if (Hdr.dwFlags & APE_HAS_HEADER)
                {
                    m_Loc.posApeTag = m_Loc.posApe - 32u;
                    m_Loc.cbApeTag += 32u;
                }
                else
                    m_Loc.posApeTag = m_Loc.posApe;
                m_uTagType |= TAG_APE;
                m_Loc.bPrependApe = FALSE;
            }
            else
            {
                w.MoveToBegin() >> Hdr;
                if (TagCheckApeHeader(Hdr) && (Hdr.dwFlags & APE_IS_HEADER))
                {
                    m_Loc.posApeHdr = 0u;
                    m_Loc.posApe = 32u;
                    m_Loc.posApeTag = m_Loc.posApeHdr;
                    m_Loc.cbApeTag = Hdr.cbBody + 32u;
                    m_uTagType |= TAG_APE;
                    m_Loc.bPrependApe = TRUE;
                }
            }
        }

        // 查找ID3v2
        if (cbSize > 10u)
        {
            ID3v2_HEADER hdr;
            w.MoveToBegin() >> hdr;
            if (TagCheckID3v2Header(hdr))
            {
                // 若已找到标签头，则使用其内部的SEEK帧来寻找尾部标签，因此此处不需要继续查找标签尾
                m_Loc.cbID3v2 = TagSyncSafeIntToUInt(hdr.Size);
                m_Loc.posV2 = 0u;
                if (hdr.Ver == 3)
                    m_uTagType |= TAG_ID3V2_3;
                else if (hdr.Ver == 4)
                    m_uTagType |= TAG_ID3V2_4;
            }
            else
            {
                // TODO:检查ape前后
                {
                    size_t cbFrames{};
                    // 若未找到标签头，则应从尾部扫描，检查是否有追加标签
                    if (m_Loc.posV1Ext != NPos)
                    {
                        if (cbSize > 128u + 227u + 10u)
                        {
                            w.MoveTo(m_Loc.posV1Ext - 10) >> hdr;
                            if (TagCheckID3v2Header(hdr, FALSE))
                            {
                                cbFrames = TagSyncSafeIntToUInt(hdr.Size);
                                if (cbSize >= 128u + 227u + 10u + cbFrames)
                                {
                                    m_Loc.posV2FooterHdr = w.GetPosition() - 10u;
                                    m_Loc.posV2Footer = m_Loc.posV2FooterHdr - cbFrames;
                                    if (hdr.Ver == 3)
                                        m_uTagType |= TAG_ID3V2_3;
                                    else if (hdr.Ver == 4)
                                        m_uTagType |= TAG_ID3V2_4;
                                }
                            }
                        }
                    }
                    else if (m_Loc.posV1 != NPos)
                    {
                        if (cbSize > 128u + 10u)
                        {
                            w.MoveTo(m_Loc.posV1 - 10) >> hdr;
                            if (TagCheckID3v2Header(hdr, FALSE))
                            {
                                cbFrames = TagSyncSafeIntToUInt(hdr.Size);
                                if (cbSize >= 128u + 10u + cbFrames)
                                {
                                    m_Loc.posV2FooterHdr = (size_t)w.GetPosition() - 10u;
                                    m_Loc.posV2Footer = m_Loc.posV2FooterHdr - cbFrames;
                                    if (hdr.Ver == 3)
                                        m_uTagType |= TAG_ID3V2_3;
                                    else if (hdr.Ver == 4)
                                        m_uTagType |= TAG_ID3V2_4;
                                }
                            }
                        }
                    }
                    else
                    {
                        w.Seek(-10, STREAM_SEEK_END);
                        w >> hdr;
                        if (TagCheckID3v2Header(hdr, FALSE))
                        {
                            cbFrames = TagSyncSafeIntToUInt(hdr.Size);
                            if (cbSize >= 10u + cbFrames)
                            {
                                m_Loc.posV2FooterHdr = w.GetPosition() - 10u;
                                m_Loc.posV2Footer = m_Loc.posV2FooterHdr - cbFrames;
                                if (hdr.Ver == 3)
                                    m_uTagType |= TAG_ID3V2_3;
                                else if (hdr.Ver == 4)
                                    m_uTagType |= TAG_ID3V2_4;
                            }
                        }
                    }

                    m_Loc.cbID3v2 = cbFrames;
                }
            }
            DetectApeById3v2(w);
        }
    }
    catch (const CStreamWalker::Xpt&)
    {
    }

    void DetectFlac(CStreamWalker& w) noexcept try
    {
        if (m_Loc.posV2 == NPos)
            w.MoveToBegin();
        else
            w.MoveTo(m_Loc.posV2 + m_Loc.cbID3v2 + 10);
        BYTE by[4];
        w >> by;
        // XXX: 若失配则向后查找
        if (memcmp(by, "fLaC", 4) == 0)
        {
            m_Loc.posFlac = w.GetPosition() - 4u;
            m_uTagType |= TAG_FLAC;
        }
    }
    catch (const CStreamWalker::Xpt&)
    {
    }
public:
    CMediaFile() = default;
    CMediaFile(IStream* pStream) noexcept : m_pStream{ pStream } { DetectTag(); }

    CMediaFile(PCWSTR pszFile, UINT grfMode = STGM_READ,
        UINT uAttr = FILE_ATTRIBUTE_NORMAL, BOOL bCreate = FALSE) noexcept
    {
        SHCreateStreamOnFileEx(pszFile, grfMode, uAttr,
            bCreate, nullptr, &m_pStream);
        DetectTag();
    }

    EckInline void SetStream(IStream* pStream) noexcept
    {
        m_pStream = pStream;
        DetectTag();
    }
    EckInlineNdCe IStream* GetStream() const noexcept { return m_pStream.Get(); }

    EckInlineNdCe UINT GetTagType() const noexcept { return m_uTagType; }
    EckInlineNdCe auto& GetTagLocation() const noexcept { return m_Loc; }
    EckInlineNdCe BOOL IsValid() const noexcept { return !!GetStream(); }

    UINT DetectTag() noexcept try
    {
        m_Loc = {};
        m_uTagType = TAG_INVALID;
        if (!IsValid())
            return TAG_INVALID;
        CStreamWalker w{ m_pStream.Get() };
        DetectId3Ape(w);
        DetectFlac(w);
        return m_uTagType;
    }
    catch (const CStreamWalker::Xpt&)
    {
    }
};

class CTag
{
protected:
    CMediaFile& m_File;
    CStreamWalker m_Stream{};
    HRESULT m_hrLast{};
public:
    ECK_DISABLE_COPY_MOVE(CTag);
    CTag(CMediaFile& mf) noexcept : m_File{ mf }, m_Stream{ mf.GetStream() } {}

    virtual ~CTag() = default;

    virtual Result SimpleGet(MUSICINFO& mi, const SIMPLE_OPT& Opt) noexcept = 0;
    virtual Result SimpleSet(MUSICINFO& mi, const SIMPLE_OPT& Opt) noexcept = 0;

    virtual Result ReadTag(UINT uFlags) noexcept = 0;
    virtual Result WriteTag(UINT uFlags) noexcept = 0;
    virtual void Reset() noexcept = 0;
    virtual BOOL IsEmpty() noexcept = 0;
};
ECK_MEDIATAG_NAMESPACE_END
ECK_NAMESPACE_END