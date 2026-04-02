#pragma once
#include "UiElement.h"
#include "UiColor.h"
#include "CReferenceCounted.h"

ECK_NAMESPACE_BEGIN
ECK_DUI_NAMESPACE_BEGIN
using namespace UiBasic::Declaration;

class CElement;

// 元素样式
enum
{
    DES_BLURBKG = (1u << 31),	// 模糊背景
    // 元素的内容受周边其他内容影响，若无效区域与元素相交，
    // 则必须更新整个元素，设置时DES_BLURBKG强制设置此样式
    DES_CONTENT_EXPAND = (1u << 30),
    // 元素的内容受周边其他内容影响，确定更新区域时DUI系统发送
    // EWM_QUERY_EXPAND_RECT以获取扩展矩形
    DES_CONTENT_EXPAND_RECT = (1u << 29),
    // DUI系统应当检查当前元素的祖元素，因为它们可能设置了混合器
    DES_PARENT_COMP = (1u << 28),
    DES_NO_REDRAW = (1u << 27),	// 不允许重绘，一般不使用此样式
    // 对于手动混合元素，DUI不应自行分配后台缓存，而应按下列顺序请求缓存：
    // 调用CCompositor::CreateCacheBitmap，若失败，向元素的父级发送
    // EWM_CREATE_CACHE_BITMAP
    DES_OWNER_COMP_CACHE = (1u << 26),
    // 指示当前手动混合元素不使用后台缓存，
    // 设置后DUI系统适时调用CCompositor::PreRender
    DES_COMP_NO_REDIRECTION = (1u << 25),
    // 【仅供内部使用】元素的后台缓存内容需要更新。
    // 设置该标志的意图是尽量减少手动混合元素的重渲染（特别是在播放动画时）。
    // 普通元素需要每次重渲染是因为其内容没有保存，但未设置DES_COMP_NO_REDIRECTION
    // 的手动混合标志都具有一副后台位图，内容为上一次渲染结果。
    // DUI系统直接在m_dwStyle字段上操作该位，该位永远不会传递到SetStyle
    DESP_COMP_CONTENT_INVALID = (1u << 24),
    // 指示基类事件处理函数应调用BeginPaint/EndPaint对
    DES_BASE_BEGIN_END_PAINT = (1u << 23),

    // 【仅供内部使用】
    DESP_EXPANDED = DES_CONTENT_EXPAND | DES_CONTENT_EXPAND_RECT,
};

// 元素产生的通知
enum : UINT
{
    ENC_DUI_DUMMY = ENC_SYSBEGIN,

    // TrackBar
    TBE_POSCHANGED,		// 位置改变
    // List
    LEE_GETDISPINFO,	// 【渲染线程】获取显示信息(NMLEDISPINFO*)
    // ListTemplate
    LTE_ITEMCHANED,		// 项改变(NMLTITEMCHEANGED*)，返回TRUE禁止修改
    LTE_HOTITEMCHANED,	// 热点项改变(NMLTHOTITEMCHEANGED*)，返回TRUE禁止修改
    LTE_SCROLLED,		// 滚动结束(NMLTSCROLLED*)
    // TabList
    TBLE_GETDISPINFO,	// 【渲染线程】获取显示信息(NMTBLDISPINFO*)
    TBLE_SELCHANGED,	// 选项卡改变(NMTBLITEMINDEX*)
    // Header
    HEE_GETDISPINFO,	// 【渲染线程】获取显示信息(NMHEDISPINFO*)
    HEE_BEGINDRAG,		// 开始拖拽(NMHEDRAG*)
    HEE_ENDDRAG,		// 结束拖拽(NMHEDRAG*)
    HEE_WIDTHCHANGED,	// 宽度改变(NMHEITEMNOTIFY*)
    HEE_ORDERCHANGED,	// 顺序改变(NMHEITEMNOTIFY*)
    // Edit
    EDE_TXNOTIFY,		// 来自文本服务的通知(NMEDTXNOTIFY*)
    // TabHeader
    THE_GETDISPINFO,	// 【渲染线程】获取显示信息(NMTTHDISPINFO*)

    EE_PRIVATE_BEGIN = 0x0400
};

enum class PresentMode : BYTE
{
    // WS_EX_NRB = WS_EX_NOREDIRECTIONBITMAP
    DCompositionSurface,
    DCompositionVisual,
    BitBltSwapChain,	// 支持透明混合，必须无WS_EX_NRB
    FlipSwapChain,		// 不支持透明混合
    WindowRenderTarget,	// 支持透明混合，必须无WS_EX_NRB
    DxgiSurface,		// TODO
    UpdateLayeredWindow,
};

