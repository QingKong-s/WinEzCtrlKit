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
	THEME_PART aPart[12];
	THEME_STATE aState[25];
};

enum class StdPal
{
	Dummy,

	SysText,
	SysBk,
	SysMainTitle,

	ButtonNormalBk,
	ButtonNormalBorder,
	ButtonHotBk,
	ButtonHotBorder,
	ButtonSelectedBk,
	ButtonSelectedBorder,

	CircleButtonNormalBk,
	CircleButtonHotBk,
	CircleButtonSelectedBk,

	EditNormalBk,
	EditHotBk,
	EditFocusBk,
	EditBorder,

	EditBtmBarNormal,

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

	HeaderNormalBk,
	HeaderHotBk,
	HeaderSelectedBk,

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
	ColorrefToD2DColorF(RGB(0,51,153)),	// SysMainTitle
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
	// Edit
	{ 0.98f,0.98f,0.98f,0.8f },			// EditNormalBk
	{ 0.93f,0.93f,0.93f,0.8f },			// EditHotBk
	StMakeBackgroundColorLight(0.9f),	// EditFocusBk
	StMakeForegroundColorLight(0.2f),	// EditBorder
	// EditBtmBar
	{ 0.5f,0.5f,0.5f,0.8f },
	// ListSelRect
	{ 0.4f,0.5f,0.6f,0.4f },			// ListSelRectBk
	{ 0.7f,0.8f,0.9f,0.8f },			// ListSelRectBorder
	// ListItem
	StMakeForegroundColorLight(0.1f),	// ListItemHotBk
	StMakeForegroundColorLight(0.2f),	// ListItemSelectedBk
	StMakeForegroundColorLight(0.25f),	// ListItemHotSelectedBk
	// TrackBar
	{ 0.85f,0.85f,0.85f,1.0f },			// TrackBarNormalBk
	// TrackBarThumb
	{ 1.f,1.f,1.f,1.0f },				// TrackBarThumbNormalBk
	{ 0.9f,0.9f,0.9f,1.0f },			// TrackBarThumbNormalBorder
	// ScrollBar
	{ 0.85f,0.85f,0.9f,0.5f },			// ScrollBarHotBk
	// ScrollThumb
	{ 0.6f,0.6f,0.65f,1.0f },			// ScrollThumbNormalBk
	// Header
	StMakeBackgroundColorLight(0.8f),	// HeaderNormalBk
	StMakeForegroundColorLight(0.2f),	// HeaderHotBk
	StMakeForegroundColorLight(0.25f),	// HeaderSelectedBk
};

