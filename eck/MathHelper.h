#pragma once
#include "ECK.h"

#include <DirectXMath.h>

ECK_NAMESPACE_BEGIN
template<std::floating_point T>
EckInlineNdCe T Deg2Rad(T fDeg) noexcept { return fDeg * (T)Pi / 180; }
template<std::floating_point T>
EckInlineNdCe T Rad2Deg(T fRad) noexcept { return fRad * 180 / (T)Pi; }

template<std::floating_point T>
EckInlineNd T DegIn0To360(T fDeg) noexcept
{
    fDeg = fmod(fDeg, 360);
    return fDeg < 0 ? fDeg + 360 : fDeg;
}

template<class T>
EckInline void CalcPointFromCircleAngle(auto r, auto fAngle,
    _Out_ T& xRet, _Out_ T& yRet) noexcept
{
    xRet = T(r * cos(fAngle));
    yRet = T(r * sin(fAngle));
}

template<class T = float>
EckInline T CalcLineLength(auto x1, auto y1, auto x2, auto y2) noexcept
{
    return (T)sqrt(((double)x1 - x2) * ((double)x1 - x2) +
        ((double)y1 - y2) * ((double)y1 - y2));
}
template<class T = float>
EckInline T CalcLineLength(POINT pt1, POINT pt2) noexcept
{
    return CalcLineLength<T>(pt1.x, pt1.y, pt2.x, pt2.y);
}

template<class T>
EckInline void CalcPointFromLineScalePos(auto x1, auto y1, auto x2, auto y2,
    float fScale, _Out_ T& x, _Out_ T& y) noexcept
{
    x = T(x1 - (x1 - x2) * fScale);
    y = T(y1 - (y1 - y2) * fScale);
}

// 从角度计算椭圆上一段弧的端点
template<class T>
void CalcArcFromEllipseAngle(auto x, auto y, auto xr, auto yr,
    auto fStartAngle, auto fSweepAngle,
    _Out_ T& x1, _Out_ T& y1, _Out_ T& x2, _Out_ T& y2) noexcept
{
    T x10, y10, x20, y20;
    const auto a = T((xr - x) / 2), b = T((yr - y) / 2);
    CalcPointFromEllipseAngle(a, b, fStartAngle, x10, y10);
    CalcPointFromEllipseAngle(a, b, fStartAngle + fSweepAngle, x20, y20);
    x1 = x10 + a + x;
    y1 = -y10 + b + y;
    x2 = x20 + a + x;
    y2 = -y20 + b + y;
}

// 从两点计算直线方程
template<std::floating_point T>
EckInlineCe void CalcLineEquation(auto x1, auto y1, auto x2, auto y2,
    _Out_ T& A, _Out_ T& B, _Out_ T& C) noexcept
{
    const auto m = y1 - y2;
    const auto n = x1 - x2;
    A = T(m);
    B = T(-n);
    C = T(-m * x2 + n * y2);
}
// 从一点和斜率计算直线方程
template<std::floating_point T>
EckInlineCe void CalcLineEquation(auto x, auto y, auto k,
    _Out_ T& A, _Out_ T& B, _Out_ T& C) noexcept
{
    A = T(k);
    B = T(-1);
    C = T(y - k * x);
}

template<class T>
EckInline void Polar2Rect(auto fRho, auto fTheta,
    _Out_ T& x, _Out_ T& y) noexcept
{
    x = T(fRho * cos(fTheta));
    y = T(fRho * sin(fTheta));
}
template<class T>
EckInline void Rect2Polar(auto x, auto y,
    _Out_ T& fRho, _Out_ T& fTheta) noexcept
{
    fRho = (T)sqrt(x * x + y * y);
    fTheta = (T)atan2(y, x);
}

/// <summary>
/// 计算椭圆上一点
/// </summary>
/// <param name="a">半长轴</param>
/// <param name="b">半短轴</param>
/// <param name="fAngle">角度</param>
/// <param name="xRet">点X坐标</param>
/// <param name="yRet">点Y坐标</param>
template<class T>
EckInline void CalcPointFromEllipseAngle(auto a, auto b, auto fAngle,
    _Out_ T& xRet, _Out_ T& yRet) noexcept
{
    const auto fEccentricAngle = atan2(tan(fAngle) * a, b);
    xRet = T(a * cos(fEccentricAngle));
    yRet = T(b * sin(fEccentricAngle));
}

