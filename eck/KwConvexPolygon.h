#pragma once
#include "KwFlatten.h"

#include <DirectXMath.h>

ECK_NAMESPACE_BEGIN
KW2D_NAMESPACE_BEGIN
using namespace DirectX;

struct ConvexPolygon
{
    CTrivialBuffer<Vec2> vPoint{};

    EckInline void Begin(Vec2 ptBegin) noexcept
    {
        vPoint.ReSizeExtra(1);
        vPoint[0] = ptBegin;
    }
    EckInline void AddPoint(Vec2 pt) noexcept
    {
        vPoint.PushBack(pt);
    }

    EckInline void MakeLine(Vec2 pt1, Vec2 pt2) noexcept
    {
        vPoint.ReSizeExtra(2);
        vPoint[0] = pt1;
        vPoint[1] = pt2;
    }
    EckInline void MakeTriangle(Vec2 pt1, Vec2 pt2, Vec2 pt3) noexcept
    {
        vPoint.ReSizeExtra(3);
        vPoint[0] = pt1;
        vPoint[1] = pt2;
        vPoint[2] = pt3;
    }
    EckInline void MakeRect(const Rect& rc) noexcept
    {
        vPoint.ReSizeExtra(4);
        vPoint[0] = { rc.left, rc.top };
        vPoint[1] = { rc.right, rc.top };
        vPoint[2] = { rc.right, rc.bottom };
        vPoint[3] = { rc.left, rc.bottom };
    }

    // WARNING 如果使用此函数创建完整圆，需要弹出最后一点
    void AddArcAsBezier(
        Vec2 ptCenter, float a, float b,
        float agStart, float agSweep,
        float fTolerance = DefaultHfdTolerance) noexcept
    {
        const auto n = (int)ceilf(fabsf(agSweep) / XM_PIDIV2);
        const auto d = agSweep / n;
        const auto k = 4.f / 3.f * tanf(d * 0.25f);

        float fCosT1 = cosf(agStart), fSinT1 = sinf(agStart);

        EckCounter(n, i)
        {
            const auto t1 = agStart + i * d;
            const auto t2 = t1 + d;

            const auto fCosT2 = cosf(t2);
            const auto fSinT2 = sinf(t2);

            const Vec2 p0{ ptCenter.x + a * fCosT1, ptCenter.y + b * fSinT1 };
            const Vec2 p3{ ptCenter.x + a * fCosT2, ptCenter.y + b * fSinT2 };

            const Vec2 d0{ -a * fSinT1, b * fCosT1 };
            const Vec2 d1{ -a * fSinT2, b * fCosT2 };

            CHfdCubicBezier::Flatten(
                vPoint,
                p0,
                p0 + d0 * k,
                p3 - d1 * k,
                p3,
                fTolerance);
            // 如果下一段曲线与当前曲线连接，弹掉最后一点
            if (i != n - 1)
                vPoint.PopBack();

            fCosT1 = fCosT2;
            fSinT1 = fSinT2;
        }
    }

    // WARNING 如果使用此函数创建完整圆，需要弹出最后一点
    void AddArc(
        Vec2 ptCenter, float r,
        float agStart, float agSweep,
        float fTolerance = DefaultTolerance) noexcept
    {
        FlattenArc(vPoint, ptCenter, r, agStart, agSweep, fTolerance);
    }

    EckInline void MakeArcAsBezier(
        Vec2 ptCenter, float a, float b,
        float agStart, float agSweep,
        float fTolerance = DefaultHfdTolerance) noexcept
    {
        vPoint.Clear();
        AddArcAsBezier(ptCenter, a, b, agStart, agSweep, fTolerance);
    }
    EckInline void MakeArc(
        Vec2 ptCenter, float r,
        float agStart, float agSweep,
        float fTolerance = DefaultTolerance) noexcept
    {
        vPoint.Clear();
        AddArc(ptCenter, r, agStart, agSweep, fTolerance);
    }

    EckInline void MakeEllipse(
        Vec2 ptCenter, float a, float b,
        float fTolerance = DefaultHfdTolerance) noexcept
    {
        vPoint.Clear();
        AddArcAsBezier(ptCenter, a, b, 0, XM_2PI, fTolerance);
        vPoint.PopBack();// 最后一点不需要
    }
    EckInline void MakeCircle(
        Vec2 ptCenter, float r,
        float fTolerance = DefaultTolerance) noexcept
    {
        vPoint.Clear();
        AddArc(ptCenter, r, 0, XM_2PI, fTolerance);
        vPoint.PopBack();// 最后一点不需要
    }
    EckInline void MakeRoundRect(
        const Rect& rc, float r,
        float fTolerance = DefaultTolerance) noexcept
    {
        vPoint.Clear();
        AddArc({ rc.right - r, rc.top + r }, r, -XM_PIDIV2, XM_PIDIV2, fTolerance);
        AddArc({ rc.right - r, rc.bottom - r }, r, 0.f, XM_PIDIV2, fTolerance);
        AddArc({ rc.left + r , rc.bottom - r }, r, XM_PIDIV2, XM_PIDIV2, fTolerance);
        AddArc({ rc.left + r , rc.top + r }, r, XM_PI, XM_PIDIV2, fTolerance);
    }
};
KW2D_NAMESPACE_END
ECK_NAMESPACE_END