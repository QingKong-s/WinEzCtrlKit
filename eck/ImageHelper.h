/*
* WinEzCtrlKit Library
*
* ImageHelper.h ： 图像帮助函数
*
* Copyright(C) 2023 QingKong
*/
#pragma once
#include "Utility.h"
#include "GdiplusFlatDef.h"

#include <numbers>
#include <execution>
#include <unordered_map>

#include <DirectXMath.h>
#include <dxgi1_2.h>

ECK_NAMESPACE_BEGIN

/// <summary>
/// 从字节流创建HBITMAP
/// </summary>
/// <param name="pData">字节流</param>
/// <param name="cbData">字节流尺寸</param>
/// <returns>位图句柄</returns>
HBITMAP CreateHBITMAP(PCVOID pData, SIZE_T cbData);

/// <summary>
/// 从文件创建HBITMAP
/// </summary>
/// <param name="pszFile">文件名</param>
/// <returns>位图句柄</returns>
HBITMAP CreateHBITMAP(PCWSTR pszFile);

/// <summary>
/// 从WIC位图创建HBITMAP
/// </summary>
/// <param name="pBmp">WIC位图</param>
/// <returns>位图句柄</returns>
HBITMAP CreateHBITMAP(IWICBitmap* pBmp);

/// <summary>
/// 从字节流创建HICON
/// </summary>
/// <param name="pData">字节流</param>
/// <param name="cbData">字节流尺寸</param>
/// <returns>图标句柄</returns>
HICON CreateHICON(PCVOID pData, SIZE_T cbData);

/// <summary>
/// 从文件创建HICON
/// </summary>
/// <param name="pszFile">文件名</param>
/// <returns>图标句柄</returns>
HICON CreateHICON(PCWSTR pszFile);

/// <summary>
/// 从WIC位图创建HICON
/// </summary>
/// <param name="pBmp">WIC位图</param>
/// <param name="hbmMask">掩码，若为NULL，则使用全1掩码</param>
/// <returns>图标句柄</returns>
HICON CreateHICON(IWICBitmap* pBmp, HBITMAP hbmMask);

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
/// 角度转弧度
/// </summary>
/// <param name="fDeg">角度</param>
/// <returns>弧度</returns>
EckInline constexpr float Deg2Rad(float fDeg)
{
	return fDeg * std::numbers::pi_v<float> / 180.f;
}

/// <summary>
/// 弧度转角度
/// </summary>
/// <param name="fRad">弧度</param>
/// <returns>角度</returns>
EckInline constexpr float Rad2Deg(float fRad)
{
	return fRad * 180.f / std::numbers::pi_v<float>;
}

template<class TVal = int, class TPt>
/// <summary>
/// 计算繁花曲线各点
/// </summary>
/// <param name="vPt">点集合</param>
/// <param name="rOut">外圆半径</param>
/// <param name="rInt">内圆半径</param>
/// <param name="iOffsetPtPen">描绘点距内圆圆心的偏移</param>
/// <param name="fStep">步长</param>
inline void CalcSpirographPoint(std::vector<TPt>& vPt, TVal rOut, TVal rInt, TVal iOffsetPtPen, float fStep = 0.1f)
{
	vPt.clear();
	float t = 0.f;
	const float k = (float)rInt / (float)rOut;
	const float l = (float)iOffsetPtPen / (float)rInt;
	const float tEnd = 2.f * (std::numbers::pi_v<float>) * (float)rInt / (float)Gcd((UINT)rOut, (UINT)rInt);
	const float fOneMinusK = 1 - k;
	const float fOneMinusKDivK = fOneMinusK / k;
	while (t < tEnd)
	{
		vPt.emplace_back(
			(TVal)(rOut * (fOneMinusK * cosf(t) + l * k * cosf(fOneMinusKDivK * t))),
			(TVal)(rOut * (fOneMinusK * sinf(t) - l * k * sinf(fOneMinusKDivK * t))));
		t += fStep;
	}
	vPt.emplace_back(vPt.front());
}

/// <summary>
/// 画繁花曲线。
/// 函数假定坐标轴情况类似MM_TEXT
/// </summary>
/// <param name="hDC">设备场景</param>
/// <param name="xCenter">中心点X</param>
/// <param name="yCenter">中心点Y</param>
/// <param name="rOut">外圆半径</param>
/// <param name="rInt">内圆半径</param>
/// <param name="iOffsetPtPen">描绘点距内圆圆心的偏移</param>
/// <param name="fStep">步长</param>
/// <returns>Polyline返回值</returns>
EckInline BOOL DrawSpirograph(HDC hDC, int xCenter, int yCenter, int rOut, int rInt, int iOffsetPtPen, float fStep = 0.1f)
{
	std::vector<POINT> vPt{};
	CalcSpirographPoint(vPt, rOut, rInt, iOffsetPtPen, fStep);
	std::transform(std::execution::par_unseq, vPt.begin(), vPt.end(), vPt.begin(), [xCenter, yCenter](POINT& pt)
		{
			return POINT{ pt.x + xCenter,yCenter - pt.y };
		});
	return Polyline(hDC, vPt.data(), (int)vPt.size());
}

