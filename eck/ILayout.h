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

	virtual void LoSetPos(int x, int y) = 0;

	virtual void LoSetSize(int cx, int cy) = 0;

	virtual void LoSetPosSize(int x, int y, int cx, int cy) = 0;

	virtual std::pair<int, int> LoGetPos() = 0;

	virtual std::pair<int, int> LoGetSize() = 0;

	virtual void LoShow(BOOL bShow) = 0;

	// 提交对布局所做的修改，通常仅对非原子布局生效
	virtual void LoCommit() {}

	// 若当前类为窗口，则返回窗口句柄
	virtual HWND LoGetHWND() { return NULL; }
};
ECK_NAMESPACE_END