#pragma once
#include "WinRtD2DInteropFx.h"
#include "Utility2.h"
#include "ImageHelper.h"
#include "CStreamView.h"

#include <windows.ui.composition.interop.h>

ECK_NAMESPACE_BEGIN
namespace becwuc = winrt::Windows::UI::Composition;
namespace becwucabi = ABI::Windows::UI::Composition;

// ==============================================
// 这些画刷提供的都是伪效果。参考：
// https://github.com/microsoft/microsoft-ui-xaml
// https://github.com/ALTaleX531/Win32Acrylic
// https://github.com/Maplespe/DWMBlurGlass
// ==============================================

inline HRESULT BecCreateNoiseBrush(
	const becwuc::Compositor& Compositor,
	IWICBitmap* pBitmap,
	becwuc::CompositionSurfaceBrush& NoiseBrush,
	ID2D1Device* pD2DDevice = nullptr)
{
	if (!pD2DDevice)
		pD2DDevice = eck::g_pD2dDevice;
	namespace wgdx = winrt::Windows::Graphics::DirectX;
	HRESULT hr;
	UINT cx, cy;
	pBitmap->GetSize(&cx, &cy);

	becwuc::CompositionGraphicsDevice Device{ nullptr };
	hr = Compositor.as<becwucabi::ICompositorInterop>()
		->CreateGraphicsDevice(pD2DDevice,
			(becwucabi::ICompositionGraphicsDevice**)winrt::put_abi(Device));
	if (FAILED(hr))
		return hr;
	becwuc::CompositionDrawingSurface Surface{
		Device.CreateDrawingSurface({ (float)cx,(float)cy },
		wgdx::DirectXPixelFormat::B8G8R8A8UIntNormalized,
		wgdx::DirectXAlphaMode::Premultiplied) };
	if (FAILED(hr))
		return hr;
	NoiseBrush = Compositor.CreateSurfaceBrush(Surface);
	auto pSurfaceInterop{ Surface.as<becwucabi::ICompositionDrawingSurfaceInterop>() };
	POINT ptOffset;
	winrt::com_ptr<ID2D1DeviceContext> pDC;
	hr = pSurfaceInterop->BeginDraw(nullptr, IID_PPV_ARGS(pDC.put()), &ptOffset);
	if (FAILED(hr))
		return hr;
	winrt::com_ptr<ID2D1Bitmap> pBmpD2D;
	hr = pDC->CreateBitmapFromWicBitmap(pBitmap, nullptr, pBmpD2D.put());
	if (FAILED(hr))
		return hr;
	pDC->BeginDraw();
	pDC->Clear({});
	D2D1_POINT_2F ptOffsetF{ (float)ptOffset.x,(float)ptOffset.y };
	pDC->DrawImage(pBmpD2D.get(), &ptOffsetF);
	pDC->EndDraw();
	hr = pSurfaceInterop->EndDraw();
	return hr;
}

inline HRESULT BecCreateNoiseBrush(const becwuc::Compositor& Compositor,
	becwuc::CompositionSurfaceBrush& NoiseBrush)
{
	const auto hModule = LoadLibraryExW(L"Windows.UI.Xaml.Controls.dll",
		nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32 | LOAD_LIBRARY_AS_DATAFILE |
		LOAD_LIBRARY_AS_IMAGE_RESOURCE);
	if (!hModule)
		return HRESULT_FROM_WIN32(NaGetLastError());
	const auto spImgData = GetResource(MAKEINTRESOURCEW(2000), RT_RCDATA, hModule);
	IWICBitmap* pBmp;
	const auto pStream = new CStreamView{ spImgData };
	auto hr = CreateWicBitmap(pBmp, pStream);
	if (FAILED(hr))
		return hr;
	hr = BecCreateNoiseBrush(Compositor, pBmp, NoiseBrush);
	pBmp->Release();
	pStream->LeaveRelease();
	FreeLibrary(hModule);
	return hr;
}

namespace Priv
{
	struct BecRgb { double r, g, b; };
	struct BecHsv { double h, s, v; };

	inline BecHsv BecpRgbToHsv(const BecRgb& rgb)
	{
		double hue = 0;
		double saturation = 0;
		double value = 0;
		const double max = rgb.r >= rgb.g ? (rgb.r >= rgb.b ? rgb.r : rgb.b) : (rgb.g >= rgb.b ? rgb.g : rgb.b);
		const double min = rgb.r <= rgb.g ? (rgb.r <= rgb.b ? rgb.r : rgb.b) : (rgb.g <= rgb.b ? rgb.g : rgb.b);
		value = max;
		const double chroma = max - min;
		if (chroma == 0)
		{
			hue = 0.0;
			saturation = 0.0;
		}
		else
		{
			if (rgb.r == max)
				hue = 60 * (rgb.g - rgb.b) / chroma;
			else if (rgb.g == max)
				hue = 120 + 60 * (rgb.b - rgb.r) / chroma;
			else
				hue = 240 + 60 * (rgb.r - rgb.g) / chroma;
			if (hue < 0.0)
				hue += 360.0;
			saturation = chroma / value;
		}

		return { hue, saturation, value };
	}

