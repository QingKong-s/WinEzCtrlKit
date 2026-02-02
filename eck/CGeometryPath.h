#pragma once
#include "CUnknown.h"
#include "Utility.h"
#include "CTrivialBuffer.h"
#include "KwFlatten.h"
#include "IGeometrySinkTransformer.h"

#include "../ThirdPartyLib/SkiaTessellator/GrAATriangulator.h"
#include "../ThirdPartyLib/SkiaTessellator/ISkiaTessellationHost.h"

ECK_NAMESPACE_BEGIN
enum class PathType : BYTE
{
    // 以下为图形指令

    // 直线段
    GmLine = SkiaTessellator::PathType::Line,
    // 三次贝塞尔曲线，仅录制两个控点和终点，具有如下形式
    // Point: |    P0   |   P1   |   P2   |   P3   |
    // Type:  | Unknown | Bezier | Bezier | Bezier |
    GmBezier = 1 << 0,

    // 以下为子图形起点终点标志

    // 子图形起点，BeginFigure创建此点
    FgBegin = SkiaTessellator::PathType::Begin,
    // 子图形终点
    FgEnd = 1 << 1,
    // 在子图形起点设置，指示子图形是实心的，填充时应忽略不含此标志的子图形
    FgFill = SkiaTessellator::PathType::Fill,
    // 在子图形终点设置，指示子图形闭合，一般情况下，此标志仅影响描边
    FgClose = 1 << 2,
    FgMask = FgClose | FgFill | FgEnd,

    // 以下联接标志在每个点（贝塞尔曲线的三个点标志相同）设置
    // 子图形的第一点不设置联接标志，第二点设置但通常无意义

    // D2D1_PATH_SEGMENT_NONE，由外部指定联接和笔划
    SgDefault = 0,
    // D2D1_PATH_SEGMENT_FORCE_UNSTROKED，描边时应忽略含此标志的段
    SgUnstroked = 1 << 3,
    // D2D1_PATH_SEGMENT_FORCE_ROUND_LINE_JOIN，描边时应使用圆形联接
    SgRoundJoin = 1 << 4,
    SgMask = SgUnstroked | SgRoundJoin,
};
ECK_ENUM_BIT_FLAGS(PathType);

