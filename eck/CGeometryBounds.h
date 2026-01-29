#pragma once
#include "KwDef.h"
#include "MathHelper.h"

ECK_NAMESPACE_BEGIN
class CGeometryBounds
{
private:
    Kw::Rect m_rcBounds{ FLT_MAX, FLT_MAX, -FLT_MAX, -FLT_MAX };

    float Bzr3Evaluate(
        float p0, float p1, float p2, float p3, float t) noexcept
    {
        const auto u = 1.f - t;
        return p0 * u * u * u
            + p1 * 3.f * u * u * t
            + p2 * 3.f * u * t * t
            + p3 * t * t * t;
    }

    Kw::Vec2 Bzr3CalculateExtrema(
        float p0, float p1, float p2, float p3) noexcept
    {
        const auto a = -p0 + 3.f * p1 - 3.f * p2 + p3;
        const auto b = 2.f * (p0 - 2.f * p1 + p2);
        const auto c = -p0 + p1;

        if (fabsf(a) < 1e-6f)// a = 0，退化为一次方程
        {
            if (fabsf(b) < 1e-6f)// a = 0且b = 0，无解
                return {};
            return { -c / b };
        }
        const auto d = b * b - 4.f * a * c;
        if (d < 0.f)// delta < 0，无解
            return {};
        const auto s = sqrtf(d);
        const auto t1 = (-b + s) / (2.f * a);
        const auto t2 = (-b - s) / (2.f * a);
        Kw::Vec2 r{};
        if (t1 > 0.f && t1 < 1.f)
            r.x = t1;
        if (t2 > 0.f && t2 < 1.f)
            r.y = t2;
        return r;
    }

    float Bzr2Evaluate(float p0, float p1, float p2, float t) noexcept
    {
        const auto u = 1.f - t;
        return p0 * u * u + p1 * 2.f * u * t + p2 * t * t;
    }

    float Bzr2CalculateExtrema(float p0, float p1, float p2) noexcept
    {
        const auto m = p0 - 2.f * p1 + p2;
        if (fabs(m) < 1e-6f)
            return 0.f;
        const auto t = (p0 - p1) / m;
        if (t > 0.f && t < 1.f)
            return t;
        return 0.f;
    }
public:
    EckInline void AddPoint(Kw::Vec2 pt) noexcept
    {
        if (pt.x < m_rcBounds.left)
            m_rcBounds.left = pt.x;
        if (pt.x > m_rcBounds.right)
            m_rcBounds.right = pt.x;
        if (pt.y < m_rcBounds.top)
            m_rcBounds.top = pt.y;
        if (pt.y > m_rcBounds.bottom)
            m_rcBounds.bottom = pt.y;
    }

    void AddPoint(_In_reads_(cPt) const Kw::Vec2* pPt, size_t cPt) noexcept
    {
        EckCounter(cPt, i)
            AddPoint(pPt[i]);
    }

    void AddLine(Kw::Vec2 pt1, Kw::Vec2 pt2) noexcept
    {
        AddPoint(pt1);
        AddPoint(pt2);
    }

    void AddRect(const Kw::Rect& rc) noexcept
    {
        if (rc.left < m_rcBounds.left)
            m_rcBounds.left = rc.left;
        if (rc.right > m_rcBounds.right)
            m_rcBounds.right = rc.right;
        if (rc.top < m_rcBounds.top)
            m_rcBounds.top = rc.top;
        if (rc.bottom > m_rcBounds.bottom)
            m_rcBounds.bottom = rc.bottom;
    }