	inline BecRgb BecpHsvToRgb(const BecHsv& hsv)
	{
		double hue = hsv.h;
		double saturation = hsv.s;
		double value = hsv.v;
		while (hue >= 360.0)
			hue -= 360.0;
		while (hue < 0.0)
			hue += 360.0;
		saturation = saturation < 0.0 ? 0.0 : saturation;
		saturation = saturation > 1.0 ? 1.0 : saturation;
		value = value < 0.0 ? 0.0 : value;
		value = value > 1.0 ? 1.0 : value;
		const double chroma = saturation * value;
		const double min = value - chroma;
		if (chroma == 0)
			return { min, min, min };
		const int sextant = static_cast<int>(hue / 60);
		const double intermediateColorPercentage = hue / 60 - sextant;
		const double max = chroma + min;
		double r = 0;
		double g = 0;
		double b = 0;
		switch (sextant)
		{
		case 0:
			r = max;
			g = min + chroma * intermediateColorPercentage;
			b = min;
			break;
		case 1:
			r = min + chroma * (1 - intermediateColorPercentage);
			g = max;
			b = min;
			break;
		case 2:
			r = min;
			g = max;
			b = min + chroma * intermediateColorPercentage;
			break;
		case 3:
			r = min;
			g = min + chroma * (1 - intermediateColorPercentage);
			b = max;
			break;
		case 4:
			r = min + chroma * intermediateColorPercentage;
			g = min;
			b = max;
			break;
		case 5:
			r = max;
			g = min;
			b = min + chroma * (1 - intermediateColorPercentage);
			break;
		}
		return { r, g, b };
	}

	inline double BecpGetTintOpacityModifier(const D2D1_COLOR_F& tintColor)
	{
		const double midPoint = 0.50;
		const double whiteMaxOpacity = 0.45;
		const double midPointMaxOpacity = 0.90;
		const double blackMaxOpacity = 0.85;
		const BecRgb rgb{ tintColor.r, tintColor.g, tintColor.b };
		const BecHsv hsv = BecpRgbToHsv(rgb);
		double opacityModifier = midPointMaxOpacity;
		if (hsv.v != midPoint)
		{
			double lowestMaxOpacity = midPointMaxOpacity;
			double maxDeviation = midPoint;
			if (hsv.v > midPoint)
			{
				lowestMaxOpacity = whiteMaxOpacity;
				maxDeviation = 1 - maxDeviation;
			}
			else if (hsv.v < midPoint)
				lowestMaxOpacity = blackMaxOpacity;
			double maxOpacitySuppression = midPointMaxOpacity - lowestMaxOpacity;
			const double deviation = abs(hsv.v - midPoint);
			const double normalizedDeviation = deviation / maxDeviation;
			if (hsv.s > 0)
				maxOpacitySuppression *= std::max(1 - (hsv.s * 2), 0.0);
			const double opacitySuppression = maxOpacitySuppression * normalizedDeviation;
			opacityModifier = midPointMaxOpacity - opacitySuppression;
		}
		return opacityModifier;
	}

	inline void BecpGetLuminosityColor(_Inout_ D2D1_COLOR_F& tintColor,
		std::optional<double> luminosityOpacity)
	{
		const BecRgb rgbTintColor{ tintColor.r, tintColor.g, tintColor.b };
		if (luminosityOpacity)
			tintColor.a = (float)std::clamp(luminosityOpacity.value(), 0.0, 1.0);
		else
		{
			const double minHsvV = 0.125;
			const double maxHsvV = 0.965;

			const BecHsv hsvTintColor = BecpRgbToHsv(rgbTintColor);

			const auto clampedHsvV = std::clamp(hsvTintColor.v, minHsvV, maxHsvV);

			const BecHsv hsvLuminosityColor{ hsvTintColor.h, hsvTintColor.s, clampedHsvV };
			const BecRgb rgbLuminosityColor = BecpHsvToRgb(hsvLuminosityColor);
			const double minLuminosityOpacity = 0.15;
			const double maxLuminosityOpacity = 1.03;

			const double luminosityOpacityRangeMax = maxLuminosityOpacity - minLuminosityOpacity;
			const double mappedTintOpacity = (tintColor.a * luminosityOpacityRangeMax) + minLuminosityOpacity;
			tintColor = { (float)rgbLuminosityColor.r, (float)rgbLuminosityColor.g, (float)rgbLuminosityColor.b, (float)mappedTintOpacity };
		}
	}
}

