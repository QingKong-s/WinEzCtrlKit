#pragma once
#include "ECK.h"

ECK_NAMESPACE_BEGIN
struct CGpColorMatrix
{
    struct Proxy
    {
        REAL* p;

        EckInlineNdCe float& operator[](int i) { return p[i]; }
    };

    struct ConstProxy
    {
        const REAL* p;

        EckInlineNdCe const float& operator[](int i) const { return p[i]; }
    };
    GpColorMatrix Mat;

    constexpr CGpColorMatrix() noexcept : Mat{}
    {
        Mat.m[0][0] = Mat.m[1][1] = Mat.m[2][2] = Mat.m[3][3] = Mat.m[4][4] = 1.f;
    }
    constexpr CGpColorMatrix(const float(&Matrix)[5][5]) noexcept
    {
        std::copy_n((float*)&Matrix[0], 25, (float*)&Mat.m);
    }
    constexpr CGpColorMatrix(const float* Matrix) noexcept
    {
        std::copy_n((float*)&Matrix, 25, (float*)&Mat.m);
    }
    constexpr CGpColorMatrix(const GpColorMatrix& Mat) noexcept : Mat(Mat) {}

    EckInlineNdCe Proxy operator[](size_t i) noexcept { return Proxy{ Mat.m[i] }; }
    EckInlineNdCe ConstProxy operator[](size_t i) const noexcept { return ConstProxy{ Mat.m[i] }; }

    // 重置为单位矩阵
    EckInlineCe void Identity() noexcept
    {
        Mat = {};
        Mat.m[0][0] = Mat.m[1][1] = Mat.m[2][2] = Mat.m[3][3] = Mat.m[4][4] = 1.f;
    }

    /// <summary>
    /// 缩放红色
    /// </summary>
    /// <param name="fScale">比例，-1~1</param>
    EckInlineCe void ScaleR(float fScale) noexcept { Mat.m[3][0] = fScale; }

    /// <summary>
    /// 缩放绿色
    /// </summary>
    /// <param name="fScale">比例，-1~1</param>
    EckInlineCe void ScaleG(float fScale) noexcept { Mat.m[3][1] = fScale; }

    /// <summary>
    /// 缩放蓝色
    /// </summary>
    /// <param name="fScale">比例，-1~1</param>
    EckInlineCe void ScaleB(float fScale) noexcept { Mat.m[3][2] = fScale; }

    /// <summary>
    /// 缩放Alpha
    /// </summary>
    /// <param name="fOffset">偏移量，-1~1</param>
    EckInlineCe void ScaleA(float fOffset) noexcept { Mat.m[3][3] = fOffset; }

    /// <summary>
    /// 亮度
    /// </summary>
    /// <param name="fBrightness">-1~1</param>
    EckInlineCe void Brightness(float fBrightness) noexcept
    {
        Mat.m[4][0] = Mat.m[4][1] = Mat.m[4][2] = fBrightness;
    }

    // 去色
    EckInlineCe void Grayscale() noexcept
    {
        Mat.m[0][0] = Mat.m[0][1] = Mat.m[0][2] = 0.299f;
        Mat.m[1][0] = Mat.m[1][1] = Mat.m[1][2] = 0.587f;
        Mat.m[2][0] = Mat.m[2][1] = Mat.m[2][2] = 0.114f;
    }

    // 反相
    EckInlineCe void Invert() noexcept
    {
        Mat.m[0][0] = Mat.m[1][1] = Mat.m[2][2] = -1.0f;
        Mat.m[3][0] = Mat.m[3][1] = Mat.m[3][2] = 1.f;
    }

    /// <summary>
    /// 饱和度
    /// </summary>
    /// <param name="fSaturation">0~2</param>
    EckInlineCe void Saturation(float fSaturation) noexcept
    {
        Mat.m[0][0] = Mat.m[0][1] = Mat.m[0][2] = 0.299f * (1.f - fSaturation);
        Mat.m[1][0] = Mat.m[1][1] = Mat.m[1][2] = 0.587f * (1.f - fSaturation);
        Mat.m[2][0] = Mat.m[2][1] = Mat.m[2][2] = 0.114f * (1.f - fSaturation);
        Mat.m[0][0] += fSaturation;
        Mat.m[1][1] += fSaturation;
        Mat.m[2][2] += fSaturation;
    }

