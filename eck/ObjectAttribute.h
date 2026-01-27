#pragma once
#include "CRefStr.h"
#include "CRefBin.h"
#include "StringConvert.h"

ECK_NAMESPACE_BEGIN
#define ECK_OBJA_DECL \
	constexpr static OBJA_DECL ObjAttr[]

#define ECK_OBJA_BEGIN() \
	virtual ObjAttrErr GetSetAttribute(std::wstring_view svName, \
		std::wstring_view svValue, CRefStrW& rsValue, BOOL bSet) noexcept override \
	{
#define ECK_OBJA_END() \
		return ObjAttrErr::InvalidAttr; \
	}

#define ECK_OBJA_INT(Name, Field) \
	if (::eck::TcsCompareLen2I(svName.data(), svName.size(), EckStrAndLen(Name)) == 0) { \
		if (bSet) { \
			int v; \
			const auto r = ::eck::OamFromString<::eck::ObjAttrType::Int>(svValue, v); \
			if (r == ::eck::ObjAttrErr::Ok) Field = v; \
			return r; \
		} \
		return ::eck::OamToString<::eck::ObjAttrType::Int>(Field, rsValue); }
#define ECK_OBJA_UINT(Name, Field) \
	if (::eck::TcsCompareLen2I(svName.data(), svName.size(), EckStrAndLen(Name)) == 0) { \
		if (bSet) { \
			unsigned int v; \
			const auto r = ::eck::OamFromString<::eck::ObjAttrType::UInt>(svValue, v); \
			if (r == ::eck::ObjAttrErr::Ok) Field = v; \
			return r; \
		} \
		return ::eck::OamToString<::eck::ObjAttrType::UInt>(Field, rsValue); }
#define ECK_OBJA_LONGLONG(Name, Field) \
	if (::eck::TcsCompareLen2I(svName.data(), svName.size(), EckStrAndLen(Name)) == 0) { \
		if (bSet) { \
			long long v; \
			const auto r = ::eck::OamFromString<::eck::ObjAttrType::LongLong>(svValue, v); \
			if (r == ::eck::ObjAttrErr::Ok) Field = v; \
			return r; \
		} \
		return ::eck::OamToString<::eck::ObjAttrType::LongLong>(Field, rsValue); }
#define ECK_OBJA_ULONGLONG(Name, Field) \
	if (::eck::TcsCompareLen2I(svName.data(), svName.size(), EckStrAndLen(Name)) == 0) { \
		if (bSet) { \
			unsigned long long v; \
			const auto r = ::eck::OamFromString<::eck::ObjAttrType::ULongLong>(svValue, v); \
			if (r == ::eck::ObjAttrErr::Ok) Field = v; \
			return r; \
		} \
		return ::eck::OamToString<::eck::ObjAttrType::ULongLong>(Field, rsValue); }
#define ECK_OBJA_FLOAT(Name, Field) \
	if (::eck::TcsCompareLen2I(svName.data(), svName.size(), EckStrAndLen(Name)) == 0) { \
		if (bSet) { \
			float v; \
			const auto r = ::eck::OamFromString<::eck::ObjAttrType::Float>(svValue, v); \
			if (r == ::eck::ObjAttrErr::Ok) Field = v; \
			return r; \
		} \
		return ::eck::OamToString<::eck::ObjAttrType::Float>(Field, rsValue); }
#define ECK_OBJA_DOUBLE(Name, Field) \
	if (::eck::TcsCompareLen2I(svName.data(), svName.size(), EckStrAndLen(Name)) == 0) { \
		if (bSet) { \
			double v; \
			const auto r = ::eck::OamFromString<::eck::ObjAttrType::Double>(svValue, v); \
			if (r == ::eck::ObjAttrErr::Ok) Field = v; \
			return r; \
		} \
		return ::eck::OamToString<::eck::ObjAttrType::Double>(Field, rsValue); }
#define ECK_OBJA_BOOL(Name, Field) \
	if (::eck::TcsCompareLen2I(svName.data(), svName.size(), EckStrAndLen(Name)) == 0) { \
		if (bSet) { \
			BOOL v; \
			const auto r = ::eck::OamFromString<::eck::ObjAttrType::Bool>(svValue, v); \
			if (r == ::eck::ObjAttrErr::Ok) Field = v; \
			return r; \
		} \
		return ::eck::OamToString<::eck::ObjAttrType::Bool>(Field, rsValue); }
#define ECK_OBJA_STRING(Name, Field) \
	if (::eck::TcsCompareLen2I(svName.data(), svName.size(), EckStrAndLen(Name)) == 0) { \
		if (bSet) { \
			CRefStrW v; \
			const auto r = ::eck::OamFromString<::eck::ObjAttrType::String>(svValue, v); \
			if (r == ::eck::ObjAttrErr::Ok) Field = v; \
			return r; \
		} \
		return ::eck::OamToString<::eck::ObjAttrType::String>(Field, rsValue); }