class CGeometryPath final : public CUnknown<CGeometryPath,
    ID2D1SimplifiedGeometrySink,
    IGeometrySinkTransformer>
{
public:
    struct PORTABLE_DATA
    {
        CTrivialBuffer<Kw::Vec2> vPoint{};
        CTrivialBuffer<PathType> vType{};
        Kw::Rect rcBounds{};
        float fTolerance{};
        BYTE eFillMode{ D2D1_FILL_MODE_WINDING };
        BOOLEAN bBoundsValid{ FALSE };

        void Stream(ID2D1SimplifiedGeometrySink* pSink) const noexcept
        {
            CGeometryPath::Stream(vPoint.ToSpan(), vType.ToSpan(), pSink);
        }
    };
private:
    CTrivialBuffer<Kw::Vec2> m_vPoint{};
    CTrivialBuffer<PathType> m_vType{};
    Kw::Vec2 m_ptOffset{};
    PathType m_eSegFlags{ PathType::SgDefault };
    BYTE m_eFillMode{ D2D1_FILL_MODE_WINDING };
    // 指定贝塞尔曲线展平生成的线段以何种方式联接
    PathType m_eBezierSegFlags{ PathType::SgDefault };
    // 设为TRUE使用m_eBezierSegFlags
    BOOLEAN m_bUseBezierSegFlags{};
    BOOLEAN m_bHasBezier{};
    BOOLEAN m_bTranslate{};
    BOOLEAN m_bDbgInFigure{};
    BOOLEAN m_bDbgClosed{};
public:
    STDMETHOD_(void, SetFillMode)(D2D1_FILL_MODE eFillMode) override
    {
        m_eFillMode = (BYTE)eFillMode;
    }

    STDMETHOD_(void, SetSegmentFlags)(D2D1_PATH_SEGMENT eSegOpt) override
    {
        switch (eSegOpt)
        {
        case D2D1_PATH_SEGMENT_FORCE_UNSTROKED:
            m_eSegFlags = PathType::SgUnstroked;
            break;
        case D2D1_PATH_SEGMENT_FORCE_ROUND_LINE_JOIN:
            m_eSegFlags = PathType::SgRoundJoin;
            break;
        default:
            m_eSegFlags = PathType::SgDefault;
            break;
        }
    }

    STDMETHOD_(void, BeginFigure)(D2D1_POINT_2F ptStart, D2D1_FIGURE_BEGIN eOpt) override
    {
        EckAssert(!m_bDbgClosed);
        if (m_bDbgInFigure)
        {
            EckDbgBreak();
            return;
        }
        m_bDbgInFigure = TRUE;

        if (m_bTranslate)
            m_vPoint.PushBack({ ptStart.x + m_ptOffset.x, ptStart.y + m_ptOffset.y });
        else
            m_vPoint.PushBack({ ptStart.x, ptStart.y });

        if (eOpt == D2D1_FIGURE_BEGIN_FILLED)
            m_vType.PushBack(PathType::FgBegin | PathType::FgFill);
        else
            m_vType.PushBack(PathType::FgBegin);
    }

    STDMETHOD_(void, AddLines)(_In_reads_(cPt) CONST D2D1_POINT_2F* ppt, UINT32 cPt) override
    {
        EckAssert(!m_bDbgClosed);
        const auto pNew = m_vPoint.PushBackSize(cPt);
        memcpy(pNew, ppt, cPt * sizeof(D2D1_POINT_2F));
        if (m_bTranslate)
        {
            EckCounter(cPt, i)
                pNew[i] += m_ptOffset;
        }
        m_vType.PushBackMultiple(PathType::GmLine | m_eSegFlags, cPt);
    }

    STDMETHOD_(void, AddBeziers)(_In_reads_(cBezier)
        CONST D2D1_BEZIER_SEGMENT* pBezier, UINT32 cBezier) override
    {
        EckAssert(!m_bDbgClosed);
        m_bHasBezier = TRUE;
        const auto pNew = m_vPoint.PushBackSize(cBezier * 3);
        memcpy(pNew, pBezier, cBezier * sizeof(D2D1_BEZIER_SEGMENT));
        if (m_bTranslate)
        {
            EckCounter(cBezier * 3, i)
                pNew[i] += m_ptOffset;
        }
        m_vType.PushBackMultiple(PathType::GmBezier | m_eSegFlags, cBezier * 3);
    }

    STDMETHOD_(void, EndFigure)(D2D1_FIGURE_END eFigureEnd) override
    {
        EckAssert(!m_bDbgClosed);
        if (!m_bDbgInFigure)
        {
            EckDbgBreak();
            return;
        }
        m_bDbgInFigure = FALSE;

        if (m_vPoint.Size() < 2)
            return;
        auto eType = PathType::GmLine | PathType::FgEnd;
        if (eFigureEnd == D2D1_FIGURE_END_CLOSED)
            eType |= PathType::FgClose;
        m_vType.Back() |= eType;
    }

    STDMETHOD(Close)() override
    {
        EckAssert(!m_bDbgClosed);
        if (m_bDbgInFigure)
        {
            EckDbgBreak();
            return E_FAIL;
        }
        m_bDbgClosed = TRUE;
        return S_OK;
    }

    STDMETHOD(GstSetSink)(ID2D1SimplifiedGeometrySink* pSink) override { return E_NOTIMPL; }
    STDMETHOD(GstGetSink)(ID2D1SimplifiedGeometrySink** ppSink) override
    {
        *ppSink = nullptr;
        return E_NOTIMPL;
    }

    STDMETHOD(GstEnableTransform)(BOOL b) override { return E_NOTIMPL; }
    STDMETHOD(GstIsTransformEnabled)(BOOL* pIsEnabled) override { return E_NOTIMPL; }
    STDMETHOD(GstSetMatrix)(const D2D1_MATRIX_3X2_F* pMatrix) override { return E_NOTIMPL; }

    STDMETHOD(GstEnableOffset)(BOOL b) override
    {
        m_bTranslate = b;
        return S_OK;
    }
    STDMETHOD(GstIsOffsetEnabled)(BOOL* pIsEnabled) override
    {
        *pIsEnabled = m_bTranslate;
        return S_OK;
    }
    STDMETHOD(GstSetOffset)(float dx, float dy) override
    {
        m_ptOffset = { dx,dy };
        return S_OK;
    }

    // 将裸路径数据写入D2D简单几何接收器
    static void Stream(
        std::span<const Kw::Vec2> spPoint,
        std::span<const PathType> spType,
        _In_ ID2D1SimplifiedGeometrySink* pSink) noexcept
    {
        PathType eLastSegFlags = PathType::SgDefault;
        pSink->SetSegmentFlags(D2D1_PATH_SEGMENT_NONE);
        EckCounter(spPoint.size(), i)
        {
            const D2D1_POINT_2F pt{ spPoint[i].x, spPoint[i].y };
            auto eType = spType[i];
            if (IsBitSet(eType, PathType::FgBegin))
                if (IsBitSet(eType, PathType::FgFill))
                    pSink->BeginFigure(pt, D2D1_FIGURE_BEGIN_FILLED);
                else
                    pSink->BeginFigure(pt, D2D1_FIGURE_BEGIN_HOLLOW);
            else
            {
                const auto eSegFlags = eType & PathType::SgMask;
                if (eSegFlags != eLastSegFlags)
                {
                    eLastSegFlags = eSegFlags;
                    switch (eSegFlags)
                    {
                    case PathType::SgDefault:
                        pSink->SetSegmentFlags(D2D1_PATH_SEGMENT_NONE);
                        break;
                    case PathType::SgUnstroked:
                        pSink->SetSegmentFlags(D2D1_PATH_SEGMENT_FORCE_UNSTROKED);
                        break;
                    case PathType::SgRoundJoin:
                        pSink->SetSegmentFlags(D2D1_PATH_SEGMENT_FORCE_ROUND_LINE_JOIN);
                        break;
                    }
                }

                switch (eType & ~PathType::FgMask)
                {
                case PathType::GmLine:
                    pSink->AddLines(&pt, 1);
                    break;
                case PathType::GmBezier:
                    pSink->AddBeziers((D2D1_BEZIER_SEGMENT*)&spPoint[i], 1);
                    i += 2;
                    eType = spType[i];
                    break;
                }

                if (IsBitSet(eType, PathType::FgEnd))
                    if (IsBitSet(eType, PathType::FgClose))
                        pSink->EndFigure(D2D1_FIGURE_END_CLOSED);
                    else
                        pSink->EndFigure(D2D1_FIGURE_END_OPEN);
            }
        }
    }

    // 将路径内容写入D2D简单几何接收器
    // WARNING D2D实现要求在第一次BeginFigure前设置填充模式，此工作由调用方负责
    void Stream(_In_ ID2D1SimplifiedGeometrySink* pSink) const noexcept
    {
        Stream(m_vPoint.ToSpan(), m_vType.ToSpan(), pSink);
    }

    void Flatten(
        CTrivialBuffer<Kw::Vec2>& vPoint,
        CTrivialBuffer<PathType>& vType,
        float fTolerance = Kw::DefaultHfdTolerance) const noexcept
    {
        if (m_vPoint.IsEmpty() || m_vType.IsEmpty())
            return;
        EckCounter(m_vPoint.Size(), i)
        {
            const auto eType = m_vType[i];
            if (IsBitSet(eType, PathType::GmBezier))
            {
                EckAssert(i + 2 < m_vPoint.Size());// 曲线不完整
                EckAssert(i);// 起点缺失

                const auto cPreOp = vPoint.Size();
                Kw::CHfdCubicBezier::Flatten(
                    vPoint,
                    m_vPoint[i - 1],
                    m_vPoint[i + 0],
                    m_vPoint[i + 1],
                    m_vPoint[i + 2],
                    fTolerance);

                const auto eBezierType = eType & ~PathType::GmBezier;
                const auto eNewType = (m_bUseBezierSegFlags ?
                    ((eBezierType & ~PathType::SgMask) | m_eBezierSegFlags) :
                    eBezierType);

                vType.PushBackMultiple(
                    PathType::GmLine | eNewType,
                    vPoint.Size() - cPreOp);
                if (m_bUseBezierSegFlags)// 第一个点仍继承贝塞尔曲线的原联接标志
                    vType[cPreOp] = eBezierType;

                // 如果贝塞尔曲线终点是子图形终点，则复制它的标志
                if (IsBitSet(m_vType[i + 2], PathType::FgEnd))
                    vType.Back() |= (m_vType[i + 2] & PathType::FgMask);
                i += 2;// 跳过本段使用的3个点
            }
            else
            {
                vPoint.PushBack(m_vPoint[i]);
                vType.PushBack(eType);
            }
        }
    }

    // 函数清除Data的内容
    void Flatten(
        PORTABLE_DATA& Data,
        float fTolerance = Kw::DefaultHfdTolerance) const noexcept
    {
        Data.fTolerance = fTolerance;
        Data.eFillMode = (D2D1_FILL_MODE)m_eFillMode;
        Data.bBoundsValid = FALSE;
        if (HasBezier() && !m_vPoint.IsEmpty())
        {
            Data.vPoint.Clear();
            Data.vType.Clear();
            Flatten(Data.vPoint, Data.vType, fTolerance);
        }
        else
        {
            Data.vPoint = m_vPoint;
            Data.vType = m_vType;
        }
    }

    EckInlineNdCe D2D1_FILL_MODE GetFillMode() const noexcept { return (D2D1_FILL_MODE)m_eFillMode; }
    EckInlineNdCe D2D1_PATH_SEGMENT GetSegmentFlags() const noexcept
    {
        switch (m_eSegFlags)
        {
        case PathType::SgUnstroked:
            return D2D1_PATH_SEGMENT_FORCE_UNSTROKED;
        case PathType::SgRoundJoin:
            return D2D1_PATH_SEGMENT_FORCE_ROUND_LINE_JOIN;
        default:
            return D2D1_PATH_SEGMENT_NONE;
        }
    }

    EckInlineCe void SetBezierFlattenSegmentFlags(D2D1_PATH_SEGMENT eSegOpt) noexcept
    {
        switch (eSegOpt)
        {
        case D2D1_PATH_SEGMENT_FORCE_UNSTROKED:
            m_eBezierSegFlags = PathType::SgUnstroked;
            break;
        case D2D1_PATH_SEGMENT_FORCE_ROUND_LINE_JOIN:
            m_eBezierSegFlags = PathType::SgRoundJoin;
            break;
        default:
            m_eBezierSegFlags = PathType::SgDefault;
            break;
        }
    }
    EckInlineCe void SetUseBezierFlattenSegmentFlags(BOOL b) noexcept { m_bUseBezierSegFlags = b; }
    EckInlineNdCe BOOL GetUseBezierFlattenSegmentFlags() const noexcept { return m_bUseBezierSegFlags; }

    EckInlineNdCe BOOL HasBezier() const noexcept { return m_bHasBezier; }

    EckInlineNdCe auto& DbgGetPointList() noexcept { return m_vPoint; }
    EckInlineNdCe auto& DbgGetTypeList() noexcept { return m_vType; }
};