constexpr inline D2D1_COLOR_F Palette_StdDark[]
{
	{ /*占位*/ },
	// SysColor
	StMakeForegroundColorDark(1.f),		// SysText
	StMakeBackgroundColorDark(1.f),		// SysBk
	ColorrefToD2DColorF(RGB(0,168,255)),// SysMainTitle
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
	// Edit
	{ 0.15f,0.15f,0.15f,0.8f },			// EditNormalBk
	{ 0.19f,0.19f,0.19f,0.8f },			// EditHotBk
	StMakeBackgroundColorDark(0.9f),	// EditFocusBk
	StMakeForegroundColorDark(0.2f),	// EditBorder
	// EditBtmBar
	{ 0.6f,0.6f,0.6f,0.8f },
	// ListSelRect
	{ 0.4f,0.5f,0.6f,0.4f },			// ListSelRectBk
	{ 0.7f,0.8f,0.9f,0.8f },			// ListSelRectBorder
	// ListItem
	StMakeForegroundColorDark(0.1f),	// ListItemHotBk
	StMakeForegroundColorDark(0.2f),	// ListItemSelectedBk
	StMakeForegroundColorDark(0.25f),	// ListItemHotSelectedBk
	// TrackBar
	{ 0.25f,0.25f,0.25f,1.0f },			// TrackBarNormalBk
	// TrackBarThumb
	{ 0.3f,0.3f,0.3f,1.0f },			// TrackBarThumbNormalBk
	{ 0.35f,0.35f,0.35f,1.0f },			// TrackBarThumbNormalBorder
	// ScrollBar
	{ 0.25f,0.25f,0.3f,0.5f },			// ScrollBarHotBk
	// ScrollThumb
	{ 0.45f,0.45f,0.5f,1.0f },			// ScrollThumbNormalBk
	// Header
	StMakeBackgroundColorDark(0.8f),	// HeaderNormalBk
	StMakeForegroundColorDark(0.7f),	// HeaderHotBk
	StMakeForegroundColorDark(0.75f),	// HeaderSelectedBk
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
			.ePart = Part::Edit,
			.idxState = 6,
			.cState = 3,
		},
		{
			.ePart = Part::EditBottomBar,
			.idxState = 9,
			.cState = 2,
		},
		{
			.ePart = Part::ListSelRect,
			.idxState = 11,
			.cState = 1,
		},
		{
			.ePart = Part::ListItem,
			.idxState = 12,
			.cState = 3,
		},
		{
			.ePart = Part::HeaderItem,
			.idxState = 15,
			.cState = 3,
		},
		{
			.ePart = Part::TrackBar,
			.idxState = 18,
			.cState = 2,
		},
		{
			.ePart = Part::TrackBarThumb,
			.idxState = 20,
			.cState = 3,
		},
		{
			.ePart = Part::ScrollBar,
			.idxState = 23,
			.cState = 1,
		},
		{
			.ePart = Part::ScrollButton,
			.idxState = 0,
			.cState = 0,
		},
		{
			.ePart = Part::ScrollThumb,
			.idxState = 24,
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
		// Edit
		{
			.eState = State::Normal,
			.bGeometry = TRUE,
			.Geo = {
				.eType = GeoType::FillFrameRect,
				.fWidth = 1.f,
			},
			.ColorIdx = {
				.idxClrBk = (UINT)StdPal::EditNormalBk,
				.idxClrBorder = (UINT)StdPal::EditBorder,
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
				.idxClrBk = (UINT)StdPal::EditHotBk,
				.idxClrBorder = (UINT)StdPal::EditBorder,
			}
		},
		{
			.eState = State::Focused,
			.bGeometry = TRUE,
			.Geo = {
				.eType = GeoType::FillFrameRect,
				.fWidth = 1.f,
			},
			.ColorIdx = {
				.idxClrBk = (UINT)StdPal::EditFocusBk,
				.idxClrBorder = (UINT)StdPal::EditBorder,
			}
		},
		// EditBottomBar
		{
			.eState = State::Normal,
			.bGeometry = TRUE,
			.Geo = { .eType = GeoType::FillRect },
			.ColorIdx = { .idxClrBk = (UINT)StdPal::EditBtmBarNormal }
		},
		{
			.eState = State::Focused,
			.bGeometry = TRUE,
			.Geo = { .eType = GeoType::FillRect },
			.ColorIdx = { .idxClrBk = IdxColorizationColor }
		},
		// ListSelRect
		{
			.eState = State::None,
			.bGeometry = TRUE,
			.Geo = {
				.eType = GeoType::FillFrameRect,
				.fWidth = 1.f
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
		// HeaderItem
		{
			.eState = State::Normal,
			.bGeometry = TRUE,
			.Geo = {.eType = GeoType::FillRect },
			.ColorIdx = {.idxClrBk = (UINT)StdPal::HeaderNormalBk }
		},
		{
			.eState = State::Hot,
			.bGeometry = TRUE,
			.Geo = {.eType = GeoType::FillRect },
			.ColorIdx = {.idxClrBk = (UINT)StdPal::HeaderHotBk }
		},
		{
			.eState = State::Selected,
			.bGeometry = TRUE,
			.Geo = {.eType = GeoType::FillRect },
			.ColorIdx = {.idxClrBk = (UINT)StdPal::HeaderSelectedBk }
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