EckInline void BecGetEffectiveTintColor(_Inout_ D2D1_COLOR_F& TintColor,
	float TintOpacity, bool TintLuminosityOpacity)
{
	if (TintLuminosityOpacity)
		TintColor.a *= TintOpacity;
	else
	{
		const double tintOpacityModifier = Priv::BecpGetTintOpacityModifier(TintColor);
		TintColor.a = float(TintColor.a * TintOpacity * tintOpacityModifier);
	}
}

EckInline void BecGetEffectiveLuminosityColor(_Inout_ D2D1_COLOR_F& TintColor,
	float TintOpacity, std::optional<float> TintLuminosityOpacity)
{
	TintColor.a *= TintOpacity;
	Priv::BecpGetLuminosityColor(TintColor, TintLuminosityOpacity);
}

EckInline void BecGetEffectiveColor(_Inout_ D2D1_COLOR_F& TintColor,
	_Out_ D2D1_COLOR_F& LuminosityColor,
	float TintOpacity, std::optional<float> TintLuminosityOpacity)
{
	LuminosityColor = TintColor;
	BecGetEffectiveTintColor(TintColor, TintOpacity, TintLuminosityOpacity.has_value());
	BecGetEffectiveLuminosityColor(LuminosityColor, TintOpacity, TintLuminosityOpacity);
}

struct CBecAcrylic
{
	constexpr static WCHAR NNoise[]{ L"Noise" };
	constexpr static WCHAR NBackdrop[]{ L"Backdrop" };
	/*
	NoiseTexture--FxBorder--FxOpacity---------------------------+
																|
	Backdrop--FxBlur----+                                       +--FxBlendNoise
						+--FxBlendLuminosity----+               |
	FxFloodLuminosity---+                       +--FxBlendTint--+
							FxFloodTint---------+
	*/

	winrt::com_ptr<CWrdiFxBorder> FxBorder{ winrt::make_self<CWrdiFxBorder>() };
	winrt::com_ptr<CWrdiFxOpacity> FxOpacity{ winrt::make_self<CWrdiFxOpacity>() };
	winrt::com_ptr<CWrdiFxBlend> FxBlendNoise{ winrt::make_self<CWrdiFxBlend>() };
	winrt::com_ptr<CWrdiFxGaussianBlur> FxBlur{ winrt::make_self<CWrdiFxGaussianBlur>() };
	winrt::com_ptr<CWrdiFxBlend> FxBlendLuminosity{ winrt::make_self<CWrdiFxBlend>() };
	winrt::com_ptr<CWrdiFxFlood> FxFloodLuminosity{ winrt::make_self<CWrdiFxFlood>() };
	winrt::com_ptr<CWrdiFxBlend> FxBlendTint{ winrt::make_self<CWrdiFxBlend>() };
	winrt::com_ptr<CWrdiFxFlood> FxFloodTint{ winrt::make_self<CWrdiFxFlood>() };

	winrt::com_ptr<CWinRtD2DInteropFx> GetOutputEffect() const { return FxBlendNoise; }

	void Connect(
		const D2D1_COLOR_F& TintColor,
		const D2D1_COLOR_F& LuminosityColor,
		float fStandardDeviation = 20.f,
		float fNoiseOpacity = 0.02f)
	{
		FxBorder->SetInput(0, NNoise);
		FxBorder->SetValueT(D2D1_BORDER_PROP_EDGE_MODE_X, D2D1_BORDER_EDGE_MODE_WRAP);
		FxBorder->SetValueT(D2D1_BORDER_PROP_EDGE_MODE_Y, D2D1_BORDER_EDGE_MODE_WRAP);

		FxOpacity->SetInput(0, FxBorder.get());
		FxOpacity->SetValueT(D2D1_OPACITY_PROP_OPACITY, fNoiseOpacity);

		FxBlur->SetInput(0, NBackdrop);
		FxBlur->SetValueT(D2D1_GAUSSIANBLUR_PROP_STANDARD_DEVIATION, fStandardDeviation);
		FxBlur->SetValueT(D2D1_GAUSSIANBLUR_PROP_BORDER_MODE, D2D1_BORDER_MODE_HARD);
		FxBlur->SetValueT(D2D1_GAUSSIANBLUR_PROP_OPTIMIZATION,
			D2D1_GAUSSIANBLUR_OPTIMIZATION_SPEED);

		FxFloodLuminosity->SetValueT(D2D1_FLOOD_PROP_COLOR, LuminosityColor);

		FxBlendLuminosity->SetValueT(D2D1_BLEND_PROP_MODE, D2D1_BLEND_MODE_COLOR);
		FxBlendLuminosity->SetInput(0, FxBlur.get());
		FxBlendLuminosity->SetInput(1, FxFloodLuminosity.get());

		FxFloodTint->SetValueT(D2D1_FLOOD_PROP_COLOR, TintColor);

		FxBlendTint->SetValueT(D2D1_BLEND_PROP_MODE, D2D1_BLEND_MODE_LUMINOSITY);
		FxBlendTint->SetInput(0, FxBlendLuminosity.get());
		FxBlendTint->SetInput(1, FxFloodTint.get());

		FxBlendNoise->SetValueT(D2D1_BLEND_PROP_MODE, D2D1_BLEND_MODE_MULTIPLY);
		FxBlendNoise->SetInput(0, FxBlendTint.get());
		FxBlendNoise->SetInput(1, FxOpacity.get());
	}

