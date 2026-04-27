#pragma once
#include "ITimeLine.h"
#include "CUnknown.h"

#define ECK_EASING_NAMESPACE_BEGIN namespace Easing {
#define ECK_EASING_NAMESPACE_END }

ECK_NAMESPACE_BEGIN
ECK_EASING_NAMESPACE_BEGIN
using FAn = float(*)(float t, float x0, float dx, float dt) noexcept;

struct FInCircle
{
    EckInline float operator()(float t, float x0, float dx, float dt) const noexcept
    {
        t /= dt;
        return -dx * (sqrt(1.f - t * t) - 1.f) + x0;
    }
};

EckInline float InCircle(float t, float x0, float dx, float dt) noexcept
{
    return FInCircle{}(t, x0, dx, dt);
}

struct FOutCircle
{
    EckInline float operator()(float t, float x0, float dx, float dt) const noexcept
    {
        t = t / dt - 1.f;
        return dx * sqrt(1 - t * t) + x0;
    }
};

EckInline float OutCircle(float t, float x0, float dx, float dt) noexcept
{
    return FOutCircle{}(t, x0, dx, dt);
}

struct FInOutCircle
{
    EckInline float operator()(float t, float x0, float dx, float dt) const noexcept
    {
        t /= (dt / 2.f);
        if (t < 1.f)
            return -dx / 2.f * (sqrt(1.f - t * t) - 1.f) + x0;
        else
        {
            t -= 2.f;
            return dx / 2.f * (sqrt(1.f - t * t) + 1.f) + x0;
        }
    }
};

EckInline float InOutCircle(float t, float x0, float dx, float dt) noexcept
{
    return FInOutCircle{}(t, x0, dx, dt);
}

struct FInSine
{
    EckInline float operator()(float t, float x0, float dx, float dt) const noexcept
    {
        return -dx * cosf(t / dt * PiF / 2.f) + dx + x0;
    }
};

EckInline float InSine(float t, float x0, float dx, float dt) noexcept
{
    return FInSine{}(t, x0, dx, dt);
}

struct FOutSine
{
    EckInline float operator()(float t, float x0, float dx, float dt) const noexcept
    {
        return dx * sinf(t / dt * PiF / 2.f) + x0;
    }
};

EckInline float OutSine(float t, float x0, float dx, float dt) noexcept
{
    return FOutSine{}(t, x0, dx, dt);
}

struct FInOutSine
{
    EckInline float operator()(float t, float x0, float dx, float dt) const noexcept
    {
        return -dx / 2.f * (cosf(PiF * t / dt) - 1.f) + x0;
    }
};

EckInline float InOutSine(float t, float x0, float dx, float dt) noexcept
{
    return FInOutSine{}(t, x0, dx, dt);
}

struct FInQuad
{
    EckInline float operator()(float t, float x0, float dx, float dt) const noexcept
    {
        t /= dt;
        return dx * t * t + x0;
    }
};

EckInline float InQuad(float t, float x0, float dx, float dt) noexcept
{
    return FInQuad{}(t, x0, dx, dt);
}

struct FOutQuad
{
    EckInline float operator()(float t, float x0, float dx, float dt) const noexcept
    {
        t /= dt;
        return -dx * t * (t - 2.f) + x0;
    }
};

EckInline float OutQuad(float t, float x0, float dx, float dt) noexcept
{
    return FOutQuad{}(t, x0, dx, dt);
}

struct FInOutQuad
{
    EckInline float operator()(float t, float x0, float dx, float dt) const noexcept
    {
        t /= (dt / 2.f);
        if (t < 1.f)
            return dx / 2.f * t * t + x0;
        else
        {
            t -= 1.f;
            return -dx / 2.f * (t * (t - 2.f) - 1.f) + x0;
        }
    }
};

EckInline float InOutQuad(float t, float x0, float dx, float dt) noexcept
{
    return FInOutQuad{}(t, x0, dx, dt);
}

struct FInCubic
{
    EckInline float operator()(float t, float x0, float dx, float dt) const noexcept
    {
        t /= dt;
        return dx * t * t * t + x0;
    }
};

