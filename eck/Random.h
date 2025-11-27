#pragma once
#include "ECK.h"

ECK_NAMESPACE_BEGIN
class CRmPcg32
{
public:
    using TSeed = UINT64;

    constexpr static UINT64 Multiplier = 6364136223846793005ull;
    constexpr static UINT64 Increment = 1442695040888963407ull;
private:
    UINT64 State;
public:
    CRmPcg32(UINT64 Seed = NtGetTickCount64()) noexcept : State{ Seed + Increment } {}

    constexpr UINT Next32() noexcept
    {
        const auto Old = State;
        State = Old * Multiplier + Increment;
        const auto XorShifted = UINT((Old ^ (Old >> 18)) >> 27);
        const auto Rot = UINT(Old >> 59);
#pragma warning(suppress:4146)// 一元负运算符应用于无符号类型，结果仍为无符号类型
        return XorShifted >> Rot | XorShifted << (-Rot & 31);
    }

    constexpr void Seed(UINT64 Seed) noexcept { State = Seed + Increment; }
};

class CRmXorShift32
{
public:
    using TSeed = UINT;
private:
    UINT State;
public:
    CRmXorShift32(UINT Seed = NtGetTickCount()) noexcept : State{ Seed ? Seed : 1 } {}

    constexpr UINT Next32() noexcept
    {
        auto x = State;
        x ^= x << 13;
        x ^= x >> 17;
        x ^= x << 5;
        State = x;
        return x;
    }

    constexpr void Seed(UINT32 SeedVal) noexcept { State = SeedVal ? SeedVal : 1; }
};


template<class TBase>
struct CRandom : public TBase
{
    using TBase::Next32;

    template<std::integral T = UINT>
    constexpr T Next() noexcept
    {
        if constexpr (sizeof(T) == 64)
            return (Next32() << 32) | Next32();
        else
            return (T)Next32();
    }
    template<std::integral T>
    constexpr T Next(T Min, T Max) noexcept
    {
        using TUnsigned = std::make_unsigned_t<T>;
        using TLimits = std::numeric_limits<TUnsigned>;

        const auto Span = (TUnsigned)Max - (TUnsigned)Min + 1;
        const auto Limit = TLimits::max() - (TLimits::max() % Span);
        TUnsigned r;
        do
        {
            r = Next<TUnsigned>();
        } while (r >= Limit);
        return Min + T(r % Span);
    }
    template<std::floating_point T>
    constexpr T Next(T Min, T Max) noexcept
    {
        if constexpr (std::is_same_v<T, float>)
        {
            const auto r = Next32() >> 8;
            return Min + (r / 16777216.f/* 2^24 */) * (Max - Min);
        }
        else
        {
            const auto r = ((UINT64)Next32() << 21) ^ (Next32() >> 11);
            return Min + (r / 9007199254740992./* 2^53 */) * (Max - Min);
        }
    }

    template<std::floating_point T>
    constexpr T Triangular(T Min, T Mode, T Max) noexcept
    {
        const auto u = Next(T(0), T(1));
        const auto c = (Mode - Min) / (Max - Min);
        if (u < c)
            return Min + (T)std::sqrt(u * (Max - Min) * (Mode - Min));
        else
            return Max - (T)std::sqrt((T(1) - u) * (Max - Min) * (Max - Mode));
    }
    // 均值为0，标准差为1
    template<std::floating_point T>
    T Gaussian() noexcept
    {
        auto u1 = Next(T(0), T(1));
        const auto u2 = Next(T(0), T(1));
        if (u1 < T(1e-16))
            u1 = T(1e-16);
        return T(sqrt(-2. * log(u1)) * cos(2. * 3.141592653589793 * u2));
    }
    template<std::floating_point T>
    constexpr T Gaussian(T Mean, T Dev) noexcept { return Mean + Dev * Gaussian<T>(); }
};

using CPcg32 = CRandom<CRmPcg32>;
using CXorShift32 = CRandom<CRmXorShift32>;
ECK_NAMESPACE_END