	becwuc::CompositionEffectBrush CreateBrush(
		const becwuc::Compositor& Compositor,
		const becwuc::CompositionBrush& BackdropBrush,
		const becwuc::CompositionBrush& NoiseBrush,
		becwuc::CompositionEffectFactory* pEffectFactory = nullptr) const
	{
		auto Factory{ Compositor.CreateEffectFactory(
			GetOutputEffect().as<winrt::Windows::Graphics::Effects::IGraphicsEffect>()) };
		auto Brush{ Factory.CreateBrush() };
		Brush.SetSourceParameter(NBackdrop, BackdropBrush);
		Brush.SetSourceParameter(NNoise, NoiseBrush);
		if (pEffectFactory)
			*pEffectFactory = std::move(Factory);
		return Brush;
	}
};

struct CBecAcrylicLegacy
{
	constexpr static WCHAR NNoise[]{ L"Noise" };
	constexpr static WCHAR NBackdrop[]{ L"Backdrop" };
	/*
	NoiseTexture--FxBorder--FxOpacity-----------------------------------+
																		|
	Backdrop--FxBlur--FxSaturation--+                                   +--FxBlendNoise
									+--FxBlendExclusion-+               |
	FxFloodExclusion----------------+                   +--FxComposite--+
										FxFloodTint-----+
	*/

	winrt::com_ptr<CWrdiFxBorder> FxBorder{ winrt::make_self<CWrdiFxBorder>() };
	winrt::com_ptr<CWrdiFxOpacity> FxOpacity{ winrt::make_self<CWrdiFxOpacity>() };
	winrt::com_ptr<CWrdiFxBlend> FxBlendNoise{ winrt::make_self<CWrdiFxBlend>() };
	winrt::com_ptr<CWrdiFxGaussianBlur> FxBlur{ winrt::make_self<CWrdiFxGaussianBlur>() };
	winrt::com_ptr<CWrdiFxSaturation> FxSaturation{ winrt::make_self<CWrdiFxSaturation>() };
	winrt::com_ptr<CWrdiFxBlend> FxBlendExclusion{ winrt::make_self<CWrdiFxBlend>() };
	winrt::com_ptr<CWrdiFxFlood> FxFloodExclusion{ winrt::make_self<CWrdiFxFlood>() };
	winrt::com_ptr<CWrdiFxComposite> FxComposite{ winrt::make_self<CWrdiFxComposite>() };
	winrt::com_ptr<CWrdiFxFlood> FxFloodTint{ winrt::make_self<CWrdiFxFlood>() };

	winrt::com_ptr<CWinRtD2DInteropFx> GetOutputEffect() const { return FxBlendNoise; }

