/*
* WinEzCtrlKit Library
*
* MathHelper.h ： 数学帮助函数
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
EckInline void CalcPointFromCircleAngle(TVal r, float fAngle,
	_Out_ TVal& xRet, _Out_ TVal& yRet)
{
	xRet = (TVal)((float)r * cosf(fAngle));
	yRet = (TVal)((float)r * sinf(fAngle));
}

template<class TVal = int>
EckInline TVal CalcLineLength(TVal x1, TVal y1, TVal x2, TVal y2)
{
	return (TVal)sqrtf(float((x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2)));
}

template<class TVal = float>
EckInline TVal CalcLineLength(POINT pt1, POINT pt2)
{
	return (TVal)sqrtf(float((pt1.x - pt2.x) * (pt1.x - pt2.x) + (pt1.y - pt2.y) * (pt1.y - pt2.y)));
}

template<class TVal = int>
EckInline void CalcPointFromLineScalePos(TVal x1, TVal y1, TVal x2, TVal y2,
	float fScale, _Out_ TVal& x, _Out_ TVal& y)
{
	x = (TVal)(x1 - (x1 - x2) * fScale);
	y = (TVal)(y1 - (y1 - y2) * fScale);
}

template<class TVal = int>
/// <summary>
/// 从角度计算椭圆上一段弧的端点。
/// 计算结果可供Arc、Chord函数等使用
/// </summary>
EckInline void CalcArcFromEllipseAngle(HDC hDC, TVal x, TVal y, TVal xr, TVal yr,
	float fStartAngle, float fSweepAngle,
	_Out_ TVal& x1, _Out_ TVal& y1, _Out_ TVal& x2, _Out_ TVal& y2)
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
EckInline void CalcLineEquation(float x1, float y1, float x2, float y2,
	_Out_ float& A, _Out_ float& B, _Out_ float& C)
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
EckInline void CalcLineEquation(float x, float y, float k,
	_Out_ float& A, _Out_ float& B, _Out_ float& C)
{
	A = k;
	B = -1.f;
	C = y - k * x;
}

/// <summary>
/// 极坐标到直角坐标
/// </summary>
EckInline void Polar2Rect(float fRho, float fTheta, _Out_ float& x, _Out_ float& y)
{
	x = fRho * cosf(fTheta);
	y = fRho * sinf(fTheta);
}

/// <summary>
/// 直角坐标到极坐标
/// </summary>
EckInline void Rect2Polar(float x, float y, _Out_ float& fRho, _Out_ float& fTheta)
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
EckInline void CalcPointFromEllipseAngle(TVal a, TVal b, float fAngle,
	_Out_ TVal& xRet, _Out_ TVal& yRet)
{
	const float fEccentricAngle = atan2f(tanf(fAngle) * a, b);
	xRet = (TVal)((float)a * cosf(fEccentricAngle));
	yRet = (TVal)((float)b * sinf(fEccentricAngle));
}

/// <summary>
/// 计算扭曲矩阵。
/// 计算将矩形映射到任意凸四边形的4x4矩阵
/// </summary>
/// <param name="rc">矩形</param>
/// <param name="pt">映射到的点，至少指向四个D2D1_POINT_2F，分别对应左上角、右上角、左下角、右下角</param>
/// <returns>结果矩阵</returns>
inline DirectX::XMMATRIX CalcDistortMatrix(const D2D1_RECT_F& rc,
	_In_reads_(4) const D2D1_POINT_2F* pt)
{
	/*
	*			N		  B			A
	* (k0, l0) -> (0, 0) -> (0, 0) -> (x0, y0)
	* (k1, l1) -> (1, 0) -> (1, 0) -> (x1, y1)
	* (k2, l2) -> (0, 1) -> (0, 1) -> (x2, y2)
	* (k3, l3) -> (1, 1) -> (a, b) -> (x3, y3)
	* 首先利用N将任意矩形变换为单位正方形，之后的变换可以拆解为一个仿射变换
	* 和一个非仿射变换。规定B为仿射变换，则由前三条已知变换可确定矩阵B，因此
	* 可得(a, b)的值。设A[3][3] = 1，利用全部四条变换可知变换矩阵A的各元
	* 素与(a, b)的关系，从而可求得A矩阵。此时N、B、A矩阵均已确定，顺次相乘
	* 即为最终变换。
	*/
	// 求N变换
	const float cx = rc.right - rc.left;
	const float cy = rc.bottom - rc.top;
	const auto TN = DirectX::XMMatrixSet(
		1.f / cx, 0, 0, 0,
		0, 1.f / cy, 0, 0,
		0, 0, 0, 0,
		-rc.left / cx, -rc.top / cy, 0, 1.f);
	// 求A变换
	const DirectX::XMFLOAT4X4 MA(
		pt[1].x - pt[0].x, pt[1].y - pt[0].y, 0, 0,
		pt[2].x - pt[0].x, pt[2].y - pt[0].y, 0, 0,
		0, 0, 0, 0,
		pt[0].x, pt[0].y, 0, 1.f);
	const auto TA = DirectX::XMLoadFloat4x4(&MA);
	// 求B变换
	const float fDen = MA._11 * MA._22 - MA._12 * MA._21;
	const float a = (
		MA._22 * pt[3].x -
		MA._21 * pt[3].y +
		MA._21 * MA._42 -
		MA._22 * MA._41) / fDen;
	const float b = (
		MA._11 * pt[3].y -
		MA._12 * pt[3].x +
		MA._12 * MA._41 -
		MA._11 * MA._42) / fDen;
	const auto TB = DirectX::XMMatrixSet(
		a / (a + b - 1), 0, 0, a / (a + b - 1) - 1,
		0, b / (a + b - 1), 0, b / (a + b - 1) - 1,
		0, 0, 0, 0,
		0, 0, 0, 1.f);
	return TN * TB * TA;
}