EckInline float InCubic(float t, float x0, float dx, float dt) noexcept
{
    return FInCubic{}(t, x0, dx, dt);
}

struct FOutCubic
{
    EckInline constexpr float operator()(float t, float x0, float dx, float dt) const noexcept
    {
        t = t / dt - 1.f;
        return dx * (t * t * t + 1.f) + x0;
    }
};

EckInline float OutCubic(float t, float x0, float dx, float dt) noexcept
{
    return FOutCubic{}(t, x0, dx, dt);
}

struct FInOutCubic
{
    EckInline float operator()(float t, float x0, float dx, float dt) const noexcept
    {
        t /= (dt / 2.f);
        if (t < 1.f)
            return dx / 2.f * t * t * t + x0;
        else
        {
            t -= 2.f;
            return dx / 2.f * (t * t * t + 2.f) + x0;
        }
    }
};

EckInline float InOutCubic(float t, float x0, float dx, float dt) noexcept
{
    return FInOutCubic{}(t, x0, dx, dt);
}

struct FInQuart
{
    EckInline float operator()(float t, float x0, float dx, float dt) const noexcept
    {
        t /= dt;
        return dx * t * t * t * t + x0;
    }
};

EckInline float InQuart(float t, float x0, float dx, float dt) noexcept
{
    return FInQuart{}(t, x0, dx, dt);
}

struct FOutQuart
{
    EckInline float operator()(float t, float x0, float dx, float dt) const noexcept
    {
        t = t / dt - 1.f;
        return -dx * (t * t * t * t - 1.f) + x0;
    }
};

EckInline float OutQuart(float t, float x0, float dx, float dt) noexcept
{
    return FOutQuart{}(t, x0, dx, dt);
}

struct FInOutQuart
{
    EckInline float operator()(float t, float x0, float dx, float dt) const noexcept
    {
        t /= (dt / 2.f);
        if (t < 1.f)
            return dx / 2.f * t * t * t * t + x0;
        else
        {
            t -= 2.f;
            return -dx / 2.f * (t * t * t * t - 2.f) + x0;
        }
    }
};

EckInline float InOutQuart(float t, float x0, float dx, float dt) noexcept
{
    return FInOutQuart{}(t, x0, dx, dt);
}

struct FInQuint
{
    EckInline float operator()(float t, float x0, float dx, float dt) const noexcept
    {
        t /= dt;
        return dx * t * t * t * t * t + x0;
    }
};

EckInline float InQuint(float t, float x0, float dx, float dt) noexcept
{
    return FInQuint{}(t, x0, dx, dt);
}

struct FOutQuint
{
    EckInline float operator()(float t, float x0, float dx, float dt) const noexcept
    {
        t = t / dt - 1.f;
        return dx * (t * t * t * t * t + 1.f) + x0;
    }
};

EckInline float OutQuint(float t, float x0, float dx, float dt) noexcept
{
    return FOutQuint{}(t, x0, dx, dt);
}

struct FInOutQuint
{
    EckInline float operator()(float t, float x0, float dx, float dt) const noexcept
    {
        t /= (dt / 2.f);
        if (t < 1.f)
            return dx / 2.f * t * t * t * t * t + x0;
        else
        {
            t -= 2.f;
            return dx / 2.f * (t * t * t * t * t + 2.f) + x0;
        }
    }
};

EckInline float InOutQuint(float t, float x0, float dx, float dt) noexcept
{
    return FInOutQuint{}(t, x0, dx, dt);
}

struct FInExpo
{
    EckInline float operator()(float t, float x0, float dx, float dt) const noexcept
    {
        return dx * powf(2.f, 10.f * (t / dt - 1.f)) + x0;
    }
};

EckInline float InExpo(float t, float x0, float dx, float dt) noexcept
{
    return FInExpo{}(t, x0, dx, dt);
}

struct FOutExpo
{
    EckInline float operator()(float t, float x0, float dx, float dt) const noexcept
    {
        return dx * (-powf(2.f, -10.f * t / dt) + 1.f) + x0;
    }
};

