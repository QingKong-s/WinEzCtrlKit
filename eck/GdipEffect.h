/*
* WinEzCtrlKit Library
*
* GdipEffect.h : GDI+效果包装
*
* Copyright(C) 2024 QingKong
*/
#pragma once
#include "ECK.h"

ECK_NAMESPACE_BEGIN
// 效果基类
class CGpFx
{
protected:
	GpEffect* m_pEffect{};
public:
	ECK_DISABLE_COPY_DEF_CONS(CGpFx);
	CGpFx(GpEffect* pEffect) : m_pEffect{ pEffect } {}

	CGpFx(CGpFx&& x) noexcept : m_pEffect{ x.m_pEffect } { x.m_pEffect = nullptr; }

	CGpFx& operator=(CGpFx&& x) noexcept { std::swap(m_pEffect, x.m_pEffect); return *this; }

	~CGpFx() { GdipDeleteEffect(m_pEffect); }

	EckInline [[nodiscard]] UINT GetParamSize() const
	{
		UINT cbSize{};
		GdipGetEffectParameterSize(m_pEffect, &cbSize);
		return cbSize;
	}

	EckInline [[nodiscard]] constexpr GpEffect* GetEffect() const { return m_pEffect; }
};
// 高斯模糊
class CGpFxBlur : public CGpFx
{
public:
	CGpFxBlur() = default;
	CGpFxBlur(const GpBlurParams& Params)
	{
		GdipCreateEffect(Gdiplus::BlurEffectGuid, &m_pEffect);
		GdipSetEffectParameters(m_pEffect, &Params, sizeof(Params));
	}

	CGpFxBlur(float fRadius, BOOL bExpandEdges) :CGpFxBlur{ { fRadius, bExpandEdges } } {}

	EckInline GpStatus SetParams(const GpBlurParams& Params)
	{
		return GdipSetEffectParameters(m_pEffect, &Params, sizeof(Params));
	}

	EckInline GpStatus GetParams(GpBlurParams& Params) const
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
	CGpFxSharpen(const GpSharpenParams& Params)
	{
		GdipCreateEffect(Gdiplus::SharpenEffectGuid, &m_pEffect);
		GdipSetEffectParameters(m_pEffect, &Params, sizeof(Params));
	}

	CGpFxSharpen(float fRadius, float fAmount) :CGpFxSharpen{ { fRadius, fAmount } } {}

	EckInline GpStatus SetParams(const GpSharpenParams& Params)
	{
		return GdipSetEffectParameters(m_pEffect, &Params, sizeof(Params));
	}

	EckInline GpStatus GetParams(GpSharpenParams& Params) const
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
	CGpFxTint(const GpTintParams& Params)
	{
		GdipCreateEffect(Gdiplus::TintEffectGuid, &m_pEffect);
		GdipSetEffectParameters(m_pEffect, &Params, sizeof(Params));
	}

	CGpFxTint(int Hue, int Amount) :CGpFxTint{ { Hue, Amount } } {}

	EckInline GpStatus SetParams(const GpTintParams& Params)
	{
		return GdipSetEffectParameters(m_pEffect, &Params, sizeof(Params));
	}

	EckInline GpStatus GetParams(GpTintParams& Params) const
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
	CGpFxRedEyeCorrection(const GpRedEyeCorrectionParams& Params)
	{
		GdipCreateEffect(Gdiplus::RedEyeCorrectionEffectGuid, &m_pEffect);
		GdipSetEffectParameters(m_pEffect, &Params, sizeof(Params));
	}

	CGpFxRedEyeCorrection(const std::vector<RECT>& vAreas)
		:CGpFxRedEyeCorrection{ { (UINT)vAreas.size(), (RECT*)vAreas.data() } } {
	}

	template<size_t N>
	CGpFxRedEyeCorrection(const RECT(&Areas)[N])
		: CGpFxRedEyeCorrection{ { (UINT)N, (RECT*)Areas } } {
	}

	CGpFxRedEyeCorrection(UINT cAreas, const RECT* pAreas)
		:CGpFxRedEyeCorrection{ { cAreas, (RECT*)pAreas } } {
	}

	EckInline GpStatus SetParams(const GpRedEyeCorrectionParams& Params)
	{
		return GdipSetEffectParameters(m_pEffect, &Params, sizeof(Params));
	}

	EckInline GpStatus GetParams(GpRedEyeCorrectionParams& Params) const
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
	CGpFxColorMatrix(const GpColorMatrix& Params)
	{
		GdipCreateEffect(Gdiplus::ColorMatrixEffectGuid, &m_pEffect);
		GdipSetEffectParameters(m_pEffect, &Params, sizeof(Params));
	}

	CGpFxColorMatrix(const float(&Matrix)[5][5])
	{
		GdipCreateEffect(Gdiplus::ColorMatrixEffectGuid, &m_pEffect);
		GdipSetEffectParameters(m_pEffect, &Matrix, sizeof(Matrix));
	}

	CGpFxColorMatrix(const float* Matrix)
	{
		GdipCreateEffect(Gdiplus::ColorMatrixEffectGuid, &m_pEffect);
		GdipSetEffectParameters(m_pEffect, Matrix, 25 * sizeof(float));
	}

	EckInline GpStatus SetParams(const GpColorMatrix& Params)
	{
		return GdipSetEffectParameters(m_pEffect, &Params, sizeof(Params));
	}

