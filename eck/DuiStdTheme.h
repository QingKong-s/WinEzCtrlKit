#pragma once
#include "DuiTheme.h"

#if !ECKCXX20
#error "EckDui requires C++20"
#endif// !ECKCXX20

ECK_NAMESPACE_BEGIN
ECK_DUI_NAMESPACE_BEGIN
struct STD_THEME_DATA
{
	THEME_FILE_HEADER Hdr;
	THEME_PART aPart[9];
	THEME_STATE aState[17];
};

enum class StdPal
{
	Dummy,

	SysText,
	SysBk,

	ButtonNormalBk,
	ButtonNormalBorder,
	ButtonHotBk,
	ButtonHotBorder,
	ButtonSelectedBk,
	ButtonSelectedBorder,

	CircleButtonNormalBk,
	CircleButtonHotBk,
	CircleButtonSelectedBk,

	ListSelRectBk,
	ListSelRectBorder,

	ListItemHotBk,
	ListItemSelectedBk,
	ListItemHotSelectedBk,

	TrackBarNormalBk,

	TrackBarThumbNormalBk,
	TrackBarThumbNormalBorder,

	ScrollBarHotBk,
	ScrollThumbNormalBk,

	Max
};

EckInlineNdCe D2D1_COLOR_F StMakeForegroundColorLight(float fOpacity)
{
	return { 0.f,0.f,0.f,fOpacity };
}
EckInlineNdCe D2D1_COLOR_F StMakeForegroundColorDark(float fOpacity)
{
	return { 1.f,1.f,1.f,fOpacity };
}
EckInlineNdCe D2D1_COLOR_F StMakeBackgroundColorLight(float fOpacity)
{
	return { 1.f,1.f,1.f,fOpacity };
}
EckInlineNdCe D2D1_COLOR_F StMakeBackgroundColorDark(float fOpacity)
{
	return { 0.f,0.f,0.f,fOpacity };
}

constexpr inline D2D1_COLOR_F Palette_StdLight[]
{
	{ /*占位*/ },
	// SysColor
	StMakeForegroundColorLight(1.f),	// SysText
	StMakeBackgroundColorLight(1.f),	// SysBk
	// Button
	StMakeBackgroundColorLight(0.7f),	// ButtonNormalBk
	StMakeForegroundColorLight(0.1f),	// ButtonNormalBorder
	StMakeForegroundColorLight(0.1f),	// ButtonHotBk
	StMakeForegroundColorLight(0.1f),	// ButtonHotBorder
	StMakeForegroundColorLight(0.2f),	// ButtonSelectedBk
	StMakeForegroundColorLight(0.1f),	// ButtonSelectedBorder
	// CircleButton
	StMakeBackgroundColorLight(0.7f), 	// CircleButtonNormalBk
	StMakeForegroundColorLight(0.1f),	// CircleButtonHotBk
	StMakeForegroundColorLight(0.2f),	// CircleButtonSelectedBk
	// ListSelRect
	{ 0.4f,0.5f,0.6f,0.4f },
	{ 0.7f,0.8f,0.9f,0.8f },
	// ListItem
	StMakeForegroundColorLight(0.1f),
	StMakeForegroundColorLight(0.2f),
	StMakeForegroundColorLight(0.25f),
	// TrackBar
	{ 0.85f,0.85f,0.85f,1.0f },
	// TrackBarThumb
	{ 1.f,1.f,1.f,1.0f },
	{ 0.9f,0.9f,0.9f,1.0f },
	// ScrollBar
	{ 0.8f,0.85f,0.9f,0.5f },
	// ScrollThumb
	{ 0.5f,0.6f,0.7f,1.0f },
};

