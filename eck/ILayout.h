#pragma once
#include "CObject.h"

ECK_NAMESPACE_BEGIN
struct __declspec(novtable) ILayout : public CObject
{
	ECK_RTTI(ILayout);

	/// <summary>
	/// 取理想尺寸
	/// </summary>
	/// <param name="cx">理想宽度</param>
	/// <param name="cy">理想高度</param>
	virtual SIZE LoGetAppropriateSize() = 0;

	virtual void LoSetPos(int x, int y) = 0;

	virtual void LoSetSize(int cx, int cy) = 0;

	virtual void LoSetPosSize(int x, int y, int cx, int cy) = 0;

	virtual POINT LoGetPos() = 0;

	virtual SIZE LoGetSize() = 0;

	virtual void LoShow(BOOL bShow) = 0;

	// 提交对布局所做的修改，通常仅对非原子布局生效
	virtual void LoCommit() {}

	// 若当前类为窗口，则返回窗口句柄
	virtual HWND LoGetHWND() { return nullptr; }

	// 初始化布局所用DPI，通常仅对非原子布局生效
	virtual void LoInitDpi(int iDpi) {}

	// 布局的DPI已改变，通常仅对非原子布局生效
	virtual void LoOnDpiChanged(int iDpi) {}
};
ECK_RTTI_IMPL_BASE_INLINE(ILayout, CObject);
ECK_NAMESPACE_END