/// <summary>
/// 计算扭曲矩阵。
/// 计算将矩形映射到任意凸四边形的4x4矩阵
/// </summary>
/// <param name="rc">矩形</param>
/// <param name="pt">映射到的点，至少指向四个D2D1_POINT_2F，分别对应左上角、右上角、左下角、右下角</param>
/// <param name="MatrixResult">结果矩阵</param>
EckInline void CalcDistortMatrix(const D2D1_RECT_F& rc,
	_In_reads_(4) const D2D1_POINT_2F* pt, D2D1_MATRIX_4X4_F& MatrixResult)
{
	DirectX::XMStoreFloat4x4((DirectX::XMFLOAT4X4*)&MatrixResult,
		CalcDistortMatrix(rc, pt));
}

/// <summary>
/// 计算逆扭曲矩阵。
/// 计算将任意凸四边形映射回矩形的4x4矩阵
/// </summary>
/// <param name="rc">矩形</param>
/// <param name="pt">映射到的点，至少指向四个D2D1_POINT_2F，分别对应左上角、右上角、左下角、右下角</param>
/// <returns>结果矩阵</returns>
inline DirectX::XMMATRIX CalcInverseDistortMatrix(const D2D1_RECT_F& rc,
	_In_reads_(4) const D2D1_POINT_2F* pt)
{
	/*
	*			A		  B			N
	* (x0, y0) -> (0, 0) -> (0, 0) -> (k0, l0)
	* (x1, y1) -> (1, 0) -> (1, 0) -> (k1, l1)
	* (x2, y2) -> (0, 1) -> (0, 1) -> (k2, l2)
	* (x3, y3) -> (a, b) -> (1, 1) -> (k3, l3)
	* 过程与CalcDistortMatrix相反
	*/
	// 求A变换
	const float fDen = (pt[0].x * pt[2].y
		- pt[2].x * pt[0].y
		- pt[0].x * pt[1].y
		+ pt[1].x * pt[0].y
		+ pt[2].x * pt[1].y
		- pt[1].x * pt[2].y);
	const DirectX::XMFLOAT4X4 MA(
		(pt[0].y - pt[2].y) / fDen, -(pt[0].y - pt[1].y) / fDen, 0, 0,
		-(pt[0].x - pt[2].x) / fDen, (pt[0].x - pt[1].x) / fDen, 0, 0,
		0, 0, 0, 0,
		(pt[0].x * pt[2].y - pt[2].x * pt[0].y) / fDen,
		-(pt[0].x * pt[1].y - pt[1].x * pt[0].y) / fDen, 0, 1.f);
	const auto TA = DirectX::XMLoadFloat4x4(&MA);
	// 求B变换
	const float a = (MA._11 * pt[3].x + MA._21 * pt[3].y + MA._41);
	const float b = (MA._12 * pt[3].x + MA._22 * pt[3].y + MA._42);
	const auto TB = DirectX::XMMatrixSet(
		(a + b - 1) / a, 0, 0, (a + b - 1) / a - 1,
		0, (a + b - 1) / b, 0, (a + b - 1) / b - 1,
		0, 0, 0, 0,
		0, 0, 0, 1.f);
	// 求N变换
	const float cx = rc.right - rc.left;
	const float cy = rc.bottom - rc.top;
	const auto TN = DirectX::XMMatrixSet(
		cx, 0, 0, 0,
		0, cy, 0, 0,
		0, 0, 0, 0,
		rc.left, rc.top, 0, 1.f);
	return TA * TB * TN;
}

/// <summary>
/// 计算逆扭曲矩阵。
/// 计算将任意凸四边形映射回矩形的4x4矩阵
/// </summary>
/// <param name="rc">矩形</param>
/// <param name="pt">映射到的点，至少指向四个D2D1_POINT_2F，分别对应左上角、右上角、左下角、右下角</param>
/// <param name="MatrixResult">结果矩阵</param>
EckInline void CalcInverseDistortMatrix(const D2D1_RECT_F& rc,
	_In_reads_(4) const D2D1_POINT_2F* pt, D2D1_MATRIX_4X4_F& MatrixResult)
{
	DirectX::XMStoreFloat4x4((DirectX::XMFLOAT4X4*)&MatrixResult,
		CalcInverseDistortMatrix(rc, pt));
}

/// <summary>
/// D2D取矩阵对称
/// </summary>
EckInline D2D1::Matrix3x2F D2dMatrixReflection(float A, float B, float C)
{
	const float t = A * A + B * B;
	const float u = -2.f * A * B / t;
	return D2D1::Matrix3x2F(
		1.f - 2.f * A * A / t, u,
		u, 1.f - 2.f * B * B / t,
		-2.f * A * C / t, -2.f * B * C / t);
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
	const float t = A * A + B * B;
	const float u = -2.f * A * B / t;
	return
	{
		1.f - 2.f * A * A / t,
		u,
		u,
		1.f - 2.f * B * B / t,
		-2.f * A * C / t,
		-2.f * B * C / t
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
inline void CalcBezierControlPoints(std::vector<TPt>& vPt,
	_In_reads_(cPt) const TPt* pPt, int cPt, float K = 1.f)
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

EckInline [[nodiscard]] float CalcPointToLineDistance(POINT pt, POINT pt1, POINT pt2)
{
	return fabsf(float((pt2.y - pt1.y) * pt.x - (pt2.x - pt1.x) * pt.y + pt2.x * pt1.y - pt2.y * pt1.x)) /
		sqrtf(float((pt2.x - pt1.x) * (pt2.x - pt1.x) + (pt2.y - pt1.y) * (pt2.y - pt1.y)));
}
ECK_NAMESPACE_END