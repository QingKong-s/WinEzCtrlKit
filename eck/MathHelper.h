#pragma once
#include "ECK.h"

ECK_NAMESPACE_BEGIN
/// <summary>
/// 角度转弧度
/// </summary>
/// <param name="fDeg">角度</param>
/// <returns>弧度</returns>
template<class TVal>
EckInline constexpr TVal Deg2Rad(TVal fDeg)
{
	return fDeg * (TVal)Pi / (TVal)180.;
}

/// <summary>
/// 弧度转角度
/// </summary>
/// <param name="fRad">弧度</param>
/// <returns>角度</returns>
template<class TVal>
EckInline constexpr TVal Rad2Deg(TVal fRad)
{
	return fRad * (TVal)180. / (TVal)Pi;
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

template<class TVal = int>
/// <summary>
/// 计算圆上一点
/// </summary>
EckInline void CalcPointFromCircleAngle(TVal r, float fAngle, TVal& xRet, TVal& yRet)
{
	xRet = (TVal)((float)r * cosf(fAngle));
	yRet = (TVal)((float)r * sinf(fAngle));
}

template<class TVal = int>
EckInline TVal CalcLineLength(TVal x1, TVal y1, TVal x2, TVal y2)
{
	return (TVal)sqrtf((float)((x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2)));
}

template<class TVal = int>
EckInline void CalcPointFromLineScalePos(TVal x1, TVal y1, TVal x2, TVal y2, float fScale, TVal& x, TVal& y)
{
	x = (TVal)(x1 - (x1 - x2) * fScale);
	y = (TVal)(y1 - (y1 - y2) * fScale);
}

template<class TVal = int>
/// <summary>
/// 从角度计算椭圆上一段弧的端点。
/// 计算结果可供Arc、Chord函数等使用
/// </summary>
EckInline void CalcArcFromEllipseAngle(HDC hDC, TVal x, TVal y, TVal xr, TVal yr, float fStartAngle, float fSweepAngle,
	TVal& x1, TVal& y1, TVal& x2, TVal& y2)
{
	TVal x10, y10, x20, y20;
	const TVal a = (xr - x) / 2, b = (yr - y) / 2;
	CalcPointFromEllipseAngle(a, b, fStartAngle, x10, y10);
	CalcPointFromEllipseAngle(a, b, fStartAngle + fSweepAngle, x20, y20);
	x1 = x10 + a + x;
	y1 = -y10 + b + y;
	x2 = x20 + a + x;
	y2 = -y20 + b + y;
}

/// <summary>
/// 计算直线方程。
/// 从两点计算直线的方程
/// </summary>
EckInline void CalcLineEquation(float x1, float y1, float x2, float y2, float& A, float& B, float& C)
{
	const float m = y1 - y2, n = x1 - x2;
	A = m;
	B = -n;
	C = -m * x2 + n * y2;
}

/// <summary>
/// 计算直线方程。
/// 从一点和斜率计算直线方程
/// </summary>
EckInline void CalcLineEquation(float x, float y, float k, float& A, float& B, float& C)
{
	A = k;
	B = -1.f;
	C = y - k * x;
}

/// <summary>
/// 极坐标到直角坐标
/// </summary>
EckInline void Polar2Rect(float fRho, float fTheta, float& x, float& y)
{
	x = fRho * cosf(fTheta);
	y = fRho * sinf(fTheta);
}

/// <summary>
/// 直角坐标到极坐标
/// </summary>
EckInline void Rect2Polar(float x, float y, float& fRho, float& fTheta)
{
	fRho = sqrtf(x * x + y * y);
	fTheta = atan2f(y, x);
}

template<class TVal>
/// <summary>
/// 计算椭圆上一点
/// </summary>
/// <param name="a">半长轴</param>
/// <param name="b">半短轴</param>
/// <param name="fAngle">角度</param>
/// <param name="xRet">点X坐标</param>
/// <param name="yRet">点Y坐标</param>
EckInline void CalcPointFromEllipseAngle(TVal a, TVal b, float fAngle, TVal& xRet, TVal& yRet)
{
	const float fEccentricAngle = atan2f(tanf(fAngle) * a, b);
	xRet = (TVal)((float)a * cosf(fEccentricAngle));
	yRet = (TVal)((float)b * sinf(fEccentricAngle));
}
ECK_NAMESPACE_END