	EckInline GpStatus GetParams(GpColorMatrix& Params) const
	{
		UINT Dummy{ sizeof(Params) };
		return GdipGetEffectParameters(m_pEffect, &Dummy, &Params);
	}
};
// 颜色查找表
class CGpFxColorLUT : public CGpFx
{
public:
	CGpFxColorLUT() = default;
	CGpFxColorLUT(const GpColorLUTParams& Params)
	{
		GdipCreateEffect(Gdiplus::ColorLUTEffectGuid, &m_pEffect);
		GdipSetEffectParameters(m_pEffect, &Params, sizeof(Params));
	}

	EckInline GpStatus SetParams(const GpColorLUTParams& Params)
	{
		return GdipSetEffectParameters(m_pEffect, &Params, sizeof(Params));
	}

	EckInline GpStatus GetParams(GpColorLUTParams& Params) const
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
	CGpFxBrightnessContrast(const GpBrightnessContrastParams& Params)
	{
		GdipCreateEffect(Gdiplus::BrightnessContrastEffectGuid, &m_pEffect);
		GdipSetEffectParameters(m_pEffect, &Params, sizeof(Params));
	}

	CGpFxBrightnessContrast(int nBrightness, int nContrast)
		:CGpFxBrightnessContrast{ { nBrightness, nContrast } } {
	}

	EckInline GpStatus SetParams(const GpBrightnessContrastParams& Params)
	{
		return GdipSetEffectParameters(m_pEffect, &Params, sizeof(Params));
	}

	EckInline GpStatus GetParams(GpBrightnessContrastParams& Params) const
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
	CGpFxHSL(const GpHSLParams& Params)
	{
		GdipCreateEffect(Gdiplus::HueSaturationLightnessEffectGuid, &m_pEffect);
		GdipSetEffectParameters(m_pEffect, &Params, sizeof(Params));
	}

	/// <param name="nHue">-180~180</param>
	/// <param name="nSaturation">-100~100</param>
	/// <param name="nLightness">-100~100</param>
	CGpFxHSL(int nHue, int nSaturation, int nLightness)
		:CGpFxHSL{ { nHue, nSaturation, nLightness } } {
	}

	EckInline GpStatus SetParams(const GpHSLParams& Params)
	{
		return GdipSetEffectParameters(m_pEffect, &Params, sizeof(Params));
	}

	EckInline GpStatus GetParams(GpHSLParams& Params) const
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
	CGpFxColorBalance(const GpColorBalanceParams& Params)
	{
		GdipCreateEffect(Gdiplus::ColorBalanceEffectGuid, &m_pEffect);
		GdipSetEffectParameters(m_pEffect, &Params, sizeof(Params));
	}

	/// <param name="nCyanRed">-100~100，青->红</param>
	/// <param name="nMagentaGreen">-100~100，品红->绿</param>
	/// <param name="nYellowBlue">-100~100，黄->蓝</param>
	CGpFxColorBalance(int nCyanRed, int nMagentaGreen, int nYellowBlue)
		:CGpFxColorBalance{ { nCyanRed, nMagentaGreen, nYellowBlue } } {
	}

	EckInline GpStatus SetParams(const GpColorBalanceParams& Params)
	{
		return GdipSetEffectParameters(m_pEffect, &Params, sizeof(Params));
	}

	EckInline GpStatus GetParams(GpColorBalanceParams& Params) const
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
	CGpFxLevels(const GpLevelsParams& Params)
	{
		GdipCreateEffect(Gdiplus::LevelsEffectGuid, &m_pEffect);
		GdipSetEffectParameters(m_pEffect, &Params, sizeof(Params));
	}

	/// <param name="nHighlight">0~100</param>
	/// <param name="nMidtone">-100~100</param>
	/// <param name="nShadow">0~100</param>
	CGpFxLevels(int nHighlight, int nMidtone, int nShadow)
		:CGpFxLevels{ { nHighlight, nMidtone, nShadow } } {
	}

	EckInline GpStatus SetParams(const GpLevelsParams& Params)
	{
		return GdipSetEffectParameters(m_pEffect, &Params, sizeof(Params));
	}

	EckInline GpStatus GetParams(GpLevelsParams& Params) const
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
	CGpFxColorCurve(const GpColorCurveParams& Params)
	{
		GdipCreateEffect(Gdiplus::ColorCurveEffectGuid, &m_pEffect);
		GdipSetEffectParameters(m_pEffect, &Params, sizeof(Params));
	}

	CGpFxColorCurve(Gdiplus::CurveAdjustments Adjustments, Gdiplus::CurveChannel Channel,
		int nAdjustVal) :CGpFxColorCurve{ { Adjustments, Channel, nAdjustVal } } {
	}

	EckInline GpStatus SetParams(const GpColorCurveParams& Params)
	{
		return GdipSetEffectParameters(m_pEffect, &Params, sizeof(Params));
	}

	EckInline GpStatus GetParams(GpColorCurveParams& Params) const
	{
		UINT Dummy{ sizeof(Params) };
		return GdipGetEffectParameters(m_pEffect, &Dummy, &Params);
	}
};

EckInline GpStatus ApplyEffect(GpBitmap* pBitmap, const CGpFx& Fx,
	RECT* prcROI = nullptr, void** pAuxData = nullptr, INT* pcbAuxData = nullptr)
{
	return GdipBitmapApplyEffect(pBitmap, Fx.GetEffect(),
		prcROI, !!pAuxData, pAuxData, pcbAuxData);
}
ECK_NAMESPACE_END