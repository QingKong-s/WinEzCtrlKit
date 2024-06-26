﻿/*
* WinEzCtrlKit Library
*
* MathHelper.h ： 数学帮助
*
* Copyright(C) 2023 QingKong
*/
#pragma once
#include "ECK.h"

#include <DirectXMath.h>

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

/// <summary>
/// 计算扭曲矩阵。
/// 计算将矩形映射到任意凸四边形的4x4矩阵
/// </summary>
/// <param name="rcOrg">矩形</param>
/// <param name="ptDistort">映射到的点，至少指向四个D2D1_POINT_2F，分别对应左上角、右上角、左下角、右下角</param>
/// <param name="MatrixResult">结果矩阵</param>
inline void CalcDistortMatrix(const D2D1_RECT_F& rcOrg,
	const D2D1_POINT_2F* ptDistort, D2D1_MATRIX_4X4_F& MatrixResult)
{
	const float cx = rcOrg.right - rcOrg.left;
	const float cy = rcOrg.bottom - rcOrg.top;

	const auto TN = DirectX::XMMatrixSet(
		1.f / cx, 0, 0, 0,
		0, 1.f / cy, 0, 0,
		0, 0, 0, 0,
		-rcOrg.left / cx, -rcOrg.top / cy, 0, 1.f);

	const DirectX::XMFLOAT4X4 MA(
		ptDistort[1].x - ptDistort[0].x, ptDistort[1].y - ptDistort[0].y, 0, 0,
		ptDistort[2].x - ptDistort[0].x, ptDistort[2].y - ptDistort[0].y, 0, 0,
		0, 0, 0, 0,
		ptDistort[0].x, ptDistort[0].y, 0, 1.f);
	const auto TA = DirectX::XMLoadFloat4x4(&MA);

	const float fDen = MA._11 * MA._22 - MA._12 * MA._21;
	const float a = (
		MA._22 * ptDistort[3].x -
		MA._21 * ptDistort[3].y +
		MA._21 * MA._42 -
		MA._22 * MA._41) / fDen;
	const float b = (
		MA._11 * ptDistort[3].y -
		MA._12 * ptDistort[3].x +
		MA._12 * MA._41 -
		MA._11 * MA._42) / fDen;
	const auto TB = DirectX::XMMatrixSet(
		a / (a + b - 1), 0, 0, a / (a + b - 1) - 1,
		0, b / (a + b - 1), 0, b / (a + b - 1) - 1,
		0, 0, 0, 0,
		0, 0, 0, 1.f);

	DirectX::XMStoreFloat4x4((DirectX::XMFLOAT4X4*)&MatrixResult, TN * TB * TA);
}

/// <summary>
/// D2D取矩阵对称
/// </summary>
EckInline D2D1::Matrix3x2F D2dMatrixReflection(float A, float B, float C)
{
	const float fASqPlusBSq = A * A + B * B;
	const float t = -2.f * A * B / fASqPlusBSq;
	return D2D1::Matrix3x2F(
		1.f - 2.f * A * A / fASqPlusBSq, t,
		t, 1.f - 2.f * B * B / fASqPlusBSq,
		-2.f * A * C / fASqPlusBSq, -2.f * B * C / fASqPlusBSq);
}

/// <summary>
/// XFORM取矩阵平移
/// </summary>
EckInline constexpr XFORM XFORMTranslate(float dx, float dy)
{
	return
	{
		1.f,
		0.f,
		0.f,
		1.f,
		dx,
		dy
	};
}

/// <summary>
/// XFORM取矩阵旋转
/// </summary>
EckInline XFORM XFORMRotate(float fAngle)
{
	return
	{
		cosf(fAngle),
		sinf(fAngle),
		-sinf(fAngle),
		cosf(fAngle),
		0.f,
		0.f
	};
}

/// <summary>
/// XFORM取矩阵旋转
/// </summary>
EckInline XFORM XFORMRotate(float fAngle, float x, float y)
{
	/*
	|1   0   0|   |m11 m12 0|   |1   0   0|   |m11                m12                0|
	|0   1   0| * |m21 m22 0| * |0   1   0| = |m21                m22                0|
	|-dx -dy 1|   |0   0   1|   |dx  dy  1|   |-dx*m11-dy*m21+dx  -dx*m12-dy*m22+dy  1|
	应用到XFORM时m12与m21应互换位置
	*/
	return
	{
		cosf(fAngle),
		sinf(fAngle),
		-sinf(fAngle),
		cosf(fAngle),
		-x * cosf(fAngle) - y * sinf(fAngle) + x,
		x * sinf(fAngle) - y * cosf(fAngle) + y
	};
}