/// <summary>
/// 画繁花曲线。
/// 函数假定坐标轴情况类似MM_TEXT
/// </summary>
/// <param name="pGraphics">图形对象</param>
/// <param name="pPen">画笔对象</param>
/// <param name="xCenter">中心点X</param>
/// <param name="yCenter">中心点Y</param>
/// <param name="rOut">外圆半径</param>
/// <param name="rInt">内圆半径</param>
/// <param name="iOffsetPtPen">描绘点距内圆圆心的偏移</param>
/// <param name="fStep">步长</param>
/// <returns>GdipDrawLines返回值</returns>
EckInline GpStatus DrawSpirograph(GpGraphics* pGraphics, GpPen* pPen, float xCenter, float yCenter, float rOut, float rInt,
	float fOffsetPtPen, float fStep = 0.1f)
{
	std::vector<GpPointF> vPt{};
	CalcSpirographPoint<float>(vPt, rOut, rInt, fOffsetPtPen, fStep);
	std::transform(std::execution::par_unseq, vPt.begin(), vPt.end(), vPt.begin(), [xCenter, yCenter](GpPointF& pt)
		{
			return GpPointF{ pt.x + xCenter,yCenter - pt.y };
		});
	return GdipDrawLines(pGraphics, pPen, vPt.data(), (int)vPt.size());
}

struct DRAW_SPIROGRAPH_D2D_PARAM
{
	ID2D1Factory* pFactory = NULL;// D2D工厂
	ID2D1RenderTarget* pRT = NULL;// D2D渲染目标
	ID2D1Brush* pBrush = NULL;// D2D画刷
	float cxStroke = 1.f;// 笔画宽度
	ID2D1StrokeStyle* pStrokeStyle = NULL;// 笔画样式
	float xCenter = 0.f;// 中心点X
	float yCenter = 0.f;// 中心点Y
	float rOut = 0.f;// 外圆半径
	float rInt = 0.f;// 内圆半径
	float fOffsetPtPen = 0.f;// 描绘点距内圆圆心的偏移
	float fStep = 0.1f;// 步长
};

/// <summary>
/// 画繁花曲线。
/// 函数假定坐标轴情况类似MM_TEXT
/// </summary>
/// <param name="Info">参数</param>
/// <param name="ppPathGeometry">接收路径几何形变量的指针</param>
/// <returns>构建路径几何形时的失败信息，无法判断绘制操作成功与否，调用方应检查EndDraw返回值</returns>
HRESULT DrawSpirograph(const DRAW_SPIROGRAPH_D2D_PARAM& Info, ID2D1PathGeometry** ppPathGeometry = NULL);

/// <summary>
/// 计算扭曲矩阵。
/// 计算将矩形映射到任意凸四边形的4x4矩阵
/// </summary>
/// <param name="rcOrg">矩形</param>
/// <param name="ptDistort">映射到的点，分别对应左上角、右上角、左下角、右下角</param>
/// <param name="MatrixResult">结果矩阵</param>
void CalcDistortMatrix(const D2D1_RECT_F& rcOrg, const D2D1_POINT_2F(&ptDistort)[4], D2D1_MATRIX_4X4_F& MatrixResult);

template<class TVal = int, class TPt>
/// <summary>
/// 计算蝴蝶曲线各点
/// </summary>
/// <param name="vPt">点集合</param>
/// <param name="fDeformationCoefficient">变形系数</param>
/// <param name="fScaleX">X方向缩放</param>
/// <param name="fScaleY">Y方向缩放</param>
/// <param name="fStep">步长</param>
EckInline void CalcButterflyCurvePoint(std::vector<TPt>& vPt, float fDeformationCoefficient = 4.f,
	float fScaleX = 100.f, float fScaleY = 100.f, float fStep = 0.01f)
{
	vPt.clear();
	float t = 0.f;
	while (t < (std::numbers::pi_v<float>) * 20.f)
	{
		const float f = (expf(cosf(t)) - 2.f * cosf(fDeformationCoefficient * t) - powf(sinf(t / 12.f), 5.f));
		vPt.emplace_back((TVal)(sinf(t) * f * fScaleX), (TVal)(cosf(t) * f * fScaleY));
		t += fStep;
	}
	vPt.emplace_back(vPt.front());
}

/// <summary>
/// 画蝴蝶曲线。
/// 函数假定坐标轴情况类似MM_TEXT
/// </summary>
/// <param name="hDC">设备场景</param>
/// <param name="xCenter">中心点X</param>
/// <param name="yCenter">中心点Y</param>
/// <param name="fDeformationCoefficient">变形系数</param>
/// <param name="fScaleX">X方向缩放</param>
/// <param name="fScaleY">Y方向缩放</param>
/// <param name="fStep">步长</param>
EckInline BOOL DrawButterflyCurve(HDC hDC, int xCenter, int yCenter, float fDeformationCoefficient = 4.f,
	float fScaleX = 100.f, float fScaleY = 100.f, float fStep = 0.01f)
{
	std::vector<POINT> vPt{};
	CalcButterflyCurvePoint(vPt, fDeformationCoefficient, fScaleX, fScaleY, fStep);
	std::transform(std::execution::par_unseq, vPt.begin(), vPt.end(), vPt.begin(), [xCenter, yCenter](POINT& pt)
		{
			return POINT{ pt.x + xCenter,yCenter - pt.y };
		});
	return Polyline(hDC, vPt.data(), (int)vPt.size());
}

