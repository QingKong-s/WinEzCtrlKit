#pragma once
#include "CObject.h"

ECK_NAMESPACE_BEGIN
struct __declspec(novtable) ILayout : public CObject
{
    ECK_RTTI(ILayout);

    // 取理想尺寸
    virtual SIZE LoGetAppropriateSize() noexcept = 0;
    virtual void LoSetPos(int x, int y) noexcept = 0;
    virtual void LoSetSize(int cx, int cy) noexcept = 0;
    virtual void LoSetPosSize(int x, int y, int cx, int cy) noexcept = 0;
    virtual POINT LoGetPos() noexcept = 0;
    virtual SIZE LoGetSize() noexcept = 0;
    virtual void LoShow(BOOL bShow) noexcept = 0;
    // 提交对布局所做的修改，通常仅对非原子布局生效
    virtual void LoCommit() noexcept {}
    // 若当前类为窗口，则返回窗口句柄
    virtual HWND LoGetHWND() noexcept { return nullptr; }
    // 初始化布局所用DPI，通常仅对非原子布局生效
    virtual void LoInitDpi(int iDpi) noexcept {}
    // 布局的DPI已改变，通常仅对非原子布局生效
    virtual void LoOnDpiChanged(int iDpi) noexcept {}
};
ECK_RTTI_IMPL_BASE_INLINE(ILayout, CObject);
ECK_NAMESPACE_END