	void Connect(
		const D2D1_COLOR_F& TintColor,
		const D2D1_COLOR_F& ExclusionColor = { 1.f,1.f,1.f,0.1f },
		float fStandardDeviation = 20.f,
		float fNoiseOpacity = 0.02f,
		float fSaturation = 1.25f)
	{
		FxBorder->SetInput(0, NNoise);
		FxBorder->SetValueT(D2D1_BORDER_PROP_EDGE_MODE_X, D2D1_BORDER_EDGE_MODE_WRAP);
		FxBorder->SetValueT(D2D1_BORDER_PROP_EDGE_MODE_Y, D2D1_BORDER_EDGE_MODE_WRAP);

		FxOpacity->SetInput(0, FxBorder.get());
		FxOpacity->SetValueT(D2D1_OPACITY_PROP_OPACITY, fNoiseOpacity);

		FxBlur->SetInput(0, NBackdrop);
		FxBlur->SetValueT(D2D1_GAUSSIANBLUR_PROP_STANDARD_DEVIATION, fStandardDeviation);
		FxBlur->SetValueT(D2D1_GAUSSIANBLUR_PROP_BORDER_MODE, D2D1_BORDER_MODE_HARD);
		FxBlur->SetValueT(D2D1_GAUSSIANBLUR_PROP_OPTIMIZATION,
			D2D1_GAUSSIANBLUR_OPTIMIZATION_BALANCED);

		FxSaturation->SetInput(0, FxBlur.get());
		FxSaturation->SetValueT(D2D1_SATURATION_PROP_SATURATION, fSaturation);

		FxFloodExclusion->SetValueT(D2D1_FLOOD_PROP_COLOR, ExclusionColor);

		FxBlendExclusion->SetValueT(D2D1_BLEND_PROP_MODE, D2D1_BLEND_MODE_EXCLUSION);
		FxBlendExclusion->SetInput(0, FxSaturation.get());
		FxBlendExclusion->SetInput(1, FxFloodExclusion.get());

		FxFloodTint->SetValueT(D2D1_FLOOD_PROP_COLOR, TintColor);

		FxComposite->SetValueT(D2D1_COMPOSITE_PROP_MODE, D2D1_COMPOSITE_MODE_SOURCE_OVER);
		FxComposite->SetInput(0, FxBlendExclusion.get());
		FxComposite->SetInput(1, FxFloodTint.get());

		FxBlendNoise->SetValueT(D2D1_BLEND_PROP_MODE, D2D1_BLEND_MODE_MULTIPLY);
		FxBlendNoise->SetInput(0, FxComposite.get());
		FxBlendNoise->SetInput(1, FxOpacity.get());
	}

	becwuc::CompositionEffectBrush CreateBrush(
		const becwuc::Compositor& Compositor,
		const becwuc::CompositionBrush& BackdropBrush,
		const becwuc::CompositionBrush& NoiseBrush,
		becwuc::CompositionEffectFactory* pEffectFactory = nullptr) const
	{
		auto Factory{ Compositor.CreateEffectFactory(
			GetOutputEffect().as<winrt::Windows::Graphics::Effects::IGraphicsEffect>()) };
		auto Brush{ Factory.CreateBrush() };
		Brush.SetSourceParameter(NBackdrop, BackdropBrush);
		Brush.SetSourceParameter(NNoise, NoiseBrush);
		if (pEffectFactory)
			*pEffectFactory = std::move(Factory);
		return Brush;
	}
};

struct CBecMica
{
	constexpr static WCHAR NNoise[]{ L"Noise" };
	constexpr static WCHAR NBlurredWallpaper[]{ L"BlurredWallpaper" };
	/*
	NoiseTexture--FxBorder--FxOpacity---------------------------+
																|
	BlurredWallpaper----+                                       +--FxBlendNoise
						+--FxBlendLuminosity----+               |
	FxFloodLuminosity---+                       +--FxBlendTint--+
							FxFloodTint---------+

	BlurredWallpaper -> FxColorMatrix -> FxBlendLuminosity is optional.
	*/

	winrt::com_ptr<CWrdiFxBorder> FxBorder{ winrt::make_self<CWrdiFxBorder>() };
	winrt::com_ptr<CWrdiFxOpacity> FxOpacity{ winrt::make_self<CWrdiFxOpacity>() };
	winrt::com_ptr<CWrdiFxBlend> FxBlendNoise{ winrt::make_self<CWrdiFxBlend>() };
	winrt::com_ptr<CWrdiFxBlend> FxBlendLuminosity{ winrt::make_self<CWrdiFxBlend>() };
	winrt::com_ptr<CWrdiFxFlood> FxFloodLuminosity{ winrt::make_self<CWrdiFxFlood>() };
	winrt::com_ptr<CWrdiFxBlend> FxBlendTint{ winrt::make_self<CWrdiFxBlend>() };
	winrt::com_ptr<CWrdiFxFlood> FxFloodTint{ winrt::make_self<CWrdiFxFlood>() };
	winrt::com_ptr<CWrdiFxColorMatrix> FxColorMatrix{ winrt::make_self<CWrdiFxColorMatrix>() };

	winrt::com_ptr<CWinRtD2DInteropFx> GetOutputEffect() const { return FxBlendNoise; }

