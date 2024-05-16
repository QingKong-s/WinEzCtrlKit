/*
* WinEzCtrlKit Library
*
* ILayout.h ： 布局接口
*
* Copyright(C) 2024 QingKong
*/
#pragma once
#include "ECK.h"

ECK_NAMESPACE_BEGIN
struct ILayout
{
	virtual ~ILayout() {}

	/// <summary>
	/// 取理想尺寸
	/// </summary>
	/// <param name="cx">理想宽度</param>
	/// <param name="cy">理想高度</param>
	virtual void LoGetAppropriateSize(int& cx, int& cy) = 0;

	virtual void LoSetPos(int x, int y) {}

	virtual void LoSetSize(int cx, int cy) {}

	virtual void LoSetPosSize(int x, int y, int cx, int cy) {}

	virtual std::pair<int, int> LoGetPos() = 0;

	virtual std::pair<int, int> LoGetSize() = 0;

	virtual void LoCommit() {}

	virtual void LoSetParent(ILayout* p) {}

	virtual HDWP LoGetCurrHDWP() { return NULL; }

	virtual HWND LoGetHWND() { return NULL; }
};
ECK_NAMESPACE_END