/*
* WinEzCtrlKit Library
*
* CDuiColorTheme.h ： DUI颜色主题
*
* Copyright(C) 2024 QingKong
*/
#pragma once
#include "CWnd.h"

#define ECK_DUI_NAMESPACE_BEGIN namespace Dui {
#define ECK_DUI_NAMESPACE_END }

ECK_NAMESPACE_BEGIN
ECK_DUI_NAMESPACE_BEGIN
struct COLORTHEME
{
	// Text
	D2D1_COLOR_F crTextNormal;
	D2D1_COLOR_F crTextHot;
	D2D1_COLOR_F crTextSelected;
	D2D1_COLOR_F crTextDisabled;
	// Background
	D2D1_COLOR_F crBkNormal;
	D2D1_COLOR_F crBkHot;
	D2D1_COLOR_F crBkSelected;
	D2D1_COLOR_F crBkDisabled;
	D2D1_COLOR_F crBkHotSel;
	// Others
	D2D1_COLOR_F crShadow;
	D2D1_COLOR_F crBorder;
};

class CColorTheme
{
protected:
	ULONG m_cRef = 1;

	COLORTHEME m_Theme{};
public:
	CColorTheme() = default;
	CColorTheme(const COLORTHEME& Theme) :m_Theme(Theme){}

	EckInline ULONG Ref() { return ++m_cRef; }

	EckInline ULONG DeRef()
	{
		if (m_cRef == 1)
		{
			delete this;
			return 0;
		}
		return --m_cRef;
	}

	EckInline void Set(const COLORTHEME& Theme) { m_Theme = Theme; }

	EckInline const COLORTHEME& Get() const { return m_Theme; }

	EckInline COLORTHEME& Get() { return m_Theme; }
};

enum
{
	CTI_BUTTON,
	CTI_LIST,
	CTI_LABEL,
	CTI_TRACKBAR,
	CTI_CIRCLEBUTTON,
	CTI_SCROLLBAR,

	CTI_COUNT
};

inline void MakeStdThemeLight(CColorTheme** pTheme)
{
	constexpr auto c_Black = ColorrefToD2dColorF(Colorref::Black);
	constexpr auto c_White = ColorrefToD2dColorF(Colorref::White);
	constexpr auto c_Gray = ColorrefToD2dColorF(Colorref::Gray);
	constexpr auto c_DisableText = ColorrefToD2dColorF(Colorref::NeutralGray);
	constexpr auto c_DisableBkg = RgbToD2dColorF(0xfbfbf9);
	constexpr auto c_Unused = RgbToD2dColorF(0);
	pTheme[CTI_BUTTON] = new CColorTheme{};
	pTheme[CTI_BUTTON]->Set(
		{
			c_Black,c_Black,RgbToD2dColorF(0x707070),c_DisableText,
			RgbToD2dColorF(0xfefefd),RgbToD2dColorF(0xfafbfa),RgbToD2dColorF(0xfafbfa),c_DisableBkg,RgbToD2dColorF(0xfafbfa),
			RgbToD2dColorF(0xd3d3d2),RgbToD2dColorF(0xecedeb)
		});

	pTheme[CTI_LIST] = new CColorTheme{};
	pTheme[CTI_LIST]->Set(
		COLORTHEME
		{
			.crTextNormal = c_Black,
			.crBkHot = ColorrefToD2dColorF(Colorref::Black, 0.1f),
			.crBkSelected = ColorrefToD2dColorF(Colorref::Black, 0.2f),
			.crBkHotSel = ColorrefToD2dColorF(Colorref::Black, 0.3f),
		});
	pTheme[CTI_LIST]->Set(
		COLORTHEME
		{
			.crTextNormal = c_White,
			.crBkHot = ColorrefToD2dColorF(Colorref::White, 0.3f),
			.crBkSelected = ColorrefToD2dColorF(Colorref::White, 0.4f),
			.crBkHotSel = ColorrefToD2dColorF(Colorref::White, 0.5f),
		});

	pTheme[CTI_LABEL] = new CColorTheme{};
	pTheme[CTI_LABEL]->Set(
		{
			c_Black,c_Black,c_Black,c_DisableText,
			c_White,c_White,c_White,c_DisableBkg,
			c_Unused,c_Unused
		});

	pTheme[CTI_TRACKBAR] = new CColorTheme{};
	pTheme[CTI_TRACKBAR]->Set(
		{
			c_Unused,c_Unused,c_Unused,c_Unused,
			RgbToD2dColorF(0xc6bfbe),c_White/*滑块空白颜色*/,RgbToD2dColorF(0x227988),c_DisableBkg,c_Unused,
			c_Unused,ColorrefToD2dColorF(Colorref::Gray, 0.5)/*滑块描边颜色*/
		});

	pTheme[CTI_CIRCLEBUTTON] = new CColorTheme{};
	pTheme[CTI_CIRCLEBUTTON]->Set(
		{
			c_Unused,c_Unused,c_Unused,c_Unused,
			RgbToD2dColorF(0xededee, 0.4f),
			RgbToD2dColorF(0xe1e1e2, 0.7f),
			RgbToD2dColorF(0xe1e1e2, 0.9f),
			c_DisableBkg,
			RgbToD2dColorF(0xe1e1e2, 0.9f),
			c_Unused,c_Unused
		});
	pTheme[CTI_SCROLLBAR] = new CColorTheme{};
	pTheme[CTI_SCROLLBAR]->Set(
		{
			RgbToD2dColorF(0x85897e),c_Unused,c_Unused,c_Unused,
			RgbToD2dColorF(0xffffff, 0.6f),c_Unused,c_Unused,c_Unused,c_Unused,
			c_Unused,c_Unused
		});
}