/// <summary>
/// 画蝴蝶曲线。
/// 函数假定坐标轴情况类似MM_TEXT
/// </summary>
/// <param name="pGraphics">图形对象</param>
/// <param name="pPen">画笔</param>
/// <param name="xCenter">中心点X</param>
/// <param name="yCenter">中心点Y</param>
/// <param name="fDeformationCoefficient">变形系数</param>
/// <param name="fScaleX">X方向缩放</param>
/// <param name="fScaleY">Y方向缩放</param>
/// <param name="fStep">步长</param>
/// <returns>GdipDrawLines返回值</returns>
EckInline GpStatus DrawButterflyCurve(GpGraphics* pGraphics, GpPen* pPen, int xCenter, int yCenter, float fDeformationCoefficient = 4.f,
	float fScaleX = 100.f, float fScaleY = 100.f, float fStep = 0.01f)
{
	std::vector<GpPointF> vPt{};
	CalcButterflyCurvePoint<float>(vPt, fDeformationCoefficient, fScaleX, fScaleY, fStep);
	std::transform(std::execution::par_unseq, vPt.begin(), vPt.end(), vPt.begin(), [xCenter, yCenter](GpPointF& pt)
		{
			return GpPointF{ pt.x + xCenter,yCenter - pt.y };
		});
	return GdipDrawLines(pGraphics, pPen, vPt.data(), (int)vPt.size());
}

struct DRAW_BUTTERFLYCURVE_D2D_PARAM
{
	ID2D1Factory* pFactory = NULL;// D2D工厂
	ID2D1RenderTarget* pRT = NULL;// D2D渲染目标
	ID2D1Brush* pBrush = NULL;// D2D画刷
	float cxStroke = 1.f;// 笔画宽度
	ID2D1StrokeStyle* pStrokeStyle = NULL;// 笔画样式
	float xCenter = 0.f;// 中心点X
	float yCenter = 0.f;// 中心点Y
	float fDeformationCoefficient = 4.f;// 变形系数
	float fScaleX = 100.f;// X方向缩放
	float fScaleY = 100.f;// Y方向缩放
	float fStep = 0.01f;// 步长
};

/// <summary>
/// 画蝴蝶曲线。
/// 函数假定坐标轴情况类似MM_TEXT
/// </summary>
/// <param name="Info">参数</param>
/// <param name="ppPathGeometry">接收路径几何形变量的指针</param>
/// <returns>构建路径几何形时的失败信息，无法判断绘制操作成功与否，调用方应检查EndDraw返回值</returns>
HRESULT DrawButterflyCurve(const DRAW_BUTTERFLYCURVE_D2D_PARAM& Info, ID2D1PathGeometry** ppPathGeometry = NULL);