#define ECK_OBJA_ENUM(Name, Field) \
	if (::eck::TcsCompareLen2I(svName.data(), svName.size(), EckStrAndLen(Name)) == 0) { \
		using TUnderlying = ::eck::UnderlyingType_T<decltype(Field)>; \
		if (bSet) { \
			TUnderlying v; \
			const auto r = ::eck::OamFromString<::eck::ObjAttrTypeEnum<TUnderlying>>(svValue, v); \
			if (r == ::eck::ObjAttrErr::Ok) Field = static_cast<decltype(Field)>(v); \
			return r; \
		} \
		return ::eck::OamToString<::eck::ObjAttrTypeEnum<TUnderlying>>(static_cast<TUnderlying>(Field), rsValue); }
// 设置枚举，并检查范围，范围为[Min, Max]
#define ECK_OBJA_ENUM_CHECK(Name, Field, Min, Max) \
	if (::eck::TcsCompareLen2I(svName.data(), svName.size(), EckStrAndLen(Name)) == 0) { \
		using TUnderlying = ::eck::UnderlyingType_T<decltype(Field)>; \
		if (bSet) { \
			TUnderlying v; \
			const auto r = ::eck::OamFromString<::eck::ObjAttrTypeEnum<TUnderlying>>(svValue, v); \
			if (r == ::eck::ObjAttrErr::Ok) \
				if (v <= static_cast<TUnderlying>(Max) && v >= static_cast<TUnderlying>(Min)) \
					Field = static_cast<decltype(Field)>(v); \
				else \
					return ::eck::ObjAttrErr::InvalidValue; \
			return r; \
		} \
		return ::eck::OamToString<::eck::ObjAttrTypeEnum<TUnderlying>>(static_cast<TUnderlying>(Field), rsValue); }
#define ECK_OBJA_CUSTOM(Name, GetSetFunc) \
	if (::eck::TcsCompareLen2I(svName.data(), svName.size(), EckStrAndLen(Name)) == 0) \
		return GetSetFunc(svValue, rsValue, bSet);


enum class ObjAttrErr
{
    Ok,
    Overflow,
    InvalidAttr,
    InvalidValue,
};

enum class ObjAttrType : BYTE
{
    None,
    Custom,
    Char,
    UChar,
    Short,
    UShort,
    Int,
    UInt,
    LongLong,
    ULongLong,
    Float,
    Double,
    Bool,
    String,
    Bin,
    Colorref,

    Array = 1_by << 7,

    FlagsMask = Array,
};
ECK_ENUM_BIT_FLAGS(ObjAttrType);

template<ObjAttrType E>
using TObjAttrType =
std::conditional_t<E == ObjAttrType::Char, char,
    std::conditional_t<E == ObjAttrType::UChar, unsigned char,
    std::conditional_t<E == ObjAttrType::Short, short,
    std::conditional_t<E == ObjAttrType::UShort, unsigned short,
    std::conditional_t<E == ObjAttrType::Int, int,
    std::conditional_t<E == ObjAttrType::UInt, unsigned int,
    std::conditional_t<E == ObjAttrType::LongLong, long long,
    std::conditional_t<E == ObjAttrType::ULongLong, unsigned long long,
    std::conditional_t<E == ObjAttrType::Float, float,
    std::conditional_t<E == ObjAttrType::Double, double,
    std::conditional_t<E == ObjAttrType::Bool, BOOL,
    std::conditional_t<E == ObjAttrType::String, CRefStrW,
    std::conditional_t<E == ObjAttrType::Bin, CRefBin,
    std::conditional_t<E == ObjAttrType::Colorref, COLORREF,
    void>>>>>>>>>>>>>>;

// 并不准确，仅为方便使用
template<class T>
constexpr inline ObjAttrType ObjAttrTypeEnum =
std::is_same_v<T, char> ? ObjAttrType::Char :
    std::is_same_v<T, unsigned char> ? ObjAttrType::UChar :
    std::is_same_v<T, short> ? ObjAttrType::Short :
    std::is_same_v<T, unsigned short> ? ObjAttrType::UShort :
    std::is_same_v<T, int> ? ObjAttrType::Int :
    std::is_same_v<T, unsigned int> ? ObjAttrType::UInt :
    std::is_same_v<T, long long> ? ObjAttrType::LongLong :
    std::is_same_v<T, unsigned long long> ? ObjAttrType::ULongLong :
    std::is_same_v<T, float> ? ObjAttrType::Float :
    std::is_same_v<T, double> ? ObjAttrType::Double :
    std::is_same_v<T, BOOL> ? ObjAttrType::Bool :
    std::is_same_v<T, CRefStrW> ? ObjAttrType::String :
    std::is_same_v<T, CRefBin> ? ObjAttrType::Bin :
    std::is_same_v<T, COLORREF> ? ObjAttrType::Colorref :
    ObjAttrType::None;

EckInlineNdCe bool OatIsArray(ObjAttrType E) noexcept
{
    return (E & ObjAttrType::Array) != ObjAttrType::None;
}