// 基于CGeometryPath的展平结果，为ITessellationHost的多边形访问部分提供默认实现
struct CGeometryPathTessellationHost :
    public CGeometryPath::PORTABLE_DATA,
    public SkiaTessellator::ITessellationHost
{
    virtual ~CGeometryPathTessellationHost() = default;

    STDMETHOD_(BOOL, PaIsEmpty)() noexcept override { return vPoint.IsEmpty(); }

    STDMETHOD_(pk::SkPathFillType, PaGetFillType)() noexcept override
    {
        if (eFillMode == D2D1_FILL_MODE_ALTERNATE)
            return pk::SkPathFillType::kEvenOdd;
        else
            return pk::SkPathFillType::kWinding;
    }

    STDMETHOD(PaGetFlattenData)(
        _Out_opt_ pk::SkPoint const** ppt,
        _Out_opt_ SkiaTessellator::PathType const** ppeType,
        _Out_ UINT* pcPoint) noexcept override
    {
        if (ppt)
            *ppt = (pk::SkPoint*)vPoint.Data();
        if (ppeType)
            *ppeType = (SkiaTessellator::PathType*)vType.Data();
        *pcPoint = (UINT)vPoint.Size();
        return S_OK;
    }

    STDMETHOD(PaGetBounds)(_Out_ pk::SkRect* prc) noexcept override
    {
        if (!bBoundsValid)
        {
            rcBounds = { FLT_MAX, FLT_MAX, -FLT_MAX, -FLT_MAX };
            for (auto& e : vPoint)
            {
                rcBounds.left = std::min(rcBounds.left, e.x);
                rcBounds.top = std::min(rcBounds.top, e.y);
                rcBounds.right = std::max(rcBounds.right, e.x);
                rcBounds.bottom = std::max(rcBounds.bottom, e.y);
            }
            bBoundsValid = TRUE;
        }
        *prc = *(pk::SkRect*)&rcBounds;
        return S_OK;
    }

    /*
        STDMETHOD(TslReserveVertex)(UINT cVert) noexcept override
        { return S_OK; }
        STDMETHOD(TslReserveIndex)(UINT cIdx) noexcept override
        { return S_OK; }

        STDMETHOD_(void, TslAppendVertex)(float x, float y, float fAlpha) noexcept override
        {}
        STDMETHOD_(void, TslAppendTriangle)(UINT i0, UINT i1, UINT i2) noexcept override
        {}

        STDMETHOD_(UINT, TslGetVertexCount)() noexcept override
        {}
    */
};

EckInline void Tessellate(
    SkiaTessellator::ITessellationHost* pHost, BOOL bAntiAlias = TRUE) noexcept
{
    if (bAntiAlias)
        pk::GrAATriangulator::PathToAATriangles(pHost);
    else
    {
        bool b;
        pk::GrTriangulator::PathToTriangles(pHost, &b);
    }
}
ECK_NAMESPACE_END