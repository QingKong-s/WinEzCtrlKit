#pragma once
#include "KwDef.h"
#include "CTrivialBuffer.h"

ECK_NAMESPACE_BEGIN
KW2D_NAMESPACE_BEGIN
// https://patents.google.com/patent/EP0577131B1/en

class CHfdCubicBezier
{
public:
    using T = float;
private:
    Vec2 e0;
    Vec2 e1;
    Vec2 e2;
    Vec2 e3;

    constexpr CHfdCubicBezier(
        const Vec2& b0, const Vec2& b1,
        const Vec2& b2, const Vec2& b3) noexcept :
        e0{ b0 },
        e1{ b3 - b0 },
        e2{ (b3 - b2 * 2.f + b1) * 6.f },
        e3{ (b2 - b1 * 2.f + b0) * 6.f }
    {
    }

    EckInlineNdCe static T Abs(T f) noexcept
    {
        return f >= 0 ? f : -f;
    }

    EckInlineNdCe BOOL IsFlatEnough(T Tol) const noexcept
    {
        return std::max(
            std::max(Abs(e2.x), Abs(e2.y)),
            std::max(Abs(e3.x), Abs(e3.y))) <= Tol;
    }

    EckInlineNdCe BOOL IsLastFlatEnough(T Tol) const noexcept
    {
        const auto oe2 = e3;
        const auto oe3 = e2 + e2 - e3;
        return std::max(
            std::max(Abs(oe2.x), Abs(oe2.y)),
            std::max(Abs(oe3.x), Abs(oe3.y))) <= Tol;
    }

    EckInlineCe void AdjustDown() noexcept
    {
        const auto ne1 = (e1 * 0.5f) - (e2 * 0.0625f) - (e3 * 0.0625f);
        const auto ne2 = (e2 * 0.125f) + (e3 * 0.125f);
        const auto ne3 = e3 * 0.25f;
        e1 = ne1;
        e2 = ne2;
        e3 = ne3;
    }

    EckInlineCe void StepForward() noexcept
    {
        e0 += e1;
        e1 += e2;
        const auto oe2 = e2;
        e2 += (oe2 - e3);
        e3 = oe2;
    }

    EckInlineCe void AdjustUp() noexcept
    {
        const auto ne1 = e1 * 2.f + e2;
        const auto ne2 = e2 * 8.f - e3 * 4.f;
        const auto ne3 = e3 * 4.f;
        e1 = ne1;
        e2 = ne2;
        e3 = ne3;
    }

    EckInlineNdCe static Vec2 PtToVec2(auto pt) noexcept
    {
        const auto [x, y] = pt;
        return { x,y };
    }
public:
    template<class TPt>
    static void Flatten(
        const TPt& b0, const TPt& b1,
        const TPt& b2, const TPt& b3,
        T fTolerance,
        CTrivialBuffer<TPt>& vResult) noexcept
    {
        vResult.PushBack(b0);
        fTolerance *= 6.f;
        const auto fMiniTol = fTolerance / 4.f/*AdjustUp*/;
        CHfdCubicBezier s{ PtToVec2(b0), PtToVec2(b1), PtToVec2(b2), PtToVec2(b3) };

        UINT cStep = 1u;
        EckLoop()
        {
            // 误差过大，下调为f(t/2)
            if (!s.IsFlatEnough(fTolerance))
            {
                s.AdjustDown();
                cStep <<= 1u;
                continue;
            }
            // 误差过小，上调为f(2t)
            while (!(cStep & 1u) && s.IsLastFlatEnough(fMiniTol))
            {
                s.AdjustUp();
                cStep >>= 1u;
            }
            // 接受此点
            if (--cStep)
            {
                s.StepForward();
                vResult.PushBack({ s.e0.x, s.e0.y });
            }
            else// 展平过程中误差累积，导致最后一点与p3有微小偏移
            {
                vResult.PushBack(b3);
                break;
            }
        }
    }
};

class CHfdQuadraticBezier
{
public:
    using T = float;
private:
    Vec2 e0;
    Vec2 e1;
    Vec2 e2;

    constexpr CHfdQuadraticBezier(
        const Vec2& b0,
        const Vec2& b1,
        const Vec2& b2) noexcept :
        e0{ b0 },
        e1{ b2 - b0 },
        e2{ (b2 - b1 * 2.0f + b0) * 2.f }
    {
    }

    EckInlineNdCe static T Abs(T v) noexcept
    {
        return v >= 0 ? v : -v;
    }

    EckInlineNdCe BOOL IsFlatEnough(T tol) const noexcept
    {
        return std::max(Abs(e2.x), Abs(e2.y)) <= tol;
    }

    EckInlineCe void AdjustDown() noexcept
    {
        const auto ne1 = e1 * 0.5f - e2 * 0.125f;
        const auto ne2 = e2 * 0.25f;
        e1 = ne1;
        e2 = ne2;
    }

    EckInlineCe void StepForward() noexcept
    {
        e0 += e1;
        e1 += e2;
    }

    EckInlineCe void AdjustUp() noexcept
    {
        const auto ne1 = e1 * 2.f + e2;
        const auto ne2 = e2 * 4.f;
        e1 = ne1;
        e2 = ne2;
    }

    EckInlineNdCe static Vec2 PtToVec2(auto pt) noexcept
    {
        const auto [x, y] = pt;
        return { x, y };
    }
public:
    template<class TPt>
    static void Flatten(
        const TPt& b0, const TPt& b1, const TPt& b2,
        T fTolerance,
        CTrivialBuffer<TPt>& vResult) noexcept
    {
        vResult.PushBack(b0);
        fTolerance *= 6.f;
        CHfdQuadraticBezier s{ PtToVec2(b0), PtToVec2(b1), PtToVec2(b2) };

        UINT cStep = 1u;
        EckLoop()
        {
            if (!s.IsFlatEnough(fTolerance))
            {
                s.AdjustDown();
                cStep <<= 1u;
                continue;
            }

            if (--cStep)
            {
                s.StepForward();
                vResult.PushBack({ s.e0.x, s.e0.y });
            }
            else
            {
                vResult.PushBack(b2);
                break;
            }
        }
    }
};
KW2D_NAMESPACE_END
ECK_NAMESPACE_END