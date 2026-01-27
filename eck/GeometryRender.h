#pragma once
#include "MathHelper.h"

ECK_NAMESPACE_BEGIN
inline HRESULT MakePloyLinePath(const D2D1_POINT_2F* pPt, int cPt,
    ID2D1Factory* pFactory, ID2D1PathGeometry*& pPathGeo) noexcept
{
    ID2D1PathGeometry* pPathGeometry;
    HRESULT hr = pFactory->CreatePathGeometry(&pPathGeometry);
    if (!pPathGeometry)
        return hr;
    ID2D1GeometrySink* pSink;
    pPathGeometry->Open(&pSink);
    pSink->BeginFigure(*pPt, D2D1_FIGURE_BEGIN_HOLLOW);
    pSink->AddLines(pPt, cPt);
    pSink->EndFigure(D2D1_FIGURE_END_OPEN);
    hr = pSink->Close();
    if (FAILED(hr))
    {
        pSink->Release();
        pPathGeometry->Release();
        return hr;
    }
    pSink->Release();
    pPathGeo = pPathGeometry;
    return S_OK;
}

/// <summary>
/// 计算繁花曲线各点
/// </summary>
/// <param name="vPt">点集合</param>
/// <param name="rOut">外圆半径</param>
/// <param name="rInt">内圆半径</param>
/// <param name="iOffsetPtPen">描绘点距内圆圆心的偏移</param>
/// <param name="fStep">步长</param>
template<class TVal = int, class TPt>
inline void FlattenSpirograph(std::vector<TPt>& vPt,
    TVal rOut, TVal rInt, TVal iOffsetPtPen, float fStep = 0.1f) noexcept
{
    vPt.clear();
    float t = 0.f;
    const float k = (float)rInt / (float)rOut;
    const float l = (float)iOffsetPtPen / (float)rInt;
    const float tEnd = 2.f * PiF * (float)rInt / (float)Gcd((UINT)rOut, (UINT)rInt);
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
/// 画繁花曲线
/// </summary>
/// <param name="hDC">设备场景</param>
/// <param name="xCenter">中心点X</param>
/// <param name="yCenter">中心点Y</param>
/// <param name="rOut">外圆半径</param>
/// <param name="rInt">内圆半径</param>
/// <param name="iOffsetPtPen">描绘点距内圆圆心的偏移</param>
/// <param name="fStep">步长</param>
/// <returns>Polyline返回值</returns>
EckInline BOOL DrawSpirograph(HDC hDC, int xCenter, int yCenter,
    int rOut, int rInt, int iOffsetPtPen, float fStep = 0.1f) noexcept
{
    std::vector<POINT> vPt{};
    FlattenSpirograph(vPt, rOut, rInt, iOffsetPtPen, fStep);
    std::transform(vPt.begin(), vPt.end(), vPt.begin(), [xCenter, yCenter](POINT& pt)
        {
            return POINT{ pt.x + xCenter,yCenter - pt.y };
        });
    return Polyline(hDC, vPt.data(), (int)vPt.size());
}

/// <summary>
/// 画繁花曲线
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
EckInline Gdiplus::GpStatus DrawSpirograph(GpGraphics* pGraphics, GpPen* pPen,
    float xCenter, float yCenter, float rOut, float rInt,
    float fOffsetPtPen, float fStep = 0.1f) noexcept
{
    std::vector<GpPointF> vPt{};
    FlattenSpirograph<float>(vPt, rOut, rInt, fOffsetPtPen, fStep);
    std::transform(vPt.begin(), vPt.end(), vPt.begin(), [xCenter, yCenter](GpPointF& pt)
        {
            return GpPointF{ pt.X + xCenter,yCenter - pt.Y };
        });
    return GdipDrawLines(pGraphics, pPen, vPt.data(), (int)vPt.size());
}

struct DRAW_SPIROGRAPH_D2D_PARAM
{
    ID2D1Factory* pFactory = nullptr;// D2D工厂
    ID2D1RenderTarget* pRT = nullptr;// D2D渲染目标
    ID2D1Brush* pBrush = nullptr;// D2D画刷
    float cxStroke = 1.f;// 笔画宽度
    ID2D1StrokeStyle* pStrokeStyle = nullptr;// 笔画样式
    float xCenter = 0.f;// 中心点X
    float yCenter = 0.f;// 中心点Y
    float rOut = 0.f;// 外圆半径
    float rInt = 0.f;// 内圆半径
    float fOffsetPtPen = 0.f;// 描绘点距内圆圆心的偏移
    float fStep = 0.1f;// 步长
};

/// <summary>
/// 画繁花曲线
/// </summary>
/// <param name="Info">参数</param>
/// <param name="ppPathGeometry">接收路径几何形变量的指针</param>
/// <returns>构建路径几何形时的失败信息，无法判断绘制操作成功与否，调用方应检查EndDraw返回值</returns>
inline HRESULT DrawSpirograph(const DRAW_SPIROGRAPH_D2D_PARAM& Info,
    ID2D1PathGeometry** ppPathGeometry = nullptr) noexcept
{
    if (ppPathGeometry)
        *ppPathGeometry = nullptr;
    std::vector<D2D1_POINT_2F> vPt{};
    FlattenSpirograph<float>(vPt, Info.rOut, Info.rInt, Info.fOffsetPtPen, Info.fStep);
    std::transform(vPt.begin(), vPt.end(), vPt.begin(), [&Info](D2D1_POINT_2F& pt)
        {
            return D2D1_POINT_2F{ pt.x + Info.xCenter,Info.yCenter - pt.y };
        });
    ID2D1PathGeometry* pPathGeometry;
    HRESULT hr;
    if (FAILED(hr = MakePloyLinePath(vPt.data(), (int)vPt.size(), Info.pFactory, pPathGeometry)))
        return hr;
    Info.pRT->DrawGeometry(pPathGeometry, Info.pBrush, Info.cxStroke, Info.pStrokeStyle);
    if (ppPathGeometry)
        *ppPathGeometry = pPathGeometry;
    else
        pPathGeometry->Release();
    return S_OK;
}

/// <summary>
/// 计算蝴蝶曲线各点
/// </summary>
/// <param name="vPt">点集合</param>
/// <param name="fDeformationCoefficient">变形系数</param>
/// <param name="fScaleX">X方向缩放</param>
/// <param name="fScaleY">Y方向缩放</param>
/// <param name="fStep">步长</param>
template<class TVal = int, class TPt>
EckInline void FlattenButterflyCurve(std::vector<TPt>& vPt,
    float fDeformationCoefficient = 4.f,
    float fScaleX = 100.f, float fScaleY = 100.f, float fStep = 0.01f) noexcept
{
    vPt.clear();
    float t = 0.f;
    while (t < PiF * 20.f)
    {
        const float f = (expf(cosf(t)) - 2.f * cosf(fDeformationCoefficient * t) - powf(sinf(t / 12.f), 5.f));
        vPt.emplace_back((TVal)(sinf(t) * f * fScaleX), (TVal)(cosf(t) * f * fScaleY));
        t += fStep;
    }
    vPt.emplace_back(vPt.front());
}

/// <summary>
/// 画蝴蝶曲线
/// </summary>
/// <param name="hDC">设备场景</param>
/// <param name="xCenter">中心点X</param>
/// <param name="yCenter">中心点Y</param>
/// <param name="fDeformationCoefficient">变形系数</param>
/// <param name="fScaleX">X方向缩放</param>
/// <param name="fScaleY">Y方向缩放</param>
/// <param name="fStep">步长</param>
EckInline BOOL DrawButterflyCurve(HDC hDC, int xCenter, int yCenter,
    float fDeformationCoefficient = 4.f,
    float fScaleX = 100.f, float fScaleY = 100.f, float fStep = 0.01f) noexcept
{
    std::vector<POINT> vPt{};
    FlattenButterflyCurve(vPt, fDeformationCoefficient, fScaleX, fScaleY, fStep);
    std::transform(vPt.begin(), vPt.end(), vPt.begin(), [xCenter, yCenter](POINT& pt)
        {
            return POINT{ pt.x + xCenter,yCenter - pt.y };
        });
    return Polyline(hDC, vPt.data(), (int)vPt.size());
}

/// <summary>
/// 画蝴蝶曲线
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
EckInline Gdiplus::GpStatus DrawButterflyCurve(GpGraphics* pGraphics, GpPen* pPen,
    int xCenter, int yCenter, float fDeformationCoefficient = 4.f,
    float fScaleX = 100.f, float fScaleY = 100.f, float fStep = 0.01f) noexcept
{
    std::vector<GpPointF> vPt{};
    FlattenButterflyCurve<float>(vPt, fDeformationCoefficient, fScaleX, fScaleY, fStep);
    std::transform(vPt.begin(), vPt.end(), vPt.begin(), [xCenter, yCenter](GpPointF& pt)
        {
            return GpPointF{ pt.X + xCenter,yCenter - pt.Y };
        });
    return GdipDrawLines(pGraphics, pPen, vPt.data(), (int)vPt.size());
}

struct DRAW_BUTTERFLYCURVE_D2D_PARAM
{
    ID2D1Factory* pFactory = nullptr;// D2D工厂
    ID2D1RenderTarget* pRT = nullptr;// D2D渲染目标
    ID2D1Brush* pBrush = nullptr;// D2D画刷
    float cxStroke = 1.f;// 笔画宽度
    ID2D1StrokeStyle* pStrokeStyle = nullptr;// 笔画样式
    float xCenter = 0.f;// 中心点X
    float yCenter = 0.f;// 中心点Y
    float fDeformationCoefficient = 4.f;// 变形系数
    float fScaleX = 100.f;// X方向缩放
    float fScaleY = 100.f;// Y方向缩放
    float fStep = 0.01f;// 步长
};

/// <summary>
/// 画蝴蝶曲线
/// </summary>
/// <param name="Info">参数</param>
/// <param name="ppPathGeometry">接收路径几何形变量的指针</param>
/// <returns>构建路径几何形时的失败信息，无法判断绘制操作成功与否，调用方应检查EndDraw返回值</returns>
inline HRESULT DrawButterflyCurve(const DRAW_BUTTERFLYCURVE_D2D_PARAM& Info,
    ID2D1PathGeometry** ppPathGeometry = nullptr) noexcept
{
    if (ppPathGeometry)
        *ppPathGeometry = nullptr;
    std::vector<D2D1_POINT_2F> vPt{};
    FlattenButterflyCurve<float>(vPt, Info.fDeformationCoefficient,
        Info.fScaleX, Info.fScaleY, Info.fStep);
    std::transform(vPt.begin(), vPt.end(), vPt.begin(), [&Info](D2D1_POINT_2F& pt)
        {
            return D2D1_POINT_2F{ pt.x + Info.xCenter,Info.yCenter - pt.y };
        });
    ID2D1PathGeometry* pPathGeometry;
    HRESULT hr;
    if (FAILED(hr = MakePloyLinePath(vPt.data(), (int)vPt.size(), Info.pFactory, pPathGeometry)))
        return hr;
    Info.pRT->DrawGeometry(pPathGeometry, Info.pBrush, Info.cxStroke, Info.pStrokeStyle);
    if (ppPathGeometry)
        *ppPathGeometry = pPathGeometry;
    else
        pPathGeometry->Release();
    return S_OK;
}

/// <summary>
/// 计算玫瑰曲线各点
/// </summary>
/// <param name="vPt">点集合</param>
/// <param name="a">花瓣长度</param>
/// <param name="n">花瓣数量参数</param>
/// <param name="fStep">步长</param>
template<class TVal = int, class TPt>
EckInline void FlattenRoseCurve(std::vector<TPt>& vPt,
    float a = 10.f, float n = 1.f, float fStep = 0.01f) noexcept
{
    vPt.clear();
    float t = 0.f;
    float x, y;
    while (t < PiF * 2.f)
    {
        PolarToRect(a * sinf(n * t), t, x, y);
        vPt.emplace_back((TVal)x, (TVal)y);
        t += fStep;
    }
    vPt.emplace_back(vPt.front());
}

/// <summary>
/// 画玫瑰曲线
/// </summary>
/// <param name="hDC">设备场景</param>
/// <param name="xCenter">中心点X</param>
/// <param name="yCenter">中心点Y</param>
/// <param name="a">花瓣长度</param>
/// <param name="n">花瓣数量参数</param>
/// <param name="fStep">步长</param>
/// <returns>Polyline返回值</returns>
EckInline BOOL DrawRoseCurve(HDC hDC, int xCenter, int yCenter,
    float a = 300.f, float n = 10.f, float fStep = 0.01f) noexcept
{
    std::vector<POINT> vPt{};
    FlattenRoseCurve(vPt, a, n, fStep);
    std::transform(vPt.begin(), vPt.end(), vPt.begin(), [xCenter, yCenter](POINT& pt)
        {
            return POINT{ pt.x + xCenter,yCenter - pt.y };
        });
    return Polyline(hDC, vPt.data(), (int)vPt.size());
}

/// <summary>
/// 画玫瑰曲线
/// </summary>
/// <param name="pGraphics">图形对象</param>
/// <param name="pPen">画笔对象</param>
/// <param name="xCenter">中心点X</param>
/// <param name="yCenter">中心点Y</param>
/// <param name="a">花瓣长度</param>
/// <param name="n">花瓣数量参数</param>
/// <param name="fStep">步长</param>
/// <returns>GdipDrawLines返回值</returns>
EckInline Gdiplus::GpStatus DrawRoseCurve(GpGraphics* pGraphics, GpPen* pPen,
    float xCenter, float yCenter,
    float a = 300.f, float n = 10.f, float fStep = 0.01f) noexcept
{
    std::vector<GpPointF> vPt{};
    FlattenRoseCurve<float>(vPt, a, n, fStep);
    std::transform(vPt.begin(), vPt.end(), vPt.begin(), [xCenter, yCenter](GpPointF& pt)
        {
            return GpPointF{ pt.X + xCenter,yCenter - pt.Y };
        });
    return GdipDrawLines(pGraphics, pPen, vPt.data(), (int)vPt.size());
}

struct DRAW_ROSECURVE_D2D_PARAM
{
    ID2D1Factory* pFactory = nullptr;// D2D工厂
    ID2D1RenderTarget* pRT = nullptr;// D2D渲染目标
    ID2D1Brush* pBrush = nullptr;// D2D画刷
    float cxStroke = 1.f;// 笔画宽度
    ID2D1StrokeStyle* pStrokeStyle = nullptr;// 笔画样式
    float xCenter = 0.f;// 中心点X
    float yCenter = 0.f;// 中心点Y
    float fDeformationCoefficient = 4.f;// 变形系数
    float a = 300.f;// 花瓣长度
    float n = 10.f;// 花瓣数量参数
    float fStep = 0.01f;// 步长
};

/// <summary>
/// 画玫瑰曲线
/// </summary>
/// <param name="Info">参数</param>
/// <param name="ppPathGeometry">接收路径几何形变量的指针</param>
/// <returns>构建路径几何形时的失败信息，无法判断绘制操作成功与否，调用方应检查EndDraw返回值</returns>
inline HRESULT DrawRoseCurve(const DRAW_ROSECURVE_D2D_PARAM& Info,
    ID2D1PathGeometry** ppPathGeometry = nullptr) noexcept
{
    if (ppPathGeometry)
        *ppPathGeometry = nullptr;
    std::vector<D2D1_POINT_2F> vPt{};
    FlattenRoseCurve<float>(vPt, Info.a, Info.n, Info.fStep);
    std::transform(vPt.begin(), vPt.end(), vPt.begin(), [&Info](D2D1_POINT_2F& pt)
        {
            return D2D1_POINT_2F{ pt.x + Info.xCenter,Info.yCenter - pt.y };
        });
    ID2D1PathGeometry* pPathGeometry;
    HRESULT hr;
    if (FAILED(hr = MakePloyLinePath(vPt.data(), (int)vPt.size(), Info.pFactory, pPathGeometry)))
        return hr;
    Info.pRT->DrawGeometry(pPathGeometry, Info.pBrush, Info.cxStroke, Info.pStrokeStyle);
    if (ppPathGeometry)
        *ppPathGeometry = pPathGeometry;
    else
        pPathGeometry->Release();
    return S_OK;
}

/// <summary>
/// 计算正星形/正多边形各点
/// </summary>
/// <param name="vPt">点集合</param>
/// <param name="r">外接圆半径</param>
/// <param name="n">边数或角数</param>
/// <param name="fAngle">起始点相对X轴正方向的旋转角度</param>
/// <param name="bLinkStar">是否连接为星形</param>
template<class TVal = int, class TPt>
inline void CalculateRegularStar(std::vector<TPt>& vPt, TVal r,
    int n, float fAngle = DegreeToRadian(90.f), BOOL bLinkStar = TRUE) noexcept
{
    vPt.clear();
    const float fAngleUnit = PiF * 2.f / n;
    float fTheta = fAngle;
    TVal x, y;
    if (bLinkStar)
    {
        int i = 0;
        const int cLoop = (n % 2) ? n : (n / 2);
        EckCounterNV(cLoop)
        {
            CalculatePointFromCircleAngle<TVal>(r, fTheta + fAngleUnit * i, x, y);
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
                CalculatePointFromCircleAngle<TVal>(r, fTheta + fAngleUnit * i, x, y);
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
            CalculatePointFromCircleAngle<TVal>(r, fTheta, x, y);
            vPt.emplace_back(x, y);
            fTheta += fAngleUnit;
        }
        vPt.emplace_back(vPt.front());
    }
}

/// <summary>
/// 画正星形/正多边形
/// </summary>
/// <param name="hDC">设备场景</param>
/// <param name="xCenter">中心点X</param>
/// <param name="yCenter">中心点Y</param>
/// <param name="r">外接圆半径</param>
/// <param name="n">边数或角数</param>
/// <param name="fAngle">起始点相对X轴正方向的旋转角度</param>
/// <param name="bLinkStar">是否连接为星形</param>
/// <returns>Polyline返回值</returns>
inline BOOL DrawRegularStar(HDC hDC, int xCenter, int yCenter,
    int r, int n, float fAngle = DegreeToRadian(90.f), BOOL bLinkStar = TRUE) noexcept
{
    std::vector<POINT> vPt{};
    CalculateRegularStar(vPt, r, n, fAngle, bLinkStar);
    std::transform(vPt.begin(), vPt.end(), vPt.begin(), [xCenter, yCenter](POINT& pt)
        {
            return POINT{ pt.x + xCenter,yCenter - pt.y };
        });

    if (n % 2 || !bLinkStar)
        return Polyline(hDC, vPt.data(), (int)vPt.size());
    else
        return Polyline(hDC, vPt.data(), (int)vPt.size() / 2) &&
        Polyline(hDC, vPt.data() + n / 2 + 1, (int)vPt.size() / 2);
}

/// <summary>
/// 画正星形/正多边形
/// </summary>
/// <param name="pGraphics">图形对象</param>
/// <param name="pPen">画笔句柄</param>
/// <param name="xCenter">中心点X</param>
/// <param name="yCenter">中心点Y</param>
/// <param name="r">外接圆半径</param>
/// <param name="n">边数或角数</param>
/// <param name="fAngle">起始点相对X轴正方向的旋转角度</param>
/// <param name="bLinkStar">是否连接为星形</param>
/// <returns>GdipDrawLines返回值</returns>
inline Gdiplus::GpStatus DrawRegularStar(GpGraphics* pGraphics, GpPen* pPen,
    float xCenter, float yCenter,
    float r, int n, float fAngle = DegreeToRadian(90.f), BOOL bLinkStar = TRUE) noexcept
{
    std::vector<GpPointF> vPt{};
    CalculateRegularStar(vPt, r, n, fAngle, bLinkStar);
    std::transform(vPt.begin(), vPt.end(), vPt.begin(), [xCenter, yCenter](GpPointF& pt)
        {
            return GpPointF{ pt.X + xCenter,yCenter - pt.Y };
        });

    if (n % 2 || !bLinkStar)
        return GdipDrawLines(pGraphics, pPen, vPt.data(), (int)vPt.size());
    else
    {
        GdipDrawLines(pGraphics, pPen, vPt.data(), (int)vPt.size() / 2);
        return GdipDrawLines(pGraphics, pPen, vPt.data() + n / 2 + 1, (int)vPt.size() / 2);
    }
}

struct DRAW_REGULARSTAR_D2D_PARAM
{
    ID2D1Factory* pFactory = nullptr;// D2D工厂
    ID2D1RenderTarget* pRT = nullptr;// D2D渲染目标
    ID2D1Brush* pBrush = nullptr;// D2D画刷
    float cxStroke = 1.f;// 笔画宽度
    ID2D1StrokeStyle* pStrokeStyle = nullptr;// 笔画样式
    float xCenter = 0.f;// 中心点X
    float yCenter = 0.f;// 中心点Y
    float r = 300;// 外接圆半径
    int n = 5;// 边数或角数
    float fAngle = DegreeToRadian(90.f);// 起始点相对X轴正方向的旋转角度
    BOOL bLinkStar = TRUE;// 是否连接为星形
};

/// <summary>
/// 画正星形/正多边形
/// </summary>
/// <param name="Info">参数</param>
/// <param name="ppPathGeometry">接收路径几何形变量的指针</param>
/// <returns>构建路径几何形时的失败信息，无法判断绘制操作成功与否，调用方应检查EndDraw返回值</returns>
inline HRESULT DrawRegularStar(const DRAW_REGULARSTAR_D2D_PARAM& Info,
    ID2D1PathGeometry** ppPathGeometry = nullptr) noexcept
{
    if (ppPathGeometry)
        *ppPathGeometry = nullptr;
    std::vector<D2D1_POINT_2F> vPt{};
    CalculateRegularStar(vPt, Info.r, Info.n, Info.fAngle, Info.bLinkStar);
    std::transform(vPt.begin(), vPt.end(), vPt.begin(), [&Info](D2D1_POINT_2F& pt)
        {
            return D2D1_POINT_2F{ pt.x + Info.xCenter,Info.yCenter - pt.y };
        });

    ID2D1PathGeometry* pPathGeometry;
    HRESULT hr = Info.pFactory->CreatePathGeometry(&pPathGeometry);
    if (!pPathGeometry)
        return hr;
    ID2D1GeometrySink* pSink;
    pPathGeometry->Open(&pSink);
    if (Info.n % 2 || !Info.bLinkStar)
    {
        pSink->BeginFigure(vPt.front(), D2D1_FIGURE_BEGIN_HOLLOW);
        pSink->AddLines(vPt.data(), (UINT32)vPt.size());
        pSink->EndFigure(D2D1_FIGURE_END_OPEN);
    }
    else
    {
        pSink->BeginFigure(vPt.front(), D2D1_FIGURE_BEGIN_HOLLOW);
        pSink->AddLines(vPt.data(), (UINT32)vPt.size() / 2);
        pSink->EndFigure(D2D1_FIGURE_END_OPEN);

        pSink->BeginFigure(vPt[Info.n / 2 + 1], D2D1_FIGURE_BEGIN_HOLLOW);
        pSink->AddLines(vPt.data() + Info.n / 2 + 1, (UINT32)vPt.size() / 2);
        pSink->EndFigure(D2D1_FIGURE_END_OPEN);
    }
    hr = pSink->Close();
    if (FAILED(hr))
    {
        pSink->Release();
        pPathGeometry->Release();
        return hr;
    }
    pSink->Release();
    Info.pRT->DrawGeometry(pPathGeometry, Info.pBrush, Info.cxStroke, Info.pStrokeStyle);
    if (ppPathGeometry)
        *ppPathGeometry = pPathGeometry;
    else
        pPathGeometry->Release();
    return S_OK;
}
ECK_NAMESPACE_END