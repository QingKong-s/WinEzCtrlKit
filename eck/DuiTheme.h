#pragma once
#include "GraphicsHelper.h"
#include "CUnknown.h"

ECK_NAMESPACE_BEGIN
ECK_DUI_NAMESPACE_BEGIN
enum class Part : BYTE
{
    Button,
    CircleButton,
    RadioButton,
    CheckButton,
    TriStateCheckButton,
    GroupBox,

    Edit,
    EditBottomBar,

    List,
    ListSelRect,
    ListItem,

    Header,
    HeaderItem,

    Progress,

    TrackBar,
    TrackBarThumb,

    ScrollBar,
    ScrollButton,
    ScrollThumb,

    LabelText,
    LabelBk,

    MaxStdControl,

    Invalid = 0xFF,
    UserBegin = 128,
};

enum class State : BYTE
{
    None,// 不关心状态时的占位符
    // 以下是基本状态
    Normal,
    Hot,
    Selected,
    Disabled,
    // 以下状态可与基本状态组合
    Checked,
    Mixed,
    Focused,
    // 以下为组合状态
    HotSelected,

    CheckedHot,
    CheckedSelected,
    CheckedDisabled,

    MixedHot,
    MixedSelected,
    MixedDisabled,

    NoFocusSelected,

    UserBegin = 128,
};

enum class SysColor
{
    Text,
    Bk,
    MainTitle,

    Max
};

enum class Metrics
{
    CxVScroll,		// 垂直滚动条宽度
    CxVThumb,		// 垂直滚动条滑块宽度
    CyMinVThumb,	// 最小垂直滚动条滑块高度
    CyHScroll,		// 水平滚动条高度
    CyHThumb,		// 水平滚动条滑块高度
    CxMinHThumb,	// 最小水平滚动条滑块宽度
    Padding,		// 通用空白，一般用于外部，如内容与边框之间
    LargePadding,	// 通用空白（大）
    SmallPadding,	// 通用空白（小），一般用于内部，如图片与文本之间
    CxFocusBorder,	// 焦点矩形水平边距
    CyFocusBorder,	// 焦点矩形垂直边距
    CxIconSpacing,	// 图标水平间距
    CyIconSpacing,	// 图标垂直间距

    Max				// 度量索引的最大值
};

enum class GeoType : UINT
{
    None,

    FillBegin,
    FillRect,
    FillRoundRect,
    FillEllipse,
    FillEnd,

    FrameBegin,
    FrameRect,
    FrameRoundRect,
    FrameEllipse,
    FrameEnd,

    FillFrameBegin,
    FillFrameRect,
    FillFrameRoundRect,
    FillFrameEllipse,
    FillFrameEnd,

    Line,
};
union GEO_PARAM
{
    D2D1_RECT_F Rect;
    D2D1_ROUNDED_RECT RRect;
    D2D1_ELLIPSE Ellipse;
    D2D1_POINT_2F Point[2];
};

enum class ThemeDraw : BYTE
{
    None,
    Solid,
    LinerGradient,
    RadiusGradient,
    Image,
    ImageNineGrid,
    Geometry,
};
struct THEME_DRAW_ENTRY
{
    Part ePart;
    State eState;
    BYTE cState : 6;
    BYTE bNoStretch : 1;
    BYTE bColorization : 1;
    ThemeDraw eDraw;
    float f;
    union
    {
        D2D1_COLOR_F Color;// f = Unused
        struct
        {
            BkImgMode eImgMode;
            ID2D1Bitmap* pBmp;
            D2D1_RECT_F rcSrc;
            D2D1_RECT_F Margins;
        } Img;// f = Opacity
        struct
        {
            GeoType eType;
            D2D1_COLOR_F crFill;
            D2D1_COLOR_F crFrame;
            GEO_PARAM Param;
        } Geometry;// f = FrameWidth
    };
};

enum : UINT
{
    DTBO_NONE = 0,
    DTBO_CLIP_RECT = 1u << 0,
    DTBO_NEW_COLOR = 1u << 1,// 暂未实现
    DTBO_NEW_STROKE_WIDTH = 1u << 2,
    DTBO_NEW_OPACITY = 1u << 3,
    DTBO_NEW_STROKE_STYLE = 1u << 4,
    DTBO_NEW_INTERPOLATION_MODE = 1u << 5,
    DTBO_NEW_RADX = 1u << 6,
    DTBO_NEW_RADY = 1u << 7,
};
struct DTB_OPT
{
    UINT uFlags;// DTBO_
    D2D1_RECT_F rcClip;
    float fStrokeWidth;
    float fOpacity;
    ID2D1StrokeStyle* pStrokeStyle;
    D2D1_INTERPOLATION_MODE eInterpolationMode;
    float fRadX;
    float fRadY;
};
// 默认DrawBackground选项
constexpr inline DTB_OPT DtbOptDefault{};