EckInline float OutExpo(float t, float x0, float dx, float dt) noexcept
{
    return FOutExpo{}(t, x0, dx, dt);
}

struct FInOutExpo
{
    EckInline float operator()(float t, float x0, float dx, float dt) const noexcept
    {
        t /= (dt / 2.f);
        if (t < 1.f)
            return dx / 2.f * powf(2.f, 10.f * (t - 1.f)) + x0;
        else
            return dx / 2.f * (-powf(2.f, -10.f * (t - 1.f)) + 2.f) + x0;
    }
};

EckInline float InOutExpo(float t, float x0, float dx, float dt) noexcept
{
    return FInOutExpo{}(t, x0, dx, dt);
}

struct FInElastic
{
    EckInline float operator()(float t, float x0, float dx, float dt) const noexcept
    {
        t /= dt;
        const float f = t - 1.f;
        return -dx * powf(2.f, 10.f * f) * sinf((f - 0.075f) * 2.f * PiF / 0.3f) + x0;
    }
};

EckInline float InElastic(float t, float x0, float dx, float dt) noexcept
{
    return FInElastic{}(t, x0, dx, dt);
}

struct FOutElastic
{
    EckInline float operator()(float t, float x0, float dx, float dt) const noexcept
    {
        t /= dt;
        return dx * powf(2.f, -10.f * t) * sinf((t - 0.075f) * 2.f * PiF / 0.3f) + dx + x0;
    }
};

EckInline float OutElastic(float t, float x0, float dx, float dt) noexcept
{
    return FOutElastic{}(t, x0, dx, dt);
}

struct FInOutElastic
{
    EckInline float operator()(float t, float x0, float dx, float dt) const noexcept
    {
        t /= (dt / 2.f);
        const float f = t - 1.f;
        if (t < 1.f)
            return -0.5f * dx * powf(2.f, 10.f * f) * sinf((f - 0.1125f) * 2.f * PiF / 0.45f) + x0;
        else
            return dx * powf(2.f, -10.f * f) * sinf((f - 0.1125f) * 2.f * PiF / 0.45f) * 0.5f + dx + x0;
    }
};

EckInline float InOutElastic(float t, float x0, float dx, float dt) noexcept
{
    return FInOutElastic{}(t, x0, dx, dt);
}

struct FInBack
{
    EckInline float operator()(float t, float x0, float dx, float dt) const noexcept
    {
        const float f = 1.70158f;
        t /= dt;
        return dx * t * t * ((f + 1.f) * t - f) + x0;
    }
};

EckInline float InBack(float t, float x0, float dx, float dt) noexcept
{
    return FInBack{}(t, x0, dx, dt);
}

struct FOutBack
{
    EckInline float operator()(float t, float x0, float dx, float dt) const noexcept
    {
        const float f = 1.70158f;
        t = t / dt - 1.f;
        return dx * (t * t * ((f + 1.f) * t + f) + 1.f) + x0;
    }
};

EckInline float OutBack(float t, float x0, float dx, float dt) noexcept
{
    return FOutBack{}(t, x0, dx, dt);
}

struct FInOutBack
{
    EckInline float operator()(float t, float x0, float dx, float dt) const noexcept
    {
        const float f = 1.70158f * 1.525f;
        t /= (dt / 2.f);
        if (t < 1.f)
            return dx / 2.f * (t * t * ((f + 1.f) * t - f)) + x0;
        else
        {
            t -= 2.f;
            return dx / 2.f * (t * t * ((f + 1.f) * t + f) + 2.f) + x0;
        }
    }
};

EckInline float InOutBack(float t, float x0, float dx, float dt) noexcept
{
    return FInOutBack{}(t, x0, dx, dt);
}