    /// <summary>
    /// 对比度
    /// </summary>
    /// <param name="fContrast">0~2</param>
    EckInlineCe void Contrast(float fContrast) noexcept
    {
        Mat.m[0][0] = Mat.m[1][1] = Mat.m[2][2] = fContrast;
    }

    /// <summary>
    /// 色相
    /// </summary>
    /// <param name="fHue">-180~180</param>
    EckInline void Hue(float fHue) noexcept
    {
        const float fAngle = fHue * PiF / 180.f;
        const float fCos = cos(fHue * PiF / 180.f);
        const float a = (fCos < 0 ? -fCos : 0),
            b = (fCos < 0 ? 1 + fCos : 1 - fCos);
        Mat.m[0][0] = Mat.m[1][1] = Mat.m[2][2] = (fCos < 0 ? 0 : fCos);
        Mat.m[0][1] = Mat.m[1][2] = Mat.m[2][0] = (fAngle < 0 ? a : b);
        Mat.m[0][2] = Mat.m[1][0] = Mat.m[2][1] = (fAngle < 0 ? b : a);
    }

    /// <summary>
    /// 阈值
    /// </summary>
    /// <param name="byThreshold">灰度阈值，0~255</param>
    EckInlineCe void Threshold(BYTE byThreshold) noexcept
    {
        Mat.m[0][0] = Mat.m[0][1] = Mat.m[0][2] = -0.299f * 255.f;
        Mat.m[1][0] = Mat.m[1][1] = Mat.m[1][2] = -0.587f * 255.f;
        Mat.m[2][0] = Mat.m[2][1] = Mat.m[2][2] = -0.114f * 255.f;
        Mat.m[3][0] = Mat.m[3][1] = Mat.m[3][2] = float(256 - byThreshold);
    }
};


// 效果基类
class CGpFx
{
protected:
    GpEffect* m_pEffect{};
public:
    ECK_DISABLE_COPY_DEF_CONS(CGpFx);
    CGpFx(GpEffect* pEffect) noexcept : m_pEffect{ pEffect } {}

    CGpFx(CGpFx&& x) noexcept : m_pEffect{ x.m_pEffect } { x.m_pEffect = nullptr; }

    CGpFx& operator=(CGpFx&& x) noexcept { std::swap(m_pEffect, x.m_pEffect); return *this; }

    ~CGpFx() { GdipDeleteEffect(m_pEffect); }

    EckInlineNd UINT GetParamSize() const noexcept
    {
        UINT cbSize{};
        GdipGetEffectParameterSize(m_pEffect, &cbSize);
        return cbSize;
    }

    EckInlineNdCe GpEffect* GetEffect() const noexcept { return m_pEffect; }

    EckInlineCe GpEffect* Attach(GpEffect* pEffect) noexcept
    {
        return std::exchange(m_pEffect, pEffect);
    }

    EckInlineNdCe GpEffect* Detach() noexcept
    {
        return std::exchange(m_pEffect, nullptr);
    }
};
// 高斯模糊
class CGpFxBlur : public CGpFx
{
public:
    CGpFxBlur() = default;
    CGpFxBlur(const GpBlurParams& Params) noexcept
    {
        GdipCreateEffect(Gdiplus::BlurEffectGuid, &m_pEffect);
        GdipSetEffectParameters(m_pEffect, &Params, sizeof(Params));
    }

    CGpFxBlur(float fRadius, BOOL bExpandEdges) noexcept : CGpFxBlur{ { fRadius, bExpandEdges } } {}

    EckInline GpStatus SetParameters(const GpBlurParams& Params) noexcept
    {
        return GdipSetEffectParameters(m_pEffect, &Params, sizeof(Params));
    }

    EckInline GpStatus GetParameters(GpBlurParams& Params) const
    {
        UINT Dummy{ sizeof(Params) };
        return GdipGetEffectParameters(m_pEffect, &Dummy, &Params);
    }
};
// 锐化
class CGpFxSharpen : public CGpFx
{
public:
    CGpFxSharpen() = default;
    CGpFxSharpen(const GpSharpenParams& Params) noexcept
    {
        GdipCreateEffect(Gdiplus::SharpenEffectGuid, &m_pEffect);
        GdipSetEffectParameters(m_pEffect, &Params, sizeof(Params));
    }