// 渲染事件代码
enum
{
    // 即将开始渲染
    // 下列字段有效：PreRender
    // 下列返回值有效：
    // RER_NONE = 执行默认操作
    // RER_REDIRECTION = 应用程序重定向渲染，DUI系统应使用pSfcNewDst和prcNewDirtyPhy
    RE_PRERENDER,
    // 渲染完毕，仅当RE_PRERENDER返回RER_REDIRECTION时产生
    RE_POSTRENDER,
    // DUI系统认为有必要冲洗一切挂起的工作
    RE_COMMIT,
    // 正在填充背景
    // 下列字段有效：FillBkg
    // 下列返回值有效：
    // RER_NONE = 执行默认操作
    // RER_NO_ERASE = 跳过背景填充
    RE_FILLBACKGROUND,
};
// 渲染事件返回值
enum : LRESULT
{
    RER_NONE = 0,
    RER_REDIRECTION = 1 << 0,
    RER_NO_ERASE = 1 << 1,
};
struct RENDER_EVENT
{
    union
    {
        struct
        {
            IDXGISurface* pSfcFinalDst;
            POINT ptOffsetPhy;
            const RECT* prcDirtyPhy;
            IDXGISurface* pSfcNewDst;
            RECT* prcNewDirtyPhy;
        } PreRender;
        struct
        {
            D2D1_RECT_F rc;
        } FillBkg;
    };
};

class CCompCacheSurface;
struct CREATE_CACHE_BITMAP_INFO
{
    int cxPhy;
    int cyPhy;
    CCompCacheSurface* pCacheSurface;
    HRESULT hr;
};
// 元素事件
enum
{
    ECKPRIV_EWM_PLACEHOLDER = WM_USER_SAFE,
    WM_DRAGENTER,
    WM_DRAGOVER,
    WM_DRAGLEAVE,
    WM_DROP,

    EWM_COLORSCHEMECHANGED,	// 颜色主题改变 (BOOL 是否为暗色, 0)
    EWM_QUERY_EXPAND_RECT,	// 查询扩展矩形 (_Inout_ D2D1_RECT_F*, 0)
    // 创建缓存位图 (_Inout_ CREATE_CACHE_BITMAP_INFO*, 0)
    // 若成功则应返回TRUE
    EWM_CREATE_CACHE_BITMAP,

    EWM_PRIVBEGIN,
};

namespace Priv
{
    struct PAINT_EXTRA// WM_PAINT的不透明lParam
    {
        float ox;
        float oy;
        const D2D1_RECT_F* prcClipInClient;
    };
}

struct PAINTINFO
{
    D2D1_RECT_F rcfClip;        // 剪裁矩形，相对客户区
    D2D1_RECT_F rcfClipInElem;  // 剪裁矩形，相对元素
    float ox;
    float oy;
    BOOLEAN bClip;
};

constexpr inline auto DrawTextLayoutFlags =
D2D1_DRAW_TEXT_OPTIONS_ENABLE_COLOR_FONT | D2D1_DRAW_TEXT_OPTIONS_NO_SNAP;

class CBitmap final : public CReferenceCountedT<CBitmap>
{
private:
    ComPtr<ID2D1Bitmap1> m_pBitmap{};
    D2D1_RECT_F m_rcSource{};
public:
    void SetSourceRect(const D2D1_RECT_F* prc) noexcept
    {
        if (prc)
        {
            m_rcSource = *prc;
#ifdef _DEBUG
            EckAssert(m_rcSource.left >= 0 && m_rcSource.top >= 0);
            if (m_pBitmap.Get())
            {
                const auto size = m_pBitmap->GetSize();
                EckAssert(m_rcSource.right <= size.width);
                EckAssert(m_rcSource.bottom <= size.height);
            }
#endif
        }
        else
            m_rcSource = { FLT_MAX, FLT_MAX, FLT_MAX, FLT_MAX };
    }

    void Set(ID2D1Bitmap1* p, const D2D1_RECT_F* prc = nullptr) noexcept
    {
        m_pBitmap = p;
        SetSourceRect(prc);
    }
    auto Get() const noexcept { return m_pBitmap.Get(); }

    const D2D1_RECT_F* GetSourceRect() const noexcept
    {
        if (m_rcSource.left == FLT_MAX)
            return nullptr;
        else
            return &m_rcSource;
    }
    D2D1_RECT_F GetActualSourceRect() const noexcept
    {
        if (m_rcSource.left == FLT_MAX)
        {
            const auto size = m_pBitmap->GetSize();
            return { 0, 0, size.width, size.height };
        }
        else
            return m_rcSource;
    }
};
ECK_DUI_NAMESPACE_END
ECK_NAMESPACE_END