inline void MakeStdThemeDark(CColorTheme** pTheme)
{
	constexpr auto c_Black = ColorrefToD2dColorF(Colorref::Black);
	constexpr auto c_White = ColorrefToD2dColorF(Colorref::White);
	constexpr auto c_Gray = ColorrefToD2dColorF(Colorref::Gray);
	constexpr auto c_DisableText = ColorrefToD2dColorF(Colorref::NeutralGray);
	constexpr auto c_DisableBkg = RgbToD2dColorF(0xfbfbf9);
	constexpr auto c_Unused = RgbToD2dColorF(0);
	pTheme[CTI_BUTTON] = new CColorTheme{};
	pTheme[CTI_BUTTON]->Set(
		{
			c_Black,c_Black,RgbToD2dColorF(0x707070),c_DisableText,
			RgbToD2dColorF(0xfefefd),RgbToD2dColorF(0xfafbfa),RgbToD2dColorF(0xfafbfa),c_DisableBkg,RgbToD2dColorF(0xfafbfa),
			RgbToD2dColorF(0xd3d3d2),RgbToD2dColorF(0xecedeb)
		});

	pTheme[CTI_LIST] = new CColorTheme{};
	pTheme[CTI_LIST]->Set(
		COLORTHEME
		{
			.crTextNormal = c_Black,
			.crBkHot = ColorrefToD2dColorF(Colorref::Black, 0.1f),
			.crBkSelected = ColorrefToD2dColorF(Colorref::Black, 0.2f),
			.crBkHotSel = ColorrefToD2dColorF(Colorref::Black, 0.3f),
		});
	pTheme[CTI_LIST]->Set(
		COLORTHEME
		{
			.crTextNormal = c_White,
			.crBkHot = ColorrefToD2dColorF(Colorref::White, 0.3f),
			.crBkSelected = ColorrefToD2dColorF(Colorref::White, 0.4f),
			.crBkHotSel = ColorrefToD2dColorF(Colorref::White, 0.5f),
		});

	pTheme[CTI_LABEL] = new CColorTheme{};
	pTheme[CTI_LABEL]->Set(
		{
			c_Black,c_Black,c_Black,c_DisableText,
			c_White,c_White,c_White,c_DisableBkg,
			c_Unused,c_Unused
		});

	pTheme[CTI_TRACKBAR] = new CColorTheme{};
	pTheme[CTI_TRACKBAR]->Set(
		{
			c_Unused,c_Unused,c_Unused,c_Unused,
			RgbToD2dColorF(0xc6bfbe),c_White/*滑块空白颜色*/,RgbToD2dColorF(0x227988),c_DisableBkg,c_Unused,
			c_Unused,ColorrefToD2dColorF(Colorref::Gray, 0.5)/*滑块描边颜色*/
		});

	pTheme[CTI_CIRCLEBUTTON] = new CColorTheme{};
	pTheme[CTI_CIRCLEBUTTON]->Set(
		{
			c_Unused,c_Unused,c_Unused,c_Unused,
			RgbToD2dColorF(0xededee, 0.4f),
			RgbToD2dColorF(0xe1e1e2, 0.7f),
			RgbToD2dColorF(0xe1e1e2, 0.9f),
			c_DisableBkg,
			RgbToD2dColorF(0xe1e1e2, 0.9f),
			c_Unused,c_Unused
		});
	pTheme[CTI_SCROLLBAR] = new CColorTheme{};
	pTheme[CTI_SCROLLBAR]->Set(
		{
			.crTextNormal = RgbToD2dColorF(0xC0C0C0),
			.crBkNormal = RgbToD2dColorF(0x202020, 0.5f)
		});
}
ECK_DUI_NAMESPACE_END
ECK_NAMESPACE_END