struct FOutBounce
{
    EckInline float operator()(float t, float x0, float dx, float dt) const noexcept
    {
        t /= dt;
        if (t < 1.f / 2.75f)
            return dx * (7.5625f * t * t) + x0;
        else if (t < 2.f / 2.75f)
        {
            t -= 1.5f / 2.75f;
            return dx * (7.5625f * t * t + 0.75f) + x0;
        }
        else if (t < 2.5f / 2.75f)
        {
            t -= 2.25f / 2.75f;
            return dx * (7.5625f * t * t + 0.9375f) + x0;
        }
        else
        {
            t -= 2.625f / 2.75f;
            return dx * (7.5625f * t * t + 0.984375f) + x0;
        }
    }
};

EckInline float OutBounce(float t, float x0, float dx, float dt) noexcept
{
    return FOutBounce{}(t, x0, dx, dt);
}

struct FInBounce
{
    EckInline float operator()(float t, float x0, float dx, float dt) const noexcept
    {
        return dx - FOutBounce{}(dt - t, 0.f, dx, dt) + x0;
    }
};

EckInline float InBounce(float t, float x0, float dx, float dt) noexcept
{
    return FInBounce{}(t, x0, dx, dt);
}

struct FInOutBounce
{
    EckInline float operator()(float t, float x0, float dx, float dt) const noexcept
    {
        if (t < dt / 2.f)
            return FInBounce{}(t * 2.f, 0.f, dx, dt) * 0.5f + x0;
        else
            return FOutBounce{}(t * 2.f - dt, 0.f, dx, dt) * 0.5f + dx * 0.5f + x0;
    }
};

EckInline float InOutBounce(float t, float x0, float dx, float dt) noexcept
{
    return FInOutBounce{}(t, x0, dx, dt);
}

struct FLinear
{
    EckInline float operator()(float t, float x0, float dx, float dt) const noexcept
    {
        return dx * t / dt + x0;
    }
};

EckInline float Linear(float t, float x0, float dx, float dt) noexcept
{
    return FLinear{}(t, x0, dx, dt);
}

struct FPunch
{
    EckInline float operator()(float t, float x0, float dx, float dt) const noexcept
    {
        const float f = 9.f;
        const float f1 = t / dt - 1.f;
        return dx * expf(-f1 * f) * sinf(6.f * PiF * f1) + dx + x0;
    }
};

EckInline float Punch(float t, float x0, float dx, float dt) noexcept
{
    return FPunch{}(t, x0, dx, dt);
}

struct FSpring
{
    EckInline float operator()(float t, float x0, float dx, float dt) const noexcept
    {
        constexpr float f = 0.3f;
        const float g = t / dt - 1.f;
        return -dx * expf(-g * f) * sinf(2.f * PiF * g / 0.3f) + dx + x0;
    }
};

EckInline float Spring(float t, float x0, float dx, float dt) noexcept
{
    return FSpring{}(t, x0, dx, dt);
}
ECK_EASING_NAMESPACE_END

// 表示单条动画曲线
// 调用方通常使用EasingCurve表示曲线当前参数，一个BOOLEAN指示曲线是否正在运行
// 和开始、结束、动画时间等常量描述一个完整的动画曲线
// 本类不存储任何可能为常量的信息，也不存储状态的BOOLEAN变量，有助于节约内存
template<class FAn>
struct EasingCurve
{
    float Time{};
    float Begin{};
    float Dist{};
    float K{};

    /// <summary>
    /// 启动曲线
    /// </summary>
    /// <param name="fBegin">开始位置，若当前曲线正在运行，则以当前位置作为起始位置并忽略此参数</param>
    /// <param name="fEnd">结束位置</param>
    /// <param name="bActive">当前曲线是否正在运行</param>
    EckInlineCe void Start(float fBegin, float fEnd, BOOLEAN bActive = FALSE) noexcept
    {
        Begin = bActive ? K : fBegin;
        Dist = fEnd - Begin;
        Time = 0.f;
    }

    // 返回曲线是否正在运行（TRUE = 正在运行，FALSE = 已结束）
    EckInline BOOLEAN Tick(float fElapse, float fDuration) noexcept
    {
        Time += fElapse;
        K = FAn{}(Time, Begin, Dist, fDuration);
        return !(Dist > 0.f ? K >= Begin + Dist : K <= Begin + Dist);
    }
};
ECK_NAMESPACE_END