    void AddCubicBezier(Kw::Vec2 p0, Kw::Vec2 p1, Kw::Vec2 p2, Kw::Vec2 p3) noexcept
    {
        // 端点
        auto x0 = p0.x;
        auto y0 = p0.y;
        auto x1 = p3.x;
        auto y1 = p3.y;
        if (x0 > x1)
            std::swap(x0, x1);
        if (y0 > y1)
            std::swap(y0, y1);

        if (x0 < m_rcBounds.left)
            m_rcBounds.left = x0;
        if (x1 > m_rcBounds.right)
            m_rcBounds.right = x1;
        if (y0 < m_rcBounds.top)
            m_rcBounds.top = y0;
        if (y1 > m_rcBounds.bottom)
            m_rcBounds.bottom = y1;
        // 极值
        const auto ex = Bzr3CalculateExtrema(p0.x, p1.x, p2.x, p3.x);
        const auto ey = Bzr3CalculateExtrema(p0.y, p1.y, p2.y, p3.y);

        if (ex.x)
        {
            const auto x = Bzr3Evaluate(p0.x, p1.x, p2.x, p3.x, ex.x);
            if (x < m_rcBounds.left)
                m_rcBounds.left = x;
            if (x > m_rcBounds.right)
                m_rcBounds.right = x;
        }
        if (ex.y)
        {
            const auto x = Bzr3Evaluate(p0.x, p1.x, p2.x, p3.x, ex.y);
            if (x < m_rcBounds.left)
                m_rcBounds.left = x;
            if (x > m_rcBounds.right)
                m_rcBounds.right = x;
        }
        if (ey.x)
        {
            const auto y = Bzr3Evaluate(p0.y, p1.y, p2.y, p3.y, ey.x);
            if (y < m_rcBounds.top)
                m_rcBounds.top = y;
            if (y > m_rcBounds.bottom)
                m_rcBounds.bottom = y;
        }
        if (ey.y)
        {
            const auto y = Bzr3Evaluate(p0.y, p1.y, p2.y, p3.y, ey.y);
            if (y < m_rcBounds.top)
                m_rcBounds.top = y;
            if (y > m_rcBounds.bottom)
                m_rcBounds.bottom = y;
        }
    }

    void AddQuadraticBezier(Kw::Vec2 p0, Kw::Vec2 p1, Kw::Vec2 p2) noexcept
    {
        // 端点
        auto x0 = p0.x;
        auto y0 = p0.y;
        auto x1 = p2.x;
        auto y1 = p2.y;
        if (x0 > x1)
            std::swap(x0, x1);
        if (y0 > y1)
            std::swap(y0, y1);

        if (x0 < m_rcBounds.left)
            m_rcBounds.left = x0;
        if (x1 > m_rcBounds.right)
            m_rcBounds.right = x1;
        if (y0 < m_rcBounds.top)
            m_rcBounds.top = y0;
        if (y1 > m_rcBounds.bottom)
            m_rcBounds.bottom = y1;
        // 极值
        const auto ex = Bzr2CalculateExtrema(p0.x, p1.x, p2.x);
        const auto ey = Bzr2CalculateExtrema(p0.y, p1.y, p2.y);

        if (ex)
        {
            const auto x = Bzr2Evaluate(p0.x, p1.x, p2.x, ex);
            if (x < m_rcBounds.left)
                m_rcBounds.left = x;
            if (x > m_rcBounds.right)
                m_rcBounds.right = x;
        }
        if (ey)
        {
            const auto y = Bzr2Evaluate(p0.y, p1.y, p2.y, ey);
            if (y < m_rcBounds.top)
                m_rcBounds.top = y;
            if (y > m_rcBounds.bottom)
                m_rcBounds.bottom = y;
        }
    }

    void AddArc(Kw::Vec2 ptCenter, float a, float b,
        TAngle agStart, TAngle agSweep) noexcept
    {
        if (fabsf(agSweep) >= Pi * 2.f)
        {
            AddRect({
                    ptCenter.x - a, ptCenter.y - b,
                    ptCenter.x + a, ptCenter.y + b
                });
            return;
        }

        // 端点
        float x0, y0;
        CalculatePointFromEllipseAngle(a, b, agStart, x0, y0);
        float x1, y1;
        CalculatePointFromEllipseAngle(a, b, agStart + agSweep, x1, y1);
        x0 += ptCenter.x;
        y0 += ptCenter.y;
        x1 += ptCenter.x;
        y1 += ptCenter.y;
        if (x0 > x1)
            std::swap(x0, x1);
        if (y0 > y1)
            std::swap(y0, y1);
        if (x0 < m_rcBounds.left)
            m_rcBounds.left = x0;
        if (x1 > m_rcBounds.right)
            m_rcBounds.right = x1;
        if (y0 < m_rcBounds.top)
            m_rcBounds.top = y0;
        if (y1 > m_rcBounds.bottom)
            m_rcBounds.bottom = y1;
        // 极值
        constexpr TAngle ExtremeAngles[]
        {
            0,
            Pi / 2,
            Pi,
            3 * Pi / 2
        };

        for (const auto e : ExtremeAngles)
        {
            if (!RadianInArc(e, agStart, agSweep))
                continue;
            // 特殊位置，无需计算离心角
            AddPoint({
                    a * cosf(e) + ptCenter.x,
                    b * sinf(e) + ptCenter.y
                });
        }
    }

    EckInlineNdCe auto& GetBounds() const noexcept { return m_rcBounds; }

    EckInlineCe void Reset() noexcept
    {
        m_rcBounds = { FLT_MAX, FLT_MAX, -FLT_MAX, -FLT_MAX };
    }
};
ECK_NAMESPACE_END