struct __declspec(uuid("85623275-F66F-4D96-8EFE-6F97E2519AC8"))
    ITheme : public IUnknown
{
    virtual ~ITheme() = default;
    virtual HRESULT RealizeForDC(ID2D1DeviceContext* pDC) noexcept = 0;
    virtual HRESULT DrawBackground(Part ePart, State eState,
        const D2D1_RECT_F& rc, _In_opt_ const DTB_OPT* pOpt) noexcept = 0;
    virtual HRESULT SetColorizationColor(const D2D1_COLOR_F& cr) noexcept = 0;
    virtual HRESULT GetColorizationColor(_Out_ D2D1_COLOR_F& cr) noexcept = 0;
    virtual HRESULT GetColor(Part ePart, State eState,
        ClrPart eClrPart, _Out_ D2D1_COLOR_F& cr) noexcept = 0;
    virtual HRESULT GetSysColor(SysColor eSysColor, _Out_ D2D1_COLOR_F& cr) noexcept = 0;
    virtual float GetMetrics(Metrics eMetrics) noexcept = 0;
    // 指定将未实现操作落入的上级主题
    virtual HRESULT SetParent(ITheme* pParent) noexcept = 0;
    // 取上级主题，调用方负责释放结果
    virtual HRESULT GetParent(_Out_ ITheme*& pParent) noexcept = 0;
};

class CThemeDrawList
{
private:
    std::vector<THEME_DRAW_ENTRY> m_vDraw{};
public:
    using TIterator = decltype(m_vDraw)::const_iterator;

    EckInlineNdCe auto InvalidIterator() const { return m_vDraw.end(); }

    auto LookupPart(Part ePart) const noexcept
    {
        const auto it = std::lower_bound(m_vDraw.begin(), m_vDraw.end(), ePart,
            [](const THEME_DRAW_ENTRY& e, Part ePart) { return e.ePart < ePart; });
        if (it != m_vDraw.end() && it->ePart == ePart)
            return it;
        return m_vDraw.end();
    }
    auto LookupState(TIterator itFirst, State eState) const noexcept
    {
        const auto itEnd = itFirst + itFirst->cState;
        const auto it = std::lower_bound(itFirst, itEnd, eState,
            [](const THEME_DRAW_ENTRY& e, State eState) { return e.eState < eState; });
        // 如果调用方指定State::None，返回第一个状态
        if (it != itEnd && (eState == State::None || it->eState == eState))
            return it;
        return m_vDraw.end();
    }
    auto LookupState(State eState) const noexcept
    {
        const auto it = std::lower_bound(m_vDraw.begin(), m_vDraw.end(), eState,
            [](const THEME_DRAW_ENTRY& e, State eState) { return e.eState < eState; });
        if (it != m_vDraw.end() && (eState == State::None || it->eState == eState))
            return it;
        return m_vDraw.end();
    }

    auto LookupPartState(Part ePart, State eState) const noexcept
    {
        const auto itFirst = LookupPart(ePart);
        if (itFirst == m_vDraw.end())
            return m_vDraw.end();
        return LookupState(itFirst, eState);
    }

    auto& Add(Part ePart, State eState) noexcept
    {
        auto itFirst = std::lower_bound(m_vDraw.begin(), m_vDraw.end(), ePart,
            [](const THEME_DRAW_ENTRY& e, Part ePart) { return e.ePart < ePart; });
        if (itFirst != m_vDraw.end() && itFirst->ePart == ePart)
        {
            const auto itEnd = itFirst + itFirst->cState;
            const auto it = std::lower_bound(itFirst, itEnd, eState,
                [](const THEME_DRAW_ENTRY& e, State eState) { return e.eState < eState; });
            if (it == itEnd)
            {
                ++itFirst->cState;
                return *m_vDraw.emplace(itEnd, ePart, eState);
            }
            else if (it->eState == eState)
                return *it;
            else
                return *m_vDraw.emplace(it, ePart, eState,
                    itFirst == it ? itFirst->cState + 1 : 0);
        }
        else
            return *m_vDraw.emplace(itFirst, ePart, eState, 1);
    }
    auto& Add(State eState) noexcept
    {
        const auto it = std::lower_bound(m_vDraw.begin(), m_vDraw.end(), eState,
            [](const THEME_DRAW_ENTRY& e, State eState) { return e.eState < eState; });
        if (it != m_vDraw.end() && it->eState == eState)
            return *it;
        return *m_vDraw.emplace(it, Part::Invalid, eState);
    }
};

