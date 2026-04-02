#pragma once
#include "ECK.h"

ECK_NAMESPACE_BEGIN
ECK_UIBASIC_NAMESPACE_BEGIN
namespace Declaration
{
    constexpr inline UINT
        CrlFore = 0xF2'141414,
        CrlForeDisabled = 0xB2'787878,

        CrlBorder = 0x66'727272,
        CrlBorderHot = 0x99'646464,
        CrlBorderPressed = CrlBorderHot,
        CrlBorderDisabled = CrlBorder,

        CrlBack = 0x66'FFFFFF,
        CrlBackHot = 0xB2'FFFFFF,
        CrlBackPressed = 0xCC'DCDCDC,
        CrlBackDisabled = 0x4C'E0E0E0,

        CrlAccent = 0xCC'007AFF,
        CrlAccentHot = 0xE5'268DFF,
        CrlAccentPressed = 0xFF'0056B3,
        CrlAccentDisabled = 0xB2'CCCCCC,
        CrlAccentFore = 0xF2'FFFFFF,
        CrlAccentForeDisabled = 0xB2'FFFFFF,

        CrlDanger = 0xCC'FF2D55,
        CrlDangerHot = 0xE5'FF5475,
        CrlDangerPressed = 0xFF'D02445;
    constexpr inline UINT
        CrdFore = 0xF2'E6E6E6,
        CrdForeDisabled = 0xB2'646464,

        CrdBorder = 0x66'646464,
        CrdBorderHot = 0x99'969696,
        CrdBorderPressed = CrdBorderHot,
        CrdBorderDisabled = CrdBorder,

        CrdBack = 0x7F'282828,
        CrdBackHot = 0x99'464646,
        CrdBackPressed = 0xB2'5A5A5A,
        CrdBackDisabled = 0x66'1E1E1E,

        CrdAccent = 0xCC'00A3FF,
        CrdAccentHot = 0xE5'33B5FF,
        CrdAccentPressed = 0xFF'0082CC,
        CrdAccentDisabled = 0xB2'CCCCCC,
        CrdAccentFore = 0xF2'FFFFFF,
        CrdAccentForeDisabled = 0xB2'FFFFFF,

        CrdDanger = 0xCC'FF3B30,
        CrdDangerHot = 0xE5'FF5E55,
        CrdDangerPressed = 0xFF'D63028;
}
ECK_UIBASIC_NAMESPACE_END
ECK_NAMESPACE_END