    CGpFxSharpen(float fRadius, float fAmount) noexcept : CGpFxSharpen{ { fRadius, fAmount } } {}

    EckInline GpStatus SetParameters(const GpSharpenParams& Params) noexcept
    {
        return GdipSetEffectParameters(m_pEffect, &Params, sizeof(Params));
    }

    EckInline GpStatus GetParameters(GpSharpenParams& Params) const noexcept
    {
        UINT Dummy{ sizeof(Params) };
        return GdipGetEffectParameters(m_pEffect, &Dummy, &Params);
    }
};
// 色调
class CGpFxTint : public CGpFx
{
public:
    CGpFxTint() = default;
    CGpFxTint(const GpTintParams& Params) noexcept
    {
        GdipCreateEffect(Gdiplus::TintEffectGuid, &m_pEffect);
        GdipSetEffectParameters(m_pEffect, &Params, sizeof(Params));
    }

    CGpFxTint(int Hue, int Amount) noexcept : CGpFxTint{ { Hue, Amount } } {}

    EckInline GpStatus SetParameters(const GpTintParams& Params) noexcept
    {
        return GdipSetEffectParameters(m_pEffect, &Params, sizeof(Params));
    }

    EckInline GpStatus GetParameters(GpTintParams& Params) const noexcept
    {
        UINT Dummy{ sizeof(Params) };
        return GdipGetEffectParameters(m_pEffect, &Dummy, &Params);
    }
};
// 红眼修正
class CGpFxRedEyeCorrection : public CGpFx
{
public:
    CGpFxRedEyeCorrection() = default;
    CGpFxRedEyeCorrection(const GpRedEyeCorrectionParams& Params) noexcept
    {
        GdipCreateEffect(Gdiplus::RedEyeCorrectionEffectGuid, &m_pEffect);
        GdipSetEffectParameters(m_pEffect, &Params, sizeof(Params));
    }

    CGpFxRedEyeCorrection(const std::vector<RECT>& vAreas) noexcept
        : CGpFxRedEyeCorrection{ { (UINT)vAreas.size(), (RECT*)vAreas.data() } }
    {
    }

    template<size_t N>
    CGpFxRedEyeCorrection(const RECT(&Areas)[N]) noexcept
        : CGpFxRedEyeCorrection{ { (UINT)N, (RECT*)Areas } }
    {
    }

    CGpFxRedEyeCorrection(UINT cAreas, const RECT* pAreas) noexcept
        : CGpFxRedEyeCorrection{ { cAreas, (RECT*)pAreas } }
    {
    }

    EckInline GpStatus SetParameters(const GpRedEyeCorrectionParams& Params) noexcept
    {
        return GdipSetEffectParameters(m_pEffect, &Params, sizeof(Params));
    }

    EckInline GpStatus GetParameters(GpRedEyeCorrectionParams& Params) const noexcept
    {
        UINT Dummy{ GetParamSize() };
        return GdipGetEffectParameters(m_pEffect, &Dummy, &Params);
    }
};
// 颜色矩阵
class CGpFxColorMatrix : public CGpFx
{
public:
    CGpFxColorMatrix() = default;
    CGpFxColorMatrix(const GpColorMatrix& Params) noexcept
    {
        GdipCreateEffect(Gdiplus::ColorMatrixEffectGuid, &m_pEffect);
        GdipSetEffectParameters(m_pEffect, &Params, sizeof(Params));
    }

    CGpFxColorMatrix(const float(&Matrix)[5][5]) noexcept
    {
        GdipCreateEffect(Gdiplus::ColorMatrixEffectGuid, &m_pEffect);
        GdipSetEffectParameters(m_pEffect, &Matrix, sizeof(Matrix));
    }

    CGpFxColorMatrix(const float* Matrix) noexcept
    {
        GdipCreateEffect(Gdiplus::ColorMatrixEffectGuid, &m_pEffect);
        GdipSetEffectParameters(m_pEffect, Matrix, 25 * sizeof(float));
    }

    CGpFxColorMatrix(const CGpColorMatrix& Matrix) noexcept : CGpFxColorMatrix{ Matrix.Mat } {}

    EckInline GpStatus SetParameters(const GpColorMatrix& Params) noexcept
    {
        return GdipSetEffectParameters(m_pEffect, &Params, sizeof(Params));
    }