	void Connect(
		const D2D1_COLOR_F& TintColor,
		const D2D1_COLOR_F& LuminosityColor,
		BOOL bAdjustColor = TRUE,
		float fBrightness = 0.4f,
		float fSaturation = 5.f,
		float fContrast = 0.f,
		float fNoiseOpacity = 0.02f)
	{
		FxBorder->SetInput(0, NNoise);
		FxBorder->SetValueT(D2D1_BORDER_PROP_EDGE_MODE_X, D2D1_BORDER_EDGE_MODE_WRAP);
		FxBorder->SetValueT(D2D1_BORDER_PROP_EDGE_MODE_Y, D2D1_BORDER_EDGE_MODE_WRAP);

		FxOpacity->SetInput(0, FxBorder.get());
		FxOpacity->SetValueT(D2D1_OPACITY_PROP_OPACITY, fNoiseOpacity);

		FxFloodLuminosity->SetValueT(D2D1_FLOOD_PROP_COLOR, LuminosityColor);

		if (bAdjustColor)
		{
			D2D1::Matrix5x4F Mat{};
			Mat.m[4][0] = Mat.m[4][1] = Mat.m[4][2] = fBrightness;
			Mat.m[0][0] = Mat.m[0][1] = Mat.m[0][2] = 0.299f * (1.f - fSaturation);
			Mat.m[1][0] = Mat.m[1][1] = Mat.m[1][2] = 0.587f * (1.f - fSaturation);
			Mat.m[2][0] = Mat.m[2][1] = Mat.m[2][2] = 0.114f * (1.f - fSaturation);
			Mat.m[0][0] += fSaturation;
			Mat.m[1][1] += fSaturation;
			Mat.m[2][2] += fSaturation;
			Mat.m[0][0] += fContrast;
			Mat.m[1][1] += fContrast;
			Mat.m[2][2] += fContrast;

			FxColorMatrix->SetInput(0, NBlurredWallpaper);
			FxColorMatrix->SetValueT(D2D1_COLORMATRIX_PROP_COLOR_MATRIX, Mat);
			FxColorMatrix->SetValueT(D2D1_COLORMATRIX_PROP_ALPHA_MODE,
				D2D1_COLORMATRIX_ALPHA_MODE_PREMULTIPLIED);
			FxColorMatrix->SetValueT(D2D1_COLORMATRIX_PROP_CLAMP_OUTPUT, true);
			FxBlendLuminosity->SetInput(0, FxColorMatrix.get());
		}
		else
			FxBlendLuminosity->SetInput(0, NBlurredWallpaper);
		FxBlendLuminosity->SetValueT(D2D1_BLEND_PROP_MODE, D2D1_BLEND_MODE_COLOR);
		FxBlendLuminosity->SetInput(1, FxFloodLuminosity.get());

		FxFloodTint->SetValueT(D2D1_FLOOD_PROP_COLOR, TintColor);

		FxBlendTint->SetValueT(D2D1_BLEND_PROP_MODE, D2D1_BLEND_MODE_LUMINOSITY);
		FxBlendTint->SetInput(0, FxBlendLuminosity.get());
		FxBlendTint->SetInput(1, FxFloodTint.get());

		FxBlendNoise->SetValueT(D2D1_BLEND_PROP_MODE, D2D1_BLEND_MODE_MULTIPLY);
		FxBlendNoise->SetInput(0, FxBlendTint.get());
		FxBlendNoise->SetInput(1, FxOpacity.get());
	}

	becwuc::CompositionEffectBrush CreateBrush(
		const becwuc::Compositor& Compositor,
		const becwuc::CompositionBrush& BlurredWallpaperBrush,
		const becwuc::CompositionBrush& NoiseBrush,
		becwuc::CompositionEffectFactory* pEffectFactory = nullptr) const
	{
		auto Factory{ Compositor.CreateEffectFactory(
			GetOutputEffect().as<winrt::Windows::Graphics::Effects::IGraphicsEffect>()) };
		auto Brush{ Factory.CreateBrush() };
		Brush.SetSourceParameter(NBlurredWallpaper, BlurredWallpaperBrush);
		Brush.SetSourceParameter(NNoise, NoiseBrush);
		if (pEffectFactory)
			*pEffectFactory = std::move(Factory);
		return Brush;
	}
};

struct CBecAero
{
	constexpr static WCHAR NBlurredBackdrop[]{ L"Blurred" };
	/*
	BlurredBackdrop = Backdrop--FxBlur

	FxFloodFallback-----------------+
									+--FxCompositeNoTrans--FxTintGlow
	BlurredBackdrop--FxSaturation---+                          |
	BlurredBackdrop--FxTint-----+                              |
								+--FxComposite---------+       |
	BlurredBackdrop--FxOpacity--+                      |       |
													FxCompositeGlow
															|
								+---<<<---------------------+
		FxCompositeFinal--<<<---+
								+---<<<---FxFloodMainColor
	*/

