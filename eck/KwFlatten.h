#pragma once
#include "KwDef.h"
#include "CTrivialBuffer.h"

ECK_NAMESPACE_BEGIN
KW2D_NAMESPACE_BEGIN
constexpr inline float DefaultHfdTolerance = 0.4f;
constexpr inline float DefaultTolerance = 0.25f;

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
public:
    static void Flatten(
        CTrivialBuffer<Vec2>& vResult,
        const Vec2& b0, const Vec2& b1,
        const Vec2& b2, const Vec2& b3,
        T fTolerance = DefaultHfdTolerance) noexcept
    {
        vResult.PushBack(b0);
        fTolerance *= 6.f;
        const auto fMiniTol = fTolerance / 4.f;
        CHfdCubicBezier s{ b0, b1, b2, b3 };

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

    EckInlineNdCe BOOL IsFlatEnough(T Tol) const noexcept
    {
        return std::max(Abs(e2.x), Abs(e2.y)) <= Tol;
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
public:
    static void Flatten(
        CTrivialBuffer<Vec2>& vResult,
        const Vec2& b0, const Vec2& b1, const Vec2& b2,
        T fTolerance = DefaultHfdTolerance) noexcept
    {
        vResult.PushBack(b0);
        fTolerance *= 6.f;
        CHfdQuadraticBezier s{ b0, b1, b2 };

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


inline void FlattenArc(
    CTrivialBuffer<Vec2>& vResult,
    Vec2 ptCenter, float r,
    double agStart, double agSweep,
    float fTolerance = DefaultTolerance) noexcept
{
    const auto e = fTolerance / r;
    const auto dt = sqrt(48.f * e / (6.f - e));
    const auto n = std::max(1u, UINT(ceil(abs(agSweep) / dt)));
    const auto fTheta = agSweep / n;

    const auto fCosT = cos(fTheta);
    const auto fSinT = sin(fTheta);
    auto fCos = cos(agStart);
    auto fSin = sin(agStart);

    auto p = vResult.PushBackSize(n + 1);

    for (UINT i = 0; i <= n; ++i)
    {
        *p++ =
        {
            float(ptCenter.x + r * fCos),
            float(ptCenter.y + r * fSin)
        };
        const auto nc = fCos * fCosT - fSin * fSinT;
        const auto ns = fCos * fSinT + fSin * fCosT;
        fCos = nc;
        fSin = ns;
    }
}

inline void FlattenEllipticalArc(
    CTrivialBuffer<Vec2>& vResult,
    Vec2 ptCenter, float a, float b,
    double agStart, double agSweep,
    float fTolerance = DefaultTolerance) noexcept
{
    const auto B2 = b * b;
    const auto A2MinusB2 = a * a - B2;
    const auto fFactor = sqrtf(8.f * fTolerance / (a * b));

    auto t = agStart;
    const auto tEnd = agStart + agSweep;
    auto fSin = (float)sin(agStart);
    auto fCos = (float)cos(agStart);
    vResult.PushBack(
        {
            ptCenter.x + a * fCos,
            ptCenter.y + b * fSin
        });

    BOOL bExit{};
    EckLoop()
    {
        const auto D = B2 + A2MinusB2 * fSin * fSin;
        const auto dt = fFactor * (float)sqrt(sqrt(D));

        if (agSweep > 0.f)
        {
            t += dt;
            if (t > tEnd)
            {
                t = tEnd;
                bExit = TRUE;
            }
        }
        else
        {
            t -= dt;
            if (t < tEnd)
            {
                t = tEnd;
                bExit = TRUE;
            }
        }

        fSin = (float)sin(t);
        fCos = (float)cos(t);
        vResult.PushBack(
            {
                ptCenter.x + a * fCos,
                ptCenter.y + b * fSin
            });

        if (bExit)
            break;
    }
}
KW2D_NAMESPACE_END
ECK_NAMESPACE_END