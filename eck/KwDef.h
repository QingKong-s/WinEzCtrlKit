#pragma once
#include "RectTraits.h"

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


struct Rect
{
    float left;
    float top;
    float right;
    float bottom;
};
struct RectU
{
    UINT left;
    UINT top;
    UINT right;
    UINT bottom;
};
struct RectI
{
    int left;
    int top;
    int right;
    int bottom;
};

EckInlineNdCe Rect MakeRect(const RECT& rc) noexcept
{
    return { (float)rc.left, (float)rc.top, (float)rc.right, (float)rc.bottom };
}

#ifdef _D2D1_H_
EckInlineNdCe const D2D1_RECT_F& MakeD2DRectF(const Kw::Rect& rc) noexcept
{
    return *(D2D1_RECT_F*)&rc;
}
#endif

struct ColorF
{
    float r, g, b, a;
};
KW2D_NAMESPACE_END
ECK_DEF_RECT_TRAITS(Kw::Rect);
ECK_DEF_RECT_TRAITS(Kw::RectU);
ECK_DEF_RECT_TRAITS(Kw::RectI);
EckInlineNdCe BOOL PtInRect(CcpRect auto const& rc, Kw::Vec2 pt) noexcept
{
    return ((pt.x >= (float)rc.left) && (pt.x < (float)rc.right) &&
        (pt.y >= (float)rc.top) && (pt.y < (float)rc.bottom));
}

EckInlineNdCe RECT MakeRect(const Kw::Rect& rc) noexcept
{
    return { (int)rc.left, (int)rc.top, (int)rc.right, (int)rc.bottom };
}
ECK_NAMESPACE_END