    EckInline GpStatus GetParameters(GpColorMatrix& Params) const noexcept
    {
        UINT Dummy{ sizeof(Params) };
        return GdipGetEffectParameters(m_pEffect, &Dummy, &Params);
    }
};
// 颜色查找表
class CGpFxColorLut : public CGpFx
{
public:
    CGpFxColorLut() = default;
    CGpFxColorLut(const GpColorLUTParams& Params) noexcept
    {
        GdipCreateEffect(Gdiplus::ColorLUTEffectGuid, &m_pEffect);
        GdipSetEffectParameters(m_pEffect, &Params, sizeof(Params));
    }

    EckInline GpStatus SetParameters(const GpColorLUTParams& Params) noexcept
    {
        return GdipSetEffectParameters(m_pEffect, &Params, sizeof(Params));
    }

    EckInline GpStatus GetParameters(GpColorLUTParams& Params) const noexcept
    {
        UINT Dummy{ sizeof(Params) };
        return GdipGetEffectParameters(m_pEffect, &Dummy, &Params);
    }
};
// 亮度对比度
class CGpFxBrightnessContrast : public CGpFx
{
public:
    CGpFxBrightnessContrast() = default;
    CGpFxBrightnessContrast(const GpBrightnessContrastParams& Params) noexcept
    {
        GdipCreateEffect(Gdiplus::BrightnessContrastEffectGuid, &m_pEffect);
        GdipSetEffectParameters(m_pEffect, &Params, sizeof(Params));
    }

    CGpFxBrightnessContrast(int nBrightness, int nContrast) noexcept
        : CGpFxBrightnessContrast{ { nBrightness, nContrast } }
    {
    }

    EckInline GpStatus SetParameters(const GpBrightnessContrastParams& Params) noexcept
    {
        return GdipSetEffectParameters(m_pEffect, &Params, sizeof(Params));
    }

    EckInline GpStatus GetParameters(GpBrightnessContrastParams& Params) const noexcept
    {
        UINT Dummy{ sizeof(Params) };
        return GdipGetEffectParameters(m_pEffect, &Dummy, &Params);
    }
};
// 色相亮度饱和度
class CGpFxHSL : public CGpFx
{
public:
    CGpFxHSL() = default;
    CGpFxHSL(const GpHSLParams& Params) noexcept
    {
        GdipCreateEffect(Gdiplus::HueSaturationLightnessEffectGuid, &m_pEffect);
        GdipSetEffectParameters(m_pEffect, &Params, sizeof(Params));
    }

    /// <param name="nHue">-180~180</param>
    /// <param name="nSaturation">-100~100</param>
    /// <param name="nLightness">-100~100</param>
    CGpFxHSL(int nHue, int nSaturation, int nLightness) noexcept
        : CGpFxHSL{ { nHue, nSaturation, nLightness } }
    {
    }

    EckInline GpStatus SetParameters(const GpHSLParams& Params) noexcept
    {
        return GdipSetEffectParameters(m_pEffect, &Params, sizeof(Params));
    }

    EckInline GpStatus GetParameters(GpHSLParams& Params) const noexcept
    {
        UINT Dummy{ sizeof(Params) };
        return GdipGetEffectParameters(m_pEffect, &Dummy, &Params);
    }
};
// 色彩平衡
class CGpFxColorBalance : public CGpFx
{
public:
    CGpFxColorBalance() = default;
    CGpFxColorBalance(const GpColorBalanceParams& Params) noexcept
    {
        GdipCreateEffect(Gdiplus::ColorBalanceEffectGuid, &m_pEffect);
        GdipSetEffectParameters(m_pEffect, &Params, sizeof(Params));
    }

    /// <param name="nCyanRed">-100~100，青->红</param>
    /// <param name="nMagentaGreen">-100~100，品红->绿</param>
    /// <param name="nYellowBlue">-100~100，黄->蓝</param>
    CGpFxColorBalance(int nCyanRed, int nMagentaGreen, int nYellowBlue) noexcept
        : CGpFxColorBalance{ { nCyanRed, nMagentaGreen, nYellowBlue } }
    {
    }

    EckInline GpStatus SetParameters(const GpColorBalanceParams& Params) noexcept
    {
        return GdipSetEffectParameters(m_pEffect, &Params, sizeof(Params));
    }

