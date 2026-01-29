#pragma once
#include "ECK.h"

#define KW2D_NAMESPACE_BEGIN    namespace Kw {
#define KW2D_NAMESPACE_END      }

ECK_NAMESPACE_BEGIN
KW2D_NAMESPACE_BEGIN
struct Vec2
{
    float x, y;
};

EckInlineNdCe Vec2 operator+(const Vec2& a, const Vec2& b) noexcept
{
    return { a.x + b.x, a.y + b.y };
}
EckInlineCe Vec2& operator+=(Vec2& a, const Vec2& b) noexcept
{
    a.x += b.x;
    a.y += b.y;
    return a;
}
EckInlineNdCe Vec2 operator-(const Vec2& a, const Vec2& b) noexcept
{
    return { a.x - b.x, a.y - b.y };
}
EckInlineCe Vec2& operator-=(Vec2& a, const Vec2& b) noexcept
{
    a.x -= b.x;
    a.y -= b.y;
    return a;
}
EckInlineNdCe Vec2 operator*(const Vec2& v, float s) noexcept
{
    return { v.x * s, v.y * s };
}
EckInlineCe Vec2& operator*=(Vec2& v, float s) noexcept
{
    v.x *= s;
    v.y *= s;
    return v;
}
EckInlineNdCe Vec2 operator/(const Vec2& v, float s) noexcept
{
    return { v.x / s, v.y / s };
}
EckInlineCe Vec2& operator/=(Vec2& v, float s) noexcept
{
    v.x /= s;
    v.y /= s;
    return v;
}


using Rect = D2D1_RECT_F;
using RectU = D2D1_RECT_U;


struct ColorF
{
    float r, g, b, a;
};
KW2D_NAMESPACE_END
ECK_NAMESPACE_END