constexpr inline D2D1_COLOR_F Palette_StdDark[]
{
	{ /*占位*/ },
	// SysColor
	StMakeForegroundColorDark(1.f),		// SysText
	StMakeBackgroundColorDark(1.f),		// SysBk
	// Button
	StMakeBackgroundColorDark(0.7f),	// ButtonNormalBk
	StMakeForegroundColorDark(0.1f),	// ButtonNormalBorder
	StMakeForegroundColorDark(0.1f),	// ButtonHotBk
	StMakeForegroundColorDark(0.1f),	// ButtonHotBorder
	StMakeForegroundColorDark(0.2f),	// ButtonSelectedBk
	StMakeForegroundColorDark(0.1f),	// ButtonSelectedBorder
	// CircleButton
	StMakeBackgroundColorDark(0.7f), 	// CircleButtonNormalBk
	StMakeForegroundColorDark(0.1f),	// CircleButtonHotBk
	StMakeForegroundColorDark(0.2f),	// CircleButtonSelectedBk
	// ListSelRect
	{ 0.4f,0.5f,0.6f,0.4f },
	{ 0.7f,0.8f,0.9f,0.8f },
	// ListItem
	StMakeForegroundColorDark(0.1f),	// ListItemHotBk
	StMakeForegroundColorDark(0.2f),	// ListItemSelectedBk
	StMakeForegroundColorDark(0.25f),	// ListItemHotSelectedBk
	// TrackBar
	{ 0.25f,0.25f,0.25f,1.0f },
	// TrackBarThumb
	{ 0.3f,0.3f,0.3f,1.0f },
	{ 0.35f,0.35f,0.35f,1.0f },
	// ScrollBar
	{ 0.2f,0.25f,0.3f,0.5f },
	// ScrollThumb
	{ 0.4f,0.45f,0.5f,1.0f },
};

static_assert(ARRAYSIZE(Palette_StdLight) == (size_t)StdPal::Max);
static_assert(ARRAYSIZE(Palette_StdDark) == (size_t)StdPal::Max);

constexpr inline THEME_FILE_HEADER StdThemeHdr
{
	.Magic = ThemeFileMagic,
	.iVer = 0,
	.uFlags = 0,
	.cAtlas = 0,
	.cImage = 0,
	.cPart = ARRAYSIZE(STD_THEME_DATA::aPart),
	.cState = ARRAYSIZE(STD_THEME_DATA::aState),
	.onSysClr = 1,
	.fMetrics
	{
		14.f,
		10.f,
		10.f,

		14.f,
		10.f,
		10.f,

		4.f,
		6.f,
		2.f,

		1.f,
		1.f,
		6.f,
		6.f
	}
};