template<class TVal = int>
/// <summary>
/// 从角度计算椭圆上一段弧的端点。
/// 函数假定坐标轴情况类似MM_TEXT。计算结果可供Arc、Chord函数等使用
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
/// XFORM取矩阵平移
/// </summary>
EckInline XFORM XFORMTranslate(float dx, float dy)
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
EckInline XFORM XFORMScale(float xScale, float yScale)
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
EckInline XFORM XFORMScale(float xScale, float yScale, float x, float y)
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
EckInline XFORM XFORMShear(float xFactor, float yFactor)
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
EckInline XFORM XFORMShear(float xFactor, float yFactor, float x, float y)
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
EckInline XFORM XFORMReflection(float A, float B, float C)
{
	const float fASqPlusSq = A * A + B * B;
	const float t = -2.f * A * B / fASqPlusSq;
	return
	{
		1.f - 2.f * A * A / fASqPlusSq,
		t,
		t,
		1.f - 2.f * B * B / fASqPlusSq,
		-2.f * A * C / fASqPlusSq,
		-2.f * B * C / fASqPlusSq
	};
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

template<class TVal = int, class TPt>
/// <summary>
/// 计算玫瑰曲线各点
/// </summary>
/// <param name="vPt">点集合</param>
/// <param name="a">花瓣长度</param>
/// <param name="n">花瓣数量参数</param>
/// <param name="fStep">步长</param>
EckInline void CalcRoseCurvePoint(std::vector<TPt>& vPt, float a = 10.f, float n = 1.f, float fStep = 0.01f)
{
	vPt.clear();
	float t = 0.f;
	float x, y;
	while (t < (std::numbers::pi_v<float>) * 2.f)
	{
		Polar2Rect(a * sinf(n * t), t, x, y);
		vPt.emplace_back((TVal)x, (TVal)y);
		t += fStep;
	}
	vPt.emplace_back(vPt.front());
}

/// <summary>
/// 画玫瑰曲线
/// 函数假定坐标轴情况类似MM_TEXT
/// </summary>
/// <param name="hDC">设备场景</param>
/// <param name="xCenter">中心点X</param>
/// <param name="yCenter">中心点Y</param>
/// <param name="a">花瓣长度</param>
/// <param name="n">花瓣数量参数</param>
/// <param name="fStep">步长</param>
/// <returns>Polyline返回值</returns>
EckInline BOOL DrawRoseCurve(HDC hDC, int xCenter, int yCenter, float a = 300.f, float n = 10.f, float fStep = 0.01f)
{
	std::vector<POINT> vPt{};
	CalcRoseCurvePoint(vPt, a, n, fStep);
	std::transform(std::execution::par_unseq, vPt.begin(), vPt.end(), vPt.begin(), [xCenter, yCenter](POINT& pt)
		{
			return POINT{ pt.x + xCenter,yCenter - pt.y };
		});
	return Polyline(hDC, vPt.data(), (int)vPt.size());
}

/// <summary>
/// 画玫瑰曲线
/// 函数假定坐标轴情况类似MM_TEXT
/// </summary>
/// <param name="pGraphics">图形对象</param>
/// <param name="pPen">画笔对象</param>
/// <param name="xCenter">中心点X</param>
/// <param name="yCenter">中心点Y</param>
/// <param name="a">花瓣长度</param>
/// <param name="n">花瓣数量参数</param>
/// <param name="fStep">步长</param>
/// <returns>GdipDrawLines返回值</returns>
EckInline GpStatus DrawRoseCurve(GpGraphics* pGraphics, GpPen* pPen, float xCenter, float yCenter,
	float a = 300.f, float n = 10.f, float fStep = 0.01f)
{
	std::vector<GpPointF> vPt{};
	CalcRoseCurvePoint<float>(vPt, a, n, fStep);
	std::transform(std::execution::par_unseq, vPt.begin(), vPt.end(), vPt.begin(), [xCenter, yCenter](GpPointF& pt)
		{
			return GpPointF{ pt.x + xCenter,yCenter - pt.y };
		});
	return GdipDrawLines(pGraphics, pPen, vPt.data(), (int)vPt.size());
}

struct DRAW_ROSECURVE_D2D_PARAM
{
	ID2D1Factory* pFactory = NULL;// D2D工厂
	ID2D1RenderTarget* pRT = NULL;// D2D渲染目标
	ID2D1Brush* pBrush = NULL;// D2D画刷
	float cxStroke = 1.f;// 笔画宽度
	ID2D1StrokeStyle* pStrokeStyle = NULL;// 笔画样式
	float xCenter = 0.f;// 中心点X
	float yCenter = 0.f;// 中心点Y
	float fDeformationCoefficient = 4.f;// 变形系数
	float a = 300.f;// 花瓣长度
	float n = 10.f;// 花瓣数量参数
	float fStep = 0.01f;// 步长
};

/// <summary>
/// 画玫瑰曲线
/// 函数假定坐标轴情况类似MM_TEXT
/// </summary>
/// <param name="Info">参数</param>
/// <param name="ppPathGeometry">接收路径几何形变量的指针</param>
/// <returns>构建路径几何形时的失败信息，无法判断绘制操作成功与否，调用方应检查EndDraw返回值</returns>
HRESULT DrawRoseCurve(const DRAW_ROSECURVE_D2D_PARAM& Info, ID2D1PathGeometry** ppPathGeometry = NULL);

struct EZD2D_PARAM
{
	HWND hWnd = NULL;

	IDXGIFactory2* pDxgiFactory = NULL;
	IDXGIDevice* pDxgiDevice = NULL;
	ID2D1Device* pD2dDevice = NULL;

	UINT cx = 8;
	UINT cy = 8;
	UINT cBuffer = 1;
	DXGI_SCALING uScaling = DXGI_SCALING_NONE;
	DXGI_SWAP_EFFECT uSwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	DXGI_ALPHA_MODE uAlphaMode = DXGI_ALPHA_MODE_IGNORE;
	UINT uFlags = 0;

	D2D1_DEVICE_CONTEXT_OPTIONS uDcOptions = D2D1_DEVICE_CONTEXT_OPTIONS_NONE;

	D2D1_ALPHA_MODE uBmpAlphaMode = D2D1_ALPHA_MODE_IGNORE;
	D2D1_BITMAP_OPTIONS uBmpOptions = D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW;

	EckInline static EZD2D_PARAM MakeBitblt(HWND hWnd, IDXGIFactory2* pDxgiFactory,
		IDXGIDevice* pDxgiDevice, ID2D1Device* pD2dDevice, int cx, int cy)
	{
		return
		{
			hWnd,pDxgiFactory,pDxgiDevice,pD2dDevice,(UINT)cx,(UINT)cy,
			1,DXGI_SCALING_STRETCH,DXGI_SWAP_EFFECT_DISCARD,DXGI_ALPHA_MODE_IGNORE,0,
			D2D1_DEVICE_CONTEXT_OPTIONS_NONE,D2D1_ALPHA_MODE_IGNORE,
			D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW
		};
	}

	EckInline static EZD2D_PARAM MakeFlip(HWND hWnd, IDXGIFactory2* pDxgiFactory,
		IDXGIDevice* pDxgiDevice, ID2D1Device* pD2dDevice, int cx, int cy)
	{
		return
		{
			hWnd,pDxgiFactory,pDxgiDevice,pD2dDevice,(UINT)cx,(UINT)cy,
			2,DXGI_SCALING_NONE,DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL,DXGI_ALPHA_MODE_IGNORE,0,
			D2D1_DEVICE_CONTEXT_OPTIONS_NONE,D2D1_ALPHA_MODE_IGNORE,
			D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW
		};
	}
};

/// <summary>
/// 创建D2D资源
/// </summary>
/// <param name="Param">参数</param>
/// <param name="pDC_">返回设备上下文</param>
/// <param name="pSwapChain_">返回交换链</param>
/// <param name="pBitmap_">返回位图</param>
/// <returns>成功返回S_OK，失败返回错误代码</returns>
HRESULT EzD2D(const EZD2D_PARAM& Param, ID2D1DeviceContext*& pDC_, IDXGISwapChain1*& pSwapChain_, ID2D1Bitmap1*& pBitmap_);

/// <summary>
/// 调整交换链大小
/// </summary>
/// <param name="pDC">设备上下文</param>
/// <param name="pSwapChain">交换链</param>
/// <param name="pBitmap">位图的引用</param>
/// <param name="cBuffer">缓冲区数量</param>
/// <param name="cx">宽度</param>
/// <param name="cy">高度</param>
/// <param name="uSwapChainFlags">交换链标志</param>
/// <param name="uBmpAlphaMode">位图Alpha模式</param>
/// <param name="uBmpOptions">位图选项</param>
/// <returns>成功返回S_OK，失败返回错误代码</returns>
HRESULT EzD2dReSize(ID2D1DeviceContext* pDC, IDXGISwapChain1* pSwapChain, ID2D1Bitmap1*& pBitmap, UINT cBuffer,
	int cx, int cy, UINT uSwapChainFlags, D2D1_ALPHA_MODE uBmpAlphaMode = D2D1_ALPHA_MODE_IGNORE,
	D2D1_BITMAP_OPTIONS uBmpOptions = D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW);

/// <summary>
/// 创建WIC位图
/// </summary>
/// <param name="vResult">各帧位图</param>
/// <param name="pDecoder">解码器</param>
/// <param name="pWicFactory">WIC工厂</param>
/// <returns>成功返回S_OK，失败返回错误代码</returns>
HRESULT CreateWicBitmap(std::vector<IWICBitmap*>& vResult, IWICBitmapDecoder* pDecoder, IWICImagingFactory* pWicFactory);

/// <summary>
/// 创建WIC位图解码器
/// </summary>
/// <param name="pszFile">文件名</param>
/// <param name="pDecoder">接收解码器变量引用</param>
/// <param name="pWicFactory">WIC工厂</param>
/// <returns>成功返回S_OK，失败返回错误代码</returns>
EckInline HRESULT CreateWicBitmapDecoder(PCWSTR pszFile, IWICBitmapDecoder*& pDecoder, IWICImagingFactory* pWicFactory)
{
	pDecoder = NULL;
	return pWicFactory->CreateDecoderFromFilename(pszFile, NULL, GENERIC_READ, WICDecodeMetadataCacheOnDemand, &pDecoder);
}

/// <summary>
/// 创建WIC位图解码器
/// </summary>
/// <param name="pStream">流对象</param>
/// <param name="pDecoder">接收解码器变量引用</param>
/// <param name="pWicFactory">WIC工厂</param>
/// <returns>成功返回S_OK，失败返回错误代码</returns>
EckInline HRESULT CreateWicBitmapDecoder(IStream* pStream, IWICBitmapDecoder*& pDecoder, IWICImagingFactory* pWicFactory)
{
	pDecoder = NULL;
	return pWicFactory->CreateDecoderFromStream(pStream, NULL, WICDecodeMetadataCacheOnDemand, &pDecoder);
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

template<class TVal = int, class TPt>
/// <summary>
/// 计算正N角星各点。
/// 等效于计算正N边形各点
/// </summary>
/// <param name="vPt">点集合</param>
/// <param name="r">外接圆半径</param>
/// <param name="n">边数/角数</param>
/// <param name="fAngle">角度</param>
/// <param name="bLinkStar">是否连接为星形</param>
inline void CalcRegularStar(std::vector<TPt>& vPt, TVal r, int n, float fAngle = Deg2Rad(90.f), BOOL bLinkStar = TRUE)
{
	vPt.clear();
	const float fAngleUnit = (std::numbers::pi_v<float>) * 2.f / n;
	float fTheta = fAngle;
	TVal x, y;
	if (bLinkStar)
	{
		int i = 0;
		const int cLoop = n % 2 ? n : n / 2;
		EckCounterNV(cLoop)
		{
			CalcPointFromCircleAngle<TVal>(r, fTheta + fAngleUnit * i, x, y);
			vPt.emplace_back(x, y);
			i += 2;
			if (i >= n)
				i %= n;
		}
		vPt.emplace_back(vPt.front());

		if (n % 2 == 0)
		{
			i = 1;
			EckCounterNV(cLoop)
			{
				CalcPointFromCircleAngle<TVal>(r, fTheta + fAngleUnit * i, x, y);
				vPt.emplace_back(x, y);
				i += 2;
				if (i >= n)
					i %= n;
			}
			vPt.emplace_back(vPt[n / 2 + 1]);
		}
	}
	else
	{
		EckCounter(n, i)
		{
			CalcPointFromCircleAngle<TVal>(r, fTheta, x, y);
			vPt.emplace_back(x, y);
			fTheta += fAngleUnit;
		}
		vPt.emplace_back(vPt.front());
	}
}

/// <summary>
/// 画正N角星/正N边形
/// 函数假定坐标轴情况类似MM_TEXT
/// </summary>
/// <param name="hDC">设备场景</param>
/// <param name="xCenter">中心点X</param>
/// <param name="yCenter">中心点Y</param>
/// <param name="r">外接圆半径</param>
/// <param name="n">边数/角数</param>
/// <param name="fAngle">角度</param>
/// <param name="bLinkStar">是否连接为星形</param>
/// <returns>Polyline返回值</returns>
BOOL DrawRegularStar(HDC hDC, int xCenter, int yCenter,
	int r, int n, float fAngle = Deg2Rad(90.f), BOOL bLinkStar = TRUE);

/// <summary>
/// 画正N角星/正N边形
/// 函数假定坐标轴情况类似MM_TEXT
/// </summary>
/// <param name="pGraphics">图形对象</param>
/// <param name="pPen">画笔句柄</param>
/// <param name="xCenter">中心点X</param>
/// <param name="yCenter">中心点Y</param>
/// <param name="r">外接圆半径</param>
/// <param name="n">边数/角数</param>
/// <param name="fAngle">角度</param>
/// <param name="bLinkStar">是否连接为星形</param>
/// <returns>GdipDrawLines返回值</returns>
GpStatus DrawRegularStar(GpGraphics* pGraphics, GpPen* pPen, float xCenter, float yCenter,
	float r, int n, float fAngle = Deg2Rad(90.f), BOOL bLinkStar = TRUE);

struct DRAW_REGULARSTAR_D2D_PARAM
{
	ID2D1Factory* pFactory = NULL;// D2D工厂
	ID2D1RenderTarget* pRT = NULL;// D2D渲染目标
	ID2D1Brush* pBrush = NULL;// D2D画刷
	float cxStroke = 1.f;// 笔画宽度
	ID2D1StrokeStyle* pStrokeStyle = NULL;// 笔画样式
	float xCenter = 0.f;// 中心点X
	float yCenter = 0.f;// 中心点Y
	float r = 300;// 外接圆半径
	int n = 5;// 边数/角数
	float fAngle = Deg2Rad(90.f);// 起始角度
	BOOL bLinkStar = TRUE;// 是否连接为星形
};

/// <summary>
/// 画正N角星/正N边形
/// 函数假定坐标轴情况类似MM_TEXT
/// </summary>
/// <param name="Info">参数</param>
/// <param name="ppPathGeometry">接收路径几何形变量的指针</param>
/// <returns>构建路径几何形时的失败信息，无法判断绘制操作成功与否，调用方应检查EndDraw返回值</returns>
HRESULT DrawRegularStar(const DRAW_REGULARSTAR_D2D_PARAM& Info, ID2D1PathGeometry** ppPathGeometry = NULL);

/// <summary>
/// D2D取矩阵对称
/// </summary>
EckInline D2D1::Matrix3x2F D2dMatrixReflection(float A, float B, float C)
{
	const float fASqPlusSq = A * A + B * B;
	const float t = -2.f * A * B / fASqPlusSq;
	return D2D1::Matrix3x2F(
		1.f - 2.f * A * A / fASqPlusSq, t,
		t, 1.f - 2.f * B * B / fASqPlusSq,
		-2.f * A * C / fASqPlusSq, -2.f * B * C / fASqPlusSq);
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

	const auto& [x2, y2] = pPt[1];
	const auto& [x1, y1] = pPt[0];
	pptMid[0] = { (x1 + x2) / 2,(y1 + y2) / 2 };
	piLineLen[0] = CalcLineLength(x1, y1, x2, y2);
	for (int i = 1; i < cPt - 1; ++i)
	{
		const auto& [x2, y2] = pPt[i + 1];
		const auto& [x1, y1] = pPt[i];
		pptMid[i] = { (x1 + x2) / 2,(y1 + y2) / 2 };
		piLineLen[i] = CalcLineLength(x1, y1, x2, y2);
		const auto& [x00, y00] = pptMid[i - 1];
		const auto& [x01, y01] = pptMid[i];
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

class CMifptpHBITMAP
{
private:
	HBITMAP m_hBitmap = NULL;
	BITMAP m_Bmp{};
public:
	using TColor = COLORREF;
	using TColorComp = BYTE;
	using TCoord = POINT;

	CMifptpHBITMAP() = default;
	explicit CMifptpHBITMAP(HBITMAP hBitmap) :m_hBitmap{ hBitmap } { GetObjectW(hBitmap, sizeof(m_Bmp), &m_Bmp); }

	EckInline constexpr static TCoord MakeCoord(int x, int y) { return { x,y }; }

	EckInline constexpr static BYTE GetColorComp(COLORREF cr, int k) { return ((BYTE*)&cr)[k]; }

	EckInline constexpr static TColor MakeColor(const TColorComp(&Comp)[4])
	{
		return RGB(Comp[0], Comp[1], Comp[2]) | (Comp[3] << 24);
	}

	CMifptpHBITMAP New(TCoord Dimension) const
	{
		BITMAPINFO bmi{};
		bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
		bmi.bmiHeader.biWidth = Dimension.x;
		bmi.bmiHeader.biHeight = -Dimension.y;
		bmi.bmiHeader.biPlanes = 1;
		bmi.bmiHeader.biBitCount = 32;
		const HDC hDC = GetDC(NULL);
#pragma warning (suppress:6387)
		const HBITMAP hbm = CreateDIBSection(hDC, &bmi, 0, NULL, NULL, 0);
		ReleaseDC(NULL, hDC);
		return CMifptpHBITMAP(hbm);
	}

	EckInline COLORREF GetPixel(TCoord pt) const
	{
		return *(((COLORREF*)(((BYTE*)m_Bmp.bmBits) + pt.y * m_Bmp.bmWidthBytes)) + pt.x);
	}

	EckInline void SetPixel(TCoord pt, TColor cr)
	{
		*(((COLORREF*)(((BYTE*)m_Bmp.bmBits) + pt.y * m_Bmp.bmWidthBytes)) + pt.x) = cr;
	}

	EckInline int GetWidth() const { return m_Bmp.bmWidth; }

	EckInline int GetHeight() const { return m_Bmp.bmHeight; }

	EckInline HBITMAP GetHBITMAP() const { return m_hBitmap; }

	EckInline void Lock() const {}
	EckInline void UnLock() const {}
};

class CMifptpGpBitmap
{
private:
	GpBitmap* m_pBitmap = NULL;
	GpBitmapData m_Data{};
	int m_cx = 0,
		m_cy = 0;
public:
	using TColor = ARGB;
	using TColorComp = BYTE;
	using TCoord = GpPoint;

	CMifptpGpBitmap() = default;
	explicit CMifptpGpBitmap(GpBitmap* pBitmap) :m_pBitmap{ pBitmap } 
	{
		GdipGetImageWidth(pBitmap, (UINT*)&m_cx);
		GdipGetImageHeight(pBitmap, (UINT*)&m_cy);
	}

	EckInline constexpr static TCoord MakeCoord(int x, int y) { return { x,y }; }

	EckInline constexpr static TColorComp GetColorComp(TColor cr, int k) { return ((BYTE*)&cr)[k]; }

	EckInline constexpr static TColor MakeColor(const TColorComp(&Comp)[4])
	{
		return Comp[0] | (Comp[1] << 8) | (Comp[2] << 16) | (Comp[3] << 24);
	}

	CMifptpGpBitmap New(TCoord Dimension) const
	{
		GpBitmap* pBitmap;
		GdipCreateBitmapFromScan0(Dimension.x, Dimension.y, 0, PixelFormat32bppPARGB, NULL, &pBitmap);
		return CMifptpGpBitmap(pBitmap);
	}

	EckInline COLORREF GetPixel(TCoord pt) const
	{
		return *(((COLORREF*)(((BYTE*)m_Data.Scan0) + pt.y * m_Data.Stride)) + pt.x);
	}

	EckInline void SetPixel(TCoord pt, TColor cr)
	{
		*(((COLORREF*)(((BYTE*)m_Data.Scan0) + pt.y * m_Data.Stride)) + pt.x) = cr;
	}

	EckInline int GetWidth() const { return m_cx; }

	EckInline int GetHeight() const { return m_cy; }

	EckInline GpBitmap* GetGpBitmap() const { return m_pBitmap; }

	EckInline void Lock()
	{
		const GpRect rc{ 0,0,m_cx,m_cy };
		GdipBitmapLockBits(m_pBitmap, &rc, GpImageLockMode::ImageLockModeRead | GpImageLockMode::ImageLockModeWrite,
			GpPixelFormat::PixelFormat32bppPARGB, &m_Data);
	}

	EckInline void UnLock()
	{
		GdipBitmapUnlockBits(m_pBitmap, &m_Data);
	}
};

/// <summary>
/// 生成扭曲图像。
/// 生成从原图像上的多边形区域映射到目标多边形区域的扭曲图像
/// </summary>
/// <param name="Bmp">输入位图处理器</param>
/// <param name="NewBmp">结果位图处理器，应自行释放相关资源</param>
/// <param name="pptSrc">源多边形区域</param>
/// <param name="pptDst">目标多边形区域</param>
/// <param name="cPt">顶点数</param>
/// <returns></returns>
template<class TBmpHandler, class TCoord>
inline BOOL MakeImageFromPolygonToPolygon(TBmpHandler& Bmp, TBmpHandler& NewBmp,
	const TCoord* pptSrc, const TCoord* pptDst, int cPt)
{
	static_assert(std::is_same_v<TCoord, typename TBmpHandler::TCoord>);
	if (cPt < 3)
		return FALSE;
	const auto [itMinY, itMaxY] = std::minmax_element(pptDst, pptDst + cPt, [](const TCoord& pt1, const TCoord& pt2)->bool
		{
			return pt1.y < pt2.y;
		});
	const auto [itMinX, itMaxX] = std::minmax_element(pptDst, pptDst + cPt, [](const TCoord& pt1, const TCoord& pt2)->bool
		{
			return pt1.x < pt2.x;
		});
	const int cyPolygon = itMaxY->y - itMinY->y + 1;
	const int cxPolygon = itMaxX->x - itMinX->x + 1;
	if (cxPolygon <= 0 || cyPolygon <= 0)
		return FALSE;

	NewBmp = Bmp.New(TBmpHandler::MakeCoord(cxPolygon, cyPolygon));
	std::vector<TCoord> vPtDst(cPt);
	EckCounter(cPt, i)
		vPtDst[i] = TCoord{ pptDst[i].x - itMinX->x,pptDst[i].y - itMinY->y };
	const int yMax = vPtDst[std::distance(pptDst, itMaxY)].y;

	struct EDGE
	{
		float x;
		float dx;
		float Rx;
		float Ry;
		float dRx;
		float dRy;
		int yMax;
		int yMin;
	};

	std::unordered_map<int, std::vector<EDGE*>> ET(cyPolygon);
	using TETIt = std::unordered_map<int, std::vector<EDGE*>>::iterator;
	EckCounter(cPt, i)
	{
		const auto& pt1 = vPtDst[i];
		const auto& pt2 = vPtDst[(i + 1) % cPt];
		const auto& ptSrc1 = pptSrc[i];
		const auto& ptSrc2 = pptSrc[(i + 1) % cPt];
		if (pt1.y == pt2.y)
			continue;
		auto p = new EDGE;
		int yMax, yMin;
		if (pt1.y < pt2.y)
		{
			p->x = (float)pt1.x;
			p->Rx = (float)ptSrc1.x;
			p->Ry = (float)ptSrc1.y;
			yMax = (int)pt2.y;
			yMin = (int)pt1.y;
		}
		else
		{
			p->x = (float)pt2.x;
			p->Rx = (float)ptSrc2.x;
			p->Ry = (float)ptSrc2.y;
			yMax = (int)pt1.y;
			yMin = (int)pt2.y;
		}
		p->dx = (float)(pt1.x - pt2.x) / (float)(pt1.y - pt2.y);
		p->yMax = yMax;
		p->yMin = yMin;
		p->dRx= (float)(ptSrc1.x - ptSrc2.x) / (float)(pt1.y - pt2.y);
		p->dRy = (float)(ptSrc1.y - ptSrc2.y) / (float)(pt1.y - pt2.y);
		ET[yMin].emplace_back(p);
	}

	for (auto& x : ET)
	{
		std::sort(x.second.begin(), x.second.end(), [](const EDGE* p1, const EDGE* p2)->bool
			{
				if (p1->x == p2->x)
					return p1->dx < p2->dx;
				else
					return p1->x < p2->x;
			});
	}

	std::vector<EDGE*> AEL{};
	std::vector<size_t> vNeedDel{};

	Bmp.Lock();
	NewBmp.Lock();
	for (int y = 0; y <= yMax; ++y)
	{
		if (TETIt it; (it = ET.find(y)) != ET.end())
			AEL.insert(AEL.end(), it->second.begin(), it->second.end());
		if (!AEL.empty())
		{
			std::sort(AEL.begin(), AEL.end(), [](const EDGE* p1, const EDGE* p2)->bool
				{
					if (p1->x == p2->x)
						return p1->dx < p2->dx;
					else
						return p1->x < p2->x;
				});

			vNeedDel.clear();
			EckCounter(AEL.size(), i)
			{
				if (y == AEL[i]->yMax)
					vNeedDel.emplace_back(i);
			}
			for (auto it = vNeedDel.rbegin(); it < vNeedDel.rend(); ++it)
				AEL.erase(AEL.begin() + *it);
			EckCounter(AEL.size() / 2, i)
			{
				auto pL = AEL[i * 2];
				auto pR = AEL[i * 2 + 1];
				float dRxx = (pL->Rx - pR->Rx) / (float)(pL->x - pR->x);
				float dRyy = (pL->Ry - pR->Ry) / (float)(pL->x - pR->x);
				float Rxx = pL->Rx;
				float Ryy = pL->Ry;
				for (int x = (int)pL->x; x <= (int)pR->x; ++x)
				{
					int x0 = (int)floorf(Rxx);
					float fRateX = Rxx - x0;
					if (x0 < 0)
					{
						x0 = 0;
						fRateX = 0.f;
					}
					else if (x0 >= Bmp.GetWidth() - 1)
					{
						x0 = Bmp.GetWidth() - 2;
						fRateX = 1.f;
					}
					int y0 = (int)floorf(Ryy);
					float fRateY = Ryy - y0;
					if (y0 < 0)
					{
						y0 = 0;
						fRateY = 0.f;
					}
					else if (y0 >= Bmp.GetHeight() - 1)
					{
						y0 = Bmp.GetHeight() - 2;
						fRateY = 1.f;
					}
					fRateX = 1.f - fRateX;
					fRateY = 1.f - fRateY;
					const typename TBmpHandler::TColor cr[4]
					{
						Bmp.GetPixel(TBmpHandler::MakeCoord(x0,y0)),
						Bmp.GetPixel(TBmpHandler::MakeCoord(x0 + 1,y0)),
						Bmp.GetPixel(TBmpHandler::MakeCoord(x0,y0 + 1)),
						Bmp.GetPixel(TBmpHandler::MakeCoord(x0 + 1,y0 + 1)),
					};

					typename TBmpHandler::TColorComp crNew[4];
					EckCounter(4, k)
					{
						crNew[k] = (typename TBmpHandler::TColorComp)(
							TBmpHandler::GetColorComp(cr[0], k) * fRateX * fRateY +
							TBmpHandler::GetColorComp(cr[1], k) * (1 - fRateX) * fRateY +
							TBmpHandler::GetColorComp(cr[2], k) * fRateX * (1 - fRateY) +
							TBmpHandler::GetColorComp(cr[3], k) * (1 - fRateX) * (1 - fRateY));
					}

					NewBmp.SetPixel(TBmpHandler::MakeCoord(x, y), TBmpHandler::MakeColor(crNew));
					Rxx += dRxx;
					Ryy += dRyy;
				}
			}

			for (auto e : AEL)
			{
				e->x += e->dx;
				e->Rx += e->dRx;
				e->Ry += e->dRy;
			}
		}
	}
	NewBmp.UnLock();
	Bmp.UnLock();
	for (auto& x : ET)
		for (auto e : x.second)
			delete e;
	return TRUE;
}
ECK_NAMESPACE_END