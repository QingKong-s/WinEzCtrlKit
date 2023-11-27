#pragma once
#include "ECK.h"
#include <numbers>

ECK_NAMESPACE_BEGIN
/// <summary>
/// 角度转弧度
/// </summary>
/// <param name="fDeg">角度</param>
/// <returns>弧度</returns>
template<class TVal>
EckInline constexpr TVal Deg2Rad(TVal fDeg)
{
	return fDeg * std::numbers::pi_v<TVal> / (TVal)180.;
}

/// <summary>
/// 弧度转角度
/// </summary>
/// <param name="fRad">弧度</param>
/// <returns>角度</returns>
template<class TVal>
EckInline constexpr TVal Rad2Deg(TVal fRad)
{
	return fRad * (TVal)180. / std::numbers::pi_v<TVal>;
}

template<class TVal>
EckInline constexpr TVal DegIn0To360(TVal fDeg)
{
	if (fDeg < (TVal)0.)
		return DegIn0To360(fDeg + (TVal)360.);
	else if (fDeg > (TVal)360.)
		return DegIn0To360(fDeg - (TVal)360.);
	else
		return fDeg;
}
ECK_NAMESPACE_END