constexpr inline STD_THEME_DATA StdThemeData
{
	.Hdr = StdThemeHdr,
	.aPart{
		{
			.ePart = Part::Button,
			.idxState = 0,
			.cState = 3,
		},
		{
			.ePart = Part::CircleButton,
			.idxState = 3,
			.cState = 3,
		},
		{
			.ePart = Part::ListSelRect,
			.idxState = 6,
			.cState = 1,
		},
		{
			.ePart = Part::ListItem,
			.idxState = 7,
			.cState = 3,
		},
		{
			.ePart = Part::TrackBar,
			.idxState = 10,
			.cState = 2,
		},
		{
			.ePart = Part::TrackBarThumb,
			.idxState = 12,
			.cState = 3,
		},
		{
			.ePart = Part::ScrollBar,
			.idxState = 15,
			.cState = 1,
		},
		{
			.ePart = Part::ScrollButton,
			.idxState = 0,
			.cState = 0,
		},
		{
			.ePart = Part::ScrollThumb,
			.idxState = 16,
			.cState = 1,
		},
	},
	.aState{
		// Button
		{
			.eState = State::Normal,
			.bGeometry = TRUE,
			.Geo = {
				.eType = GeoType::FillFrameRect,
				.fWidth = 1.f,
			},
			.ColorIdx = {
				.idxClrBk = (UINT)StdPal::ButtonNormalBk,
				.idxClrBorder = (UINT)StdPal::ButtonNormalBorder,
			}
		},
		{
			.eState = State::Hot,
			.bGeometry = TRUE,
			.Geo = {
				.eType = GeoType::FillFrameRect,
				.fWidth = 1.f,
			},
			.ColorIdx = {
				.idxClrBk = (UINT)StdPal::ButtonHotBk,
				.idxClrBorder = (UINT)StdPal::ButtonHotBorder,
			}
		},
		{
			.eState = State::Selected,
			.bGeometry = TRUE,
			.Geo = {
				.eType = GeoType::FillFrameRect,
				.fWidth = 1.f,
			},
			.ColorIdx = {
				.idxClrBk = (UINT)StdPal::ButtonSelectedBk,
				.idxClrBorder = (UINT)StdPal::ButtonSelectedBorder,
			}
		},
		// CircleButton
		{
			.eState = State::Normal,
			.bGeometry = TRUE,
			.Geo = {.eType = GeoType::FillEllipse },
			.ColorIdx = {.idxClrBk = (UINT)StdPal::CircleButtonNormalBk }
		},
		{
			.eState = State::Hot,
			.bGeometry = TRUE,
			.Geo = {.eType = GeoType::FillEllipse },
			.ColorIdx = {.idxClrBk = (UINT)StdPal::CircleButtonHotBk }
		},
		{
			.eState = State::Selected,
			.bGeometry = TRUE,
			.Geo = {.eType = GeoType::FillEllipse },
			.ColorIdx = {.idxClrBk = (UINT)StdPal::CircleButtonSelectedBk }
		},
		// ListSelRect
		{
			.eState = State::None,
			.bGeometry = TRUE,
			.Geo = {
				.eType = GeoType::FillFrameRect,
				.fWidth = 3.f
			},
			.ColorIdx = {
				.idxClrBk = (UINT)StdPal::ListSelRectBk,
				.idxClrBorder = (UINT)StdPal::ListSelRectBorder,
			}
		},
		// ListItem
		{
			.eState = State::Hot,
			.bGeometry = TRUE,
			.Geo = { .eType = GeoType::FillRect },
			.ColorIdx = { .idxClrBk = (UINT)StdPal::ListItemHotBk }
		},
		{
			.eState = State::Selected,
			.bGeometry = TRUE,
			.Geo = { .eType = GeoType::FillRect },
			.ColorIdx = { .idxClrBk = (UINT)StdPal::ListItemSelectedBk }
		},
		{
			.eState = State::HotSelected,
			.bGeometry = TRUE,
			.Geo = { .eType = GeoType::FillRect },
			.ColorIdx = { .idxClrBk = (UINT)StdPal::ListItemHotSelectedBk }
		},
		// TrackBar
		{
			.eState = State::Normal,
			.bGeometry = TRUE,
			.Geo{.eType = GeoType::FillRoundRect },
			.ColorIdx{.idxClrBk = (UINT)StdPal::TrackBarNormalBk }
		},
		{
			.eState = State::Selected,
			.bGeometry = TRUE,
			.Geo{.eType = GeoType::FillRoundRect },
			.ColorIdx{.idxClrBk = IdxColorizationColor }
		},
		// TrackBarThumb
		{
			.eState = State::Normal,
			.bGeometry = TRUE,
			.Geo{
				.eType = GeoType::FrameRing,
				.fWidth = 1.f,
				.Param{
					.ring{
						.ellOutter{ .radiusX = 10.f, .radiusY = 10.f },
						.ellInner{ .radiusX = 6.f, .radiusY = 6.f },
					}
				}
			},
			.ColorIdx{
				.idxClrBk = (UINT)StdPal::TrackBarThumbNormalBk,
				.idxClrBorder = (UINT)StdPal::TrackBarThumbNormalBorder,
				.idxClrExtra1 = IdxColorizationColor,
			}
		},
		{
			.eState = State::Hot,
			.bGeometry = TRUE,
			.Geo{
				.eType = GeoType::FrameRing,
				.fWidth = 1.f,
				.Param{
					.ring{
						.ellOutter{ .radiusX = 10.f, .radiusY = 10.f },
						.ellInner{ .radiusX = 7.5f, .radiusY = 7.5f },
					}
				}
			},
			.ColorIdx{
				.idxClrBk = (UINT)StdPal::TrackBarThumbNormalBk,
				.idxClrBorder = (UINT)StdPal::TrackBarThumbNormalBorder,
				.idxClrExtra1 = IdxColorizationColor,
			}
		},
		{
			.eState = State::Selected,
			.bGeometry = TRUE,
			.Geo{
				.eType = GeoType::FrameRing,
				.fWidth = 1.f,
				.Param{
					.ring{
						.ellOutter{ .radiusX = 10.f, .radiusY = 10.f },
						.ellInner{ .radiusX = 8.f, .radiusY = 8.f },
					}
				}
			},
			.ColorIdx{
				.idxClrBk = (UINT)StdPal::TrackBarThumbNormalBk,
				.idxClrBorder = (UINT)StdPal::TrackBarThumbNormalBorder,
				.idxClrExtra1 = IdxColorizationColor,
			}
		},
		// ScrollBar
		{
			.eState = State::Hot,
			.bGeometry = TRUE,
			.Geo = {.eType = GeoType::FillRect },
			.ColorIdx = {.idxClrBk = (UINT)StdPal::ScrollBarHotBk }
		},
		// ScrollThumb
		{
			.eState = State::Normal,
			.bGeometry = TRUE,
			.Geo = {.eType = GeoType::FillRect },
			.ColorIdx = {.idxClrBk = (UINT)StdPal::ScrollThumbNormalBk }
		},
	},
};
ECK_DUI_NAMESPACE_END
ECK_NAMESPACE_END