/// <summary>
/// XFORM取矩阵缩放
/// </summary>
EckInline constexpr XFORM XFORMScale(float xScale, float yScale)
{
	return
	{
		xScale,
		0.f,
		0.f,
		yScale,
		0.f,
		0.f
	};
}

/// <summary>
/// XFORM取矩阵缩放
/// </summary>
EckInline constexpr XFORM XFORMScale(float xScale, float yScale, float x, float y)
{
	return
	{
		xScale,
		0.f,
		0.f,
		yScale,
		-x * xScale + x,
		-y * yScale + y
	};
}

/// <summary>
/// XFORM取矩阵错切
/// </summary>
EckInline constexpr XFORM XFORMShear(float xFactor, float yFactor)
{
	return
	{
		1.f,
		xFactor,
		yFactor,
		1.f,
		0.f,
		0.f
	};
}

/// <summary>
/// XFORM取矩阵错切
/// </summary>
EckInline constexpr XFORM XFORMShear(float xFactor, float yFactor, float x, float y)
{
	return
	{
		1.f,
		xFactor,
		yFactor,
		1.f,
		-x - y * yFactor + x,
		-x * xFactor - y + y
	};
}

/// <summary>
/// XFORM取矩阵对称
/// </summary>
EckInline constexpr XFORM XFORMReflection(float A, float B, float C)
{
	const float fASqPlusBSq = A * A + B * B;
	const float t = -2.f * A * B / fASqPlusBSq;
	return
	{
		1.f - 2.f * A * A / fASqPlusBSq,
		t,
		t,
		1.f - 2.f * B * B / fASqPlusBSq,
		-2.f * A * C / fASqPlusBSq,
		-2.f * B * C / fASqPlusBSq
	};
}

/// <summary>
/// 从离散点计算贝塞尔曲线
/// </summary>
/// <param name="vPt">结果点集合</param>
/// <param name="pPt">离散点</param>
/// <param name="cPt">离散点数量</param>
/// <param name="K">张力</param>
template<class TVal = int, class TPt>
inline void CalcBezierControlPoints(std::vector<TPt>& vPt, const TPt* pPt, int cPt, float K = 1.f)
{
	auto pptMid = (TPt*)_malloca(cPt * sizeof(TPt));
	auto piLineLen = (TVal*)_malloca(cPt * sizeof(TVal));
	EckAssert(pptMid);
	EckAssert(piLineLen);
	vPt.reserve(cPt * 3 + 1);
	vPt.emplace_back(*pPt);// 起点
	vPt.emplace_back(*pPt);// 控点1
	TVal xMid1, yMid1, xMid2, yMid2, dx, dy;

	const auto [x2, y2] = pPt[1];
	const auto [x1, y1] = pPt[0];
	pptMid[0] = { (x1 + x2) / 2,(y1 + y2) / 2 };
	piLineLen[0] = CalcLineLength(x1, y1, x2, y2);
	for (int i = 1; i < cPt - 1; ++i)
	{
		const auto [x2, y2] = pPt[i + 1];
		const auto [x1, y1] = pPt[i];
		pptMid[i] = { (x1 + x2) / 2,(y1 + y2) / 2 };
		piLineLen[i] = CalcLineLength(x1, y1, x2, y2);
		const auto [x00, y00] = pptMid[i - 1];
		const auto [x01, y01] = pptMid[i];
		CalcPointFromLineScalePos<TVal>(x1, y1, x00, y00, K, xMid1, yMid1);
		CalcPointFromLineScalePos<TVal>(x1, y1, x01, y01, K, xMid2, yMid2);
		CalcPointFromLineScalePos<TVal>(x00, y00, x01, y01, (float)piLineLen[i - 1] / (float)(piLineLen[i] + piLineLen[i - 1]), dx, dy);
		dx = x1 - dx;
		dy = y1 - dy;
		vPt.emplace_back(xMid1 + dx, yMid1 + dy);// 控点2
		vPt.emplace_back(x1, y1);// 终点
		vPt.emplace_back(xMid2 + dx, yMid2 + dy);// 下一段曲线的控点1
	}
	vPt.emplace_back(pPt[cPt - 1]);// 控点2
	vPt.emplace_back(pPt[cPt - 1]);// 终点

	_freea(piLineLen);
	_freea(pptMid);
}
ECK_NAMESPACE_END