EckInlineNdCe ObjAttrType OatGetBaseType(ObjAttrType E) noexcept
{
    return E & ~ObjAttrType::FlagsMask;
}

struct OBJA_DECL
{
    std::wstring_view Name;
    ObjAttrType Type;
};

template<ObjAttrType E>
using TOamValTo = std::conditional_t<
    OatIsArray(E),
    std::span<const TObjAttrType<E>>,
    const TObjAttrType<E>&
>;
template<ObjAttrType E>
using TOamValFrom = std::conditional_t<
    OatIsArray(E),
    std::vector<TObjAttrType<E>>&,
    TObjAttrType<E>&
>;

template<ObjAttrType E>
inline ObjAttrErr OamFromString(std::wstring_view Str, TOamValFrom<E> Val) noexcept
{
    constexpr auto BaseType = OatGetBaseType(E);
    if constexpr (BaseType <= ObjAttrType::ULongLong &&
        BaseType >= ObjAttrType::Char)
    {
        const auto r = TcsToInt(Str.data(), Str.size(), Val);
        switch (r)
        {
        case TcsCvtErr::Ok:			return ObjAttrErr::Ok;
        case TcsCvtErr::Overflow:	return ObjAttrErr::Overflow;
        case TcsCvtErr::Nothing:	return ObjAttrErr::InvalidValue;
        }
    }
    else if constexpr (BaseType == ObjAttrType::Float ||
        BaseType == ObjAttrType::Double)
    {
        const auto pszBegin = LTrimStr(Str.data(), (int)Str.size());
        const auto cch = Str.size() - (pszBegin - Str.data());
        if (cch == 0)
            Val = 0;
        const auto pszTmp = (PWSTR)_malloca(Cch2CbW(cch));
        TcsCopyLenEnd(pszTmp, pszBegin, cch);
        PWSTR pszEnd;
        Val = (TObjAttrType<E>)wcstod(pszTmp, &pszEnd);
        _freea(pszTmp);
        if (pszEnd == pszTmp)
            return ObjAttrErr::InvalidValue;
        return ObjAttrErr::Ok;
    }
    else if constexpr (BaseType == ObjAttrType::Bool)
    {
        if (TcsCompareLen2I(Str.data(), Str.size(), EckStrAndLen(L"true")) == 0 ||
            TcsCompareLen2(Str.data(), Str.size(), EckStrAndLen(L"1")) == 0 ||
            TcsCompareLen2(Str.data(), Str.size(), EckStrAndLen(L"真")) == 0)
            Val = TRUE;
        else
            Val = FALSE;
        return ObjAttrErr::Ok;
    }
    else if constexpr (BaseType == ObjAttrType::String)
    {
        Val.PushBack(Str);
        return ObjAttrErr::Ok;
    }
    else if constexpr (BaseType == ObjAttrType::Bin)
    {
        // TODO
        EckDbgBreak();
        return ObjAttrErr::InvalidValue;
    }
    else if constexpr (BaseType == ObjAttrType::Colorref)
    {
        if (TcsToInt(Str.data(), Str.size(), Val) != TcsCvtErr::Ok)
            return ObjAttrErr::InvalidValue;
        return ObjAttrErr::Ok;
    }
    return ObjAttrErr::Ok;
}

template<ObjAttrType E>
inline ObjAttrErr OamToString(TOamValTo<E> Val, CRefStrW& Str) noexcept
{
    constexpr auto BaseType = OatGetBaseType(E);
    if constexpr (BaseType <= ObjAttrType::ULongLong &&
        BaseType >= ObjAttrType::Char)
    {
        if constexpr (BaseType == ObjAttrType::Char ||
            BaseType == ObjAttrType::Short ||
            BaseType == ObjAttrType::Int ||
            BaseType == ObjAttrType::LongLong)
            Str.PushBackFormat(L"%I64d", (LONGLONG)Val);
        else
            Str.PushBackFormat(L"%I64u", (ULONGLONG)Val);
        return ObjAttrErr::Ok;
    }
    else if constexpr (BaseType == ObjAttrType::Float ||
        BaseType == ObjAttrType::Double)
    {
        Str.PushBackFormat(L"%g", Val);
        return ObjAttrErr::Ok;
    }
    else if constexpr (BaseType == ObjAttrType::Bool)
    {
        if (Val)
            Str.PushBack(L"true");
        else
            Str.PushBack(L"false");
        return ObjAttrErr::Ok;
    }
    else if constexpr (BaseType == ObjAttrType::String)
    {
        Str.PushBack(Val);
        return ObjAttrErr::Ok;
    }
    else if constexpr (BaseType == ObjAttrType::Bin)
    {
        // TODO
        EckDbgBreak();
        return ObjAttrErr::InvalidValue;
    }
    else if constexpr (BaseType == ObjAttrType::Colorref)
    {
        Str.PushBackFormat(L"0x%08X", Val);
        return ObjAttrErr::Ok;
    }
    return ObjAttrErr::Ok;
}
ECK_NAMESPACE_END