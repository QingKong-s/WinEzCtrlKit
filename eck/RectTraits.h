#pragma once
#include "ECK.h"

ECK_NAMESPACE_BEGIN
template<class T>
struct CRectTraits
{
    constexpr static BOOL Empty{};
};

#define ECK_DEF_RECT_TRAITS(Type)               \
    template<>                                  \
    struct CRectTraits<Type>                    \
    {                                           \
        using T = decltype(Type{}.left);        \
        constexpr static bool IsRcwh = false;   \
    }

#define ECK_DEF_RECT_TRAITS_RCWH(Type)          \
    template<>                                  \
    struct CRectTraits<Type>                    \
    {                                           \
        using T = decltype(Type{}.x);           \
        constexpr static bool IsRcwh = true;    \
    }

ECK_DEF_RECT_TRAITS(RECT);
ECK_DEF_RECT_TRAITS_RCWH(RCWH);

#ifdef _D2D1_H_
ECK_DEF_RECT_TRAITS(D2D1_RECT_F);
ECK_DEF_RECT_TRAITS(D2D1_RECT_U);
#endif

template<class T>
concept CcpRect = !requires { CRectTraits<T>::Empty; };
ECK_NAMESPACE_END