    EckInline GpStatus GetParameters(GpColorBalanceParams& Params) const noexcept
    {
        UINT Dummy{ sizeof(Params) };
        return GdipGetEffectParameters(m_pEffect, &Dummy, &Params);
    }
};
// 色阶
class CGpFxLevels : public CGpFx
{
public:
    CGpFxLevels() = default;
    CGpFxLevels(const GpLevelsParams& Params) noexcept
    {
        GdipCreateEffect(Gdiplus::LevelsEffectGuid, &m_pEffect);
        GdipSetEffectParameters(m_pEffect, &Params, sizeof(Params));
    }

    /// <param name="nHighlight">0~100</param>
    /// <param name="nMidTone">-100~100</param>
    /// <param name="nShadow">0~100</param>
    CGpFxLevels(int nHighlight, int nMidTone, int nShadow) noexcept
        : CGpFxLevels{ { nHighlight, nMidTone, nShadow } }
    {
    }

    EckInline GpStatus SetParameters(const GpLevelsParams& Params) noexcept
    {
        return GdipSetEffectParameters(m_pEffect, &Params, sizeof(Params));
    }

    EckInline GpStatus GetParameters(GpLevelsParams& Params) const noexcept
    {
        UINT Dummy{ sizeof(Params) };
        return GdipGetEffectParameters(m_pEffect, &Dummy, &Params);
    }
};
// 颜色曲线
class CGpFxColorCurve : public CGpFx
{
public:
    CGpFxColorCurve() = default;
    CGpFxColorCurve(const GpColorCurveParams& Params) noexcept
    {
        GdipCreateEffect(Gdiplus::ColorCurveEffectGuid, &m_pEffect);
        GdipSetEffectParameters(m_pEffect, &Params, sizeof(Params));
    }

    CGpFxColorCurve(Gdiplus::CurveAdjustments Adjustments, Gdiplus::CurveChannel Channel,
        int nAdjustVal) noexcept
        : CGpFxColorCurve{ { Adjustments, Channel, nAdjustVal } }
    {
    }

    EckInline GpStatus SetParameters(const GpColorCurveParams& Params) noexcept
    {
        return GdipSetEffectParameters(m_pEffect, &Params, sizeof(Params));
    }

    EckInline GpStatus GetParameters(GpColorCurveParams& Params) const noexcept
    {
        UINT Dummy{ sizeof(Params) };
        return GdipGetEffectParameters(m_pEffect, &Dummy, &Params);
    }
};


EckInline GpStatus ApplyEffect(GpBitmap* pBitmap, GpEffect* pEffect,
    RECT* prcROI = nullptr, void** pAuxData = nullptr, INT* pcbAuxData = nullptr) noexcept
{
    return GdipBitmapApplyEffect(pBitmap, pEffect,
        prcROI, !!pAuxData, pAuxData, pcbAuxData);
}

EckInline GpStatus ApplyEffect(GpBitmap* pBitmap, const CGpFx& Fx,
    RECT* prcROI = nullptr, void** pAuxData = nullptr, INT* pcbAuxData = nullptr) noexcept
{
    return ApplyEffect(pBitmap, Fx.GetEffect(), prcROI, pAuxData, pcbAuxData);
}

EckInline GpStatus ApplyEffectNew(GpBitmap* pSrc, GpBitmap*& pDst, GpEffect* pEffect,
    RECT* prcROI = nullptr, RECT* prcOutput = nullptr,
    void** pAuxData = nullptr, INT* pcbAuxData = nullptr) noexcept
{
    return GdipBitmapCreateApplyEffect(&pSrc, 1, pEffect,
        prcROI, prcOutput, &pDst, !!pAuxData, pAuxData, pcbAuxData);
}

EckInline GpStatus ApplyEffectNew(GpBitmap* pSrc, GpBitmap*& pDst, const CGpFx& Fx,
    RECT* prcROI = nullptr, RECT* prcOutput = nullptr,
    void** pAuxData = nullptr, INT* pcbAuxData = nullptr) noexcept
{
    return ApplyEffectNew(pSrc, pDst, Fx.GetEffect(),
        prcROI, prcOutput, pAuxData, pcbAuxData);
}
ECK_NAMESPACE_END