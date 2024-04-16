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
ECK_DUI_NAMESPACE_END
ECK_NAMESPACE_END