	winrt::com_ptr<CWrdiFxGaussianBlur> FxBlur{ winrt::make_self<CWrdiFxGaussianBlur>() };
	winrt::com_ptr<CWrdiFxFlood> FxFloodFallback{ winrt::make_self<CWrdiFxFlood>() };
	winrt::com_ptr<CWrdiFxSaturation> FxSaturation{ winrt::make_self<CWrdiFxSaturation>() };
	winrt::com_ptr<CWrdiFxTint> FxTint{ winrt::make_self<CWrdiFxTint>() };
	winrt::com_ptr<CWrdiFxComposite> FxCompositeNoTrans{ winrt::make_self<CWrdiFxComposite>() };
	winrt::com_ptr<CWrdiFxTint> FxTintGlow{ winrt::make_self<CWrdiFxTint>() };
	winrt::com_ptr<CWrdiFxOpacity> FxOpacity{ winrt::make_self<CWrdiFxOpacity>() };
	winrt::com_ptr<CWrdiFxComposite> FxComposite{ winrt::make_self<CWrdiFxComposite>() };
	winrt::com_ptr<CWrdiFxComposite> FxCompositeGlow{ winrt::make_self<CWrdiFxComposite>() };
	winrt::com_ptr<CWrdiFxFlood> FxFloodMainColor{ winrt::make_self<CWrdiFxFlood>() };
	winrt::com_ptr<CWrdiFxComposite> FxCompositeFinal{ winrt::make_self<CWrdiFxComposite>() };

	winrt::com_ptr<CWinRtD2DInteropFx> GetOutputEffect() const { return FxCompositeFinal; }

	void Connect(
		const D2D1_COLOR_F& MainColor,
		const D2D1_COLOR_F& GlowColor,
		float fBlurOpacity = 0.49f)
	{
		FxSaturation->SetInput(0, NBlurredBackdrop);
		FxSaturation->SetValueT(D2D1_SATURATION_PROP_SATURATION, 0.f);

		const D2D1_COLOR_F FallbackColor
		{
			std::min(fBlurOpacity + 0.1f, 1.f),
			std::min(fBlurOpacity + 0.1f, 1.f),
			std::min(fBlurOpacity + 0.1f, 1.f),
			1.f
		};
		FxFloodFallback->SetValueT(D2D1_FLOOD_PROP_COLOR, FallbackColor);

		FxCompositeNoTrans->SetInput(0, FxFloodFallback.get());
		FxCompositeNoTrans->SetInput(1, FxSaturation.get());
		FxCompositeNoTrans->SetValueT(D2D1_COMPOSITE_PROP_MODE,
			D2D1_COMPOSITE_MODE_SOURCE_OVER);

		FxTintGlow->SetInput(0, FxCompositeNoTrans.get());
		FxTintGlow->SetValueT(D2D1_TINT_PROP_COLOR, GlowColor);
		FxTintGlow->SetValueT(D2D1_TINT_PROP_CLAMP_OUTPUT, false);
		//---
		FxTint->SetInput(0, NBlurredBackdrop);
		FxTint->SetValueT(D2D1_TINT_PROP_COLOR, D2D1_COLOR_F{ .a = 1.f });
		FxTint->SetValueT(D2D1_TINT_PROP_CLAMP_OUTPUT, false);

		FxOpacity->SetInput(0, NBlurredBackdrop);
		FxOpacity->SetValueT(D2D1_OPACITY_PROP_OPACITY, fBlurOpacity);

		FxComposite->SetInput(0, FxTint.get());
		FxComposite->SetInput(1, FxOpacity.get());
		FxComposite->SetValueT(D2D1_COMPOSITE_PROP_MODE, D2D1_COMPOSITE_MODE_PLUS);
		//---
		FxCompositeGlow->SetInput(0, FxComposite.get());
		FxCompositeGlow->SetInput(1, FxTintGlow.get());
		FxCompositeGlow->SetValueT(D2D1_COMPOSITE_PROP_MODE, D2D1_COMPOSITE_MODE_PLUS);

		FxFloodMainColor->SetValueT(D2D1_FLOOD_PROP_COLOR, MainColor);

		FxCompositeFinal->SetInput(0, FxCompositeGlow.get());
		FxCompositeFinal->SetInput(1, FxFloodMainColor.get());
		FxCompositeFinal->SetValueT(D2D1_COMPOSITE_PROP_MODE, D2D1_COMPOSITE_MODE_PLUS);
	}

	// 若要使用预设值，颜色透明通道建议设为1
	void ConnectWithFactor(
		const D2D1_COLOR_F& MainColor,
		const D2D1_COLOR_F& GlowColor,
		float fMainColorOpacity = 0.08f,
		float fGlowColorOpacity = 0.43f,
		float fBlurOpacity = 0.49f)
	{
		auto crMain{ MainColor }, crGlow{ GlowColor };
		crMain.a *= fMainColorOpacity;
		crGlow.a *= fGlowColorOpacity;
		Connect(crMain, crGlow, fBlurOpacity);
	}