class CThemeCommonBase : public CUnknown<CThemeCommonBase, ITheme>
{
protected:
    D2D1_COLOR_F m_crColorization{};
    ComPtr<ITheme> m_pParent{};
    ComPtr<ID2D1DeviceContext> m_pDC{};
public:
    HRESULT RealizeForDC(ID2D1DeviceContext* pDC) noexcept override
    {
        m_pDC = pDC;
        return S_OK;
    }
    HRESULT DrawBackground(Part ePart, State eState,
        const D2D1_RECT_F& rc, _In_opt_ const DTB_OPT* pOpt) noexcept override
    {
        if (m_pParent.Get())
            return m_pParent->DrawBackground(ePart, eState, rc, pOpt);
        return HRESULT_FROM_WIN32(ERROR_NOT_SUPPORTED);
    }
    HRESULT GetColor(Part ePart, State eState,
        ClrPart eClrPart, _Out_ D2D1_COLOR_F& cr) noexcept override
    {
        if (m_pParent.Get())
            return m_pParent->GetColor(ePart, eState, eClrPart, cr);
        return HRESULT_FROM_WIN32(ERROR_NOT_SUPPORTED);
    }
    HRESULT SetColorizationColor(const D2D1_COLOR_F& cr) noexcept override
    {
        m_crColorization = cr;
        return S_OK;
    }
    HRESULT GetColorizationColor(_Out_ D2D1_COLOR_F& cr) noexcept override
    {
        cr = m_crColorization;
        return S_OK;
    }
    HRESULT GetSysColor(SysColor eSysColor, _Out_ D2D1_COLOR_F& cr) noexcept override
    {
        if (m_pParent.Get())
            return m_pParent->GetSysColor(eSysColor, cr);
        return E_NOTIMPL;
    }
    float GetMetrics(Metrics eMetrics) noexcept override
    {
        if (m_pParent.Get())
            return m_pParent->GetMetrics(eMetrics);
        return 0.f;
    }
    HRESULT SetParent(ITheme* pParent) noexcept override
    {
        m_pParent = pParent;
        return S_OK;
    }
    HRESULT GetParent(_Out_ ITheme*& pParent) noexcept override
    {
        pParent = m_pParent.Get();
        pParent->AddRef();
        return S_OK;
    }
};



EckInlineNdCe BOOL ThGeoIsFill(GeoType e) { return e > GeoType::FillBegin && e < GeoType::FillEnd; }
EckInlineNdCe BOOL ThGeoIsFrame(GeoType e) { return e > GeoType::FrameBegin && e < GeoType::FrameEnd; }
EckInlineNdCe BOOL ThGeoIsFillFrame(GeoType e) { return e > GeoType::FillFrameBegin && e < GeoType::FillFrameEnd; }

struct THEME_DRAW_GEO_INFO
{
    const DTB_OPT* pOpt;
    ID2D1DeviceContext* pDC;
    ID2D1SolidColorBrush* pBrush;
    GeoType eType;
    BOOLEAN bNoStretch;

    float cxBorder;
    D2D1_COLOR_F crFrame;
    D2D1_COLOR_F crFill;
};