template<std::floating_point T = float, CcpPointStruct TPt>
EckInlineNd T CalcPointToLineDistance(TPt pt, TPt pt1, TPt pt2) noexcept
{
    return T(fabs((pt2.y - pt1.y) * pt.x - (pt2.x - pt1.x) * pt.y + pt2.x * pt1.y - pt2.y * pt1.x) /
        sqrt((pt2.x - pt1.x) * (pt2.x - pt1.x) + (pt2.y - pt1.y) * (pt2.y - pt1.y)));
}

/// <summary>
/// 计算扭曲矩阵。
/// 计算将矩形映射到任意凸四边形的4x4矩阵
/// </summary>
/// <param name="rc">矩形</param>
/// <param name="pt">映射到的点，至少指向四个D2D1_POINT_2F，分别对应左上、右上、左下、右下</param>
/// <returns>结果矩阵</returns>
inline DirectX::XMMATRIX CalcDistortMatrix(const D2D1_RECT_F& rc,
    _In_reads_(4) const D2D1_POINT_2F* pt) noexcept
{
    /*
    *			N		  B			A
    * (k0, l0) -> (0, 0) -> (0, 0) -> (x0, y0)
    * (k1, l1) -> (1, 0) -> (1, 0) -> (x1, y1)
    * (k2, l2) -> (0, 1) -> (0, 1) -> (x2, y2)
    * (k3, l3) -> (1, 1) -> (a, b) -> (x3, y3)
    * 首先利用N将任意矩形变换为单位正方形，之后的变换可以拆解为一个仿射变换(A)
    * 和一个非仿射变换(B)。由前三条已知变换可确定矩阵A，由此得到(a, b)的值。设
    * B[3][3] = 1，利用全部四条变换可得矩阵B各元素与(a, b)的关系。此时N、B、A
    * 矩阵均已确定，顺次相乘即为最终变换。
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
/// <param name="pt">映射到的点，至少指向四个D2D1_POINT_2F，分别对应左上、右上、左下、右下</param>
/// <param name="MatrixResult">结果矩阵</param>
EckInline void CalcDistortMatrix(
    const D2D1_RECT_F& rc,
    _In_reads_(4) const D2D1_POINT_2F* pt,
    _Out_ D2D1_MATRIX_4X4_F& MatrixResult) noexcept
{
    DirectX::XMStoreFloat4x4((DirectX::XMFLOAT4X4*)&MatrixResult,
        CalcDistortMatrix(rc, pt));
}

/// <summary>
/// 计算逆扭曲矩阵。
/// 计算将任意凸四边形映射回矩形的4x4矩阵
/// </summary>
/// <param name="rc">矩形</param>
/// <param name="pt">映射到的点，至少指向四个D2D1_POINT_2F，分别对应左上、右上、左下、右下</param>
/// <returns>结果矩阵</returns>
inline DirectX::XMMATRIX CalcInverseDistortMatrix(const D2D1_RECT_F& rc,
    _In_reads_(4) const D2D1_POINT_2F* pt) noexcept
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
/// <param name="pt">映射到的点，至少指向四个D2D1_POINT_2F，分别对应左上、右上、左下、右下</param>
/// <param name="MatrixResult">结果矩阵</param>
EckInline void CalcInverseDistortMatrix(
    const D2D1_RECT_F& rc,
    _In_reads_(4) const D2D1_POINT_2F* pt,
    _Out_ D2D1_MATRIX_4X4_F& MatrixResult) noexcept
{
    DirectX::XMStoreFloat4x4((DirectX::XMFLOAT4X4*)&MatrixResult,
        CalcInverseDistortMatrix(rc, pt));
}

EckInlineNd D2D1::Matrix3x2F D2DMatrixReflection(float A, float B, float C) noexcept
{
    const float t = A * A + B * B;
    const float u = -2.f * A * B / t;
    return D2D1::Matrix3x2F(
        1.f - 2.f * A * A / t,
        u,
        u,
        1.f - 2.f * B * B / t,
        -2.f * A * C / t,
        -2.f * B * C / t);
}

EckInlineNdCe XFORM XFormTranslate(float dx, float dy) noexcept
{
    return
    {
        1.f, 0.f,
        0.f, 1.f,
        dx, dy
    };
}
EckInlineNd XFORM XFormRotate(float fAngle) noexcept
{
    return
    {
        cosf(fAngle), sinf(fAngle),
        -sinf(fAngle), cosf(fAngle),
        0.f, 0.f
    };
}
EckInlineNd XFORM XFormRotate(float fAngle, float x, float y) noexcept
{
    /*
    |1   0   0|   |m11 m12 0|   |1   0   0|   |m11                m12                0|
    |0   1   0| * |m21 m22 0| * |0   1   0| = |m21                m22                0|
    |-dx -dy 1|   |0   0   1|   |dx  dy  1|   |-dx*m11-dy*m21+dx  -dx*m12-dy*m22+dy  1|
    应用到XFORM时m12与m21应互换位置
    */
    return
    {
        cosf(fAngle), sinf(fAngle),
        -sinf(fAngle), cosf(fAngle),
        -x * cosf(fAngle) - y * sinf(fAngle) + x,
        x * sinf(fAngle) - y * cosf(fAngle) + y
    };
}
EckInlineNdCe XFORM XFormScale(float xScale, float yScale) noexcept
{
    return
    {
        xScale, 0.f,
        0.f, yScale,
        0.f, 0.f
    };
}
EckInlineNdCe XFORM XFormScale(float xScale, float yScale, float x, float y) noexcept
{
    return
    {
        xScale, 0.f,
        0.f, yScale,
        -x * xScale + x, -y * yScale + y
    };
}
EckInlineNdCe XFORM XFormShear(float xFactor, float yFactor) noexcept
{
    return
    {
        1.f, xFactor,
        yFactor, 1.f,
        0.f, 0.f
    };
}
EckInlineNdCe XFORM XFormShear(float xFactor, float yFactor, float x, float y) noexcept
{
    return
    {
        1.f, xFactor,
        yFactor, 1.f,
        -x - y * yFactor + x, -x * xFactor - y + y
    };
}
EckInlineNdCe XFORM XFormReflection(float A, float B, float C) noexcept
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
template<class TPt>
inline void CalcBezierControlPoints(
    std::vector<TPt>& vPt,
    _In_reads_(cPt) const TPt* pPt,
    size_t cPt,
    float K = 1.f) noexcept
{
    vPt.resize((cPt - 1) * 3 + 1);
    auto itResult = vPt.begin();
    *itResult++ = *pPt;// 起点
    *itResult++ = *pPt;// 控点1

    const auto [x2, y2] = pPt[1];
    const auto [x1, y1] = pPt[0];
    using T = std::remove_cvref_t<decltype(x1)>;

    T xMid1, yMid1, xMid2, yMid2, dx, dy;
    TPt ptMidLast{ (x1 + x2) / 2,(y1 + y2) / 2 };
    float LineLenLast = CalcLineLength(x1, y1, x2, y2);
    for (int i = 1; i < cPt - 1; ++i)
    {
        const auto [x2, y2] = pPt[i + 1];
        const auto [x1, y1] = pPt[i];
        const TPt ptMid{ (x1 + x2) / 2,(y1 + y2) / 2 };
        const float LineLen = CalcLineLength(x1, y1, x2, y2);
        const auto [x00, y00] = ptMidLast;
        const auto [x01, y01] = ptMid;
        CalcPointFromLineScalePos(x1, y1, x00, y00, K, xMid1, yMid1);
        CalcPointFromLineScalePos(x1, y1, x01, y01, K, xMid2, yMid2);
        CalcPointFromLineScalePos(x00, y00, x01, y01,
            LineLenLast / (LineLen + LineLenLast), dx, dy);
        dx = x1 - dx;
        dy = y1 - dy;
        *itResult++ = { xMid1 + dx, yMid1 + dy };// 控点2
        *itResult++ = { x1, y1 };// 终点
        *itResult++ = { xMid2 + dx, yMid2 + dy };// 下一段曲线的控点1

        ptMidLast = ptMid;
        LineLenLast = LineLen;
    }
    *itResult++ = pPt[cPt - 1];// 控点2
    *itResult++ = pPt[cPt - 1];// 终点
}
ECK_NAMESPACE_END