	becwuc::CompositionEffectBrush CreateBrush(
		const becwuc::Compositor& Compositor,
		const becwuc::CompositionBrush& BlurredBackdropBrush,
		becwuc::CompositionEffectFactory* pEffectFactory = nullptr) const
	{
		auto Factory{ Compositor.CreateEffectFactory(
			GetOutputEffect().as<winrt::Windows::Graphics::Effects::IGraphicsEffect>()) };
		auto Brush{ Factory.CreateBrush() };
		Brush.SetSourceParameter(NBlurredBackdrop, BlurredBackdropBrush);
		if (pEffectFactory)
			*pEffectFactory = std::move(Factory);
		return Brush;
	}

	becwuc::CompositionEffectBrush CreateBrushAuto(
		const becwuc::Compositor& Compositor,
		const becwuc::CompositionBrush& BackdropBrush,
		float fStandardDeviation = 6.f,
		becwuc::CompositionEffectFactory* pEffectFactory = nullptr) const
	{
		FxBlur->SetInput(0, L"PRIV");
		FxBlur->SetValueT(D2D1_GAUSSIANBLUR_PROP_STANDARD_DEVIATION, fStandardDeviation);
		FxBlur->SetValueT(D2D1_GAUSSIANBLUR_PROP_BORDER_MODE, D2D1_BORDER_MODE_HARD);
		FxBlur->SetValueT(D2D1_GAUSSIANBLUR_PROP_OPTIMIZATION,
			D2D1_GAUSSIANBLUR_OPTIMIZATION_SPEED);
		auto BlurFactory{ Compositor.CreateEffectFactory(FxBlur.
			as<winrt::Windows::Graphics::Effects::IGraphicsEffect>()) };
		auto BlurBrush{ BlurFactory.CreateBrush() };
		BlurBrush.SetSourceParameter(L"PRIV", BackdropBrush);
		return CreateBrush(Compositor, BlurBrush, pEffectFactory);
	}
};

struct CBecBlur
{
	constexpr static WCHAR NBackdrop[]{ L"Backdrop" };
	/*
	Backdrop--FxBlur----+
						+--FxCompositeTint
	FxFlood-------------+
	*/

	winrt::com_ptr<CWrdiFxGaussianBlur> FxBlur{ winrt::make_self<CWrdiFxGaussianBlur>() };
	winrt::com_ptr<CWrdiFxFlood> FxFlood{ winrt::make_self<CWrdiFxFlood>() };
	winrt::com_ptr<CWrdiFxComposite> FxCompositeTint{ winrt::make_self<CWrdiFxComposite>() };

	winrt::com_ptr<CWinRtD2DInteropFx> GetOutputEffect() const { return FxCompositeTint; }

	void Connect(const D2D1_COLOR_F& TintColor, float fStandardDeviation = 6.f)
	{
		FxBlur->SetInput(0, NBackdrop);
		FxBlur->SetValueT(D2D1_GAUSSIANBLUR_PROP_STANDARD_DEVIATION, fStandardDeviation);
		FxBlur->SetValueT(D2D1_GAUSSIANBLUR_PROP_BORDER_MODE, D2D1_BORDER_MODE_HARD);
		FxBlur->SetValueT(D2D1_GAUSSIANBLUR_PROP_OPTIMIZATION,
			D2D1_GAUSSIANBLUR_OPTIMIZATION_BALANCED);

		FxFlood->SetValueT(D2D1_FLOOD_PROP_COLOR, TintColor);

		FxCompositeTint->SetInput(0, FxBlur.get());
		FxCompositeTint->SetInput(1, FxFlood.get());
		FxCompositeTint->SetValueT(D2D1_COMPOSITE_PROP_MODE,
			D2D1_COMPOSITE_MODE_SOURCE_OVER);
	}

	becwuc::CompositionEffectBrush CreateBrush(
		const becwuc::Compositor& Compositor,
		const becwuc::CompositionBrush& BackdropBrush,
		becwuc::CompositionEffectFactory* pEffectFactory = nullptr) const
	{
		auto Factory{ Compositor.CreateEffectFactory(
			GetOutputEffect().as<winrt::Windows::Graphics::Effects::IGraphicsEffect>()) };
		auto Brush{ Factory.CreateBrush() };
		Brush.SetSourceParameter(NBackdrop, BackdropBrush);
		if (pEffectFactory)
			*pEffectFactory = std::move(Factory);
		return Brush;
	}
};
ECK_NAMESPACE_END