// 调用方处理
inline BOOL ThDrawGeometry(const THEME_DRAW_GEO_INFO& Info,
    const D2D1_RECT_F& rc, const GEO_PARAM& Param)
{
    GEO_PARAM Geo{};
    switch (Info.eType)
    {
    case GeoType::FillRect:
    case GeoType::FrameRect:
    case GeoType::FillFrameRect:
        if (Info.bNoStretch)
        {
            Geo.Rect = Param.Rect;
            CenterRect(Geo.Rect, rc);
        }
        else
            Geo.Rect = rc;
        break;
    case GeoType::FillRoundRect:
    case GeoType::FrameRoundRect:
    case GeoType::FillFrameRoundRect:
        if (Info.pOpt->uFlags & DTBO_NEW_RADX)
            Geo.RRect.radiusX = Info.pOpt->fRadX;
        else
            Geo.RRect.radiusX = Param.RRect.radiusX;
        if (Info.pOpt->uFlags & DTBO_NEW_RADY)
            Geo.RRect.radiusY = Info.pOpt->fRadY;
        else
            Geo.RRect.radiusY = Param.RRect.radiusY;

        if (Info.bNoStretch)
        {
            Geo.RRect.rect = Param.RRect.rect;
            CenterRect(Geo.RRect.rect, rc);
        }
        else
            Geo.RRect.rect = rc;
        break;
    case GeoType::FillEllipse:
    case GeoType::FrameEllipse:
    case GeoType::FillFrameEllipse:
        Geo.Ellipse.point.x = (rc.left + rc.right) / 2.f;
        Geo.Ellipse.point.y = (rc.top + rc.bottom) / 2.f;
        if (Info.bNoStretch)
        {
            Geo.Ellipse.radiusX = Param.Ellipse.radiusX;
            Geo.Ellipse.radiusY = Param.Ellipse.radiusY;
        }
        else
        {
            Geo.Ellipse.radiusX = (rc.right - rc.left) / 2.f;
            Geo.Ellipse.radiusY = (rc.bottom - rc.top) / 2.f;
        }
        break;
    case GeoType::Line:
        if (Info.bNoStretch)
        {
            Geo.Rect.left = Param.Point[0].x;
            Geo.Rect.top = Param.Point[0].y;
            Geo.Rect.right = Param.Point[1].x;
            Geo.Rect.bottom = Param.Point[1].y;
            CenterRect(Geo.Rect, rc);
            Geo.Point[0].x = Geo.Rect.left;
            Geo.Point[0].y = Geo.Rect.top;
            Geo.Point[1].x = Geo.Rect.right;
            Geo.Point[1].y = Geo.Rect.bottom;
        }
        else
        {
            Geo.Point[0].x = rc.left;
            Geo.Point[0].y = rc.top;
            Geo.Point[1].x = rc.right;
            Geo.Point[1].y = rc.bottom;
        }
        break;
    default:
        return FALSE;
    }

    const auto pStroke = (Info.pOpt->uFlags & DTBO_NEW_STROKE_STYLE) ?
        Info.pOpt->pStrokeStyle : nullptr;
    const auto cxStroke = (Info.pOpt->uFlags & DTBO_NEW_STROKE_WIDTH) ?
        Info.pOpt->fStrokeWidth : Info.cxBorder;
    if (ThGeoIsFrame(Info.eType) || Info.eType == GeoType::Line)
        Info.pBrush->SetColor(Info.crFrame);
    else
        Info.pBrush->SetColor(Info.crFill);
    switch (Info.eType)
    {
    case GeoType::FillRect:		Info.pDC->FillRectangle(Geo.Rect, Info.pBrush);			break;
    case GeoType::FillRoundRect:Info.pDC->FillRoundedRectangle(Geo.RRect, Info.pBrush);	break;
    case GeoType::FillEllipse:	Info.pDC->FillEllipse(Geo.Ellipse, Info.pBrush);		break;

    case GeoType::FrameRect:
        InflateRect(Geo.Rect, -cxStroke / 2.f, -cxStroke / 2.f);
        Info.pDC->DrawRectangle(Geo.Rect, Info.pBrush, cxStroke, pStroke);
        break;
    case GeoType::FrameRoundRect:
        InflateRect(Geo.RRect.rect, -cxStroke / 2.f, -cxStroke / 2.f);
        Info.pDC->DrawRoundedRectangle(Geo.RRect, Info.pBrush, cxStroke, pStroke);
        break;
    case GeoType::FrameEllipse:
        Geo.Ellipse.radiusX -= cxStroke / 2.f;
        Geo.Ellipse.radiusY -= cxStroke / 2.f;
        Info.pDC->DrawEllipse(Geo.Ellipse, Info.pBrush, cxStroke, pStroke);
        break;

    case GeoType::FillFrameRect:
        Info.pDC->FillRectangle(Geo.Rect, Info.pBrush);
        Info.pBrush->SetColor(Info.crFrame);
        InflateRect(Geo.Rect, -cxStroke / 2.f, -cxStroke / 2.f);
        Info.pDC->DrawRectangle(Geo.Rect, Info.pBrush, cxStroke, pStroke);
        break;
    case GeoType::FillFrameRoundRect:
        Info.pDC->FillRoundedRectangle(Geo.RRect, Info.pBrush);
        Info.pBrush->SetColor(Info.crFrame);
        InflateRect(Geo.RRect.rect, -cxStroke / 2.f, -cxStroke / 2.f);
        Info.pDC->DrawRoundedRectangle(Geo.RRect, Info.pBrush, cxStroke, pStroke);
        break;
    case GeoType::FillFrameEllipse:
        Info.pDC->FillEllipse(Geo.Ellipse, Info.pBrush);
        Info.pBrush->SetColor(Info.crFrame);
        Geo.Ellipse.radiusX -= cxStroke / 2.f;
        Geo.Ellipse.radiusY -= cxStroke / 2.f;
        Info.pDC->DrawEllipse(Geo.Ellipse, Info.pBrush, cxStroke, pStroke);
        break;
    case GeoType::Line:
        Info.pDC->DrawLine(Geo.Point[0], Geo.Point[1], Info.pBrush, cxStroke, pStroke);
        break;
    }
    return TRUE;
}
ECK_DUI_NAMESPACE_END
ECK_NAMESPACE_END